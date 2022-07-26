#include "src/log.h"

int main() {
    auto logger = Logger::make_ptr();
    auto appender = LogAppender::ptr(new StdoutLogAppender());
    logger->addLogAppender(appender);

    EVA_LOG_LEVEL(logger, LogLevel::DEBUG) << "Hello Example";

    return 0;
}
