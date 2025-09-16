#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

int __fsa_dirreset(struct _reent *r,
                   DIR_ITER *dirState) {

    if (!dirState) {
        r->_errno = EINVAL;
        return -1;
    }

    const auto dir         = static_cast<__fsa_dir_t *>(dirState->dirStruct);
    const auto *deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    std::scoped_lock lock(dir->mutex);

    const FSError status = FSARewindDir(deviceData->clientHandle, dir->fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSARewindDir(0x%08X, 0x%08X) (%s) failed: %s",
                                deviceData->clientHandle, dir->fd, dir->fullPath, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return 0;
}
