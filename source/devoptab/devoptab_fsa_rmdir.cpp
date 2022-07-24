#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_rmdir(struct _reent *r,
                const char *name) {
    FSError status;

    if (!name) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, name);
    if (!fixedPath) {
        r->_errno = ENOMEM;
        return -1;
    }

    // Check if directory still has files in which case return error
    FSDirectoryHandle dirHandle = 0;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAOpenDir(deviceData->clientHandle, fixedPath, &dirHandle);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAOpenDir(0x%08X, %s, 0x%08X) failed: %s", deviceData->clientHandle, fixedPath, &dirHandle, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    auto *dir_entry = (FSADirectoryEntry *) malloc(sizeof(FSADirectoryEntry));
    if (dir_entry == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("malloc(sizeof(FSADirectoryEntry)) failed");
        free(fixedPath);
        FSACloseDir(deviceData->clientHandle, dirHandle);
        r->_errno = ENOMEM;
        return -1;
    }

    status = FSAReadDir(deviceData->clientHandle, dirHandle, dir_entry);
    FSACloseDir(deviceData->clientHandle, dirHandle);
    if (status == FS_ERROR_OK) {
        free(fixedPath);
        free(dir_entry);
        r->_errno = ENOTEMPTY;
        return -1;
    } else if (status != FS_ERROR_END_OF_DIR) {
        DEBUG_FUNCTION_LINE_ERR("FSAReadDir(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, dirHandle, dir_entry, FSAGetStatusStr(status));
        free(fixedPath);
        free(dir_entry);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(dir_entry);

    // Remove folder since folder is empty
    status = FSARemove(deviceData->clientHandle, fixedPath);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSARemove(0x%08X, %s) failed: %s", deviceData->clientHandle, fixedPath, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(fixedPath);

    return 0;
}
