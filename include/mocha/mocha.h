#pragma once

#include <coreinit/filesystem.h>
#include <mocha/commands.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MochaUtilsStatus {
    MOCHA_RESULT_SUCCESS                 = 0,
    MOCHA_RESULT_INVALID_ARGUMENT        = -0x01,
    MOCHA_RESULT_MAX_CLIENT              = -0x02,
    MOCHA_RESULT_UNSUPPORTED_API_VERSION = -0x10,
    MOCHA_RESULT_UNSUPPORTED_COMMAND     = -0x11,
    MOCHA_RESULT_LIB_UNINITIALIZED       = -0x20,
    MOCHA_RESULT_UNKNOWN_ERROR           = -0x100,
} MochaUtilsStatus;

/**
 * Initializes the mocha lib. Needs to be called before any other functions can be used
 * @return MOCHA_RESULT_SUCCESS: Library has been successfully initialized <br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Failed to initialize the library caused by an outdated mocha version.
 */
MochaUtilsStatus Mocha_InitLibrary();

MochaUtilsStatus Mocha_DeInitLibrary();

/**
 * Retrieves the API Version of the running mocha.
 *
 * @param outVersion pointer to the variable where the version will be stored.
 *
 * @return MOCHA_RESULT_SUCCESS: The API version has been store in the version ptr<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: invalid version pointer<br>
 *         MOCHA_RESULT_UNSUPPORTED_API_VERSION: Failed to get the API version caused by an outdated mocha version.
 */
MochaUtilsStatus Mocha_CheckAPIVersion(uint32_t *outVersion);

/***
 * Returns the path of the currently loaded environment
 * @param environmentPathBuffer: buffer where the result will be stored
 * @param bufferLen: length of the buffer. Required to be >= 0x100
* @return MOCHA_RESULT_SUCCESS: The environment path has been stored in environmentPathBuffer<br>
*         MOCHA_RESULT_INVALID_ARGUMENT: invalid environmentPathBuffer pointer or bufferLen \< 0x100<br>
*         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
*         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
*         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_GetEnvironmentPath(char *environmentPathBuffer, uint32_t bufferLen);

/**
 * Enables logging via USB (FTDI FT232 chipset only) via OSReport and friends. <br>
 * @param notSkipExistingLogs If set to true, existing logs of this session won't be skipped.
 * @return MOCHA_RESULT_SUCCESS: Logging via USB starts or has already been started<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_StartUSBLogging(bool notSkipExistingLogs);

/**
 * Gives a FSClient full permissions. <br>
 * Requires Mocha API Version: 1
 * @param client The FSClient that should have full permission
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully.
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_UnlockFSClient(FSClient *client);

/**
 * Gives a /dev/fsa handle full permissions. <br>
 * Requires Mocha API Version: 1
 * @param client The /dev/fsa handle that should have full permission
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully.
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_UnlockFSClientEx(int clientHandle);

MochaUtilsStatus Mocha_LoadRPXOnNextLaunch(MochaRPXLoadInfo *loadInfo);

typedef struct WUDDiscKey {
    uint8_t key[0x10];
} WUDDiscKey;

MochaUtilsStatus Mocha_ODMGetDiscKey(WUDDiscKey *discKey);

MochaUtilsStatus Mocha_SEEPROMRead(uint8_t *out_buffer, uint32_t offset, uint32_t size);

#ifdef __cplusplus
} // extern "C"
#endif