#pragma once

#include <coreinit/filesystem.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FSAMountFlags {
    FSA_MOUNT_FLAG_LOCAL_MOUNT  = 0,
    FSA_MOUNT_FLAG_BIND_MOUNT   = 1,
    FSA_MOUNT_FLAG_GLOBAL_MOUNT = 2,
} FSAMountFlags;

typedef enum FSAUnmountFlags {
    FSA_UNMOUNT_FLAG_BIND_MOUNT = 0x80000000,
} FSAUnmountFlags;

/**
 * Mounts a source to a given path for a given FSClient (or globally if FSA_MOUNT_FLAG_GLOBAL_MOUNT is set)
 *
 * @param client valid FSClient pointer with unlocked permissions
 * @param source Mount source e.g. /dev/sdcard01
 * @param target Must not start with /vol/storage_ if FSA_MOUNT_FLAG_GLOBAL_MOUNT is **not** set. Requires to start with "/vol/storage_" if FSA_MOUNT_FLAG_GLOBAL_MOUNT or FSA_UNMOUNT_FLAG_BIND_MOUNT is set
 * @param flags Determines the mount type.
 * @param arg_buf unknown
 * @param arg_len unknown
 * @return
 */
FSError FSAEx_Mount(FSClient *client, const char *source, const char *target, FSAMountFlags flags, void *arg_buf, uint32_t arg_len);

/**
 * Mounts a source to a given path for a given FSClient (or globally if FSA_MOUNT_FLAG_GLOBAL_MOUNT is set)
 *
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param source Mount source e.g. /dev/sdcard01
 * @param target Must not start with /vol/storage_ if FSA_MOUNT_FLAG_GLOBAL_MOUNT is **not** set. Requires to start with "/vol/storage_" if FSA_MOUNT_FLAG_GLOBAL_MOUNT is set
 * @param flags Determines the mount type.
 * @param arg_buf unknown
 * @param arg_len unknown
 * @return
 */
FSError FSAEx_MountEx(int clientHandle, const char *source, const char *target, FSAMountFlags flags, void *arg_buf, uint32_t arg_len);

/**
 * Unmounts a given path
 * @param client valid FSClient pointer with unlocked permissions
 * @param mountedTarget path where the mount is mounted to.
 * @param flags FSA_UNMOUNT_FLAG_BIND_MOUNT is expected for a BindMount
 * @return
 */
FSError FSAEx_Unmount(FSClient *client, const char *mountedTarget, FSAUnmountFlags flags);

/**
 * Unmounts a given path
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param mountedTarget path where the mount is mounted to.
 * @param flags FSA_UNMOUNT_FLAG_BIND_MOUNT is expected for a BindMount
 * @return
 */
FSError FSAEx_UnmountEx(int clientHandle, const char *mountedTarget, FSAUnmountFlags flags);

/**
 * Opens a device for raw read/write
 * @param client valid FSClient pointer with unlocked permissions
 * @param device_path path of the device. e.g. /dev/sdcard01
 * @param outHandle pointer where the handle of the raw device will be stored
 * @return
 */
FSError FSAEx_RawOpen(FSClient *client, char *device_path, int32_t *outHandle);

/**
 * Opens a device for raw read/write
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param device_path path of the device. e.g. /dev/sdcard01
 * @param outHandle pointer where the handle of the raw device will be stored
 * @return
 */
FSError FSAEx_RawOpenEx(int clientHandle, char *device_path, int32_t *outHandle);

/**
 * Closes a devices that was previously opened via FSAEx_RawOpen
 * @param client valid FSClient pointer with unlocked permissions
 * @param device_handle device handle
 * @return
 */
FSError FSAEx_RawClose(FSClient *client, int32_t device_handle);

/**
 * Closes a devices that was previously opened via FSAEx_RawOpen
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param device_handle device handle
 * @return
 */
FSError FSAEx_RawCloseEx(int clientHandle, int32_t device_handle);

/**
 * Read data from a device handle.
 *
 * @param client valid FSClient pointer with unlocked permissions
 * @param data buffer where the result will be stored. Requires 0x40 alignment for the buffer itself and buffer size.
 * @param size_bytes size of sector.
 * @param cnt number of sectors that should be read.
 * @param blocks_offset read offset in sectors.
 * @param device_handle valid device handle.
 * @return
 */
FSError FSAEx_RawRead(FSClient *client, void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

/**
 * Read data from a raw device handle.
 *
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param data buffer where the result will be stored. Requires 0x40 alignment for the buffer itself and buffer size.
 * @param size_bytes size of sector.
 * @param cnt number of sectors that should be read.
 * @param blocks_offset read offset in sectors.
 * @param device_handle valid device handle.
 * @return
 */
FSError FSAEx_RawReadEx(int clientHandle, void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);


/**
 * Write data to raw device handle
 *
 * @param client valid FSClient pointer with unlocked permissions
 * @param data buffer of data that should be written.. Requires 0x40 alignment for the buffer itself and buffer size.
 * @param size_bytes size of sector.
 * @param cnt number of sectors that should be written.
 * @param blocks_offset write offset in sectors.
 * @param device_handle valid device handle.
 * @return
 */
FSError FSAEx_RawWrite(FSClient *client, const void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

/**
 * Write data to raw device handle
 *
 * @param clientHandle valid /dev/fsa handle with unlocked permissions
 * @param data buffer of data that should be written.. Requires 0x40 alignment for the buffer itself and buffer size.
 * @param size_bytes size of sector.
 * @param cnt number of sectors that should be written.
 * @param blocks_offset write offset in sectors.
 * @param device_handle valid device handle.
 * @return
 */
FSError FSAEx_RawWriteEx(int clientHandle, const void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

#ifdef __cplusplus
} // extern "C"
#endif