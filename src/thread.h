#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <functional>
#include <memory>
#include <string>
#include <condition_variable>
#include <mutex>
#include "src/util.h"

namespace eva01 {

class Thread {

public:
    using ptr = std::shared_ptr<Thread>;

    Thread(std::function<void()> cb, const std::string& name);

    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;

    inline pid_t getId() const { return m_id; }
    inline std::string getName() const { return m_name; }

    ~Thread();

    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);


private:
    pthread_t m_thread = 0;
    pid_t m_id = -1;
    std::function<void()> m_cb;
    std::string m_name;
    std::condition_variable m_cv_init;
    std::mutex m_mtx_init;
    bool m_is_init;
    

private:
    static void* run(void* arg);
};


}
