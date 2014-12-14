/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "SdOsImageWriter.h"
#include "SdCardDevice.h"

#include <vector>
#include "vectorutils.h"
#include "errorhandler.h"

SdOsImageWriter::SdOsImageWriter(SdCardDevice *sd)
{
    m_sd= sd;
    m_checksum= 0;
	m_bOpen= false;
}
SdOsImageWriter::~SdOsImageWriter()
{
	if (m_bOpen)
		Close();
}

bool SdOsImageWriter::Open()
{
	if (!m_sd->OpenForWriting())
		return false;

	m_checksum= 0;
    const char *signature= "HTC$WALLABY11";
	ByteVector sigbuf(signature, signature+strlen(signature));
	sigbuf.resize(m_sd->GetBlockSize());

	if (!m_sd->WriteData(0, sigbuf))
		return false;

	m_bOpen= true;
	return true;
}

bool SdOsImageWriter::Close()
{
	if (!m_bOpen)
		return false;
	m_bOpen= false;

	// finish by writing NUL's at the end.
	ByteVector nullbuf; nullbuf.resize(0x100000);
    m_sd->WriteData(2*m_sd->GetBlockSize()+0x1ec0000, nullbuf);

    // write checksum
	ByteVector sumbuf((BYTE*)&m_checksum, (BYTE*)((&m_checksum)+1));
	sumbuf.resize(m_sd->GetBlockSize());

    m_sd->WriteData(m_sd->GetBlockSize(), sumbuf);

    m_sd->Close();

	return true;
}

// for 5.17 bootloaders the region 80040e00-80041000 is not added to
// the checksum.
void UpdateChecksum(DWORD& sum, const ByteVector& buffer)
{
    for (ByteVector::const_iterator bufp= buffer.begin() ; bufp != buffer.end() ; bufp+=4)
	{
		sum+=*(const DWORD*)iteratorptr(bufp);
	}
}

bool SdOsImageWriter::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
    UpdateChecksum(m_checksum, buffer);

    return m_sd->WriteData(dwOffset-0x80040000 + 2*m_sd->GetBlockSize(), buffer);
}

bool SdOsImageWriter::CheckSpace()
{
	// ( we don't know yet how large the target is, assume it is ok.
	if (m_sd->GetTotalBlocks()==0)
		return true;
/*
	// otherwise, it must be able to hold an os image
	if (m_sd->GetTotalBlocks() < 2+(0x81f00000-0x80040000)/m_sd->GetBlockSize())
	{
		g_err.Set("target device is too small to hold os image");
		return false;
	}
*/
	// and not be larger than 2G
	if (m_sd->GetTotalBlocks() > 0x7fffffff/m_sd->GetBlockSize())
	{
		return IDYES==MessageBox(0, 
			stringformat("Are you sure you want to write to this device:\n%s", m_sd->GetFriendlyName().c_str()).c_str(),
			"WARNING", 
			MB_YESNO);
	}
	return true;
}
