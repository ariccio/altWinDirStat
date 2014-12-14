/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include <ctype.h>

#include "SmartHandle.h"
#include "FileOsImageReader.h"

#include "NB1OsImageReader.h"
#include "NBFOsImageReader.h"
#include "BINOsImageReader.h"

#include "debug.h"
#include "errorhandler.h"

#include <vector>
#include "vectorutils.h"

bool isNBFHeader(BYTE* buf)
{
    char *pattern= "a-a-a.a-a-a-";

    for (int i=0 ; i<32 ; i++)
    {
        switch(*pattern)
        {
            case 'a':
                if (!isalnum(buf[i]))
                    pattern++;
                break;
            case '-':
                if (buf[i]!='-')
                    pattern++;
                break;
            case '.':
                if (buf[i]!='.')
                    pattern++;
                break;
            case 0:
                return false;
        }
    }
    return true;
}

// since I don't know what the block size is of the device that the file was copied
// from is, I assume it to be 512.  this is only relevant for htc sd card images.
#define FILE_BLOCKSIZE 512
FileOsImageReader* FileOsImageReader::GetFileReader(const std::string& filename)
{
	ByteVector buffer;

    SmartHandle h= CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h==INVALID_HANDLE_VALUE)
	{
		g_err.Set(stringformat("error opening file %s for reading: %s", filename.c_str(), LastError().c_str()));
        return NULL;
	}

    buffer.resize(FILE_BLOCKSIZE);
    DWORD bytes_read;
    if (!ReadFile(h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_read, NULL))
	{
		g_err.Set(stringformat("error reading from file %s : %s", filename.c_str(), LastError().c_str()));
		return NULL;
	}
    if (bytes_read!=buffer.size())
	{
		g_err.Set(stringformat("data truncated, reading from file %s", filename.c_str()));
        return NULL;
	}

    if (memcmp(vectorptr(buffer), "B000FF", 6)==0)
        return new BINOsImageReader(filename);

    if (isNBFHeader(vectorptr(buffer)))
        return new NBFOsImageReader(filename);

    if (memcmp(vectorptr(buffer), "HTC$WALLABY11", 13)==0)
        return new NB1OsImageReader(filename, 2*FILE_BLOCKSIZE);

    if (memcmp(vectorptr(buffer), "HTC$WALLABY22", 13)==0)
        return new NB1OsImageReader(filename, 2*FILE_BLOCKSIZE+0x40000);

    if (memcmp(vectorptr(buffer), "HTC$WALLABY", 11)==0)
	{
		g_err.Set("unsupported sdimage format");
        return NULL;
	}

    if (memcmp(vectorptr(buffer), "HTC FLASH KEY", 13)==0)
    {
        if (INVALID_SET_FILE_POINTER==SetFilePointer(h, 5*FILE_BLOCKSIZE, NULL, FILE_BEGIN))
		{
			g_err.Set("error determining filetype");
            return NULL;
		}

        if (!ReadFile(h, vectorptr(buffer), (DWORD)buffer.size(), &bytes_read, NULL))
		{
			g_err.Set("error determining filetype");
			return NULL;
		}
        if (bytes_read!=buffer.size())
		{
			g_err.Set("error determining filetype");
            return NULL;
		}

        if (memcmp(vectorptr(buffer), "HTC$WALLABY11", 13)==0)
            return new NB1OsImageReader(filename, 7*FILE_BLOCKSIZE);

        if (memcmp(vectorptr(buffer), "HTC$WALLABY22", 13)==0)
            return new NB1OsImageReader(filename, 7*FILE_BLOCKSIZE+0x40000);

		g_err.Set("unsupported sdimage format");
        return NULL;
    }
    if (memcmp(vectorptr(buffer)+0x298, "B000FF", 6)==0)
        return new BINOsImageReader(filename, 0x298);

	if (strcmp(filename.substr(filename.length()-4).c_str(), ".nb1")==0)
		return new NB1OsImageReader(filename, 0);

	g_err.Set("unsupported file format");
    return NULL;
}

FileOsImageReader::FileOsImageReader(const std::string& filename, DWORD offset/*=0*/) 
    : m_filename(filename), m_baseoffset(offset)
{
	m_bOpen= false;
}

FileOsImageReader::~FileOsImageReader()
{
	if (m_bOpen)
		Close();
}


bool FileOsImageReader::Open()
{
    m_h= CreateFile(m_filename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (m_h==INVALID_HANDLE_VALUE)
    {
		g_err.Set(stringformat("error opening %s for reading : %s", m_filename.c_str(), LastError().c_str()));
        return false;
    }

	m_bOpen= true;
	return true;
}

bool FileOsImageReader::Close()
{
	if (!m_bOpen)
		return false;
	m_bOpen= false;

    m_h.close();
	return true;
}

