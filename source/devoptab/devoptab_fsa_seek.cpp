#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

off_t __fsa_seek(struct _reent *r,
                 void *fd,
                 off_t pos,
                 int whence) {
    FSError status;
    FSAStat fsStat;
    uint64_t offset;
    __fsa_file_t *file;

    if (!fd) {
        r->_errno = EINVAL;
        return -1;
    }

    file = (__fsa_file_t *) fd;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);
    status = FSAGetStatFile(deviceData->clientHandle, file->fd, &fsStat);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAGetStatFile(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, &fsStat, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    // Find the offset to see from
    switch (whence) {
        // Set absolute position; start offset is 0
        case SEEK_SET:
            offset = 0;
            break;

        // Set position relative to the current position
        case SEEK_CUR:
            offset = file->offset;
            break;

        // Set position relative to the end of the file
        case SEEK_END:
            offset = fsStat.size;
            break;

        // An invalid option was provided
        default:
            r->_errno = EINVAL;
            return -1;
    }

    // TODO: A better check that prevents overflow.
    if (pos < 0 && (off_t) offset < -pos) {
        // Don't allow seek to before the beginning of the file
        r->_errno = EINVAL;
        return -1;
    }

    uint32_t old_pos = file->offset;
    file->offset     = offset + pos;

    status = FSASetPosFile(deviceData->clientHandle, file->fd, file->offset);
    if (status < 0) {
        file->offset = old_pos;
        DEBUG_FUNCTION_LINE_ERR("FSASetPosFile(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, file->offset, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    return file->offset;
}
