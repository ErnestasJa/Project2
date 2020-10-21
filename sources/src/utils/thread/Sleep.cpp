#include "util/thread/Sleep.h"
#include <threads.h>
#include <time.h>

namespace util::thread {
    void Sleep(int32_t microseconds){
        auto time = (struct timespec){.tv_nsec=microseconds*1000};
        timespec remaining {.tv_nsec=0};

        thrd_sleep(&time, &remaining);

        if(remaining.tv_nsec > 0) {
            elog::LogWarning(core::string::format("Sleep failed, Remaining nanoseconds: {}", remaining.tv_nsec));
        }
    }
}