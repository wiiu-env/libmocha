#pragma once

#include <coreinit/filesystem.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FSError FSAEx_Mount(FSClient *client, const char *source, const char *target, uint32_t flags, void *arg_buf, uint32_t arg_len);

FSError FSAEx_MountEx(int clientHandle, const char *source, const char *target, uint32_t flags, void *arg_buf, uint32_t arg_len);

FSError FSAEx_Unmount(FSClient *client, const char *mountedTarget);

FSError FSAEx_UnmountEx(int clientHandle, const char *mountedTarget);

FSError FSAEx_RawOpen(FSClient *client, char *device_path, int32_t *outHandle);

FSError FSAEx_RawOpenEx(int clientHandle, char *device_path, int32_t *outHandle);

FSError FSAEx_RawClose(FSClient *client, int32_t device_handle);

FSError FSAEx_RawCloseEx(int clientHandle, int32_t device_handle);

FSError FSAEx_RawRead(FSClient *client, void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

FSError FSAEx_RawReadEx(int clientHandle, void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

FSError FSAEx_RawWrite(FSClient *client, const void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

FSError FSAEx_RawWriteEx(int clientHandle, const void *data, uint32_t size_bytes, uint32_t cnt, uint64_t blocks_offset, int device_handle);

#ifdef __cplusplus
} // extern "C"
#endif