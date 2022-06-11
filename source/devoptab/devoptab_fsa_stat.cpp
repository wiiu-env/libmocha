#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_stat(struct _reent *r,
               const char *path,
               struct stat *st) {
    FSError status;
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

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAGetStat(deviceData->clientHandle, fixedPath, &fsStat);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAGetStat(0x%08X, %s, 0x%08X) failed: %s", deviceData->clientHandle, fixedPath, &fsStat, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(fixedPath);

    memset(st, 0, sizeof(struct stat));
    st->st_dev     = (dev_t) deviceData->clientHandle;
    st->st_ino     = fsStat.entryId;
    st->st_mode    = __fsa_translate_stat_mode(fsStat);
    st->st_nlink   = 1;
    st->st_uid     = fsStat.owner;
    st->st_gid     = fsStat.group;
    st->st_rdev    = st->st_dev;
    st->st_size    = fsStat.size;
    st->st_blksize = deviceData->deviceSectorSize;
    st->st_blocks  = (st->st_size + st->st_blksize - 1) / st->st_blksize;
    st->st_atime   = __fsa_translate_time(fsStat.modified);
    st->st_ctime   = __fsa_translate_time(fsStat.created);
    st->st_mtime   = __fsa_translate_time(fsStat.modified);
    return 0;
}