#ifndef _COL_OBJECT_ARRAY_H
#define _COL_OBJECT_ARRAY_H

#include <stdlib.h>
#include <assert.h>
#include <new>

namespace col {

#define __A_OBJECT_ARRAY__
#include "col/ArrayBase.h"
#undef __A_OBJECT_ARRAY__

} /* namespace */

#endif /* _COL_OBJECT_ARRAY_H */
