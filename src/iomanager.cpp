#include "iomanager.h"
#include "log.h"
#include "src/mutex.h"
#include "src/scheduler.h"
#include <string.h>
#include <sys/epoll.h>

namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

constexpr const size_t DEFAULT_FD_CONTEXTS_SIZE = 32;

IOManager::IOManager(int threads, const std::string& name)
        : Scheduler(threads, name) {
    start();
    resizeContexts(DEFAULT_FD_CONTEXTS_SIZE);
}

void IOManager::resizeContexts(size_t size) {
    m_fd_contexts.resize(size);
    for (size_t i = 0; i < size; ++i) {
        if (!m_fd_contexts[i]) {
            m_fd_contexts[i] = new FdContext;
            m_fd_contexts[i]->fd = (int)i;
        }
    }
}

IOManager::~IOManager(){
    stop();
    for (size_t i = 0; i < m_fd_contexts.size(); ++i) {
        if (m_fd_contexts[i]) {
            delete m_fd_contexts[i];
        }
    }
}

bool IOManager::addEvent(int fd, Event event, std::function<void()> func) {
    FdContext* fd_ctx;
    bool need_resize = false;
    {
        MutexReadGuard lk(m_mutex);
        if ((int)m_fd_contexts.size() <= fd) {
            need_resize = true;
        }
    }

    if (need_resize) {
        MutexWriteGuard lk(m_mutex);
        resizeContexts(fd * 1.5);
        fd_ctx = m_fd_contexts[fd];
    } else {
        MutexReadGuard lk(m_mutex);
        fd_ctx = m_fd_contexts[fd];
    }

    if (fd_ctx->events & event) {
        EVA_LOG_ERROR(g_logger) << "addEvent: event already exists";
        ASSERT(false);
    }

    MutexGuard<Mutex> lk(fd_ctx->mtx);

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;

    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    ASSERT(!rt);
    fd_ctx->events = (Event)(fd_ctx->events | event);
    ++m_pending_count;

    auto& event_ctx = fd_ctx->getContext(event);

    ASSERT(!event_ctx.scheduler &&
           !event_ctx.fiber && 
           !event_ctx.func);

    event_ctx.scheduler = Scheduler::GetThis();
    if (func) {
        event_ctx.func = func;
    } else {
        event_ctx.fiber = Fiber::GetThis();
        ASSERT(event_ctx.fiber->getState() == Fiber::RUNNING);
    }

    return true;
}

bool IOManager::delEvent(int fd, Event event) {
    FdContext* fd_ctx;
    {
        MutexReadGuard lk(m_mutex);
        if ((int)m_fd_contexts.size() <= fd) {
            return false;
        }
        fd_ctx = m_fd_contexts[fd];
    }
    MutexGuard<Mutex> lk(fd_ctx->mtx);

    if (!(fd_ctx->events & event)) {
        return false;
    }

    Event left_events = (Event)(fd_ctx->events & ~event);
    int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | left_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    ASSERT(!rt);

    fd_ctx->events = left_events;
    --m_pending_count;
    fd_ctx->resetContext(event);

    return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
    FdContext* fd_ctx;
    {
        MutexReadGuard lk(m_mutex);
        if ((int)m_fd_contexts.size() <= fd) {
            return false;
        }
        fd_ctx = m_fd_contexts[fd];
    }
    MutexGuard<Mutex> lk(fd_ctx->mtx);

    if (!(fd_ctx->events & event)) {
        return false;
    }

    Event left_events = (Event)(fd_ctx->events & ~event);
    int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | left_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    ASSERT(!rt);

    --m_pending_count;
    fd_ctx->scheduleEvent(event);

    return true;
}

bool IOManager::cancelAll(int fd) {
    MutexReadGuard lock(m_mutex);
    if((int)m_fd_contexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fd_contexts[fd];
    lock.unlock();

    MutexGuard<Mutex> lock2(fd_ctx->mtx);
    if(!fd_ctx->events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        EVA_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->events & READ) {
        fd_ctx->scheduleEvent(READ);
        --m_pending_count;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->scheduleEvent(WRITE);
        --m_pending_count;
    }

    ASSERT(fd_ctx->events == 0);
    return true;
}

bool IOManager::scheduleEvent(epoll_event& event){
    FdContext* fd_ctx = (FdContext*)event.data.ptr;
    MutexGuard<Mutex> lk(fd_ctx->mtx);

    // handle error
    if (event.events & (EPOLLHUP | EPOLLERR)) {
        EVA_LOG_ERROR(g_logger) << "EPOLLERR or EPOLLERR";
        // delete all events
        event.events = (EPOLLIN | EPOLLOUT) & fd_ctx->events; 
    }

    // get triggered event
    int triggered_events = NONE;
    if (event.events & EPOLLIN) {
        triggered_events |= READ;
    }

    if (event.events & EPOLLOUT) {
        triggered_events |= WRITE;
    }

    if ((fd_ctx->events & triggered_events)  == NONE) {
        EVA_LOG_DEBUG(g_logger) << "scheduleEvent: no events";
        return false;
    }

    // check for left events
    // modify or delete the events
    int left_events = (fd_ctx->events & ~triggered_events);
    int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    event.events = EPOLLET | left_events;

    int rt = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
    if (rt) {
        EVA_LOG_ERROR(g_logger) << "epoll_ctrl(" << m_epfd << ", " << op << ", " << fd_ctx->fd 
            << ", " << (EPOLL_EVENTS)event.events << "), error: " << strerror(errno);
        return false;
    }

    // schedule the triggered events
    if (triggered_events & READ) {
        fd_ctx->scheduleEvent(READ);
        --m_pending_count;
    }

    if (triggered_events & WRITE) {
        fd_ctx->scheduleEvent(WRITE);
        --m_pending_count;
    }

    return true;
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

}
