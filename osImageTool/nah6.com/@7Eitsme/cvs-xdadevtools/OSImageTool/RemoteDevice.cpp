/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include <stdlib.h>
#include <windows.h>
#include <rapi.h>

#include "itsutils.h"
#include "vectorutils.h"
#include "stringutils.h"
#include "errorhandler.h"

#include "debug.h"
#include "dllversion.h"
#include "RemoteDevice.h"
#include "Configuration.h"

class RemoteDevice g_device;

RemoteDevice::RemoteDevice()
{
    m_bConnected= false;
}
RemoteDevice::~RemoteDevice()
{
	if (m_bConnected)
		Disconnect();
}
bool RemoteDevice::WaitForDevice()
{
    char *szTitle= "OsImageTool";
	RAPIINIT rinit; 
	rinit.cbSize= sizeof(RAPIINIT);
	rinit.heRapiInit= NULL;
	rinit.hrRapiInit= -1;
	if (CeRapiInitEx(&rinit))
	{
		g_err.Set(stringformat("CeRapiInit: %s", LastError().c_str()));
		return false;
	}

	do {
		DWORD res= MsgWaitForMultipleObjects(1, &rinit.heRapiInit, false, 1000, 0);
		if (res==WAIT_OBJECT_0)
			break;

		if (res!=WAIT_TIMEOUT)
		{
			g_err.Set("waiting for rapi init");
			return false;
		}
		res= MessageBox(0, "Please connect your XDA to activesync", szTitle, MB_RETRYCANCEL);
		if (res==IDCANCEL)
			return false;
	} while(1);
	if (rinit.hrRapiInit)
	{
		g_err.Set("Error connecting to XDA");
		return false;
	}
    return true;
}

bool RemoteDevice::CheckITSDll()
{
    DWORD outsize=0;
    HRESULT res= CeRapiInvoke(L"\\Windows\\ItsUtils.dll", L"ITGetVersion",
            0, NULL, &outsize, NULL, NULL, 0);
    if (res>10000 || res<ITSDLL_VERSION)
    {
		return CeCopyFileToDevice(g_cfg.ApplicationRoot+"\\itsutils.dll", "\\windows");
    }
    return true;
}
bool RemoteDevice::Connect()
{
    if (m_bConnected)
        return true;
    if (!WaitForDevice())
        return false;
    if (!CheckITSDll())
    {
        g_err.Set("Could not connect to itsutils.dll");
        return false;
    }
    m_bConnected= true;
    return true;
}
void RemoteDevice::Disconnect()
{
	if (!m_bConnected)
		return;
    CeRapiUninit();
    m_bConnected= false;
}
bool ConnectToDevice()
{
    return g_device.Connect();
}

bool ITReadProcessMemory(HANDLE hProc, DWORD dwOffset, BYTE *buffer, DWORD dwBytesWanted, DWORD *pdwNumberOfBytesRead)
{
    ReadProcessMemoryParams inbuf;
    DWORD outsize=0;
    ReadProcessMemoryResult *outbuf=NULL;

    inbuf.hProcess= hProc;
    inbuf.dwOffset= dwOffset;
    inbuf.nSize= dwBytesWanted;
    outbuf= NULL; outsize= 0;
    HRESULT res= CeRapiInvoke(L"\\Windows\\ItsUtils.dll", L"ITReadProcessMemory",
            sizeof(ReadProcessMemoryParams), (BYTE*)&inbuf,
            &outsize, (BYTE**)&outbuf, NULL, 0);
    if (res)
    {
		g_err.Set(stringformat("ITReadProcessMemory: %s", LastError(res).c_str()));
        return false;
    }
	
    memcpy(buffer, &outbuf->buffer, outbuf->dwNumberOfBytesRead);
    *pdwNumberOfBytesRead= outbuf->dwNumberOfBytesRead;
    LocalFree(outbuf);

    return true;
}
//-------------------------------------------------------------------------
//  low level rapi access
bool ITReadSDCard(DWORD dwDiskNr, DWORD dwOffset, BYTE *buffer, DWORD dwBytesWanted, DWORD *pdwNumberOfBytesRead)
{
    ReadSDCardParams inbuf;
    DWORD outsize=0;
    ReadSDCardResult *outbuf=NULL;

    inbuf.dwDiskNr= dwDiskNr;
    inbuf.dwOffset= dwOffset;
    inbuf.dwSize= dwBytesWanted;
    outbuf= NULL; outsize= 0;
    HRESULT res= CeRapiInvoke(L"\\Windows\\ItsUtils.dll", L"ITReadSDCard",
            sizeof(ReadSDCardParams), (BYTE*)&inbuf,
            &outsize, (BYTE**)&outbuf, NULL, 0);
    if (res)
    {
		g_err.Set(stringformat("ITReadSDCard: %s", LastError(res).c_str()));
        return false;
    }
    memcpy(buffer, &outbuf->buffer, outbuf->dwNumberOfBytesRead);
    *pdwNumberOfBytesRead= outbuf->dwNumberOfBytesRead;
    LocalFree(outbuf);

    return true;
}
bool ITWriteSDCard(DWORD dwDiskNr, DWORD dwOffset, const BYTE *buffer, DWORD dwBufferSize, DWORD *pdwNumberOfBytesWritten)
{
    int insize= sizeof(WriteSDCardParams)+dwBufferSize;
    WriteSDCardParams *inbuf= (WriteSDCardParams *)LocalAlloc(LPTR, insize);
    DWORD outsize=0;
    WriteSDCardResult *outbuf=NULL;

    inbuf->dwDiskNr= dwDiskNr;
    inbuf->dwOffset= dwOffset;
    inbuf->dwSize= dwBufferSize;
    memcpy(inbuf->buffer, buffer, dwBufferSize);

    HRESULT res= CeRapiInvoke(L"\\Windows\\ItsUtils.dll", L"ITWriteSDCard",
            insize, (BYTE*)inbuf,
            &outsize, (BYTE**)&outbuf, NULL, 0);
    if (res)
    {
		g_err.Set(stringformat("ITWriteSDCard: %s", LastError(res).c_str()));
        return false;
    }
    *pdwNumberOfBytesWritten= outbuf->dwNumberOfBytesWritten;
    LocalFree(outbuf);

    return true;
}

