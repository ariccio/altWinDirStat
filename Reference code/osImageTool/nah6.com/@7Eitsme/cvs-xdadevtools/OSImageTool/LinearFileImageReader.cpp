/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "Configuration.h"
#include "LinearFileImageReader.h"

#include <vector>
#include <string>
#include "vectorutils.h"
#include "stringutils.h"

#include "debug.h"
#include "errorhandler.h"

LinearFileImageReader::LinearFileImageReader(
		const std::string& filename, DWORD filestart, DWORD wantedsize, DWORD offset) 
	: FileOsImageReader(filename, offset), m_filestart(filestart), m_wantedsize(wantedsize)
{

}


bool LinearFileImageReader::Open()
{
	if (!FileOsImageReader::Open())
		return false;

	DWORD size;
	if (INVALID_FILE_SIZE==(size=GetFileSize(m_h, NULL)))
		return false;

	if (g_cfg.bCheckFilesize && size!=m_wantedsize)
	{
		g_err.Set(stringformat("%s: incorrect filesize(%d), file should be exactly %d bytes", m_filename.c_str(), size, m_wantedsize));
		return false;
	}

	return true;
}


bool LinearFileImageReader::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, m_baseoffset+dwOffset-m_filestart, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error seeking to %08lx : %s", m_baseoffset+dwOffset-m_filestart, LastError().c_str()));
        return false;
    }

	buffer.resize(dwSize);

	DWORD bytes_read;
    if (!ReadFile(m_h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_read, NULL)
        || bytes_read!=buffer.size())
    {
		g_err.Set(stringformat("error reading %08lx bytes from %08lx : %s", buffer.size(), m_baseoffset+dwOffset-m_filestart, LastError().c_str()));
        return false;
    }

	return true;
}


