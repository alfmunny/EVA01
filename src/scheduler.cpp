#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "src/mutex.h"
#include "src/thread.h"



namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(int threads, const std::string& name) 
    : m_name(name) {
    ASSERT(threads > 0);

    Thread::SetName(m_name);
    Fiber::GetThis();

    EVA_LOG_DEBUG(g_logger) << "Main fiber create";
    m_main_fiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
    t_fiber = m_main_fiber.get();
    m_root_thread = GetThreadId();

    --threads;
    m_thread_count = threads;
}

Scheduler::~Scheduler() {
    ASSERT(!m_stopping);

}

void Scheduler::start() {
    MutexGuard<Mutex> lock(m_mutex);
    if (m_stopping) {
        return;
    }
    m_stopped = false;
    //ASSERT(m_threads.empty());

    m_threads.resize(m_thread_count);

    for (size_t i = 0; i < m_thread_count; ++i) {
        auto thr = Thread::ptr(new Thread(
                    std::bind(&Scheduler::run, this), 
                    m_name + "_" + std::to_string(i)));
        m_threads.push_back(thr);
    }
}

void Scheduler::stop() {
    ASSERT(!m_stopped);
    ASSERT(!m_stopping);

    m_stopping = true;

    m_main_fiber->call();

    EVA_LOG_DEBUG(g_logger) << "call passed";

    tickle();

    for (size_t i = 0; i < m_thread_count; ++i) {
        tickle();
    }


    for (auto& it : m_threads) {
        it->join();
    }

    m_stopping = false;
    m_stopped = true;
}

void Scheduler::run() {
    t_fiber = Fiber::GetThis().get();
    t_scheduler = this;

    ASSERT(!m_stopped);

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Task tk;

    while (true) {
        tk.reset();
        bool need_tickle = false;
        // find out task
        {
            MutexGuard<Mutex> lk(m_mutex);
            auto it = m_tasks.begin();
            while (it != m_tasks.end()) {
                if (it->thread_id != -1 && it->thread_id != GetThreadId()) {
                    continue;
                }

                tk = *it;
                m_tasks.erase(it);
                need_tickle = true;
                break;
            }

        }

        if (need_tickle) {
            tickle();
        }

        if (tk.fiber) {

            tk.fiber->call();

            if (tk.fiber->getState() == Fiber::SUSPEND ||
                tk.fiber->getState() == Fiber::READY) {
                schedule(tk.fiber);
            } else {
                // Task is done. Nothing to do
            }
            tk.reset();
        } else {
            if (idle_fiber->getState() == Fiber::TERM) {
                EVA_LOG_DEBUG(g_logger) << "idle fiber terminate";
                break;
            }
            idle_fiber->call();
        }
        // schedule task
    }
    EVA_LOG_DEBUG(g_logger) << "run finished";

}

void Scheduler::tickle() {
    EVA_LOG_DEBUG(g_logger) << "tickle";
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

bool Scheduler::should_stop() {
    MutexGuard<Mutex> lk(m_mutex);
    return m_stopping && m_tasks.empty();
}

void Scheduler::idle() {
    EVA_LOG_DEBUG(g_logger) <<  "idle";
    while(!should_stop()) {
        Fiber::Yield();
    }
}

}
