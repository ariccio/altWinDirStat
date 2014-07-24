/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __ERRORHANDLER_H__

#include <string>
#include "stringutils.h"

class ErrorHandler {
public:
    void Display();
    void Set(const std::string& msg);
private:
    StringList m_errors;
};
std::string LastError();
std::string LastError(DWORD dwErrorCode);

extern ErrorHandler g_err;

#define __ERRORHANDLER_H__
#endif
