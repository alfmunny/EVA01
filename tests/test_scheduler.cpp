#include <iostream>
#include "src/scheduler.h"
#include "src/log.h"
#include "src/timer.h"
#include "src/util.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


namespace eva01 {

static Logger::ptr logger = EVA_LOGGER("system");

static int count = 5;

void func() {
    EVA_LOG_DEBUG(logger) << "test in fiber count=" << count;
    if (--count >= 0) {
        //sleep(1);
        //wille::Scheduler::GetThis()->schedule(&func, wille::GetThreadId()); //run in same thread
        eva01::Scheduler::GetThis()->schedule(&func); // run in random thread
    }
}

void func2() {
    static int count2 = 0;
    EVA_LOG_DEBUG(logger) << "test in fiber count=" << count2++;
}

TEST_CASE("Test Fiber") {
    eva01::Scheduler sc(1, "sched");
    sc.start();
    //sleep(5);
    sc.schedule(&func);
    sc.stop();

    // start again
    //count = 5;
    //sc.start(); sc.schedule(&func);
    //sc.stop();
}

TEST_CASE("Test Timer") {
    eva01::Scheduler sc;
    sc.start();
    int count1 = 0;
    int count2 = 0;
    
    TimerManager::Timer::ptr timer = sc.addTimer(10, [&](){
            count1 += 1;
            EVA_LOG_DEBUG(logger) << "test in timer 10ms count=" << count1;
            if (count1 == 5) {
                sc.removeTimer(timer);
            }
            }, true);

    TimerManager::Timer::ptr timer2 = sc.addTimer(5, [&](){
            count2 += 1;
            EVA_LOG_DEBUG(logger) << "test in timer 5ms count=" << count2;
            if (count2 == 5) {
                sc.removeTimer(timer2);
            }
            }, true);

    sc.stop();
}


}
