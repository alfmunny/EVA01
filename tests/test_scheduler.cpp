#include <iostream>
#include "src/scheduler.h"
#include "src/log.h"
#include "src/util.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


namespace eva01 {

static Logger::ptr logger = EVA_LOGGER("system");

static int count = 5;

void func() {
    EVA_LOG_DEBUG(logger) << "test in fiber cout=" << count;
    if (--count >= 0) {
        //wille::Scheduler::GetThis()->schedule(&func, wille::GetThreadId()); //run in same thread
        eva01::Scheduler::GetThis()->schedule(&func); // run in random thread
    }
}

TEST_CASE("Test Fiber") {
    eva01::Scheduler sc(1, "sched");
    sc.start();
    sc.schedule(&func);
    sc.stop();

    // start again
    count = 5;
    sc.start();
    sc.schedule(&func);
    sc.stop();
}


}
