/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "NB1OsImageReader.h"

#include <vector>
#include <string>
#include "vectorutils.h"
#include "debug.h"

NB1OsImageReader::NB1OsImageReader(const std::string& filename, DWORD offset/*=0*/) 
	: LinearFileImageReader(filename, 0x80040000, 0x01ec0000, offset)
{

}
