/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "LinearFileImageWriter.h"
#include "debug.h"
#include "errorhandler.h"

LinearFileImageWriter::LinearFileImageWriter(const std::string& filename, DWORD filestart, DWORD baseoffset)
    : FileOsImageWriter(filename, baseoffset), m_filestart(filestart)
{
    
}

bool LinearFileImageWriter::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, m_baseoffset+dwOffset-m_filestart, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error seeking to %08lx: %s", m_baseoffset+dwOffset-m_filestart, LastError().c_str()));
        return false;
    }

	DWORD bytes_written;
    if (!WriteFile(m_h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_written, NULL)
        || bytes_written!=buffer.size())
    {
		g_err.Set(stringformat("error writing %08lx bytes to %08lx: %s", buffer.size(), dwOffset, LastError().c_str()));
        return false;
    }

	return true;
}

