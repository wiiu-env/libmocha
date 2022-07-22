#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_ftruncate(struct _reent *r,
                    void *fd,
                    off_t len) {
    FSError status;
    __fsa_file_t *file;

    // Make sure length is non-negative
    if (!fd || len < 0) {
        r->_errno = EINVAL;
        return -1;
    }

    // Set the new file size
    file = (__fsa_file_t *) fd;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);
    status = FSASetPosFile(deviceData->clientHandle, file->fd, len);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSASetPosFile(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, len, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    status = FSATruncateFile(deviceData->clientHandle, file->fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSATruncateFile(0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return 0;
}
