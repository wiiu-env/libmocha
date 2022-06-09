#include "utils.h"
#include "mocha/commands.h"
#include "mocha/mocha.h"
#include <coreinit/ios.h>
#include <cstring>
#include <stdint.h>

int mochaInitDone        = 0;
uint32_t mochaApiVersion = 0;

MochaUtilsStatus Mocha_InitLibrary() {
    mochaInitDone    = 1;
    mochaApiVersion  = 0;
    uint32_t version = 0;
    if (Mocha_CheckAPIVersion(&version) != MOCHA_RESULT_SUCCESS) {
        return MOCHA_RESULT_SUCCESS;
    }

    mochaApiVersion = version;

    return MOCHA_RESULT_SUCCESS;
}

MochaUtilsStatus Mocha_DeInitLibrary() {
    mochaInitDone   = 0;
    mochaApiVersion = 0;

    return MOCHA_RESULT_SUCCESS;
}

MochaUtilsStatus Mocha_CheckAPIVersion(uint32_t *version) {
    if (!version) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    int mcpFd            = IOS_Open("/dev/mcp", IOS_OPEN_READ);
    if (mcpFd >= 0) {
        ALIGN_0x40 uint32_t io_buffer[0x100 / 4];
        io_buffer[0] = IPC_CUSTOM_GET_MOCHA_API_VERSION;

        if (IOS_Ioctl(mcpFd, 100, io_buffer, 4, io_buffer, 4) == IOS_ERROR_OK) {
            *version = io_buffer[0];
            res      = MOCHA_RESULT_SUCCESS;
        } else {
            res = MOCHA_RESULT_UNSUPPORTED_API_VERSION;
        }

        IOS_Close(mcpFd);
    } else {
        return res;
    }

    return res;
}

MochaUtilsStatus Mocha_GetEnvironmentPath(char *environmentPathBuffer, uint32_t bufferLen) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    if (!environmentPathBuffer || bufferLen < 0x100) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    int mcpFd            = IOS_Open("/dev/mcp", (IOSOpenMode) 0);
    if (mcpFd >= 0) {
        ALIGN_0x40 uint32_t io_buffer[0x100 / 4];
        io_buffer[0] = IPC_CUSTOM_COPY_ENVIRONMENT_PATH;

        if (IOS_Ioctl(mcpFd, 100, io_buffer, 4, io_buffer, 0x100) == IOS_ERROR_OK) {
            memcpy(environmentPathBuffer, reinterpret_cast<const char *>(io_buffer), 0xFF);
            res = MOCHA_RESULT_SUCCESS;
        }

        IOS_Close(mcpFd);
    }

    return res;
}

MochaUtilsStatus Mocha_SimpleCommand(uint32_t command, uint32_t apiVersion) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < apiVersion) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    int mcpFd            = IOS_Open("/dev/mcp", (IOSOpenMode) 0);
    if (mcpFd >= 0) {
        ALIGN_0x40 uint32_t io_buffer[0x40 / 4];
        io_buffer[0] = command;

        if (IOS_Ioctl(mcpFd, 100, io_buffer, 4, io_buffer, 0x4) == IOS_ERROR_OK) {
            res = MOCHA_RESULT_SUCCESS;
        }

        IOS_Close(mcpFd);
    }

    return res;
}

MochaUtilsStatus Mocha_RPXHookCompleted() {
    return Mocha_SimpleCommand(IPC_CUSTOM_MEN_RPX_HOOK_COMPLETED, 1);
}

MochaUtilsStatus Mocha_StartMCPThread() {
    return Mocha_SimpleCommand(IPC_CUSTOM_START_MCP_THREAD, 1);
}

MochaUtilsStatus Mocha_StartUSBLogging(bool avoidLogCatchup) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    int mcpFd            = IOS_Open("/dev/mcp", (IOSOpenMode) 0);
    if (mcpFd >= 0) {
        ALIGN_0x40 uint32_t io_buffer[0x40 / 4];
        io_buffer[0] = IPC_CUSTOM_START_USB_LOGGING;
        io_buffer[1] = avoidLogCatchup;

        if (IOS_Ioctl(mcpFd, 100, io_buffer, 8, io_buffer, 0x4) == IOS_ERROR_OK) {
            res = MOCHA_RESULT_SUCCESS;
        }

        IOS_Close(mcpFd);
    }

    return res;
}

