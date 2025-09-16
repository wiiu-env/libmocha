#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

int __fsa_chdir(struct _reent *r,
                const char *path) {
    if (!path) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        r->_errno = ENOMEM;
        return -1;
    }
    const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    const FSError status = FSAChangeDir(deviceData->clientHandle, fixedPath);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAChangeDir(0x%08X, %s) failed: %s", deviceData->clientHandle, fixedPath, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }

    // Remove trailing '/'
    if (fixedPath[0] != '\0') {
        if (fixedPath[strlen(fixedPath) - 1] == '/') {
            fixedPath[strlen(fixedPath) - 1] = 0;
        }
    }

    if (snprintf(deviceData->cwd, sizeof(deviceData->cwd), "%s", fixedPath) >= static_cast<int>(sizeof(deviceData->cwd))) {
        DEBUG_FUNCTION_LINE_WARN("__wut_fsa_chdir: snprintf result was truncated");
    }

    free(fixedPath);

    return 0;
}
