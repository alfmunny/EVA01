#include "log.h"
#include <iostream>
#include <sstream>
#include <time.h>


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
             LogLevel::Level level, uint32_t thread_id, uint32_t fiber_id)
    : m_file(file), m_line(line), m_time(time)
    , m_level(level), m_thread_id(thread_id), m_fiber_id(fiber_id)
    , m_logger(logger) {

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
        os << "\n";
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
    FileLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getFile();
    }
};

class ThreadIdLogItem : public LogFormatter::Item {
public:
    ThreadIdLogItem(const std::string& pattern = "") {
    }

    void format(std::ostream& os, LogEvent::ptr event) const {
        os << event->getThreadId();
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
    std::string m_pattern;
};

std::string LogFormatter::format(LogEvent::ptr event) const {
    std::stringstream ss;
    for (auto& item : m_formats) {
        item->format(ss, event);
    }
    return ss.str();
}

void LogFormatter::format(std::ostream& os, LogEvent::ptr event) const {
    for (auto& item : m_formats) {
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
    //{'p', [](const std::string& fmt){ return LogFormatter::Item::ptr(new LevelLogItem(fmt)); }}, // p:日志级别
    ////{'r', [](const std::string& fmt){ return LogFormatter::Item::ptr(new (fmt)); }},// r:累计毫秒数
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
    if (level >= m_level) {
        for (auto appender : m_appenders) {
            appender->log(event);
        }
    }
}

void Logger::addLogAppender(LogAppender::ptr appender) {
    m_appenders.push_back(appender);
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
}

void Logger::delLogAppender(LogAppender::ptr appender) {
    m_appenders.remove(appender);
}

void Logger::clearLogAppenders() {
    m_appenders.clear();
}

void StdoutLogAppender::log(LogEvent::ptr event) {
    if (event->getLevel() >= m_level) {
        std::cout << m_formatter->format(event);
    }
}

void StreamLogAppender::log(LogEvent::ptr event) {
    if (event->getLevel() >= m_level) {
        m_ss << m_formatter->format(event);
    }
}

std::string StreamLogAppender::flush() { 
    auto s = m_ss.str(); 
    m_ss.clear();
    return s;
}

LogFormatter::LogFormatter(const std::string& pattern) 
    : m_pattern(pattern) {

    bool found = false;
    for (auto& c : pattern) {
        if (c == '%') {
            if (found) {
                std::cout  << "parse_error: "  << c << std::endl;
                break;
            } else {
                found = true;
            }
        } else {
            auto key = std::string(1, c);
            if (found) {
                if (FORMATS.count(key) > 0) {
                    m_formats.push_back(FORMATS.at(key)(""));
                    found = false;
                } else {
                    std::cout  << "parse_error: invalid item"  << c << std::endl;
                    break;
                }
            } else {
                m_formats.push_back(LogFormatter::Item::ptr(new StringLogItem(std::string(1, c))));
            }
        }
    }
}

