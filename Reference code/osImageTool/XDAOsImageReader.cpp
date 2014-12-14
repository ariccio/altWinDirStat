/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "XDAOsImageReader.h"
#include "vectorutils.h"
#include "RemoteDevice.h"
#include "debug.h"

XDAOsImageReader::XDAOsImageReader()
{
	m_bOpen= false;
}
XDAOsImageReader::~XDAOsImageReader()
{
	if (m_bOpen)
		Close();
}

bool XDAOsImageReader::Open()
{
	if (!ConnectToDevice())
		return false;

	m_bOpen= true;
	return true;
}
bool XDAOsImageReader::Close()
{
	if (!m_bOpen)
		return false;
	m_bOpen= false;

	return true;
}

bool XDAOsImageReader::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
    buffer.resize(dwSize);

    ByteVector::iterator bufp= buffer.begin();
    while(dwSize)
    {
        DWORD dwNumberOfBytesRead;
        if (!ITReadProcessMemory(NULL, dwOffset, iteratorptr(bufp), min(dwSize, 32768), &dwNumberOfBytesRead))
            return false;

        bufp += dwNumberOfBytesRead;

        dwSize   -= dwNumberOfBytesRead;
        dwOffset += dwNumberOfBytesRead;
    }

    return true;

}

