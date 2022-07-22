#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_fchmod(struct _reent *r,
                 void *fd,
                 mode_t mode) {
    FSError status;
    __fsa_file_t *file;

    file = (__fsa_file_t *) fd;

    FSMode translatedMode = __fsa_translate_permission_mode(mode);

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAChangeMode(deviceData->clientHandle, file->path, translatedMode);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAChangeMode(0x%08X, %s, 0x%X) failed: %s", deviceData->clientHandle, file->path, translatedMode, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return 0;
}
