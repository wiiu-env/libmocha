#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

DIR_ITER *
__fsa_diropen(struct _reent *r,
              DIR_ITER *dirState,
              const char *path) {
    FSADirectoryHandle fd;
    FSError status;
    __fsa_dir_t *dir;

    if (!dirState || !path) {
        r->_errno = EINVAL;
        return NULL;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        return NULL;
    }
    dir = (__fsa_dir_t *) (dirState->dirStruct);

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAOpenDir(deviceData->clientHandle, fixedPath, &fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAOpenDir(0x%08X, %s, 0x%08X) failed: %s", deviceData->clientHandle, fixedPath, &fd, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return NULL;
    }
    free(fixedPath);

    dir->magic = FS_DIRITER_MAGIC;
    dir->fd    = fd;
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));
    return dirState;
}
