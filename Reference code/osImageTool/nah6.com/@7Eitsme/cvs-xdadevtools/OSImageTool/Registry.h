/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __REGISTRY_H__

#include <windows.h>

#include <string>
#include <vector>
#include "vectorutils.h"
#include "stringutils.h"

class Registry {
public:
    static bool GetValue(HKEY hive, const std::string& path, const std::string& key, ByteVector& value, DWORD& type);
    static bool GetDword(HKEY hive, const std::string& path, const std::string& key, DWORD& value);
    static bool GetString(HKEY hive, const std::string& path, const std::string& key, std::string& value);
    static bool GetStringList(HKEY hive, const std::string& path, const std::string& key, StringList& value);

    static bool ValueToStringList(const ByteVector& value, StringList& list);
    static bool ValueToDword(const ByteVector& value, DWORD& dw);
    static bool ValueToString(const ByteVector& value, std::string& str);

    static bool SetString(HKEY hive, const std::string& path, const std::string& key, const std::string& str);
    static bool SetValue(HKEY hive, const std::string& path, const std::string& key, const ByteVector& value, DWORD type);
    static bool SetDword(HKEY hive, const std::string& path, const std::string& key, DWORD dw);
    static bool SetStringList(HKEY hive, const std::string& path, const std::string& key, const StringList& list);
    static bool StringListToValue(const StringList& list, ByteVector& value);
    static bool DwordToValue(DWORD dw, ByteVector& value);
    static bool StringToValue(const std::string& str, ByteVector& value);


    static string StringValueToString(const ByteVector& value);
    static string StringListValueToString(const ByteVector& value);
    static string DwordValueToString(const ByteVector& value);
    static string ValueToString(DWORD dwType, const ByteVector& value);

	static bool EnumRegKeys(HKEY hkey);
	static bool EnumRegValues(HKEY hkey);
    static bool DumpKey(HKEY hkey);
};
#define __REGISTRY_H__
#endif
