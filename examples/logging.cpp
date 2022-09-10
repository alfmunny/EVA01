#include "src/log.h"
#include <iostream>

using namespace eva01;

int main() {
    // create logger
    auto logger = Logger::ptr(new Logger("system"));
    auto logger2 = EVA_ROOT_LOGGER();

    // define appenders
    // Output destination
    auto appender = StdoutLogAppender::ptr(new StdoutLogAppender());
    //appender->setFormatter("%d{%Y-%m-%d %a %H:%M:%S}%T%f{5}%T[%p]%T[%c]%T%m%n");
                
    auto fappender = FileLogAppender::ptr(new FileLogAppender("test.log"));
    fappender->setLevel(LogLevel::ERROR);

    auto sappender = StreamLogAppender::ptr(new StreamLogAppender());


    //fappender->setFormatter("%d%T%m%n");
    // add appenders to logger
    logger->addLogAppender(fappender);
    logger->addLogAppender(appender);
    logger->addLogAppender(sappender);

    //EVA_LOG_LEVEL(logger, LogLevel::DEBUG) << "Hello Example";
    int times = 100;
    while (--times >= 0) {
        EVA_LOG_DEBUG(logger) << "hello example";
        EVA_LOG_INFO(logger) << "hello example";
        EVA_LOG_INFO(logger2) << "hello example";
    }

    return 0;
}
