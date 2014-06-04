/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "debug.h"
#include "errorhandler.h"

#include "XDAOsImageWriter.h"
#include "NBFFileOsImageWriter.h"


#include "Configuration.h"

bool RunApplication(const char *szAppname, const char *szCommandLine)
{
	SHELLEXECUTEINFO exec;
	memset(&exec, 0, sizeof(SHELLEXECUTEINFO));
	exec.cbSize= sizeof(SHELLEXECUTEINFO);
	exec.lpFile= szAppname;
	exec.lpParameters= szCommandLine;
	exec.lpVerb= "open";
	exec.lpDirectory= g_cfg.ApplicationRoot.c_str();
	exec.nShow= SW_SHOW;
	exec.fMask= SEE_MASK_FLAG_NO_UI|SEE_MASK_NOCLOSEPROCESS;

	if (!ShellExecuteEx(&exec))
	{
		g_err.Set(stringformat("error executing %s: %s", szAppname, LastError().c_str()));
		return false;
	}
	return true;
}
string concatpath(const string& path1, const string& path2)
{
	string path= path1;

	if (*(path.rbegin()) != '/' && *(path.rbegin()) != '\\')
		path += '\\';

	path += path2;
	return path;
}
bool FindNBFDirectory(const string& basedir, string& directory)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind= FindFirstFile(concatpath(basedir,"\\*").c_str(), &wfd);
	if (hFind==NULL || hFind==INVALID_HANDLE_VALUE)
	{
		error("FindFirstFile('%s')", basedir.c_str());
		return false;
	}

	int nDirectoryCount= 0;
	do {
		if (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(wfd.cFileName, ".") && strcmp(wfd.cFileName, ".."))
			{
				nDirectoryCount++;
				if (nDirectoryCount==1)
					directory= concatpath(basedir,wfd.cFileName);
			}
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	return (nDirectoryCount==1);
}

XDAOsImageWriter::XDAOsImageWriter()
{
	m_nbf= NULL;
	m_bOpen= false;
	if (FindNBFDirectory(g_cfg.ProgrammeApath, m_NBFFileName))
		m_NBFFileName += "\\NK.nbf";
}

XDAOsImageWriter::~XDAOsImageWriter()
{
	if (m_bOpen)
		Close();
    if (m_nbf) { 
        delete m_nbf; 
        m_nbf= NULL; 
    }
}

bool XDAOsImageWriter::Open()
{
	if (m_NBFFileName.empty())
	{
		g_err.Set("Programme A not found - run with '--register'");
		return false;
	}
    m_nbf= new NBFFileOsImageWriter(m_NBFFileName);

    if (!m_nbf->Open()) 
	{
		return false;
	}
	m_bOpen= true;
	return true;
}

bool XDAOsImageWriter::Close()
{
	if (!m_bOpen)
		return false;
	m_bOpen= false;
	
	if (!m_nbf) 
		return false;
    if (!m_nbf->Close())
		return false;

    if (!RunApplication(g_cfg.ProgrammeAcommand.c_str(), ""))
		return false;

	return true;
}


bool XDAOsImageWriter::WriteData(DWORD dwOffset, const ByteVector& buffer)
{
    return m_nbf->WriteData(dwOffset, buffer);
}

bool XDAOsImageWriter::CheckSpace()
{
	return true;	// always enough room to write os image into os rom.
}
