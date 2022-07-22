#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

// Use this extended hidden flag value to provide FS_OPEN_FLAG_ENCRYPTED in underlying FSOpenFileEx() call
#define O_ENCRYPTED 0x4000000

int __fsa_open(struct _reent *r,
               void *fileStruct,
               const char *path,
               int flags,
               int mode) {
    FSAFileHandle fd;
    FSError status;
    const char *fsMode;
    __fsa_file_t *file;

    if (!fileStruct || !path) {
        r->_errno = EINVAL;
        return -1;
    }

    // Map flags to open modes
    if ((flags & O_ACCMODE) == O_RDONLY) {
        fsMode = "r";
        if (flags & O_APPEND) {
            r->_errno = EINVAL;
            return -1;
        }
    } else if ((flags & O_ACCMODE) == O_WRONLY) {
        if (flags & O_APPEND) {
            fsMode = "a";
        } else if (flags & O_TRUNC) {
            fsMode = "w";
        } else {
            r->_errno = EINVAL;
            return -1;
        }
    } else if ((flags & O_ACCMODE) == O_RDWR) {
        if (flags & O_APPEND) {
            fsMode = "a+";
        } else if (flags & O_TRUNC) {
            fsMode = "w+";
        } else {
            fsMode = "r+";
        }
    } else {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        r->_errno = ENOMEM;
        return -1;
    }

    // Open the file
    FSOpenFileFlags openFlags = (mode & O_ENCRYPTED) ? FS_OPEN_FLAG_ENCRYPTED : FS_OPEN_FLAG_NONE;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    FSMode translatedMode = __fsa_translate_permission_mode(mode);

    uint32_t preAllocSize = 0;

    status = FSAOpenFileEx(deviceData->clientHandle, fixedPath, fsMode, translatedMode, openFlags, preAllocSize, &fd);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAOpenFileEx(0x%08X, %s, %s, 0x%X, 0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, fixedPath, fsMode, translatedMode, openFlags, preAllocSize, &fd, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        free(fixedPath);
        return -1;
    }

    file        = (__fsa_file_t *) fileStruct;
    file->fd    = fd;
    file->flags = (flags & (O_ACCMODE | O_APPEND | O_SYNC));
    strncpy(file->path, fixedPath, FS_MAX_PATH);
    free(fixedPath);

    if (flags & O_APPEND) {
        status = FSAGetPosFile(deviceData->clientHandle, fd, &file->offset);
        if (status < 0) {
            DEBUG_FUNCTION_LINE_ERR("FSAGetPosFile(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, fd, &file->offset, FSAGetStatusStr(status));
            r->_errno = __fsa_translate_error(status);
            if (FSACloseFile(deviceData->clientHandle, fd) < 0) {
                DEBUG_FUNCTION_LINE_ERR("FSACloseFile(0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, fd, FSAGetStatusStr(status));
            }
            return -1;
        }
    }
    return 0;
}