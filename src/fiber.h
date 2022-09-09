#pragma once

#include <memory>
#include <ucontext.h>
#include <functional>

namespace eva01 {

#define STACKSIZE 1024*1024 // 1mb

class Fiber : public std::enable_shared_from_this<Fiber>{

public:
    using ptr = std::shared_ptr<Fiber>;

    enum State {
        READY,
        SUSPEND,
        RUNNING,
        TERM,
        EXCEPT
    };

public:
    Fiber(std::function<void()> func, bool is_main = false);
    State getState() { return m_state; }
    uint64_t getId() { return m_id; }
    ~Fiber();

private:
    Fiber();

public:
    void call();

public:
    static Fiber::ptr GetThis();
    static void MainFunc();
    static void MakeMain();
    static void Yield();

private:
    uint64_t m_id;
    ucontext_t m_ctx;
    std::function<void()> m_func;
    bool m_main;
    char m_stack[STACKSIZE];
    State m_state = READY;

};

}
