/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "NB1FileOsImageWriter.h"
#include "debug.h"

NB1FileOsImageWriter::NB1FileOsImageWriter(const std::string& filename, DWORD baseoffset)
    : LinearFileImageWriter(filename, 0x80040000, baseoffset)
{
}

