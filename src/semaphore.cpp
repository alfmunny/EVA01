#include "src/semaphore.h"
#include <stdexcept>

namespace eva01 {

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

}
