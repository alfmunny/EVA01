#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "src/thread.h"
#include "src/mutex.h"

eva01::Mutex mtx;

void func() {
    eva01::MutexGuard<eva01::Mutex> lk(mtx);
    CHECK(mtx.isLockedByThisThread());
}

TEST_CASE("Test Mutex") {
    eva01::Thread::ptr thr1 = std::make_shared<eva01::Thread>(&func, "thread1");
    eva01::Thread::ptr thr2 = std::make_shared<eva01::Thread>(&func, "thread2");
    thr1->join();
    thr2->join();
}
