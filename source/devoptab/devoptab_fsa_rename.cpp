#include "../logger.h"
#include "devoptab_fsa.h"
#include <mutex>

int __fsa_rename(struct _reent *r,
                 const char *oldName,
                 const char *newName) {
    if (!oldName || !newName) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedOldPath = __fsa_fixpath(r, oldName);
    if (!fixedOldPath) {
        r->_errno = ENOMEM;
        return -1;
    }

    char *fixedNewPath = __fsa_fixpath(r, newName);
    if (!fixedNewPath) {
        free(fixedOldPath);
        r->_errno = ENOMEM;
        return -1;
    }

    const auto deviceData = static_cast<__fsa_device_t *>(r->deviceData);

    const FSError status = FSARename(deviceData->clientHandle, fixedOldPath, fixedNewPath);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSARename(0x%08X, %s, %s) failed: %s",
                                deviceData->clientHandle, fixedOldPath, fixedNewPath, FSAGetStatusStr(status));
        free(fixedOldPath);
        free(fixedNewPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(fixedOldPath);
    free(fixedNewPath);

    return 0;
}
