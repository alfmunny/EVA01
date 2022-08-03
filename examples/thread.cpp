#include "src/thread.h"
#include "src/log.h"

using namespace eva01;

static Logger::ptr g_logger = Logger::ptr(new Logger("root"));

void func1() {
    int count = 0;
    while (count < 100) {
        EVA_LOG_INFO(g_logger) << "name: " << Thread::GetName() << " this.thread.name: " << Thread::GetThis()->GetName() << " id: " << Thread::GetThis()->getId() << " Count: " << std::to_string(count);
        count++;
    }
}

int main() {
    g_logger->addLogAppender(LogAppender::ptr(new StdoutLogAppender()));
    g_logger->addLogAppender(LogAppender::ptr(new FileLogAppender("test.log")));
    g_logger->addLogAppender(LogAppender::ptr(new FileLogAppender("test.log")));

    std::vector<Thread::ptr> threads;
    int num = 5;
    for (int i =  0; i < num; ++i) {
        threads.push_back(Thread::ptr(new Thread(*func1, "thread_" + std::to_string(i))));
    }

    for (int i =  0; i < num; ++i) {
        threads[i]->join();
    }

    return 0;
}
