#pragma once

#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

namespace eva01 {

pid_t GetThreadId();

void SleepMs(int ms);

}
