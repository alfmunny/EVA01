#include "timer.h"
#include "src/mutex.h"
#include "src/util.h"
#include "log.h"
#include <algorithm>

namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

using Timer = TimerManager::Timer;

Timer::ptr TimerManager::addTimer(uint64_t period, std::function<void()> func, bool recurring) {
    Timer::ptr timer = std::make_shared<Timer>(period, func, recurring);
    addTimer(timer);
    return timer;
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

void TimerManager::removeTimer(Timer::ptr timer) {
    MutexWriteGuard lk(m_mutex);
    m_timers.erase(timer);
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

void TimerManager::getExpiredTimers(std::vector<Timer::ptr>& timers) {
    {
        MutexReadGuard lk(m_mutex);
        if (m_timers.empty()) {
            return;
        }
    }
    
    auto now = GetCurrentMs();

    MutexWriteGuard lk(m_mutex);

    for (auto it = m_timers.begin(); it != m_timers.end(); ) {
        auto timer = *it;
        if (timer->m_next <= now) {
            timers.push_back(timer);
            it = m_timers.erase(it);
        } else {
            ++it;
        }
    }

    for (auto timer : timers) {
        if (timer->m_recurring) {
            timer->m_next = now + timer->m_period;
            m_timers.insert(timer);
        }
    }
}

} // namespace eva01
