#pragma once

#include <assert.h>
#include "log.h"
#include "util.h"

namespace eva01 {

#define ASSERT(x) \
    if(!(x)) { \
        EVA_LOG_ERROR(EVA_ROOT_LOGGER()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

}
