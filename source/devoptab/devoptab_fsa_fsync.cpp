#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_fsync(struct _reent *r,
                void *fd) {
    FSError status;
    __fsa_file_t *file;

    if (!fd) {
        r->_errno = EINVAL;
        return -1;
    }

    file = (__fsa_file_t *) fd;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAFlushFile(deviceData->clientHandle, file->fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAFlushFile(0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return 0;
}
