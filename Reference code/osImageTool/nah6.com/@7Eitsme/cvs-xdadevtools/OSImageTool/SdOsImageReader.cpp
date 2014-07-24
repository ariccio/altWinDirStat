/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "SdOsImageReader.h"
#include "SdCardDevice.h"
#include "debug.h"
#include "ErrorHandler.h"

SdOsImageReader::SdOsImageReader(SdCardDevice* sd)
{
    m_sd= sd;
    m_baseoffset= 0;
	m_bOpen= false;
}

SdOsImageReader::~SdOsImageReader()
{
	if (m_bOpen)
		Close();
}

bool SdOsImageReader::Open()
{
    if (!m_sd->OpenForReading())
	{
		g_err.Set(stringformat("Could not open %s for reading: %s", m_sd->GetFriendlyName().c_str(), LastError().c_str()));
        return false;
	}

    ByteVector buffer;
    if (!m_sd->ReadData(0, m_sd->GetBlockSize(), buffer))
	{
		g_err.Set(stringformat("Could not read from %s : %s", m_sd->GetFriendlyName().c_str(), LastError().c_str()));
        return false;
	}

    if (memcmp(vectorptr(buffer), "HTC$WALLABY11", 13)==0)
    {
		m_baseoffset= 2*m_sd->GetBlockSize();
        return true;
    }

    if (memcmp(vectorptr(buffer), "HTC$WALLABY22", 13)==0)
    {
		m_baseoffset= 2*m_sd->GetBlockSize()+0x40000;
        return true;
    }

    if (memcmp(vectorptr(buffer), "HTC$WALLABY", 11)==0)
	{
		g_err.Set(stringformat("%s contains an unsupported HTC sdimage format", m_sd->GetFriendlyName().c_str()));
        return false;
	}

    if (memcmp(vectorptr(buffer), "HTC FLASH KEY", 13)==0)
    {
        if (!m_sd->ReadData(5*m_sd->GetBlockSize(), m_sd->GetBlockSize(), buffer))
		{
			g_err.Set(stringformat("error reading from %s : %s", m_sd->GetFriendlyName().c_str(), LastError().c_str()));
            return false;
		}
        if (memcmp(vectorptr(buffer), "HTC$WALLABY11", 13)==0)
        {
            m_baseoffset= 7*m_sd->GetBlockSize();
            return true;
        }

        if (memcmp(vectorptr(buffer), "HTC$WALLABY22", 13)==0)
        {
            m_baseoffset= 7*m_sd->GetBlockSize()+0x40000;
            return true;
        }

		g_err.Set(stringformat("%s contains an unsupported HTC sdimage format", m_sd->GetFriendlyName().c_str()));
        return false;
    }

    g_err.Set(stringformat("%s contains does not contain an HTC sdimage card", m_sd->GetFriendlyName().c_str()));

    return false;
}

bool SdOsImageReader::Close()
{
	if (!m_bOpen)
		return true;
	m_bOpen= false;
	
	if (!m_sd->Close())
		return false;

	return true;
}

bool SdOsImageReader::ReadData(DWORD dwOffset, DWORD dwSize, ByteVector& buffer)
{
    return m_sd->ReadData(m_baseoffset+dwOffset-0x80040000, dwSize, buffer);
}

