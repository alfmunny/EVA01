#include "log.h"
#include <iostream>
#include <sstream>
#include <time.h>


namespace eva01 {

std::string LogLevel::toString(LogLevel::Level level) {
    std::stringstream ss;
    switch (level) {
#define XX(name)                      \
        case name:                    \
            ss << #name;              \
            break;                    \

        XX(UNKNOW);
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            break;
    }

    return  ss.str();
};

LogEvent::LogEvent(Logger::ptr logger, const char* file, int32_t line, uint64_t time, 
             LogLevel::Level level, uint32_t thread_id, uint64_t fiber_id, const std::string& thread_name, 
             const std::string& message)
    : m_file(file), m_line(line), m_time(time)
    , m_level(level), m_thread_id(thread_id), m_fiber_id(fiber_id), m_thread_name(thread_name)
    , m_logger(logger) {
        m_ss << message;
}

class DateLogItem : public LogFormatter::Item {
public:
    DateLogItem(const std::string& pattern = "%Y-%m-%d %H:%M:%S")
        : m_pattern(pattern) {
        if (m_pattern.empty()) {
            m_pattern = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) const override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_pattern.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_pattern;
};

class MessageLogItem : public LogFormatter::Item {
public:
    MessageLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const override {
        os << event->getMessage();
    }
};

class LineLogItem : public LogFormatter::Item {
public:
    LineLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getLine();
    }
};

class NameLogItem : public LogFormatter::Item {
public:
    NameLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getLogger()->getName();
    }
};

class TabLogItem : public LogFormatter::Item {
public:
    TabLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << "\t";
    }
};

class NewLineLogItem : public LogFormatter::Item {
public:
    NewLineLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << std::endl;
    }
};

class LevelLogItem : public LogFormatter::Item {
public:
    LevelLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << LogLevel::toString(event->getLevel());
    }
};

class FileLogItem : public LogFormatter::Item {
public:
    FileLogItem(const std::string& pattern = "")
        : m_pattern(pattern) {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        int depth = atoi(m_pattern.c_str());
        auto* it = event->getFile();
        while (*it != '\n' && depth != 0) {
            if (*it == '/') --depth;
            ++it;
        }
        os << it;
    }

private:
    const std::string m_pattern;
};

class ThreadIdLogItem : public LogFormatter::Item {
public:
    ThreadIdLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getThreadId();
    }
};

class ThreadNameLogItem : public LogFormatter::Item {
public:
    ThreadNameLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getThreadName();
    }
};

class FiberIdLogItem: public LogFormatter::Item {
public:
    FiberIdLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getFiberId();
    }
};

class StringLogItem : public LogFormatter::Item {
public:
    StringLogItem(const std::string& pattern = "") 
        : m_pattern(pattern) {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << m_pattern;
    }
private:
    const std::string m_pattern;
};

std::string LogFormatter::format(LogEvent::ptr event) const {
    std::stringstream ss;
    for (auto& item : m_items) {
        item->format(ss, event);
    }
    return ss.str();
}

void LogFormatter::format(std::ostream& os, LogEvent::ptr event) const {
    for (auto& item : m_items) {
        item->format(os, event);
    }
}

