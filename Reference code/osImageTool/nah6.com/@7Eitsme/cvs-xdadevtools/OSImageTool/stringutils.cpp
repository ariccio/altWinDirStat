/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include <windows.h>

#include "stringutils.h"

#include "debug.h"

#include <string>
#include <algorithm>
using namespace std;

string ToString(WCHAR* wbuf)
{
	char *buf= new char[wcslen(wbuf)+1];

	_snprintf(buf, wcslen(wbuf), "%ls", wbuf);

	string str(buf, buf+wcslen(wbuf));
	delete buf;

	return str;
}
wstring ToWString(WCHAR* wbuf)
{
	return wstring(wbuf);
}
string ToString(char* buf)
{
	return string(buf);
}
wstring ToWString(char* buf)
{
	WCHAR *wbuf= new WCHAR[strlen(buf)+1];

	_snwprintf(wbuf, strlen(buf), L"%hs", buf);

	wstring wstr(wbuf, wbuf+strlen(buf));
	delete wbuf;

	return wstr;
}
string ToString(const wstring& wstr)
{
	return ToString((WCHAR*)wstr.c_str());
}
wstring ToWString(const string& str)
{
	return ToWString((char*)str.c_str());
}

// removes cr, lf, whitespace from end of string
void chomp(char *str)
{
	char *p= str+strlen(str)-1;

	while (p>=str && isspace(*p))
	{
		*p--= 0;
	}
}
void chomp(string& str)
{
	for (string::size_type i=str.size()-1 ; i>=0 && isspace(str.at(i)) ; --i)
	{
		str.erase(i);
	}
}

bool SplitString(const string& str, StringList& strlist, bool bWithEscape/*= true*/)
{
	string::const_iterator pos= str.begin();
	bool bQuoted= false;
	bool bEscaped= false;
	string current;

	while (pos != str.end())
	{
		if (bEscaped)
		{
			current += *pos++;
			bEscaped= false;
		}
		else if (bQuoted)
		{
			switch(*pos)
			{
			case '"':
				bQuoted= false;
				strlist.push_back(string(current));
				//debug("added %hs\n", current.c_str());
				current.clear();
				++pos;
				break;
			case '\\':
				if (bWithEscape)
				{
					bEscaped= true;
					++pos;	// skip escaped char
				}
				// else fall through
			default:
				current += *pos++;
			}
		}
		else	// not escaped, and not quoted
		{
			switch(*pos)
			{
			case ' ':
			case '\t':
				++pos;
				if (!current.empty())
				{
					strlist.push_back(string(current));
					//debug("added %hs\n", current.c_str());
					current.clear();
				}
				break;

			case '"':
				bQuoted=true;
				++pos;
				break;
			case '\\':
				if (bWithEscape) {
					bEscaped= true;
					++pos;	// skip escaped char
					break;
				}
				// else fall through
			default:
				current += *pos++;
			}
		}
	}
	if (!current.empty())
	{
		strlist.push_back(string(current));
		//debug("added %hs\n", current.c_str());
		current.clear();
	}
	if (bQuoted || bEscaped)
	{
		debug("ERROR: Unterminated commandline\n");
		return false;
	}

	return true;
}

string stringformat(char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	return string(buf);
}

string tolower(const string& str)
{
	string lstr; 
	lstr.reserve(str.size());
	for (string::const_iterator i=str.begin() ; i!=str.end() ; ++i)
		lstr += tolower(*i);
	return lstr;
}
int stringicompare(const string& a, const string& b)
{
	return stricmp(a.c_str(), b.c_str());
}
