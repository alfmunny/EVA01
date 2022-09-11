#pragma once

#include "fiber.h"
#include "thread.h"
#include "mutex.h"
#include "macro.h"
#include <vector>
#include <list>
#include <functional>
#include <atomic>

namespace eva01 {

class Scheduler {

public:
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = Mutex;

    Scheduler(int threads = 1, const std::string& name = "main");
    ~Scheduler();

public:
    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    const std::string& getName() const { return m_name; }
    void start();
    void stop();

    template <typename FiberOrFunc>
    void schedule(FiberOrFunc f, int thr = -1) {
        MutexGuard<MutexType> lk(m_mutex);

        bool need_tickle = m_tasks.empty();
        m_tasks.emplace_back(std::move(f), thr);

        if (need_tickle) {
            tickle();
        }
    }

private:
    void run();
    void tickle();
    void idle();
    bool shouldStop();

private:
struct Task {
    Fiber::ptr fiber;
    std::function<void()> func;
    int thread_id;

    Task(Fiber::ptr fb, int thr_id)
        :fiber(fb), thread_id(thr_id) {
    }

    Task(std::function<void()> fc, int thr_id)
        :func(fc), thread_id(thr_id) {
    } 

    Task() 
        : thread_id(-1) {
    }

    void reset() {
        fiber = nullptr;
        func = nullptr;
        thread_id = -1;
    }
};

private:
    MutexType m_mutex;
    Fiber::ptr m_main_fiber;
    std::string m_name;
    std::vector<Thread::ptr> m_threads;
    std::list<Task> m_tasks;

    size_t m_thread_count = 0;
    bool m_stopped = true;
    bool m_stopping = false;
    bool m_auto_stop = false;

    std::atomic<size_t> m_active_threads{ 0 };
    std::atomic<size_t> m_idle_threads{ 0 };
    int m_root_thread = 0;
    int m_epfd = 0;
    int m_tickle_fds[2];
};

}
