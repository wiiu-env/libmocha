#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>

int __fsa_dirnext(struct _reent *r,
                  DIR_ITER *dirState,
                  char *filename,
                  struct stat *filestat) {
    FSError status;
    __fsa_dir_t *dir;

    if (!dirState || !filename || !filestat) {
        r->_errno = EINVAL;
        return -1;
    }

    dir = (__fsa_dir_t *) (dirState->dirStruct);
    memset(&dir->entry_data, 0, sizeof(dir->entry_data));

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAReadDir(deviceData->clientHandle, dir->fd, &dir->entry_data);
    if (status < 0) {
        if (status != FS_ERROR_END_OF_DIR) {
            DEBUG_FUNCTION_LINE_ERR("FSAReadDir(0x%08X, 0x%08X, 0x%08X) failed: %s", deviceData->clientHandle, dir->fd, &dir->entry_data, FSAGetStatusStr(status));
        }
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    // Fill in the stat info
    memset(filestat, 0, sizeof(struct stat));
    filestat->st_dev     = (dev_t) deviceData->clientHandle;
    filestat->st_ino     = dir->entry_data.info.entryId;
    filestat->st_mode    = __fsa_translate_stat_mode(&dir->entry_data.info);
    filestat->st_nlink   = 1;
    filestat->st_uid     = dir->entry_data.info.owner;
    filestat->st_gid     = dir->entry_data.info.group;
    filestat->st_rdev    = filestat->st_dev;
    filestat->st_size    = dir->entry_data.info.size;
    filestat->st_blksize = deviceData->deviceSectorSize;
    filestat->st_blocks  = (filestat->st_size + filestat->st_blksize - 1) / filestat->st_blksize;
    filestat->st_atime   = __fsa_translate_time(dir->entry_data.info.modified);
    filestat->st_ctime   = __fsa_translate_time(dir->entry_data.info.created);
    filestat->st_mtime   = __fsa_translate_time(dir->entry_data.info.modified);

    memset(filename, 0, NAME_MAX);
    strcpy(filename, dir->entry_data.name);
    return 0;
}
