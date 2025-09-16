#include "../logger.h"
#include "devoptab_fsa.h"

#include <cstdio>

#define COMP_MAX      50

#define ispathsep(ch) ((ch) == '/' || (ch) == '\\')
#define iseos(ch)     ((ch) == '\0')
#define ispathend(ch) (ispathsep(ch) || iseos(ch))

// https://gist.github.com/starwing/2761647
static char *
__fsa_normpath(char *out, const char *in) {
    char *pos[COMP_MAX], **top = pos, *head = out;
    int isabs = ispathsep(*in);

    if (isabs) *out++ = '/';
    *top++ = out;

    while (!iseos(*in)) {
        while (ispathsep(*in)) {
            ++in;
        }

        if (iseos(*in)) {
            break;
        }

        if (memcmp(in, ".", 1) == 0 && ispathend(in[1])) {
            ++in;
            continue;
        }

        if (memcmp(in, "..", 2) == 0 && ispathend(in[2])) {
            in += 2;
            if (top != pos + 1) {
                out = *--top;
            } else if (isabs) {
                out = top[-1];
            } else {
                strcpy(out, "../");
                out += 3;
            }
            continue;
        }

        if (top - pos >= COMP_MAX) {
            return NULL; // path to complicate
        }

        *top++ = out;
        while (!ispathend(*in)) {
            *out++ = *in++;
        }
        if (ispathsep(*in)) {
            *out++ = '/';
        }
    }

    *out = '\0';
    if (*head == '\0') {
        strcpy(head, "./");
    }
    return head;
}

uint32_t
__fsa_hashstring(const char *str) {
    uint32_t h;
    uint8_t *p;

    h = 0;
    for (p = (uint8_t *) str; *p != '\0'; p++) {
        h = 37 * h + *p;
    }
    return h;
}

char *
__fsa_fixpath(struct _reent *r,
              const char *path) {
    char *p;

    if (!path) {
        r->_errno = EINVAL;
        return NULL;
    }

    p = strchr(path, ':') + 1;
    if (!strchr(path, ':')) {
        p = (char *) path;
    }

    // wii u softlocks on empty strings so give expected error back
    if (strlen(p) == 0) {
        r->_errno = ENOENT;
        return NULL;
    }

    int maxPathLength = PATH_MAX;
    auto fixedPath    = static_cast<char *>(memalign(0x40, maxPathLength));
    if (!fixedPath) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: failed to allocate memory for fixedPath");
        r->_errno = ENOMEM;
        return NULL;
    }

    // Convert to an absolute path
    if (p[0] != '\0' && p[0] != '\\' && p[0] != '/') {
        __fsa_device_t *deviceData = (__fsa_device_t *) r->deviceData;
        if (snprintf(fixedPath, maxPathLength, "%s/%s", deviceData->cwd, p) >= maxPathLength) {
            DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: fixedPath snprintf result (relative) was truncated");
        }
    } else {
        const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);
        if (snprintf(fixedPath, maxPathLength, "%s%s", deviceData->mountPath, p) >= maxPathLength) {
            DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: fixedPath snprintf result (absolute) was truncated");
        }
    }

    // Normalize path (resolve any ".", "..", or "//")
    char *normalizedPath = strdup(fixedPath);
    if (!normalizedPath) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: failed to allocate memory for normalizedPath");
        free(fixedPath);
        r->_errno = ENOMEM;
        return NULL;
    }

    char *resPath = __fsa_normpath(normalizedPath, fixedPath);
    if (!resPath) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: failed to normalize path");
        free(normalizedPath);
        free(fixedPath);
        r->_errno = EIO;
        return NULL;
    }

    if (snprintf(fixedPath, maxPathLength, "%s", resPath) >= maxPathLength) {
        DEBUG_FUNCTION_LINE_ERR("__fsa_fixpath: fixedPath snprintf result (relative) was truncated");
    }

    free(normalizedPath);

    size_t pathLength = strlen(fixedPath);
    if (pathLength > FS_MAX_PATH) {
        free(fixedPath);
        r->_errno = ENAMETOOLONG;
        return NULL;
    }

    return fixedPath;
}

mode_t __fsa_translate_stat_mode(FSStat *fsStat) {
    mode_t retMode = 0;

    if ((fsStat->flags & FS_STAT_LINK) == FS_STAT_LINK) {
        retMode |= S_IFLNK;
    } else if ((fsStat->flags & FS_STAT_DIRECTORY) == FS_STAT_DIRECTORY) {
        retMode |= S_IFDIR;
    } else if ((fsStat->flags & FS_STAT_FILE) == FS_STAT_FILE) {
        retMode |= S_IFREG;
    } else if (fsStat->size == 0) {
        // Mounted paths like /vol/external01 have no flags set.
        // If no flag is set and the size is 0, it's a (root) dir
        retMode |= S_IFDIR;
    } else if (fsStat->size > 0) {
        // Some regular Wii U files have no type info but will have a size
        retMode |= S_IFREG;
    }

    // Convert normal CafeOS hexadecimal permission bits into Unix octal permission bits
    mode_t permissionMode = (((fsStat->mode >> 2) & S_IRWXU) | ((fsStat->mode >> 1) & S_IRWXG) | (fsStat->mode & S_IRWXO));

    return retMode | permissionMode;
}

