/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __NB1FILEOSIMAGEWRITER_H__

#include <vector>
#include "vectorutils.h"
#include "LinearFileImageWriter.h"

class NB1FileOsImageWriter : public LinearFileImageWriter {
public:
	NB1FileOsImageWriter(const std::string& filename, DWORD baseoffset=0);
};

#define __NB1FILEOSIMAGEWRITER_H__
#endif
