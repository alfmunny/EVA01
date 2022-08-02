#include "src/log.h"
#include <iostream>

using namespace eva01;

int main() {
    // create logger
    auto logger = Logger::ptr(new Logger("system"));

    // define appenders
    // Output destination
    auto appender = StdoutLogAppender::ptr(new StdoutLogAppender());
    appender->setFormatter("%d{%Y-%m-%d %a %H:%M:%S}%T%f{5}%T[%p]%T[%c]%T%m%n");
                
    auto fappender = FileLogAppender::ptr(new FileLogAppender("test.log"));
    fappender->setLevel(LogLevel::ERROR);

    auto sappender = StreamLogAppender::ptr(new StreamLogAppender());


    //fappender->setFormatter("%d%T%m%n");
    // add appenders to logger
    logger->addLogAppender(fappender);
    logger->addLogAppender(appender);
    logger->addLogAppender(sappender);

    //EVA_LOG_LEVEL(logger, LogLevel::DEBUG) << "Hello Example";
    EVA_LOG_DEBUG(logger) << "hello example";
    EVA_LOG_INFO(logger) << "hello example";

    return 0;
}
