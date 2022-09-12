#include "util.h"

namespace eva01 {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

void SleepMs(int ms) {
    usleep(ms * 1000);
}

uint64_t GetCurrentMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

}
