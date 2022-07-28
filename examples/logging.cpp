#include "src/log.h"

int main() {
    auto logger = Logger::make_ptr();
    auto appender = StdoutLogAppender::ptr(new StdoutLogAppender());
    auto fappender = FileLogAppender::ptr(new FileLogAppender("test.log"));
    fappender->setFormatter("%d%T%m%n");
    logger->addLogAppender(fappender);
    logger->addLogAppender(appender);

    EVA_LOG_LEVEL(logger, LogLevel::DEBUG) << "Hello Example";

    return 0;
}
