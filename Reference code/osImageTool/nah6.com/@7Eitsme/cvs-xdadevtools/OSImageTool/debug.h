/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __DEBUG_H__

#include <TCHAR.h>
#include <string>
using namespace std;

void debug(const char *msg, ...);
void error(const char *msg, ...);
void ceerror(const char *msg, ...);
void error(DWORD dwErrorCode, const char *msg, ...);

string hexdump(const BYTE *buf, int nLength, int nDumpUnitSize=1);
string hexdump(DWORD dwOffset, const BYTE *buf, int nLength, int nDumpUnitSize=1, int nMaxUnitsPerLine=16);

class CDebugWindow  {
public:
    virtual void appendString(TCHAR *str)=0;
};

void DebugSetLogfile(const char *filename);
void DebugRegisterWindow(CDebugWindow *wnd);
void DebugMessagebox();
void DebugOutputDebugString();

#define __DEBUG_H__
#endif

