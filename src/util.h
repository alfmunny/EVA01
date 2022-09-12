#pragma once

#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <cstdint>

namespace eva01 {

pid_t GetThreadId();

void SleepMs(int ms);

uint64_t GetCurrentMs();

}
