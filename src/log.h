#ifndef __EVA01_LOG_H__
#define __EVA01_LOG_H__

#include <list>
#include <memory>
#include <map>
#include <sstream>
#include <time.h>
#include <vector>
#include <functional>


#define EVA_LOG_LEVEL(logger, level)                            \
    if (logger->getLevel() <= level)                             \
        LogWrapper(logger, LogEvent::ptr(new LogEvent(logger, __FILE__, __LINE__, time(0), level, 0, 0))).getSS()

#define EVA_LOG_DEBUG(logger) EVA_LOG_LEVEL(logger, LogLevel::DEBUG)
#define EVA_LOG_INFO(logger) EVA_LOG_LEVEL(logger, LogLevel::INFO)
#define EVA_LOG_WARN(logger) EVA_LOG_LEVEL(logger, LogLevel::WARN)
#define EVA_LOG_ERROR(logger) EVA_LOG_LEVEL(logger, LogLevel::ERROR)
#define EVA_LOG_FATAL(logger) EVA_LOG_LEVEL(logger, LogLevel::FATAL)

constexpr const char* DEFAULT_PATTERN = "%d%T%f%T%l%T[%p]%T[%c]%T%m%n";

class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static std::string toString(Level level);
    static Level fromString(std::string str);
};

class Logger;

class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;

    LogEvent(std::shared_ptr<Logger> logger, const char* file, int32_t line, uint64_t time, 
             LogLevel::Level level, uint32_t thread_id, 
             uint32_t fiber_id);

    inline const char* getFile() const { return m_file; }
    inline int32_t getLine() const { return m_line; }
    inline uint64_t getTime() const { return m_time; }
    LogLevel::Level getLevel() const { return  m_level; }
    inline std::string getMessage() const { return m_ss.str(); }
    inline uint32_t getThreadId() const { return m_thread_id; }
    inline uint32_t getFiberId() const { return m_fiber_id; }
    inline std::stringstream& getSS() { return m_ss; }
    inline std::shared_ptr<Logger> getLogger() { return m_logger; }
    
private:
    const char* m_file = nullptr;
    int32_t m_line;
    uint64_t m_time;
    LogLevel::Level m_level;
    uint32_t m_thread_id;
    uint32_t m_fiber_id;
    std::stringstream m_ss;
    std::shared_ptr<Logger> m_logger;
};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);
    std::string format(LogEvent::ptr event) const;
    void format(std::ostream& os, LogEvent::ptr event) const;

    class Item {
    public:
        using ptr = std::shared_ptr<Item>;
        virtual ~Item() = default;
        virtual void format(std::ostream& os, LogEvent::ptr event) const = 0;
    };

private:
    static const std::map<std::string, std::function<LogFormatter::Item::ptr(const std::string& str)>> FORMATS;
    std::vector<Item::ptr> m_formats;
    std::string m_pattern;
};


class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() = default;
    virtual void log(LogEvent::ptr event) = 0;

    inline void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
    inline LogFormatter::ptr getFormatter() { return m_formatter; }

    inline void setLevel(LogLevel::Level level) { m_level = level; }
    inline LogLevel::Level getLevel() { return m_level; }

protected:
    LogFormatter::ptr m_formatter;
    LogLevel::Level m_level = LogLevel::Level::DEBUG;
};

class StdoutLogAppender : public LogAppender {
public:
    void log(LogEvent::ptr event) override;
};

class StreamLogAppender : public LogAppender {
public:
    void log(LogEvent::ptr event) override;
    std::string flush();

private:
    std::stringstream  m_ss;
};

class Logger {

public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const std::string& name = "root", 
           LogLevel::Level level = LogLevel::Level::DEBUG,
           const std::string& pattern = DEFAULT_PATTERN);

    static ptr make_ptr() { return std::make_shared<Logger>(); }

    void addLogAppender(LogAppender::ptr appender);
    void delLogAppender(LogAppender::ptr appender);
    void clearLogAppenders();

    inline void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
    inline void setFormatter(const std::string& pattern) { 
        m_formatter = LogFormatter::ptr(new LogFormatter(pattern)); 
    }

    inline void setLevel(LogLevel::Level level) { m_level = level; }
    inline LogLevel::Level getLevel() const { return m_level; }

    inline std::string_view getName() const { return m_name; }

    inline void info(LogEvent::ptr event) { log(LogLevel::Level::INFO, event); }
    inline void debug(LogEvent::ptr event) { log(LogLevel::Level::DEBUG, event); }
    inline void warn(LogEvent::ptr event) { log(LogLevel::Level::WARN, event); }
    inline void error(LogEvent::ptr event) { log(LogLevel::Level::ERROR, event); }
    inline void fatal(LogEvent::ptr event) { log(LogLevel::Level::FATAL, event); }

    void log(LogLevel::Level level, LogEvent::ptr event);


private:

    std::string m_name;
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
    std::list<LogAppender::ptr> m_appenders;
};

class LogWrapper {
public:
    LogWrapper(Logger::ptr logger, LogEvent::ptr event)
        : m_logger(logger), m_event(event) {
        }

    ~LogWrapper() {
        m_logger->log(m_event->getLevel(), m_event);
    }

    inline std::stringstream& getSS() { return m_event->getSS(); }
    inline Logger::ptr getLogger() { return m_logger; }
    inline LogEvent::ptr getEvent() { return m_event; }

private:
    Logger::ptr m_logger;
    LogEvent::ptr m_event;
};

#endif
