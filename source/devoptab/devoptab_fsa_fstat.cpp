#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

int __fsa_fstat(struct _reent *r,
                void *fd,
                struct stat *st) {
    FSAStat fsStat;

    if (!fd || !st) {
        r->_errno = EINVAL;
        return -1;
    }

    const auto file        = static_cast<__fsa_file_t *>(fd);
    const auto *deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    std::scoped_lock lock(file->mutex);

    const FSError status = FSAGetStatFile(deviceData->clientHandle, file->fd, &fsStat);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAGetStatFile(0x%08X, 0x%08X, %p) (%s) failed: %s",
                                deviceData->clientHandle, file->fd, &fsStat,
                                file->fullPath, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    const ino_t ino = __fsa_hashstring(file->fullPath);
    __fsa_translate_stat(deviceData->clientHandle, &fsStat, ino, st);

    return 0;
}
