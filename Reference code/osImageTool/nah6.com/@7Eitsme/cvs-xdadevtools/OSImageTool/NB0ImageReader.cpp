/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "NB0ImageReader.h"

#include <vector>
#include <string>
#include "vectorutils.h"
#include "debug.h"

NB0ImageReader::NB0ImageReader(const std::string& filename, DWORD offset/*=0*/) 
	: LinearFileImageReader(filename, 0x80000000, 0x40000, offset)
{

}

