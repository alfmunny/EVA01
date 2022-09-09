#include <iostream>
#include "src/thread.h"
#include "src/fiber.h"
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
    Fiber::MakeMain();
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

TEST_CASE("Test Fiber") {
    eva01::Thread::ptr thr = std::make_shared<eva01::Thread>(&run_fiber, "main");
    thr->join();
}

}