const std::map<std::string, std::function<LogFormatter::Item::ptr(const std::string&)>> LogFormatter::FORMATS = {
#define XX(c, ItemType)                                                                            \
    {#c, [](const std::string& fmt){ return LogFormatter::Item::ptr(new ItemType(fmt)); }}         
    XX(m, MessageLogItem),
    XX(p, LevelLogItem),
    XX(c, NameLogItem),
    XX(t, ThreadIdLogItem),
    XX(n, NewLineLogItem),
    XX(f, FileLogItem),
    XX(l, LineLogItem),
    XX(T, TabLogItem),
    XX(d, DateLogItem),
    XX(N, ThreadNameLogItem),
    XX(F, FiberIdLogItem),
    //{'p', [](const std::string& fmt){ return LogFormatter::Item::ptr(new LevelLogItem(fmt)); }}, // p:日志级别
    //{'r', [](const std::string& fmt){ return LogFormatter::Item::ptr(new ElapseLogItem(fmt)); }},// r:累计毫秒数
    //{'c', [](const std::string& fmt){ return LogFormatter::Item::ptr(new NameLogItem(fmt)); }},// c:日志名称
    //{'t', [](const std::string& fmt){ return LogFormatter::Item::ptr(new ThreadIdLogItem(fmt)); }},// t:线程id
    //{'n', [](const std::string& fmt){ return LogFormatter::Item::ptr(new NewLineLogItem(fmt)); }},// n:换行
    //{'d', [](const std::string& fmt){ return LogFormatter::Item::ptr(new DateLogItem(fmt)); }},// d:时间
    //{'f', [](const std::string& fmt){ return LogFormatter::Item::ptr(new FileLogItem(fmt)); }},// f:文件名
    //{'l', [](const std::string& fmt){ return LogFormatter::Item::ptr(new LineLogItem(fmt)); }},// l:行号
    //{'T', [](const std::string& fmt){ return LogFormatter::Item::ptr(new TabLogItem(fmt)); }},// T:Tab
#undef XX
    //{'F', [](const std::string& fmt){ return LogFormatter::Item::ptr(new MessageLogItem(fmt)); }},// F:协程id
    //{'N', [](const std::string& fmt){ return LogFormatter::Item::ptr(new MessageLogItem(fmt)); }}// N:线程名称
};


Logger::Logger(const std::string& name, LogLevel::Level level, const std::string& pattern)
    : m_name(name), m_level(level), m_formatter(new LogFormatter(pattern)) {
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    MutexGuard<Mutex> lk(m_mtx);
    if (level >= m_level) {
        for (auto appender : m_appenders) {
            appender->log(event);
        }
    }
}

void Logger::addLogAppender(LogAppender::ptr appender) {
    MutexGuard<Mutex> lk(m_mtx);
    m_appenders.push_back(appender);
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
}

void Logger::delLogAppender(LogAppender::ptr appender) {
    MutexGuard<Mutex> lk(m_mtx);
    m_appenders.remove(appender);
}

void Logger::clearLogAppenders() {
    MutexGuard<Mutex> lk(m_mtx);
    m_appenders.clear();
}

void StdoutLogAppender::log(LogEvent::ptr event) {
    MutexGuard<Mutex> lk(m_mtx);
    if (event->getLevel() >= m_level) {
        m_formatter->format(std::cout, event);
    }
}

void StreamLogAppender::log(LogEvent::ptr event) {
    MutexGuard<Mutex> lk(m_mtx);
    if (event->getLevel() >= m_level) {
        m_ss << m_formatter->format(event);
    }
}

std::string StreamLogAppender::flush() { 
    MutexGuard<Mutex> lk(m_mtx);
    auto s = m_ss.str(); 
    m_ss.clear();
    return s;
}

FileLogAppender::FileLogAppender(const std::string& fname) 
    : m_fname(fname) {
    reopen();
}

bool FileLogAppender::reopen() {
    MutexGuard<Mutex> lk(m_mtx);
    if(m_fstream) {
        m_fstream.close();
    }
    m_fstream.open(m_fname.c_str(), std::ios::app);
    return !!m_fstream;
}

void FileLogAppender::log(LogEvent::ptr event) {
    MutexGuard<Mutex> lk(m_mtx);
    if (event->getLevel() >= m_level) {
        m_formatter->format(m_fstream, event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern) 
    : m_pattern(pattern) {
        initItems();
}

void LogFormatter::initItems() {
    bool found = false;
    for (auto it = m_pattern.begin(); it < m_pattern.end(); ++it) {
        if (*it == '%') {
            if (found) {
                std::cout  << "parse_error: "  << *it << std::endl;
                break;
            } else {
                found = true;
            }
        } else {
            auto key = std::string(1, *it);
            if (found) {
                if (FORMATS.count(key) > 0) {
                    std::stringstream fmt; 
                    if (it+1 < m_pattern.end() && *(it+1) == '{') {
                        bool fmt_found = false;
                        for (it = it+2; it < m_pattern.end(); ++it)
                        {
                            if (*it == '}') {
                                fmt_found = true;
                                break;
                            } else {
                                fmt << *it;
                            }
                        }
                        if (!fmt_found) {
                            fmt.clear();
                            std::cout << "parse_error: open {}" << std::endl;
                        }
                    }

                    m_items.push_back(FORMATS.at(key)(fmt.str()));
                    found = false;
                } else {
                    std::cout  << "parse_error: invalid item"  << *it << std::endl;
                    break;
                }
            } else {
                m_items.push_back(LogFormatter::Item::ptr(new StringLogItem(key)));
            }
        }
    }
}

LoggerManager::LoggerManager()
    : m_root_logger(std::make_shared<Logger>()){
    m_root_logger->addLogAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[m_root_logger->getName()] = m_root_logger;
    init();
}

void LoggerManager::init() {


}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexGuard<Mutex> lk(m_mtx);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    auto logger = std::make_shared<Logger>(name);
    logger->addLogAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[name] = logger;
    return logger;
}

}
