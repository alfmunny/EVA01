#pragma once
#include "src/log.h"
#include "src/mutex.h"
#include "util.h"
#include <bits/stdint-uintn.h>
#include <functional>
#include <set>
#include <memory>
#include <vector>

namespace eva01 {

class TimerManager {
public:
    struct Timer : std::enable_shared_from_this<Timer> {
        using ptr = std::shared_ptr<Timer>;
        uint64_t m_period; // in ms 
        uint64_t m_next;   // in ms
        std::function<void()> m_func;
        bool m_recurring;
        TimerManager* m_manager = nullptr;

        struct cmp {
            bool operator()(const Timer::ptr& a, const Timer::ptr& b) const {
                if (!a && !b) return false;
                if (!a) return true;
                if (!b) return false;

                if (a->m_next < b->m_next) return true;
                if (b->m_next < a->m_next) return false;

                return a.get() < b.get();
            }
        };

        Timer(uint64_t period, std::function<void()> func, bool recurring, TimerManager* manager)
            : m_period(period), m_func(func), m_recurring(recurring), m_manager(manager) {
                m_next = GetCurrentMs() + m_period;
        };

        bool cancel();
        bool refresh();

    };

public:
    Timer::ptr addTimer(uint64_t period, std::function<void()> func, bool recurring);
    uint64_t getNextTimeMs();
    void getExpiredTimers(std::vector<Timer::ptr>& timers);
    bool hasTimers() { 
        MutexReadGuard lk(m_mutex);
        return !m_timers.empty(); 
    }

protected:
    virtual void onFirstTimerChanged();

private:
    void addTimer(Timer::ptr timer);

private:
    RWMutex m_mutex;
    std::set<Timer::ptr, Timer::cmp> m_timers;
};


} // namespace eva01
