/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __NB0IMAGEREADER_H__

#include "LinearFileImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class NB0ImageReader : public LinearFileImageReader {
public:
    NB0ImageReader(const std::string& filename, DWORD offset=0);
};

#define __NB0IMAGEREADER_H__
#endif
