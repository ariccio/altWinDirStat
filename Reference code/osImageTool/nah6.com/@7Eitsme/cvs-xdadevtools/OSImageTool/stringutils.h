/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __STRINGUTILS_H_

#include <tchar.h>

#include <string>
#include <vector>
using namespace std;

typedef vector<string> StringList;

string ToString(TCHAR* tbuf);
wstring ToWString(TCHAR* tbuf);
string ToString(char* buf);
wstring ToWString(char* buf);
string ToString(const wstring& wstr);
wstring ToWString(const string& str);

void chomp(string& str);
void chomp(char *str);
bool SplitString(const string& str, StringList& strlist, bool bWithEscape= true);

string stringformat(char *fmt, ...);

int stringicompare(const string& a, const string& b);
string tolower(const string& str);

#define stringptr(v)  (&(v)[0])

#define __STRINGUTILS_H_
#endif
