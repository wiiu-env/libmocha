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
    MOCHA_RESULT_NOT_FOUND               = -0x06,
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

/**
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
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully. <br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: client was NULL <br>
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to unlock a given FSClient
 */
MochaUtilsStatus Mocha_UnlockFSClient(FSClient *client);

/**
 * Gives a /dev/fsa handle full permissions. <br>
 * Requires Mocha API Version: 1
 * @param client The /dev/fsa handle that should have full permission
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully. <br>
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to unlock the given client
 */
MochaUtilsStatus Mocha_UnlockFSClientEx(int clientHandle);

/**
 * Set the .rpx that will be loaded the next time the homebrew wrapper application is launched (e.g. Health & Safety or Daily Log).
 * <br>
 * Loading a .rpx from within a file (archive e.g. a WUHB) is supported. <br>
 * To achieve this, the fileoffset (offset inside file specified via path) and filesize (size of the .rpx) need to be set. <br>
 * If filesize is set to 0, the whole file (starting at fileoffset) will be loaded as .rpx <br>
 * <br>
 * The path is **relative** to the root of the given target device.<br>
 * The target LOAD_RPX_TARGET_EXTRA_REVERT_PREPARE will revert a prepare call. <br>
 * <br>
 * To launch the prepared RPX call Mocha_LaunchHomebrewWrapper if this call was successful.
 *
 * @param loadInfo Information about the .rpx replacement.
 * @return MOCHA_RESULT_SUCCESS: Loading the next RPX will be redirected. <br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given loadInfo was NULL <br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to setup a redirect of RPX.
 */
MochaUtilsStatus Mocha_PrepareRPXLaunch(MochaRPXLoadInfo *loadInfo);

/**
 * Launches the wrapper app for launching .rpx  <br>
 * To launch a RPX call `Mocha_PrepareRPXLaunch` before this function. <br>
 * <br>
 * see: `Mocha_LaunchRPX` to prepare and launch a RPX in one command.
 *
 * @return MOCHA_RESULT_SUCCESS: App is launching<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_NOT_FOUND: Not application that can be used as homebrew wrapper found.
 */
MochaUtilsStatus Mocha_LaunchHomebrewWrapper();

/**
 * Launches a given RPX by launching a wrapper application and replacing the RPX on the fly. <br>
 * See Mocha_PrepareRPXLaunch for more information. <br>
 *
 * Note: Combines Mocha_PrepareRPXLaunch and Mocha_LaunchHomebrewWrapper.
 * @param loadInfo
 * @return MOCHA_RESULT_SUCCESS: Requested RPX will be launched<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given loadInfo was NULL <br>
 *         MOCHA_RESULT_NOT_FOUND: Not application that can be used as homebrew wrapper found.
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to setup a redirect of RPX.
 */
MochaUtilsStatus Mocha_LaunchRPX(MochaRPXLoadInfo *loadInfo);

typedef struct WUDDiscKey {
    uint8_t key[0x10];
} WUDDiscKey;

/**
 * Reads the disc key (used to decrypt the SI partition) of the inserted disc.
 *
 * @param discKey target buffer where the result will be stored.
 * @return MOCHA_RESULT_SUCCESS: The disc key of the inserted disc has been read into the given buffer.<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given discKey buffer was NULL <br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_NOT_FOUND: Failed to obtain disc key, no disc inserted?.
 *         MOCHA_RESULT_UNKNOWN_ERROR: Unknown error.
 */
MochaUtilsStatus Mocha_ODMGetDiscKey(WUDDiscKey *discKey);

/**
 * Reads *size* bytes from *offset* from the SEEPROM of the console. Total size of SEEPROM is 0x200
 * @param out_buffer buffer where the result will be stored
 * @param offset offset in bytes. Must be an even number.
 * @param size size in bytes
 * @return MOCHA_RESULT_SUCCESS: The SEEPROM has been read into the given buffer.<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given out_buffer was NULL or the offset was < 0 or an odd value<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to read the seeprom.
 */
MochaUtilsStatus Mocha_SEEPROMRead(uint8_t *out_buffer, uint32_t offset, uint32_t size);

#ifdef __cplusplus
} // extern "C"
#endif