MochaUtilsStatus Mocha_UnlockFSClient(FSClient *client) {
    if (!client) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }

    return Mocha_UnlockFSClientEx(FSGetClientBody(client)->clientHandle);
}

MochaUtilsStatus Mocha_UnlockFSClientEx(int clientHandle) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    ALIGN_0x40 int dummy[0x40 >> 2];

    auto res = IOS_Ioctl(clientHandle, 0x28, dummy, sizeof(dummy), dummy, sizeof(dummy));
    if (res == 0) {
        return MOCHA_RESULT_SUCCESS;
    }
    if (res == -5) {
        return MOCHA_RESULT_MAX_CLIENT;
    }
    return MOCHA_RESULT_UNKNOWN_ERROR;
}

MochaUtilsStatus Mocha_LoadRPXOnNextLaunch(MochaRPXLoadInfo *loadInfo) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    int mcpFd            = IOS_Open("/dev/mcp", (IOSOpenMode) 0);
    if (mcpFd >= 0) {
        ALIGN_0x40 uint32_t io_buffer[ROUNDUP(sizeof(MochaRPXLoadInfo) + 4, 0x40)];
        io_buffer[0] = IPC_CUSTOM_LOAD_CUSTOM_RPX;
        memcpy(&io_buffer[1], loadInfo, sizeof(MochaRPXLoadInfo));

        if (IOS_Ioctl(mcpFd, 100, io_buffer, sizeof(MochaRPXLoadInfo) + 4, io_buffer, 0x4) == IOS_ERROR_OK) {
            res = MOCHA_RESULT_SUCCESS;
        }

        IOS_Close(mcpFd);
    }

    return res;
}

MochaUtilsStatus Mocha_ODMGetDiscKey(WUDDiscKey *discKey) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    if (!discKey) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }
    int odm_handle       = IOS_Open("/dev/odm", IOS_OPEN_READ);
    MochaUtilsStatus res = MOCHA_RESULT_UNKNOWN_ERROR;
    if (odm_handle >= 0) {
        ALIGN_0x40 uint32_t io_buffer[0x40 / 4];
        // disc encryption key, only works with patched IOSU
        io_buffer[0] = 3;

        if (IOS_Ioctl(odm_handle, 0x06, io_buffer, 0x14, io_buffer, 0x20) == IOS_ERROR_OK) {
            memcpy(discKey, io_buffer, 16);
            res = MOCHA_RESULT_SUCCESS;
        }
        IOS_Close(odm_handle);
    }
    return res;
}

extern int bspRead(const char *, uint32_t, const char *, uint32_t, uint16_t *);
MochaUtilsStatus Mocha_SEEPROMRead(uint8_t *out_buffer, uint32_t offset, uint32_t size) {
    if (!mochaInitDone) {
        return MOCHA_RESULT_LIB_UNINITIALIZED;
    }
    if (mochaApiVersion < 1) {
        return MOCHA_RESULT_UNSUPPORTED_COMMAND;
    }
    if (out_buffer == nullptr || offset > 0x200 || offset & 0x01) {
        return MOCHA_RESULT_INVALID_ARGUMENT;
    }

    uint32_t sizeInShorts   = size >> 1;
    uint32_t offsetInShorts = offset >> 1;
    int32_t maxReadCount    = 0x100 - offsetInShorts;

    if (maxReadCount <= 0) {
        return MOCHA_RESULT_SUCCESS;
    }

    uint32_t count = sizeInShorts > (uint32_t) maxReadCount ? (uint32_t) maxReadCount : sizeInShorts;
    auto *ptr      = (uint16_t *) out_buffer;

    int res = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (bspRead("EE", offsetInShorts + i, "access", 2, ptr) != 0) {
            return MOCHA_RESULT_UNKNOWN_ERROR;
        }
        res += 2;
        ptr++;
    }

    return static_cast<MochaUtilsStatus>(res);
}
