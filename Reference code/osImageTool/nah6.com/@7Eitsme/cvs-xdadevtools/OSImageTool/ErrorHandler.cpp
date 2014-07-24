/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "ErrorHandler.h"

#include <string>
#include "stringutils.h"

ErrorHandler g_err;

void ErrorHandler::Display()
{
    if (m_errors.size()==0)
        return;

    std::string all;
    for (StringList::iterator i= m_errors.begin() ; i != m_errors.end() ; ++i)
    {
        if (!all.empty())
            all += "\n";
        all += *i;
    }
    MessageBox(0, all.c_str(), "ERROR", 0);
    m_errors.clear();
}

void ErrorHandler::Set(const std::string& msg)
{
    m_errors.push_back(msg);
}
std::string LastError()
{
	return LastError(GetLastError());
}
std::string LastError(DWORD dwErrorCode)
{
	char *msgbuf;
    int rc= FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, dwErrorCode, 0, (char*) &msgbuf, 0, NULL);
	string result;
	if (rc)
		result= msgbuf;
	else
		result= stringformat("UNKNOWNERROR: 0x%08lx", dwErrorCode);
    LocalFree(msgbuf);
	return result;
}
