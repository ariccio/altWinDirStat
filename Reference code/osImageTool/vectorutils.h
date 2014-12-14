/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __VECTORUTILS_H_

#include <vector>
#include <string>

typedef std::vector<BYTE> ByteVector;

#define vectorptr(v)  (&(v)[0])
#define iteratorptr(v)  (&(*v))

#define __VECTORUTILS_H_
#endif
