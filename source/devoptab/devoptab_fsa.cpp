#include "devoptab_fsa.h"
#include "../logger.h"
#include "mocha/mocha.h"

#include <algorithm>
#include <complex>
#include <coreinit/cache.h>
#include <coreinit/filesystem_fsa.h>
#include <mutex>
#include <string>

static const devoptab_t fsa_default_devoptab = {
        .structSize   = sizeof(__fsa_file_t),
        .open_r       = __fsa_open,
        .close_r      = __fsa_close,
        .write_r      = __fsa_write,
        .read_r       = __fsa_read,
        .seek_r       = __fsa_seek,
        .fstat_r      = __fsa_fstat,
        .stat_r       = __fsa_stat,
        .link_r       = __fsa_link,
        .unlink_r     = __fsa_unlink,
        .chdir_r      = __fsa_chdir,
        .rename_r     = __fsa_rename,
        .mkdir_r      = __fsa_mkdir,
        .dirStateSize = sizeof(__fsa_dir_t),
        .diropen_r    = __fsa_diropen,
        .dirreset_r   = __fsa_dirreset,
        .dirnext_r    = __fsa_dirnext,
        .dirclose_r   = __fsa_dirclose,
        .statvfs_r    = __fsa_statvfs,
        .ftruncate_r  = __fsa_ftruncate,
        .fsync_r      = __fsa_fsync,
        .chmod_r      = __fsa_chmod,
        .fchmod_r     = __fsa_fchmod,
        .rmdir_r      = __fsa_rmdir,
        .lstat_r      = __fsa_stat,
        .utimes_r     = __fsa_utimes,
};

static bool fsa_initialised = false;
static FSADeviceData fsa_mounts[0x10];

static void fsaResetMount(FSADeviceData *mount, const uint32_t id) {
    *mount = {};
    memcpy(&mount->device, &fsa_default_devoptab, sizeof(fsa_default_devoptab));
    mount->device.name         = mount->name;
    mount->device.deviceData   = mount;
    mount->id                  = id;
    mount->setup               = false;
    mount->mounted             = false;
    mount->isSDCard            = false;
    mount->clientHandle        = -1;
    mount->deviceSizeInSectors = 0;
    mount->deviceSectorSize    = 0;
    mount->cwd[0]              = '/';
    mount->cwd[1]              = '\0';
    memset(mount->mountPath, 0, sizeof(mount->mountPath));
    memset(mount->name, 0, sizeof(mount->name));
    DCFlushRange(mount, sizeof(*mount));
}

void fsaInit() {
    if (!fsa_initialised) {
        constexpr uint32_t total = std::size(fsa_mounts);
        for (uint32_t i = 0; i < total; i++) {
            fsaResetMount(&fsa_mounts[i], i);
        }
        fsa_initialised = true;
    }
}

std::mutex fsaMutex;

FSADeviceData *fsa_alloc() {
    fsaInit();

    for (auto &fsa_mount : fsa_mounts) {
        if (FSADeviceData *mount = &fsa_mount; !mount->setup) {
            return mount;
        }
    }

    return nullptr;
}

static void fsa_free(FSADeviceData *mount) {
    FSError res;
    if (mount->mounted) {
        if ((res = FSAUnmount(mount->clientHandle, mount->mountPath, FSA_UNMOUNT_FLAG_FORCE)) < 0) {
            DEBUG_FUNCTION_LINE_WARN("FSAUnmount %s for %s failed: %s", mount->mountPath, mount->name, FSAGetStatusStr(res));
        }
    }
    res = FSADelClient(mount->clientHandle);
    if (res < 0) {
        DEBUG_FUNCTION_LINE_WARN("FSADelClient for %s failed: %s", mount->name, FSAGetStatusStr(res));
    }
    fsaResetMount(mount, mount->id);
}

MochaUtilsStatus Mocha_UnmountFS(const char *virt_name) {
    if (!virt_name) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    std::lock_guard lock(fsaMutex);

    fsaInit();

    for (auto &fsa_mount : fsa_mounts) {
        FSADeviceData *mount = &fsa_mount;
        if (!mount->setup) {
            continue;
        }
        if (strcmp(mount->name, virt_name) == 0) {
            const std::string removeName = std::string(mount->name).append(":");
            RemoveDevice(removeName.c_str());
            fsa_free(mount);
            return MOCHA_RESULT_SUCCESS;
        }
    }

    DEBUG_FUNCTION_LINE_WARN("Failed to find fsa mount data for %s", virt_name);
    return MOCHA_RESULT_NOT_FOUND;
}
extern int mochaInitDone;

MochaUtilsStatus Mocha_MountFS(const char *virt_name, const char *dev_path, const char *mount_path) {
    return Mocha_MountFSEx(virt_name, dev_path, mount_path, FSA_MOUNT_FLAG_GLOBAL_MOUNT, nullptr, 0);
}

