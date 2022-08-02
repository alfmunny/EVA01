#include "util.h"

namespace eva01 {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

}
