#include "../logger.h"
#include "devoptab_fsa.h"
#include <cstdio>
#include <mutex>

int __fsa_dirnext(struct _reent *r,
                  DIR_ITER *dirState,
                  char *filename,
                  struct stat *filestat) {

    if (!dirState || !filename || !filestat) {
        r->_errno = EINVAL;
        return -1;
    }

    const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);
    const auto dir        = static_cast<__fsa_dir_t *>(dirState->dirStruct);

    std::scoped_lock lock(dir->mutex);
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));

    const auto status = FSAReadDir(deviceData->clientHandle, dir->fd, &dir->entry_data);
    if (status < 0) {
        if (status != FS_ERROR_END_OF_DIR) {
            DEBUG_FUNCTION_LINE_ERR("FSAReadDir(0x%08X, 0x%08X, %p) (%s) failed: %s",
                                    deviceData->clientHandle, dir->fd, &dir->entry_data, dir->fullPath, FSAGetStatusStr(status));
        }
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    ino_t ino;
    size_t fullLen = strlen(dir->fullPath) + 1 + strlen(dir->entry_data.name) + 1;
    char *fullStr  = (char *) memalign(0x40, fullLen);
    if (fullStr) {
        if (snprintf(fullStr, fullLen, "%s/%s", dir->fullPath, dir->entry_data.name) >= (int) fullLen) {
            DEBUG_FUNCTION_LINE_ERR("__fsa_dirnext: snprintf fullStr result was truncated");
        }
        ino = __fsa_hashstring(fullStr);
        free(fullStr);
    } else {
        ino = 0;
        DEBUG_FUNCTION_LINE_ERR("__fsa_dirnext: Failed to allocate memory for fullStr. st_ino will be set to 0");
    }
    __fsa_translate_stat(deviceData->clientHandle, &dir->entry_data.info, ino, filestat);

    if (snprintf(filename, NAME_MAX, "%s", dir->entry_data.name) >= NAME_MAX) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_dirnext: snprintf filename result was truncated");
    }

    return 0;
}