bool ITSDCardInfo(DWORD dwDiskNr, DWORD& dwBlockSize, DWORD& dwTotalBlocks)
{
    SDCardInfoParams inbuf;
    DWORD outsize=0;
    SDCardInfoResult *outbuf=NULL;

    inbuf.dwDiskNr= dwDiskNr;
    outbuf= NULL; outsize= 0;
    HRESULT res= CeRapiInvoke(L"\\Windows\\ItsUtils.dll", L"ITSDCardInfo",
            sizeof(SDCardInfoParams), (BYTE*)&inbuf,
            &outsize, (BYTE**)&outbuf, NULL, 0);
    if (res)
    {
		g_err.Set(stringformat("ITSDCardInfo: %s", LastError(res).c_str()));
        return false;
    }
    dwBlockSize= outbuf->blockSize;
	dwTotalBlocks= outbuf->totalBlocks;

    LocalFree(outbuf);

    return true;
}

bool CeCopyFileToDevice(const string& srcfile, const string& dstfile) 
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile( srcfile.c_str(), &wfd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        g_err.Set("Source/host file does not exist\n");
        return false;
    }
    FindClose( hFind);

    if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        g_err.Set("Source/host file specifies a directory\n");
        return false;
    }

    string dstname= dstfile;

    DWORD dwAttr = CeGetFileAttributes( ToWString(dstname).c_str());
    if (0xFFFFFFFF  != dwAttr)
    {
        if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
        {
            dstname += "\\";
            dstname += wfd.cFileName;
        }
        else
        {
            g_err.Set("File already exists.\n");
            return false;
        }
    }

    HANDLE hSrc = CreateFile( srcfile.c_str(), GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hSrc)
    {
        g_err.Set("Unable to open source/host file");
        return false;
    }

    HANDLE hDest = CeCreateFile( ToWString(dstname).c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hDest )
    {
        g_err.Set("Unable to open WinCE file");
        return false;
    }

    debug("Copying %hs to WCE:%ls\n", srcfile.c_str(), ToWString(dstname).c_str());

    ByteVector buffer; buffer.resize(32768);

    DWORD dwNumRead;
    do
    {
        if (ReadFile( hSrc, vectorptr(buffer), (DWORD)buffer.size(), &dwNumRead, NULL))
        {
            DWORD dwNumWritten;
            if (!CeWriteFile( hDest, vectorptr(buffer), dwNumRead, &dwNumWritten, NULL))
            {
                g_err.Set("Error !!! Writing WinCE file");
                CeCloseHandle( hDest);
                CloseHandle (hSrc);
                return false;
            }
        }
        else
        {
            g_err.Set("Error !!! Reading source file");
            CeCloseHandle( hDest);
            CloseHandle (hSrc);
            return false;
        }
    } while (dwNumRead);
    CeCloseHandle( hDest);
    CloseHandle (hSrc);
    return true;
}
