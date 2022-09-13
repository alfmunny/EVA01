#include "timer.h"
#include "src/mutex.h"
#include "src/util.h"
#include "log.h"
#include <algorithm>

namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

using Timer = TimerManager::Timer;

Timer::ptr TimerManager::addTimer(uint64_t period, std::function<void()> func, bool recurring) {
    Timer::ptr timer = std::make_shared<Timer>(period, func, recurring, this);
    addTimer(timer);
    return timer;
}

bool TimerManager::Timer::cancel() {
    MutexWriteGuard lk(m_manager->m_mutex);
    return m_manager->m_timers.erase(shared_from_this()) == 1;
}

bool TimerManager::Timer::refresh() {
    auto timer = shared_from_this();
    {
        MutexWriteGuard lk(m_manager->m_mutex);

        if (m_manager->m_timers.erase(timer) != 1) {
            return false;
        }
        m_next = GetCurrentMs() + m_period;
    }
    m_manager->addTimer(timer);
    return true;
}

void TimerManager::addTimer(Timer::ptr timer) {
    bool changed = false;
    {
        MutexWriteGuard lk(m_mutex);
        auto it = m_timers.insert(timer).first;
        if (it == m_timers.begin()) {
            changed = true;
        }
    }

    if (changed) {
        onFirstTimerChanged();
    }
}

void TimerManager::onFirstTimerChanged() {

}

uint64_t TimerManager::getNextTimeMs()  {
    MutexReadGuard lk(m_mutex);
    if (m_timers.empty()) {
        return ~0ull;
    }
    const auto timer = *m_timers.begin();
    auto now = GetCurrentMs();
    if (timer->m_next > now) {
        return timer->m_next - now;
    }  else {
        return 0;
    }
}

bool TimerManager::hasTimers() { 
    MutexReadGuard lk(m_mutex);
    return !m_timers.empty(); 
}

void TimerManager::getExpiredFuncs(std::vector<std::function<void ()> > &funcs) {
    auto now = GetCurrentMs();
    {
        MutexReadGuard lk(m_mutex);
        if (m_timers.empty()) {
            return;
        }
    }
    
    std::vector<Timer::ptr> expired_timers;
    MutexWriteGuard lk(m_mutex);

    for (auto it = m_timers.begin(); it != m_timers.end(); ) {
        auto timer = *it;
        if (timer->m_next <= now) {
            funcs.emplace_back(timer->m_func);
            expired_timers.push_back(std::move(timer));
            it = m_timers.erase(it);
        } else {
            ++it;
        }
    }

    for (auto& timer : expired_timers) {
        if (timer->m_recurring) {
            timer->m_next += timer->m_period; // 
            m_timers.insert(timer);
        }
    }
}

} // namespace eva01
