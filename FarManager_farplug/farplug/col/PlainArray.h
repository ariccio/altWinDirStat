#ifndef _COL_PLAIN_ARRAY_H
#define _COL_PLAIN_ARRAY_H

#include <stdlib.h>
#include <assert.h>
#include <new>

namespace col {

#define __A_PLAIN_ARRAY__
#include "col/ArrayBase.h"
#undef __A_PLAIN_ARRAY__

} /* namespace */

#endif /* _COL_PLAIN_ARRAY_H */
