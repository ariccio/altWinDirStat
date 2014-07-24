/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "NBFFileOsImageWriter.h"
#include "NB0ImageReader.h"
#include "debug.h"
#include "errorhandler.h"
#include "Configuration.h"

NBFFileOsImageWriter::NBFFileOsImageWriter(const std::string& filename, DWORD baseoffset)
    : LinearFileImageWriter(filename, 0x7fffffe0, baseoffset), m_bootloaderBytes(0)
{
	m_checksum=0;
}

bool NBFFileOsImageWriter::Open()
{
	NB0ImageReader nb0(g_cfg.BootloaderName);
	m_checksum=0;

    if (!nb0.Open()) 
	{
		g_err.Set(stringformat("error opening source bootloader from %s", g_cfg.BootloaderName.c_str()));
		return false;
	}
    ByteVector bootloader;

    if (!nb0.ReadData(0x80000000, 0x40000, bootloader))
	{
		g_err.Set("error reading bootloader");
        return false;
	}
    nb0.Close();

	if (!LinearFileImageWriter::Open())
		return false;

    if (!WriteData(0x80000000, bootloader))
        return false;

	return true;

}


void UpdateNBFChecksum(DWORD& sum, const ByteVector& buffer)
{
    for (ByteVector::const_iterator bufp= buffer.begin() ; bufp != buffer.end() ; bufp+=1)
	{
		sum+=*(const BYTE*)iteratorptr(bufp);
	}
}
bool NBFFileOsImageWriter::Close()
{
    if (m_bootloaderBytes!=0x40000)
    {
		g_err.Set("erased target file because I could not find a bootloader to add");
        // make sure file cannot be used, when no bootloader found
        ClearFile();
		LinearFileImageWriter::Close();
        DeleteFile();
		return false;
    }

	// todo: calculate checksum
	const char*header= "PW10A1-ENG-3.16-007";
	UpdateNBFChecksum(m_checksum, ByteVector(header, header+strlen(header)));

	string headercheck=stringformat("%s-%04x------------", header, m_checksum&0xffff);

	if (!LinearFileImageWriter::WriteData(0x7fffffe0, ByteVector(headercheck.begin(), headercheck.begin()+0x20)))
	{
		g_err.Set("error writing nbf header");
		return false;
	}
	if (!LinearFileImageWriter::Close())
		return false;
	return true;
}


bool NBFFileOsImageWriter::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
	// calc first and last address in the bootloader range
    DWORD start= max(dwOffset, 0x80000000);
    DWORD end= min(dwOffset+buffer.size(), 0x80040000);

    if (start<end)
        m_bootloaderBytes += end-start;

	UpdateNBFChecksum(m_checksum, buffer);
    if (!LinearFileImageWriter::WriteData(dwOffset, buffer))
		return false;
	return true;
}