MochaUtilsStatus Mocha_MountFSEx(const char *virt_name, const char *dev_path, const char *mount_path, FSAMountFlags mountFlags, void *mountArgBuf, int mountArgBufLen) {
    if (virt_name == nullptr || mount_path == nullptr) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    if (!mochaInitDone) {
        if (Mocha_InitLibrary() != MOCHA_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Mocha_InitLibrary failed");
            return MOCHA_RESULT_UNSUPPORTED_COMMAND;
        }
    }

    FSAInit();
    std::lock_guard lock(fsaMutex);

    FSADeviceData *mount = fsa_alloc();
    if (mount == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("fsa_alloc() failed");
        OSMemoryBarrier();
        return MOCHA_RESULT_MAX_CLIENT;
    }

    // make sure the paths are normalized
    std::string normalizedMountPath(mount_path);
    std::ranges::replace(normalizedMountPath, '\\', '/');
    while (!normalizedMountPath.empty() && (normalizedMountPath.ends_with("/"))) {
        normalizedMountPath.pop_back();
    }
    std::string normalizedDevPath(dev_path ? dev_path : "");
    std::ranges::replace(normalizedDevPath, '\\', '/');
    while (!normalizedDevPath.empty() && (normalizedDevPath.ends_with("/"))) {
        normalizedDevPath.pop_back();
    }

    // Things like statvfs behave different on sd cards!
    if (normalizedDevPath.starts_with("/dev/sdcard01") || normalizedMountPath.starts_with("/vol/external01")) {
        mount->isSDCard = true;
    }

    mount->clientHandle = FSAAddClient(nullptr);
    if (mount->clientHandle < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAAddClient() failed: %s", FSAGetStatusStr(static_cast<FSError>(mount->clientHandle)));
        fsa_free(mount);
        return MOCHA_RESULT_MAX_CLIENT;
    }

    MochaUtilsStatus status;
    if ((status = Mocha_UnlockFSClientEx(mount->clientHandle)) != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Mocha_UnlockFSClientEx failed: %s", Mocha_GetStatusStr(status));
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }

    mount->mounted = false;

    strncpy(mount->name, virt_name, sizeof(mount->name) - 1);
    strncpy(mount->mountPath, normalizedMountPath.c_str(), sizeof(mount->mountPath) - 1);
    FSError res;
    if (!normalizedDevPath.empty()) {
        res = FSAMount(mount->clientHandle, normalizedDevPath.c_str(), normalizedMountPath.c_str(), mountFlags, mountArgBuf, mountArgBufLen);
        if (res < 0) {
            DEBUG_FUNCTION_LINE_ERR("FSAMount(0x%08X, %s, %s, %08X, %p, %08X) failed: %s", mount->clientHandle, normalizedDevPath.c_str(), normalizedMountPath.c_str(), mountFlags, mountArgBuf, mountArgBufLen, FSAGetStatusStr(res));
            fsa_free(mount);
            if (res == FS_ERROR_ALREADY_EXISTS) {
                return MOCHA_RESULT_ALREADY_EXISTS;
            }
            return MOCHA_RESULT_UNKNOWN_ERROR;
        }
        mount->mounted = true;
    } else {
        mount->mounted = false;
    }

    if ((res = FSAChangeDir(mount->clientHandle, mount->mountPath)) < 0) {
        DEBUG_FUNCTION_LINE_WARN("FSAChangeDir(0x%08X, %s) failed: %s", mount->clientHandle, mount->mountPath, FSAGetStatusStr(res));
    } else {
        strncpy(mount->cwd, normalizedMountPath.c_str(), sizeof(mount->mountPath) - 1);
    }

    FSADeviceInfo deviceInfo;
    if ((res = FSAGetDeviceInfo(mount->clientHandle, normalizedMountPath.c_str(), &deviceInfo)) >= 0) {
        mount->deviceSizeInSectors = deviceInfo.deviceSizeInSectors;
        mount->deviceSectorSize    = deviceInfo.deviceSectorSize;
    } else {
        mount->deviceSizeInSectors = 0xFFFFFFFF;
        mount->deviceSectorSize    = 512;
        DEBUG_FUNCTION_LINE_WARN("Failed to get DeviceInfo for %s: %s", normalizedMountPath.c_str(), FSAGetStatusStr(res));
    }

    if (AddDevice(&mount->device) < 0) {
        DEBUG_FUNCTION_LINE_ERR("AddDevice failed for %s.", virt_name);
        fsa_free(mount);
        return MOCHA_RESULT_ADD_DEVOPTAB_FAILED;
    }

    mount->setup = true;

    return MOCHA_RESULT_SUCCESS;
}
