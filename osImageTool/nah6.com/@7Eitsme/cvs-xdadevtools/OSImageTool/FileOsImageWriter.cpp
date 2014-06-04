/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "FileOsImageWriter.h"
#include "NB1FileOsImageWriter.h"
#include "NBFFileOsImageWriter.h"
#include "debug.h"
#include "errorhandler.h"

FileOsImageWriter* FileOsImageWriter::GetFileWriter(const std::string& filename)
{
	if (strcmp(filename.substr(filename.length()-4).c_str(), ".nb1")==0)
		return new NB1FileOsImageWriter(filename, 0);

	if (strcmp(filename.substr(filename.length()-4).c_str(), ".nbf")==0)
		return new NBFFileOsImageWriter(filename, 0);
/*
	if (strcmp(filename.substr(filename.length()-4).c_str(), ".bin")==0)
		return new BINFileOsImageWriter(filename, 0);
*/
	g_err.Set("unsupported target filetype");
    return NULL;
}


FileOsImageWriter::FileOsImageWriter(const std::string& filename, DWORD baseoffset)
    : m_filename(filename), m_baseoffset(baseoffset)
{
	m_bOpen= false;
}

FileOsImageWriter::~FileOsImageWriter()
{
	if (m_bOpen)
		Close();
}

bool FileOsImageWriter::Open()
{
    m_h= CreateFile(m_filename.c_str(), GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if (m_h==INVALID_HANDLE_VALUE)
    {
		g_err.Set(stringformat("error opening %s for writing: %s", m_filename.c_str(), LastError().c_str()));
        return false;
    }

	m_bOpen= true;
	return true;
}

bool FileOsImageWriter::Close()
{
	if (!m_bOpen)
		return false;
   	m_bOpen= false;

	m_h.close();
	return true;
}
bool FileOsImageWriter::ClearFile()
{
    if (INVALID_SET_FILE_POINTER==SetFilePointer(m_h, 0, NULL, FILE_BEGIN))
    {
		g_err.Set(stringformat("error clearing file: %s", LastError().c_str()));
        return false;
    }
	if (!SetEndOfFile(m_h))
	{
		g_err.Set(stringformat("error truncating file: %s", LastError().c_str()));
		return false;
	}

	return true;
}
bool FileOsImageWriter::DeleteFile()
{
	if (! ::DeleteFile(m_filename.c_str()))
	{
		g_err.Set(stringformat("error deleting file %s: %s", m_filename.c_str(), LastError().c_str()));
		return false;
	}
	return true;
}

bool FileOsImageWriter::CheckSpace()
{
	return true;	// todo: check harddrive space??
}
