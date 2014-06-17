#ifndef _COL_UNICODE_STRING_H
#define _COL_UNICODE_STRING_H

#include <stdlib.h>
#include <assert.h>
#include <new>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

namespace col {

#define __A_UNICODE_STRING__
#include "col/ArrayBase.h"
#undef __A_UNICODE_STRING__

} /* namespace */

#endif /* _COL_UNICODE_STRING_H */
