#pragma once

#include <coreinit/fastmutex.h>
class FastLockWrapper {
public:
    FastLockWrapper() {
        OSFastMutex_Init(&mutex, "generic lock");
    }

    void lock() {
        OSFastMutex_Lock(&mutex);
    }

    void unlock() {
        OSFastMutex_Unlock(&mutex);
    }

private:
    OSFastMutex mutex{};
};
