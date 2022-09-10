#include "src/thread.h"
#include "src/log.h"
#include "src/mutex.h"

using namespace eva01;

static Logger::ptr g_logger = EVA_ROOT_LOGGER();
static Logger::ptr s_logger = EVA_LOGGER("system");

Mutex mu;


void func1() {
    int count = 0;
    while (count < 100) {
        {
            MutexGuard<Mutex> lk(mu);
            EVA_LOG_INFO(g_logger)  << " Count: " << std::to_string(count);
            EVA_LOG_INFO(s_logger)  << " Count: " << std::to_string(count);
        }
        count++;
    }
}

int main() {
    g_logger->addLogAppender(LogAppender::ptr(new FileLogAppender("test.log")));
    s_logger->addLogAppender(LogAppender::ptr(new FileLogAppender("test.log")));

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
