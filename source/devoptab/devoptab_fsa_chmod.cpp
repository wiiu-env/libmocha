#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>
#include <sys/stat.h>

int __fsa_chmod(struct _reent *r,
                const char *path,
                mode_t mode) {
    if (!path) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        r->_errno = ENOMEM;
        return -1;
    }

    const FSMode translatedMode = __fsa_translate_permission_mode(mode);

    const __fsa_device_t *deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    const FSError status = FSAChangeMode(deviceData->clientHandle, fixedPath, translatedMode);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAChangeMode(0x%08X, %s, 0x%X) failed: %s",
                                deviceData->clientHandle, fixedPath, translatedMode, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(fixedPath);

    return 0;
}
