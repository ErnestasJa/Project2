#include "util/thread/Sleep.h"
#include <thread>
#include <chrono>

namespace util::thread {
    void Sleep(int32_t microseconds){
        std::this_thread::sleep_for(std::chrono::microseconds{microseconds});
    }
}