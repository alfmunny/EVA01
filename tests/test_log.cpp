#include <iostream>
#include "src/log.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("LogEvent") {

}

TEST_CASE("LogAppender") {

}

TEST_CASE("LogFormatter") {

}

TEST_CASE("Test Logger") {
    auto logger = Logger::make_ptr();
    auto appender = std::make_shared<StreamLogAppender>();
    logger->addLogAppender(appender);
    
    SUBCASE("Test default level") {
        EVA_LOG_DEBUG(logger) << "Hello DEBUG message";
        CHECK(appender->flush().find("Hello DEBUG message") != std::string::npos);

        EVA_LOG_INFO(logger) << "Hello INFO message";
        CHECK(appender->flush().find("Hello INFO message") != std::string::npos);

        EVA_LOG_ERROR(logger) << "Hello WARN message";
        CHECK(appender->flush().find("Hello WARN message") != std::string::npos);

        EVA_LOG_ERROR(logger) << "Hello ERROR message";
        CHECK(appender->flush().find("Hello ERROR message") != std::string::npos);

        EVA_LOG_FATAL(logger) << "Hello FATAL message";
        CHECK(appender->flush().find("Hello FATAL message") != std::string::npos);
    }

    SUBCASE("Test setLevel") {
        logger->setLevel(LogLevel::INFO);

        EVA_LOG_DEBUG(logger) << "Hello DEBUG message";
        CHECK(appender->flush().find("Hello DEBUG message") == std::string::npos);

        logger->setLevel(LogLevel::WARN);
        EVA_LOG_INFO(logger) << "Hello INFO message";
        CHECK(appender->flush().find("Hello INFO message") == std::string::npos);

        logger->setLevel(LogLevel::ERROR);
        EVA_LOG_WARN(logger) << "Hello WARN message";
        CHECK(appender->flush().find("Hello WARN message") == std::string::npos);

        logger->setLevel(LogLevel::FATAL);
        EVA_LOG_ERROR(logger) << "Hello ERROR message";
        CHECK(appender->flush().find("Hello ERROR message") == std::string::npos);

        EVA_LOG_FATAL(logger) << "Hello FATAL message";
        CHECK(appender->flush().find("Hello FATAL message") != std::string::npos);
    }
}
