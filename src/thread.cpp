#include "thread.h"
#include "fiber.h"

namespace eva01 {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(cb), m_name(name), m_semaphore(0) {

    if (m_name.empty()) {
        m_name = "UNKNOWN";
    }

    if (pthread_create(&m_thread, nullptr, &Thread::run, this)) {
        throw std::logic_error("pthread_create failed");
    }

    m_semaphore.wait();
};

Thread* Thread::GetThis() { return t_thread; }
const std::string& Thread::GetName() { return t_thread_name; }


void Thread::SetName(const std::string& name) {
    if (name.empty()) {
        return;
    }

    if (t_thread) {
        t_thread->m_name = name;
        t_thread_name = name;
    }
}

void Thread::join() {
    if (m_thread) {
        if (pthread_join(m_thread, nullptr)) {
            throw std::logic_error("pthread_join failed");
        }
        m_thread = 0;
    }
};

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
};

void* Thread::run(void* arg) {
    Thread* t = (Thread*) arg;
    t_thread = t;
    t_thread_name = t->m_name;
    t->m_id = GetThreadId();

    pthread_setname_np(pthread_self(), t_thread_name.substr(0, 15).c_str());

    t->m_semaphore.notify();
    t->m_cb();
    return 0;
}

}