void __fsa_translate_stat(FSAClientHandle clientHandle, FSStat *fsStat, ino_t ino, struct stat *posStat) {
    memset(posStat, 0, sizeof(struct stat));
    posStat->st_dev     = (dev_t) clientHandle;
    posStat->st_ino     = ino;
    posStat->st_mode    = __fsa_translate_stat_mode(fsStat);
    posStat->st_nlink   = 1;
    posStat->st_uid     = fsStat->owner;
    posStat->st_gid     = fsStat->group;
    posStat->st_rdev    = posStat->st_dev;
    posStat->st_size    = fsStat->size;
    posStat->st_atime   = __fsa_translate_time(fsStat->modified);
    posStat->st_ctime   = __fsa_translate_time(fsStat->created);
    posStat->st_mtime   = __fsa_translate_time(fsStat->modified);
    posStat->st_blksize = 512;
    posStat->st_blocks  = (posStat->st_size + posStat->st_blksize - 1) / posStat->st_size;
}

// The Wii U FSTime epoch is at 1980, so we must map it to 1970 for gettime
#define WIIU_FSTIME_EPOCH_YEAR         (1980)

#define EPOCH_YEAR                     (1970)
#define EPOCH_YEARS_SINCE_LEAP         2
#define EPOCH_YEARS_SINCE_CENTURY      70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

#define EPOCH_DIFF_YEARS(year)         (year - EPOCH_YEAR)
#define EPOCH_DIFF_DAYS(year)                                         \
    ((EPOCH_DIFF_YEARS(year) * 365) +                                 \
     (EPOCH_DIFF_YEARS(year) - 1 + EPOCH_YEARS_SINCE_LEAP) / 4 -      \
     (EPOCH_DIFF_YEARS(year) - 1 + EPOCH_YEARS_SINCE_CENTURY) / 100 + \
     (EPOCH_DIFF_YEARS(year) - 1 + EPOCH_YEARS_SINCE_LEAP_CENTURY) / 400)
#define EPOCH_DIFF_SECS(year) (60ull * 60ull * 24ull * (uint64_t) EPOCH_DIFF_DAYS(year))

time_t __fsa_translate_time(FSTime timeValue) {
    return (timeValue / 1000000) + EPOCH_DIFF_SECS(WIIU_FSTIME_EPOCH_YEAR);
}

int __fsa_translate_error(FSError error) {
    switch (error) {
        case FS_ERROR_END_OF_DIR:
        case FS_ERROR_END_OF_FILE:
            return ENOENT;
        case FS_ERROR_ALREADY_EXISTS:
            return EEXIST;
        case FS_ERROR_MEDIA_ERROR:
            return EIO;
        case FS_ERROR_NOT_FOUND:
            return ENOENT;
        case FS_ERROR_PERMISSION_ERROR:
            return EPERM;
        case FS_ERROR_STORAGE_FULL:
            return ENOSPC;
        case FS_ERROR_BUSY:
            return EBUSY;
        case FS_ERROR_CANCELLED:
            return ECANCELED;
        case FS_ERROR_FILE_TOO_BIG:
            return EFBIG;
        case FS_ERROR_INVALID_PATH:
            return ENAMETOOLONG;
        case FS_ERROR_NOT_DIR:
            return ENOTDIR;
        case FS_ERROR_NOT_FILE:
            return EISDIR;
        case FS_ERROR_OUT_OF_RANGE:
            return EINVAL;
        case FS_ERROR_UNSUPPORTED_COMMAND:
            return ENOTSUP;
        case FS_ERROR_WRITE_PROTECTED:
            return EROFS;
        case FS_ERROR_NOT_INIT:
            return ENODEV;
        case FS_ERROR_MAX_MOUNT_POINTS:
        case FS_ERROR_MAX_VOLUMES:
        case FS_ERROR_MAX_CLIENTS:
        case FS_ERROR_MAX_FILES:
        case FS_ERROR_MAX_DIRS:
            return EMFILE;
        case FS_ERROR_ALREADY_OPEN:
            return EBUSY;
        case FS_ERROR_NOT_EMPTY:
            return ENOTEMPTY;
        case FS_ERROR_ACCESS_ERROR:
            return EACCES;
        case FS_ERROR_DATA_CORRUPTED:
            return EILSEQ;
        case FS_ERROR_JOURNAL_FULL:
            return EBUSY;
        case FS_ERROR_UNAVAILABLE_COMMAND:
            return EBUSY;
        case FS_ERROR_INVALID_PARAM:
            return EBUSY;
        case FS_ERROR_INVALID_BUFFER:
        case FS_ERROR_INVALID_ALIGNMENT:
        case FS_ERROR_INVALID_CLIENTHANDLE:
        case FS_ERROR_INVALID_FILEHANDLE:
        case FS_ERROR_INVALID_DIRHANDLE:
            return EINVAL;
        case FS_ERROR_OUT_OF_RESOURCES:
            return ENOMEM;
        case FS_ERROR_MEDIA_NOT_READY:
            return EIO;
        case FS_ERROR_INVALID_MEDIA:
            return EIO;
        default:
            return EIO;
    }
}