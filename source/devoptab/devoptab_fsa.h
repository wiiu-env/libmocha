#pragma once
#include "MutexWrapper.h"
#include <cerrno>
#include <climits>
#include <coreinit/filesystem_fsa.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <malloc.h>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/param.h>
#include <unistd.h>

typedef struct FSADeviceData {
    devoptab_t device;
    bool setup;
    bool mounted;
    bool isSDCard;
    uint32_t id{};
    char name[32];
    char mountPath[0x80];
    char cwd[FS_MAX_PATH + 1];
    FSAClientHandle clientHandle;
    uint64_t deviceSizeInSectors;
    uint32_t deviceSectorSize;
} __fsa_device_t;

/**
 * Open file struct
 */
typedef struct {
    //! FSA file handle
    FSAFileHandle fd;

    //! Flags used in open(2)
    int flags;

    //! Current file offset
    uint32_t offset;

    //! Current file path
    char fullPath[FS_MAX_PATH + 1];

    //! Guard file access
    MutexWrapper mutex;

    //! Current file size (only valid if O_APPEND is set)
    uint32_t appendOffset;
} __fsa_file_t;

/**
 * Open directory struct
 */
typedef struct {
    //! Should be set to FSA_DIRITER_MAGIC
    uint32_t magic;

    //! FS directory handle
    FSADirectoryHandle fd;

    //! Temporary storage for reading entries
    FSADirectoryEntry entry_data;

    //! Current directory path
    char fullPath[FS_MAX_PATH + 1];

    //! Guard dir access
    MutexWrapper mutex;
} __fsa_dir_t;

#define FSA_DIRITER_MAGIC 0x77696975

#ifdef __cplusplus
extern "C" {
#endif

int __fsa_open(struct _reent *r, void *fileStruct, const char *path,
               int flags, int mode);
int __fsa_close(struct _reent *r, void *fd);
ssize_t __fsa_write(struct _reent *r, void *fd, const char *ptr,
                    size_t len);
ssize_t __fsa_read(struct _reent *r, void *fd, char *ptr, size_t len);
off_t __fsa_seek(struct _reent *r, void *fd, off_t pos, int dir);
int __fsa_fstat(struct _reent *r, void *fd, struct stat *st);
int __fsa_stat(struct _reent *r, const char *file, struct stat *st);
int __fsa_link(struct _reent *r, const char *existing,
               const char *newLink);
int __fsa_unlink(struct _reent *r, const char *name);
int __fsa_chdir(struct _reent *r, const char *name);
int __fsa_rename(struct _reent *r, const char *oldName,
                 const char *newName);
int __fsa_mkdir(struct _reent *r, const char *path, int mode);
DIR_ITER *__fsa_diropen(struct _reent *r, DIR_ITER *dirState,
                        const char *path);
int __fsa_dirreset(struct _reent *r, DIR_ITER *dirState);
int __fsa_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename,
                  struct stat *filestat);
int __fsa_dirclose(struct _reent *r, DIR_ITER *dirState);
int __fsa_statvfs(struct _reent *r, const char *path,
                  struct statvfs *buf);
int __fsa_ftruncate(struct _reent *r, void *fd, off_t len);
int __fsa_fsync(struct _reent *r, void *fd);
int __fsa_chmod(struct _reent *r, const char *path, mode_t mode);
int __fsa_fchmod(struct _reent *r, void *fd, mode_t mode);
int __fsa_rmdir(struct _reent *r, const char *name);
int __fsa_utimes(struct _reent *r, const char *filename, const struct timeval times[2]);

// devoptab_fsa_utils.c
char *__fsa_fixpath(struct _reent *r, const char *path);
int __fsa_translate_error(FSError error);
mode_t __fsa_translate_stat_mode(FSStat *fsStat);
void __fsa_translate_stat(FSAClientHandle handle, FSStat *fsStat, ino_t ino, struct stat *posStat);
uint32_t __fsa_hashstring(const char *str);

static inline FSMode
__fsa_translate_permission_mode(mode_t mode) {
    // Convert normal Unix octal permission bits into CafeOS hexadecimal permission bits
    return (FSMode) (((mode & S_IRWXU) << 2) | ((mode & S_IRWXG) << 1) | (mode & S_IRWXO));
}

time_t __fsa_translate_time(FSTime timeValue);

#ifdef __cplusplus
}
#endif