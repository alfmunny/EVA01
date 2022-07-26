#include "fiber.h"
#include "macro.h"
#include "log.h"
#include <ucontext.h>
#include <atomic>

namespace eva01 {

static std::atomic<uint64_t> s_fiber_id { 0 };
static std::atomic<uint64_t> s_fiber_count { 0 };

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_main_fiber = nullptr;

static Logger::ptr g_logger = EVA_LOGGER("system");

Fiber::Fiber(std::function<void()> func, bool is_main):
    m_id(++s_fiber_id),
    m_func(func),
    m_main(is_main) {

    ASSERT(t_main_fiber);

    ++s_fiber_count;

    if(getcontext(&m_ctx)) {
        ASSERT(false);
    }

    //!!Important: do not set resume link. We may switch thread. The context may not exist
    m_ctx.uc_link = nullptr; 
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = sizeof(m_stack);

    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    EVA_LOG_DEBUG(g_logger) << "Fiber() id: " << m_id;
}

Fiber::Fiber()
{
    // Make sure no main fiber 
    ASSERT(!t_main_fiber);
    t_fiber = this;
    m_main = true;
    m_state = RUNNING;

    ++s_fiber_count;

    if(getcontext(&m_ctx)) {
        ASSERT(false);
    };

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = sizeof(m_stack);

    EVA_LOG_DEBUG(g_logger) << "Fiber main fiber id: " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (this != t_main_fiber.get()) {
        ASSERT(m_state != RUNNING);
    } else {
        ASSERT(!m_func);
        ASSERT(m_state == RUNNING);
        Fiber* cur = t_fiber;
        if (cur == this) {
            t_fiber = nullptr;
        }
    }

    EVA_LOG_DEBUG(g_logger) << "~Fiber id: " << m_id;
}

void Fiber::reset(std::function<void()> func) {
    ASSERT(t_main_fiber);

    if(getcontext(&m_ctx)) {
        ASSERT(false);
    }

    m_func = std::move(func);
    m_state = READY;
    m_ctx.uc_link = &t_main_fiber->m_ctx;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = sizeof(m_stack);

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
}

void Fiber::MakeMain() {
    if (t_main_fiber) {
        return;
    }
    t_main_fiber = Fiber::ptr(new Fiber);
    t_fiber = t_main_fiber.get();
}

void Fiber::call() {
    if (m_main && t_fiber == t_main_fiber.get()) {
        return;
    }
    t_fiber = this;

    ASSERT(m_state != RUNNING && m_state != TERM);

    m_state = RUNNING;
    
    if(swapcontext(&t_main_fiber->m_ctx, &m_ctx)) {
        ASSERT(false);
    }
}

void Fiber::yield() {
    // if current fiber is main fiber 
    if (t_fiber == t_main_fiber.get()) {
        return;
    }
    t_fiber = t_main_fiber.get();
    if(swapcontext(&m_ctx, &t_main_fiber->m_ctx)) {
        ASSERT(false);
    };
}

void Fiber::Yield() {
    // if current fiber is main fiber 
    if (t_fiber == t_main_fiber.get()) {
        return;
    }
    Fiber::ptr cur = GetThis();
    cur->m_state = SUSPEND;
    t_fiber = t_main_fiber.get();

    if(swapcontext(&cur->m_ctx, &t_main_fiber->m_ctx)) {
        ASSERT(false);
    };

}

void Fiber::YieldReady() {
    // if current fiber is main fiber 
    if (t_fiber == t_main_fiber.get()) {
        return;
    }
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    t_fiber = t_main_fiber.get();

    if(swapcontext(&cur->m_ctx, &t_main_fiber->m_ctx)) {
        ASSERT(false);
    };

}

Fiber::ptr Fiber::GetMainFiber() {
    if (t_main_fiber) {
        return t_main_fiber;
    }
    else {
        return nullptr;
    }
}

uint64_t Fiber::GetTotalCount() {
    return s_fiber_count;
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

Fiber::ptr Fiber::GetThis() {
    if (!t_fiber) {
        MakeMain();
    }
    return t_fiber->shared_from_this();
}

void Fiber::MainFunc() {
    auto cur = GetThis();
    ASSERT(cur);

    try {
        cur->m_func();
        cur->m_state = TERM;
    }
    catch(std::exception &ex) {
        cur->m_state = EXCEPT;
        EVA_LOG_ERROR(g_logger) << "Exception: " << ex.what();
    }

    //!!Important: swap out manually to free the pointer cur.
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->yield();

}

}
