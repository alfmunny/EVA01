#include <iostream>
#include "src/thread.h"
#include "src/util.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


TEST_CASE("Test Thread") {

    bool flag = false;
    eva01::Thread::ptr thr = std::make_shared<eva01::Thread>([&]{
            flag = true;
            CHECK(eva01::Thread::GetName() == "main");
            eva01::Thread::SetName("root");
            CHECK(eva01::Thread::GetName() == "root");
            CHECK(eva01::Thread::GetThis()->getId() == eva01::GetThreadId());
            CHECK(eva01::Thread::GetThis()->getName() == "root");
            }, "main");

    thr->join();

    CHECK(thr != nullptr);
    CHECK(flag);
}

