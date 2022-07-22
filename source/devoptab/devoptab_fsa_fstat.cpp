#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_fstat(struct _reent *r,
                void *fd,
                struct stat *st) {
    FSError status;
    FSAStat fsStat;
    __fsa_file_t *file;

    if (!fd || !st) {
        r->_errno = EINVAL;
        return -1;
    }

    file = (__fsa_file_t *) fd;

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAGetStatFile(deviceData->clientHandle, file->fd, &fsStat);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSARewindDir(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, file->fd, &fsStat, FSAGetStatusStr(status));
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_size  = fsStat.size;
    st->st_uid   = fsStat.owner;
    st->st_gid   = fsStat.group;
    st->st_nlink = 1;
    st->st_mode  = __fsa_translate_stat_mode(&fsStat);
    st->st_atime = __fsa_translate_time(fsStat.modified);
    st->st_ctime = __fsa_translate_time(fsStat.created);
    st->st_mtime = __fsa_translate_time(fsStat.modified);

    return 0;
}
