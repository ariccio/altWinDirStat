/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "LocalSdCard.h"
#include "stringutils.h"

#include "diskinfo.h"
#include "debug.h"
#include "errorhandler.h"

#include <string>
#include <vector>
#include "vectorutils.h"

LocalSdCard::LocalSdCard(const DiskInfo* di) : m_di(di)
{
}
LocalSdCard::~LocalSdCard()
{
	if (m_bWriting||m_bReading)
		Close();
	delete m_di;
}
bool LocalSdCard::OpenForWriting()
{
    m_h= CreateFile(m_di->physical.c_str(), GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (m_h==INVALID_HANDLE_VALUE)
	{
		g_err.Set(stringformat("error opening device %s for writing: %s", m_di->physical.c_str(), LastError().c_str()));
        return false;
	}

    m_bWriting= true;
    m_bReading= false;

    return true;
}
bool LocalSdCard::OpenForReading()
{
    m_h= CreateFile(m_di->physical.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (m_h==INVALID_HANDLE_VALUE)
	{
		g_err.Set(stringformat("error opening device %s for reading: %s", m_di->physical.c_str(), LastError().c_str()));
        return false;
	}

    m_bWriting= false;
    m_bReading= true;

    return true;
}
bool LocalSdCard::Close()
{
	if (!m_bReading&&!m_bWriting)
		return false;
    m_h.close();
    m_bWriting= false;
    m_bReading= false;

	return true;
}

DWORD LocalSdCard::GetBlockSize()
{
	return m_di->sectorsize;
}
DWORD LocalSdCard::GetTotalBlocks()
{
	return m_di->totalsectors;
}

// this fails if buffer is not a multiple of blocksize
bool LocalSdCard::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
    if (!m_bWriting)
        return false;

    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, dwOffset, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error seeking to %08lx : %s", dwOffset, LastError().c_str()));
        return false;
    }

    DWORD bytes_written;
    if (!WriteFile(m_h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_written, NULL))
	{
		g_err.Set(stringformat("Error writing %08lx bytes to %08lx: %s", buffer.size(), dwOffset, LastError().c_str()));
		return false;
	}
    if (bytes_written!=buffer.size())
	{
		g_err.Set("not all bytes written");
		return false;
	}
	return true;
}

bool LocalSdCard::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
    if (!m_bReading)
        return false;

    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, dwOffset, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error seeking to %08lx : %s", dwOffset, LastError().c_str()));
        return false;
    }

    buffer.resize(dwSize);
    DWORD bytes_read;
    if (!ReadFile(m_h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_read, NULL))
	{
		g_err.Set(stringformat("error reading %08lx bytes from %08lx : %s", buffer.size(), dwOffset, LastError().c_str()));
		return false;
	}
	if (bytes_read!=buffer.size())
	{
		g_err.Set("not all bytes read");
		return false;
	}
	return true;
}

bool LocalSdCard::GetFriendlyName(std::string& name)
{
	if (m_friendlyname.empty())
	{
		m_friendlyname= stringformat("%c: %7s %s", m_di->drv, m_di->sizestring.c_str(), m_di->description.c_str());
	}

	name= m_friendlyname;
	return true;
}


