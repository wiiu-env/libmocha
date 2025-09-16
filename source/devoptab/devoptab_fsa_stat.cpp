#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

int __fsa_stat(struct _reent *r,
               const char *path,
               struct stat *st) {
    FSAStat fsStat;

    if (!path || !st) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        r->_errno = ENOMEM;
        return -1;
    }

    const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    const FSError status = FSAGetStat(deviceData->clientHandle, fixedPath, &fsStat);
    if (status < 0) {
        if (status != FS_ERROR_NOT_FOUND) {
            DEBUG_FUNCTION_LINE_ERR("FSAGetStat(0x%08X, %s, %p) failed: %s",
                                    deviceData->clientHandle, fixedPath, &fsStat, FSAGetStatusStr(status));
        }
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    const ino_t ino = __fsa_hashstring(fixedPath);
    free(fixedPath);

    __fsa_translate_stat(deviceData->clientHandle, &fsStat, ino, st);

    return 0;
}