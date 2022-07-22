#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_dirclose(struct _reent *r,
                   DIR_ITER *dirState) {
    FSError status;
    __fsa_dir_t *dir;

    if (!dirState) {
        r->_errno = EINVAL;
        return -1;
    }

    dir = (__fsa_dir_t *) (dirState->dirStruct);

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSACloseDir(deviceData->clientHandle, dir->fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSACloseDir(0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, dir->fd, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return 0;
}
