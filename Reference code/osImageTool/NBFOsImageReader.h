/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __NBFOSIMAGEREADER_H__

#include "LinearFileImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class NBFOsImageReader : public LinearFileImageReader {
public:
    NBFOsImageReader(const std::string& filename, DWORD offset=0);
};

#define __NBFOSIMAGEREADER_H__
#endif
