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

const std::map<char, bool> LogFormatter::FORMATS = {
    {'m', true },// m:消息
    {'p', true },// p:日志级别
    {'r', true },// r:累计毫秒数
    {'c', true },// c:日志名称
    {'t', true },// t:线程id
    {'n', true },// n:换行
    {'d', true },// d:时间
    {'f', true },// f:文件名
    {'l', true },// l:行号
    {'T', true },// T:Tab
    {'F', true },// F:协程id
    {'N', true }// N:线程名称
};

LogEvent::LogEvent(Logger::ptr logger, const char* file, int32_t line, uint64_t time, 
             LogLevel::Level level, uint32_t thread_id, uint32_t fiber_id)
    : m_file(file), m_line(line), m_time(time)
    , m_level(level), m_thread_id(thread_id), m_fiber_id(fiber_id)
    , m_logger(logger) {

}


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
            if (found) {
                if (FORMATS.count(c) > 0) {
                    m_formats.push_back(c);
                    found = false;
                } else {
                    std::cout  << "parse_error: invalid item"  << c << std::endl;
                    break;
                }
            } else {
                m_formats.push_back(c);
            }
        }
    }
}

std::string LogFormatter::format(LogEvent::ptr event) const {
    std::stringstream ss;
    for (auto& c : m_formats) {
        switch(c) {
            case 'c': 
                ss << event->getLogger()->getName();
                break;
            case 'm':
                ss << event->getMessage();
                break;
            case 'l':
                ss << event->getLine();
                break;
            case 'd':
                {
                    struct tm tm;
                    time_t time = event->getTime();
                    localtime_r(&time, &tm);
                    char buf[64];
                    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
                    ss << buf;
                }
                break;
            case 'p':
                ss << LogLevel::toString(event->getLevel());
                break;
            case 'f':
                ss << event->getFile();
                break;
            case 'T':
                ss << '\t';
                break;
            case 'n':
                ss << '\n';
                break;
            default:
                ss << c;
                break;
        }
    }
    return ss.str();
}
