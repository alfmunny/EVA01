A simple server framework written in C++

## Logging
- Logger
    - level
    - multiple appenders
- LogAppender: StdOut, File, Stream
    - its own formatter
    - its own level
- LogEvent
- LogLevel: UNKNOW, DEBUG, INFO, WARN, ERROR, FATAL
- LogFormatter

        """
        Use log4j-like pattern:

        "%d{%Y-%m-%d %a %H:%M:%S}%T%f%T%l%T[%p]%T[%c]%T%m%n";

        Every pattern has also has sub format in {}

        %m, // m: message
        %p, // p: level
        %r, // r: elapse time
        %c, // c: logger name
        %t, // t: thread id
        %n, // n: new line
        %d, // d: date, format the date %d{%Y-%m-%d %a %H:%M:%S}
        %f, // f: file, %f{5} means start from depth 5
        %l, // l: line number
        %T, // T: Tab
        %F, // F: fiber id
        %N, // N: thread name
        """
* LoggerManager
    * Singleton pattern
    * Manage multiple loggers

## Thread

Thread class using pthread

For a basic thread class, we mainly need these system calls to Pthread.

* can pass in a function
* has a name: getName(), GetName()
* can set name: SetName()
* has a pid_t: getId()
* can join: join()

```c
pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_rountine)(void *), void *arg);
pthread_join(pthread_t *thread)
pthread_detach(pthread_t *thread)
pthread_setname_np(pthread_t *thread, const char *name)
```
Note: 

* Use semaphore to block in the constructor, to make sure the function is in running before the constructor returns.

```c
Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&m_semaphore, 0, count)) {
        perror("sem_init");
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    if (sem_wait(&m_semaphore)) {
        perror("sem_wait");
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify() {
    if (sem_post(&m_semaphore)) {
        perror("sem_post");
        throw std::logic_error("sem_post error");
    }
}

```

## Mutex

Provide Mutex and Read-Write-Mutex using pthread library.

Mutex

```c
pthread_mutex_t m_mutex;
pthread_mutex_init(&m_mutex, NULL);
pthread_mutex_destroy(&m_mutex);
pthread_mutex_lock(&m_mutex); 
pthread_mutex_unlock(&m_mutex); 
```

RWMutex

```c
pthread_rwlock_t m_lock;
pthread_rwlock_init(&m_lock, NULL);
pthread_rwlock_destroy(&m_lock);
pthread_rwlock_rdlock(&m_lock); 
pthread_rwlock_wrlock(&m_lock);
pthread_rwlock_unlock(&m_lock); 
```

## Fiber

With `ucontext.h` it is possible to swtich context in user space.
Fiber is a class with a set of functions to enable coroutine.

Each thread has a main fiber. Every sub fiber will return to main fiber after it is done.

A sub fiber can give up its execution to the main fiber -> Fiber::Yield().
A sub fiber can be called and swapped into current execution -> fiber->call().

```
void func() {
    // do something
    Yield()
    // continue 
}
fiber = new Fiber(func);
fiber.call() // back from func when it yields
doSomethingElse()
fiber.call() // go into func again to continue
```

## Scheduler

A scheduler maintains a thread pool and a task queue. 
It will schedule the task with fiber, running in a random thread automatically.

It can be initialized with a threads number.

```
new Scheduler(3)
scheduler.start()
scheduler.schedule(task)
scheduler.stop()
```

Scheduler with epoll.

* implement tickle() and idle() with epoll 
    * create a pipe and register the read event in epoll
    * wait for the read event in idle()
    * tickle() write event to the pipe to wake up the idle threads


```
epoll_event event // struct for evets, event.events(EPOLLIN, EPOLLOUT, EPOLLET, EPOLLLT), event.data.fd, event.data.ptr
epoll_create() // create epoll fd
epoll_ctl()  // add or delete or modify the events
epoll_wait() // blocking the current thread for events
```

