#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

DIR_ITER *
__fsa_diropen(struct _reent *r,
              DIR_ITER *dirState,
              const char *path) {
    FSADirectoryHandle fd;

    if (!dirState || !path) {
        r->_errno = EINVAL;
        return nullptr;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        return nullptr;
    }
    const auto dir        = static_cast<__fsa_dir_t *>(dirState->dirStruct);
    const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    // Remove trailing '/'
    if (fixedPath[0] != '\0') {
        if (fixedPath[strlen(fixedPath) - 1] == '/') {
            fixedPath[strlen(fixedPath) - 1] = 0;
        }
    }

    if (snprintf(dir->fullPath, sizeof(dir->fullPath), "%s", fixedPath) >= (int) sizeof(dir->fullPath)) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_diropen: snprintf result was truncated");
    }

    free(fixedPath);

    dir->mutex.init(dir->fullPath);
    std::scoped_lock lock(dir->mutex);

    const FSError status = FSAOpenDir(deviceData->clientHandle, dir->fullPath, &fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAOpenDir(0x%08X, %s, %p) failed: %s",
                                deviceData->clientHandle, dir->fullPath, &fd, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return nullptr;
    }

    dir->magic = FSA_DIRITER_MAGIC;
    dir->fd    = fd;
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));
    return dirState;
}
