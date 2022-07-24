#include "devoptab_fs.h"

int __fsa_utimes(struct _reent *r,
                 const char *filename,
                 const struct timeval times[2]) {
    // TODO: FSChangeMode and FSStatFile?
    r->_errno = ENOSYS;
    return -1;
}
