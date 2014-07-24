#ifndef _COL_ANSI_STRING_H
#define _COL_ANSI_STRING_H

#include <stdlib.h>
#include <assert.h>
#include <new>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

namespace col {

#define __A_ANSI_STRING__
#include "col/ArrayBase.h"
#undef __A_ANSI_STRING__

} /* namespace */

#endif /* _COL_ANSI_STRING_H */
