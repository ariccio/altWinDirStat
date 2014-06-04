/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "RemoteSdCard.h"
#include "debug.h"

#include "RemoteDevice.h"
#include <string>
#include <vector>
#include "vectorutils.h"
#include "stringutils.h"

RemoteSdCard::RemoteSdCard() : m_diskNr(1)
{
	m_bOpen= false;
	m_blockSize= 512;
	m_totalBlocks= 0;
}
RemoteSdCard::~RemoteSdCard()
{
	if (m_bOpen)
		Close();
}
bool RemoteSdCard::OpenForReading()
{
    if (!ConnectToDevice())
		return false;
	if (!ITSDCardInfo(m_diskNr, m_blockSize, m_totalBlocks))
		return false;

	m_bOpen= true;
	return true;
}

bool RemoteSdCard::OpenForWriting()
{
    if (!ConnectToDevice())
		return false;
	if (!ITSDCardInfo(m_diskNr, m_blockSize, m_totalBlocks))
		return false;

	m_bOpen= true;
	return true;
}

DWORD RemoteSdCard::GetBlockSize()
{
	return m_blockSize;
}
DWORD RemoteSdCard::GetTotalBlocks()
{
	return m_totalBlocks;
}

bool RemoteSdCard::Close()
{
	if (!m_bOpen)
		return false;
	m_bOpen= false;
    return true;
}

//-------------------------------------------------------------------------
// implementation of SdCardDevice interface.
bool RemoteSdCard::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
    ByteVector::const_iterator bufp= buffer.begin();
    DWORD dwSize= (DWORD)buffer.size();
    while(dwSize)
    {
        DWORD dwNumberOfBytesWritten;
        if (!ITWriteSDCard(m_diskNr, dwOffset, iteratorptr(bufp), min(dwSize, 32768), &dwNumberOfBytesWritten))
            return false;

        if (dwNumberOfBytesWritten==0 || dwNumberOfBytesWritten>dwSize)
            return false;

        dwSize   -= dwNumberOfBytesWritten;
        dwOffset += dwNumberOfBytesWritten;
        bufp += dwNumberOfBytesWritten;
    }

    return true;
}
bool RemoteSdCard::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
    buffer.resize(dwSize);

    ByteVector::iterator bufp= buffer.begin();
    while(dwSize)
    {
        DWORD dwNumberOfBytesRead;
        if (!ITReadSDCard(m_diskNr, dwOffset, iteratorptr(bufp), min(dwSize, 32768), &dwNumberOfBytesRead))
            return false;

        bufp += dwNumberOfBytesRead;

        dwSize   -= dwNumberOfBytesRead;
        dwOffset += dwNumberOfBytesRead;
    }

    return true;
}
bool RemoteSdCard::GetFriendlyName(std::string& name)
{
	string sizestring= 
			m_totalBlocks==0 ? "unknown size"
			: (m_totalBlocks>1024.0*1024*1024/m_blockSize)? stringformat("%6.2fG", m_totalBlocks/(1024.0*1024*1024/m_blockSize))
			: (m_totalBlocks>1024.0*1024/m_blockSize)? stringformat("%6.2fM", m_totalBlocks/(1024.0*1024/m_blockSize))
			: (m_totalBlocks*m_blockSize>1024.0)? stringformat("%6.2fk", m_totalBlocks*m_blockSize/1024.0)
			: stringformat("%6.2f", m_totalBlocks);

	name= stringformat("XDA SD card slot - %s", sizestring.c_str());

	return true;
}
