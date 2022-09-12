#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "src/mutex.h"
#include "src/thread.h"
#include "src/timer.h"
#include <bits/stdint-uintn.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>

namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

constexpr const int MAX_EVENTS = 256;
constexpr const int EPOLL_WAIT_TIMEOUT_MS = 3000;

Scheduler::Scheduler(int threads, const std::string& name) 
    : m_name(name) {
    ASSERT(threads > 0);

    //initialize threads
    Thread::SetName(m_name);
    m_root_thread = GetThreadId();
    m_thread_count = threads;

    //initialize epoll fd
    m_epfd = epoll_create(5000);
    ASSERT(m_epfd > 0);

    ASSERT(!pipe(m_tickle_fds));
    ASSERT(!fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK));

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickle_fds[0];

    ASSERT(!epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickle_fds[0], &event));
}

Scheduler::~Scheduler() {
    MutexGuard<MutexType> lock(m_mutex);
    ASSERT(!m_stopping);
    ASSERT(m_stopped);
}

void Scheduler::start() {
    MutexGuard<MutexType> lock(m_mutex);

    if (m_stopping) { return; }
    m_stopped = false;

    ASSERT(m_threads.empty());
    m_threads.resize(m_thread_count);
    for (size_t i = 0; i < m_thread_count; ++i) {
        m_threads[i].reset(new Thread(
                    std::bind(&Scheduler::run, this), 
                    m_name + "_" + std::to_string(i)));
    }
}

void Scheduler::stop() {
    EVA_LOG_DEBUG(g_logger) << "name=" << getName() << " stopping";
    {
        MutexGuard<MutexType> lk(m_mutex);
        m_stopping = true;
    }

    // Wake up all threads to catch up the new m_stopping flag.
    for (size_t i = 0; i < m_thread_count; ++i) {
        tickle();
    }

    // Join all threads.
    for (auto& it : m_threads) {
        it->join();
    }

    {
        MutexGuard<MutexType> lk(m_mutex);
        m_threads.clear();
        m_stopping = false;
        m_stopped = true;
    }
}

void Scheduler::run() {
    t_fiber = Fiber::GetThis().get(); // initialize main fiber in this thread.
    t_scheduler = this;

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this))); // idle fiber for waiting on the task
    Fiber::ptr func_fiber(new Fiber([](){})); // fiber to do the task, avoid creating new fiber for each new std::function
    Task tk; // to register the next task, avoid creating a new task each loop.

    while (true) {
        tk.reset();
        bool need_tickle = false;

        // find out todo task
        {
            MutexGuard<MutexType> lk(m_mutex);
            auto it = m_tasks.begin();
            while (it != m_tasks.end()) {
                // Skip the task, if task is assigned to a specific thread
                if (it->thread_id != -1 && it->thread_id != GetThreadId()) {
                    continue;
                }
                
                tk = std::move(*it);
                m_tasks.erase(it);
                break;
            }
            need_tickle = !m_tasks.empty();
        }

        // This thread will be working, m_active_threads has to be increased before tickle()
        // See shouldStop().
        // The other threads need to know if there is still working threads.
        // If yes, the other threads will wait because m_active_threads != 0, even stop() is called, until all work is done.
        if (tk.fiber || tk.func) {
            ++m_active_threads;            
        }

        // If there is still tasks in queue,
        // wakeup the others threads
        if (need_tickle) {
            tickle();
        }

        // Schedule task
        // If the task is a fiber
        if (tk.fiber) {
            tk.fiber->call(); // Do the task
            --m_active_threads; // Back from the task

            // Reschedule if the task is not done
            if (tk.fiber->getState() == Fiber::SUSPEND) {
                schedule(std::move(tk.fiber));
            } 
            tk.reset();
        } 
        // If the task is a function
        else if (tk.func) {
            func_fiber->reset(std::move(tk.func));

            func_fiber->call(); // Do the task
            --m_active_threads; // Back from the task

            // Reschedule if the task is not done
            if (func_fiber->getState() == Fiber::SUSPEND) {
                schedule(std::move(func_fiber));
            } 

            tk.reset();
            func_fiber->reset(nullptr);
        }
        else {
            // Stopping and break out run.
            // The idle fiber has terminated.
            if (idle_fiber->getState() == Fiber::TERM) {
                EVA_LOG_DEBUG(g_logger) << "idle fiber terminate";
                break;
            }

            // Trap into idle fiber.
            ++m_idle_threads;
            idle_fiber->call();
            --m_idle_threads;
        }
    }
}

void Scheduler::tickle() {
    if (m_idle_threads <= 0) {
        return;
    }
    EVA_LOG_DEBUG(g_logger) << "tickle";
    ASSERT(write(m_tickle_fds[1], "T", 1) == 1);
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

bool Scheduler::shouldStop() {
    MutexGuard<MutexType> lk(m_mutex);
    // Only to stop when stop is called and all tasks are done.
    return !hasTimers() && m_stopping && m_tasks.empty() && m_active_threads == 0;
}

void Scheduler::idle() {
    EVA_LOG_DEBUG(g_logger) <<  "idle";
    epoll_event *events = new epoll_event[MAX_EVENTS];
    std::shared_ptr<epoll_event> events_deleter(events, [](epoll_event *ptr) { delete[] ptr; });

    std::vector<TimerManager::Timer::ptr> timers;

    while (true) {
        if(shouldStop()) {
            EVA_LOG_DEBUG(g_logger) << "Scheduler name="  << getName() << " idle stopping";
            break;
        }

        int rt = 0;
        while (true)  {
            EVA_LOG_DEBUG(g_logger) << "epoll_wait waiting";

            auto next_time = getNextTimeMs();
            auto timeout = EPOLL_WAIT_TIMEOUT_MS;

            if (next_time != 0ull && next_time < EPOLL_WAIT_TIMEOUT_MS) {
                timeout = next_time;
            }

            rt = epoll_wait(m_epfd, events, MAX_EVENTS, (int)timeout);
            if (rt < 0 && errno == EINTR) {
                // 0: timeout with no events, 
                // -1: error
                // >0: events appear
            } 
            else {
                break;
            }
        }

        // schedule expired timers
        getExpiredTimers(timers);

        for (auto& timer : timers) {
            schedule(timer->m_func);
        }
        timers.clear();

        // schedule ohter events, clear out the tickle fds
        for (int i = 0; i < rt; ++i) {
            epoll_event event = events[i];
            if (event.data.fd == m_tickle_fds[0]) {
                char dummy[256];
                while(read(m_tickle_fds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }
        }
        Fiber::Yield();
    }
}

void Scheduler::onFirstTimerChanged() {
    tickle();
}

}
