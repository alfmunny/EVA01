#pragma once

#include <assert.h>

namespace eva01 {

#define ASSERT(x) \
    if(!(x)) { \
        assert(x); \
    }

}
