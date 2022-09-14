#include "util.h"
#include <sstream>
#include <execinfo.h>
#include "log.h"

namespace eva01 {

static Logger::ptr g_logger = EVA_LOGGER("system");

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

void Backtrace(std::vector<std::string>& bt, int size, int skip) {

    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);

    if(strings==NULL) {
        EVA_LOG_DEBUG(g_logger) << "backtrace_symbols error";
        free(strings);
        free(array);
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}
}
