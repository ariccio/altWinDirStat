/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "NBFOsImageReader.h"

#include <vector>
#include <string>
#include "vectorutils.h"
#include "debug.h"

NBFOsImageReader::NBFOsImageReader(const std::string& filename, DWORD offset/*=0*/) 
        : LinearFileImageReader(filename, 0x7fffffe0, 0x1f00020, offset)
{
}

