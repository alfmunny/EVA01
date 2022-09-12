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
    count = 5;
    sc.start(); sc.schedule(&func);
    sc.stop();
}

TEST_CASE("Test Timer") {
    eva01::Scheduler sc;
    sc.start();
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    
    TimerManager::Timer::ptr timer = sc.addTimer(100, [&](){
            count1 += 1;
            EVA_LOG_INFO(logger) << "test in timer 100ms count=" << count1;
            if (count1 >= 5) {
                timer->cancel();
            }
            }, true);

    TimerManager::Timer::ptr timer2 = sc.addTimer(50, [&](){
            count2 += 1;
            EVA_LOG_INFO(logger) << "test in timer 50ms count=" << count2;
            if (count2 >= 5) {
                timer2->cancel();
            }
            }, true);

    TimerManager::Timer::ptr timer3 = sc.addTimer(200, [&](){
            count3 += 1;
            EVA_LOG_INFO(logger) << "test in timer 200ms count=" << count3;
            if (count3 >= 5) {
                timer3->cancel();
            }
            }, true);

    TimerManager::Timer::ptr timer4 = sc.addTimer(1, [&](){
            count4 += 1;
            EVA_LOG_INFO(logger) << "test in timer 1ms count=" << count4;
            if (count4 >= 200) {
                timer4->cancel();
            }
            }, true);

    sc.stop();
}


}
