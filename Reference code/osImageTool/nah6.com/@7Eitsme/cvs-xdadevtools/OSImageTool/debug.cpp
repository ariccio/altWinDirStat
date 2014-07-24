/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include <windows.h>

#include "debug.h"		// declarations for this file.
#include <string>
#include <rapi.h>

using namespace std;

#include "stringutils.h"

#define DBG_OUTPUTDEBUGSTRING 1
#define DBG_MESSAGEBOX        2
#define DBG_DEBUGWINDOW       4
#define DBG_LOGFILE           8

#define DBG_WCHAROUTPUT (0)
#define DBG_CHAROUTPUT (DBG_LOGFILE|DBG_OUTPUTDEBUGSTRING|DBG_MESSAGEBOX|DBG_DEBUGWINDOW)

int g_debugOutputFlags;
char g_logfilename[MAX_PATH+1];
HANDLE g_log_mutex;
CDebugWindow *g_debugWindow;

#define MAX_DEBUG_LENGTH 16384

void DebugSetLogfile(const char *filename)
{
    g_log_mutex= CreateMutex(NULL, false, NULL);
    strncpy(g_logfilename, filename, MAX_PATH);
    g_logfilename[MAX_PATH]=0;
    g_debugOutputFlags |= DBG_LOGFILE;
}
void DebugRegisterWindow(CDebugWindow *wnd)
{
    g_debugWindow= wnd;

    g_debugOutputFlags |= DBG_DEBUGWINDOW;
}
void DebugMessagebox()
{
    g_debugOutputFlags |= DBG_MESSAGEBOX;
}
void DebugOutputDebugString()
{
    g_debugOutputFlags |= DBG_OUTPUTDEBUGSTRING ;
}
void vwdebug(const WCHAR *msg, va_list ap)
{
	WCHAR *wbuf= new WCHAR[MAX_DEBUG_LENGTH];
	_vsnwprintf(wbuf, MAX_DEBUG_LENGTH, msg, ap);
    if (g_debugOutputFlags&DBG_CHAROUTPUT) {
        debug("%ls", wbuf);
    }

	delete[] wbuf;
}
void wdebug(const WCHAR *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vwdebug(msg, ap);
	va_end(ap);
}

void vdebug(const char *msg, va_list ap)
{
    char *buf = new char[MAX_DEBUG_LENGTH];
    _vsnprintf(buf, MAX_DEBUG_LENGTH, msg, ap);
    if (g_debugOutputFlags&DBG_WCHAROUTPUT) {
        wdebug(L"%hs", buf);
    }

    if (g_debugOutputFlags&DBG_LOGFILE) {
        WaitForSingleObject(g_log_mutex, INFINITE);
        FILE *f= fopen(g_logfilename, "a+");
        fputs(buf, f);
        fclose(f);
        ReleaseMutex(g_log_mutex);
    }

    if (g_debugOutputFlags&DBG_OUTPUTDEBUGSTRING) {
        int len= (int)strlen(buf);
        for (int i= 0 ; i<len ; i+=512)
        {
            char smallbuf[513];
            strncpy(smallbuf, buf+i, 512);
            smallbuf[512]= 0;
            OutputDebugString(smallbuf);
        }
    }
    if (g_debugOutputFlags&DBG_MESSAGEBOX) {
        MessageBox(0,buf,"debug",0);
    }
    if (g_debugOutputFlags&DBG_DEBUGWINDOW) {
        g_debugWindow->appendString(buf);
    }
    delete[] buf;
}

void debug(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vdebug(msg, ap);
	va_end(ap);
}

void verror(DWORD dwErrorCode, const char *msg, va_list ap)
{
    debug("ERROR: ");

	vdebug(msg, ap);

	char *msgbuf;
    int rc= FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, dwErrorCode, 0, (char*) &msgbuf, 0, NULL);
	if (rc)
		debug(" - %s\n", msgbuf);
	else
		debug(" - UNKNOWNERROR: 0x%08lx\n", dwErrorCode);
    LocalFree(msgbuf);
}
void error(DWORD dwErrorCode, const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verror(dwErrorCode, msg, ap);
	va_end(ap);
}

