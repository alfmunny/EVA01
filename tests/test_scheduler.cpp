#include <iostream>
#include "src/scheduler.h"
#include "src/log.h"
#include "src/util.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


namespace eva01 {

static Logger::ptr logger = EVA_ROOT_LOGGER();

void fiber1() {
    int count = 5;
    for (;count > 0; --count) {
        EVA_LOG_DEBUG(logger) << "fiber 1: count " << count;
        Fiber::Yield();
    }
    EVA_LOG_DEBUG(logger) << "fiber 1 finished";
}

void fiber2() {
    int count = 5;
    for (;count > 0; --count) {
        EVA_LOG_DEBUG(logger) << "fiber 2: count " << count;
        Fiber::Yield();
    }
    EVA_LOG_DEBUG(logger) << "fiber 2 finished";
}

void run_fiber() {
    {
        Fiber::GetThis();
        Fiber::ptr f1 = Fiber::ptr(new Fiber(&fiber1));
        Fiber::ptr f2 = Fiber::ptr(new Fiber(&fiber2));
        while (f1->getState() != Fiber::TERM || f2->getState() != Fiber::TERM) {
            if (f1->getState() != Fiber::TERM) {
                f1->call();
            }
            if (f2->getState() != Fiber::TERM) {
                f2->call();
            }
        }
        EVA_LOG_DEBUG(logger) << "main fiber finished";
    }
    EVA_LOG_DEBUG(logger) << "out of main fiber scope";
}

void func() {
    static int count = 5;
    EVA_LOG_DEBUG(logger) << "test in fiber cout=" << count;
    sleep(1);
    if (--count >= 0) {
        //wille::Scheduler::GetThis()->schedule(&func, wille::GetThreadId()); //run in same thread
        eva01::Scheduler::GetThis()->schedule(&func); // run in random thread
    }
}

TEST_CASE("Test Fiber") {
    eva01::Scheduler sc;
    sc.start();
    sleep(2);
    sc.stop();
}

}
