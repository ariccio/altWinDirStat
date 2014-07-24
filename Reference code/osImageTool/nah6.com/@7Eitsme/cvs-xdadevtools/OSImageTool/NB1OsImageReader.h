/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __NB1OSIMAGEREADER_H__

#include "LinearFileImageReader.h"
#include <string>
#include <vector>
#include "vectorutils.h"

class NB1OsImageReader : public LinearFileImageReader {
public:
    NB1OsImageReader(const std::string& filename, DWORD offset=0);
};

#define __NB1OSIMAGEREADER_H__
#endif