void error(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verror(GetLastError(), msg, ap);
	va_end(ap);
}

void ceerror(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	verror(CeRapiGetError() || CeGetLastError(), msg, ap);
	va_end(ap);
}

int g_nDumpUnitSize=1;
int g_nMaxWordsPerLine=-1;

//#define GETBYTE(p,o)  (((BYTE*)(p))[o])
//#define GETWORD(p,o)  (GETBYTE(p,o)|GETBYTE(p,o+1)<<8)
//#define GETDWORD(p,o)  (GETWORD(p,o)|GETWORD(p,o+2)<<16)

void hexdumpbytes(string &str, const BYTE *buf, int nLength)
{
	str.reserve(str.size()+nLength*3);
    while(nLength--)
	{
		str+= stringformat(" %02x", *buf++);
	}
}
void hexdumpwords(string &str, const WORD *buf, int nLength)
{
	str.reserve(str.size()+nLength*5);
    while(nLength--)
	{
		str+= stringformat(" %04x", *buf++);
	}
}
void hexdumpdwords(string &str, const DWORD *buf, int nLength)
{
	str.reserve(str.size()+nLength*9);
    while(nLength--)
	{
		str+= stringformat(" %08x", *buf++);
	}
}
void dumpascii(string &str, const BYTE *buf, int nLength)
{
    while(nLength--)
    {
        BYTE c= *buf++;
        str += (c>=' ' && c<='~')?c:'.';
    }
}
void writespaces(string &str, int n)
{
    while(n--)
	{
        str += ' ';
	}
}

std::string hexdump(const BYTE *buf, int nLength, int nDumpUnitSize /*=1*/)
{
    int nCharsInResult= nLength*(nDumpUnitSize==1?3:nDumpUnitSize==2?5:nDumpUnitSize==4?9:9);
	std::string line;

    line.reserve(nCharsInResult);

    switch(nDumpUnitSize)
    {
        case 1: hexdumpbytes(line, buf, nLength); break;
        case 2: hexdumpwords(line, (WORD*)buf, nLength); break;
        case 4: hexdumpdwords(line, (DWORD*)buf, nLength); break;
    }
    return line;
}
std::string hexdump(DWORD dwOffset, const BYTE *buf, int nLength, int nDumpUnitSize /*=1*/, int nMaxUnitsPerLine /*=16*/)
{
	int nCharsInLine= 10+nMaxUnitsPerLine*(nDumpUnitSize==1?4:nDumpUnitSize==2?6:nDumpUnitSize==4?10:10);
	int nCharsInResult= nCharsInLine*(nLength/nDumpUnitSize/nMaxUnitsPerLine+1);

	std::string all; all.reserve(nCharsInResult);

    while(nLength>0)
    {
		std::string line;
        // is rounding correct here?
        int nUnitsInLine= min(nLength/nDumpUnitSize, nMaxUnitsPerLine);
		
		line.reserve(nCharsInLine);

		line += stringformat("%08x", dwOffset);

        switch(nDumpUnitSize)
        {
            case 1: hexdumpbytes(line, buf, nUnitsInLine); break;
            case 2: hexdumpwords(line, (WORD*)buf, nUnitsInLine); break;
            case 4: hexdumpdwords(line, (DWORD*)buf, nUnitsInLine); break;
        }

        if (nUnitsInLine<nMaxUnitsPerLine)
            writespaces(line, (nMaxUnitsPerLine-nUnitsInLine)*(2*nDumpUnitSize+1));

        line += "  ";

        dumpascii(line, buf, nUnitsInLine*nDumpUnitSize);
        if (nUnitsInLine<nMaxUnitsPerLine)
            writespaces(line, nMaxUnitsPerLine-nUnitsInLine);

		all += line;
		all += "\n";

        nLength -= nUnitsInLine*nDumpUnitSize;
        dwOffset += nUnitsInLine*nDumpUnitSize;
        buf += nUnitsInLine*nDumpUnitSize;
    }

	return all;
}
