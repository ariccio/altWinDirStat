/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "registry.h"
#include <windows.h>
#include "debug.h"

#include <string>
#include <vector>
#include "vectorutils.h"
#include "stringutils.h"

// ------------ reading from the registry ------------
bool Registry::GetValue(HKEY hive, const std::string& path, const std::string& key, ByteVector& value, DWORD& type)
{
    HKEY hk;
    // doc says it needs a 'class' string, everybody else says class don't exist.
    if (ERROR_SUCCESS!=RegOpenKeyEx(hive, path.c_str(), 0, KEY_READ, &hk))
    {
        error("GetRegistryString - RegOpenKeyEx(%08lx, %s)", hive, path.c_str());
        return false;
    }

    DWORD vallen= 0;
    if (ERROR_SUCCESS!=RegQueryValueEx(hk, key.c_str(), 0, NULL, NULL, &vallen))
    {
        error("RegQueryValueEx");
        return false;
    }
    value.resize(vallen);
    if (ERROR_SUCCESS!=RegQueryValueEx(hk, key.c_str(), 0, &type, vectorptr(value), &vallen))
    {
        error("RegQueryValueEx");
        return false;
    }
    RegCloseKey(hk);
    return true;
}

bool Registry::GetString(HKEY hive, const std::string& path, const std::string& key, std::string& value)
{
    ByteVector buffer;
    DWORD type;
    if (!GetValue(hive, path, key, buffer, type))
        return false;
    if (type!=REG_SZ)
        return false;
    return ValueToString(buffer, value);
}

bool Registry::GetDword(HKEY hive, const std::string& path, const std::string& key, DWORD& value)
{
    ByteVector buffer;
    DWORD type;
    if (!GetValue(hive, path, key, buffer, type))
        return false;
    if (type!=REG_DWORD)
        return false;

    return ValueToDword(buffer, value);
}

bool Registry::GetStringList(HKEY hive, const std::string& path, const std::string& key, StringList& value)
{
    ByteVector buffer;
    DWORD type;
    if (!GetValue(hive, path, key, buffer, type))
        return false;
    if (type!=REG_MULTI_SZ)
        return false;
    return ValueToStringList(buffer, value);
}

// --------- typed value to specific type conversions
bool Registry::ValueToStringList(const ByteVector& value, StringList& list)
{
    ByteVector::const_iterator str_start= value.begin();
    for (ByteVector::const_iterator i= value.begin() ; i!=value.end() ; ++i)
    {
        if (*i==0)
        {
            list.push_back(string(str_start, i));
            str_start= i+1;
        }
    }
    if (str_start!=value.end())
    {
        list.push_back(string(str_start, value.end()));
        str_start= value.end();
    }
    return true;
}
bool Registry::ValueToDword(const ByteVector& value, DWORD& dw)
{
    dw= value[0]|(value[1]<<8)|(value[2]<<16)|(value[3]<<24);
    return true;
}
bool Registry::ValueToString(const ByteVector& value, std::string& str)
{
	str.clear();
	for (ByteVector::const_iterator i=value.begin() ; i!=value.end() && (*i)!=0 ; ++i)
	{
		str += (char)(*i);
	}
    return true;
}


// ------------ writing to the registry --------------

bool Registry::SetValue(HKEY hive, const std::string& path, const std::string& key, const ByteVector& value, DWORD type)
{
    HKEY hk;
    // doc says it needs a 'class' string, everybody else says class don't exist.
    if (ERROR_SUCCESS!=RegCreateKeyEx(hive, path.c_str(), 0,NULL, 0, KEY_WRITE, NULL, &hk, NULL))
    {
        error("GetRegistryString - RegCreateKeyEx(%08lx, %s)", hive, path.c_str());
        return false;
    }

    if (ERROR_SUCCESS!=RegSetValueEx(hk, key.c_str(), 0, type, vectorptr(value), (DWORD)value.size()))
    {
        error("RegSetValueEx");
        return false;
    }
    RegCloseKey(hk);
    return true;
}

bool Registry::SetString(HKEY hive, const std::string& path, const std::string& key, const std::string& str)
{
	ByteVector value;
	if (!StringToValue(str, value))
		return false;
	return SetValue(hive, path, key, value, REG_SZ);
}

bool Registry::SetDword(HKEY hive, const std::string& path, const std::string& key, DWORD dw)
{
	ByteVector value;
	if (!DwordToValue(dw, value))
		return false;
	return SetValue(hive, path, key, value, REG_DWORD);
}

bool Registry::SetStringList(HKEY hive, const std::string& path, const std::string& key, const StringList& list)
{
	ByteVector value;
	if (!StringListToValue(list, value))
		return false;
	return SetValue(hive, path, key, value, REG_MULTI_SZ);
}
// --------- specific type to typed value conversions

bool Registry::StringListToValue(const StringList& list, ByteVector& value)
{
	value.clear();
	ByteVector::iterator out= value.begin();
	for (StringList::const_iterator i= list.begin() ; i!=list.end() ; ++i)
	{
		value.reserve(value.size() + (*i).size()+1);
		copy((*i).begin(), (*i).end(), out);
		*out++= 0;
	}
	*out++= 0;
    return true;
}
bool Registry::DwordToValue(DWORD dw, ByteVector& value)
{
	value.resize(4);
	value[0]= (BYTE)dw;  dw>>=8;
	value[1]= (BYTE)dw;  dw>>=8;
	value[2]= (BYTE)dw;  dw>>=8;
	value[3]= (BYTE)dw;

    return true;
}
bool Registry::StringToValue(const std::string& str, ByteVector& value)
{
    value.resize(str.size());
    copy(str.begin(), str.end(), value.begin());
    return true;
}



//----------------debugging stuff
bool Registry::EnumRegKeys(HKEY hkey)
{
	string name; name.resize(1024);
	int i=0;
	while (true)
	{
		DWORD cb= (DWORD)name.size();
		LONG rc= RegEnumKeyEx(hkey, i, stringptr(name), &cb, NULL, NULL, NULL, NULL);
		if (rc==ERROR_NO_MORE_ITEMS)
			break;
		debug("key %s\n", name.c_str());
		i++;
	}
	return true;
}

bool Registry::EnumRegValues(HKEY hkey)
{
	string name; name.resize(1024);
	ByteVector data; data.resize(1024);
	int i=0;
	while (true)
	{
		DWORD cbName= (DWORD)name.size();
		DWORD cbData= (DWORD)data.size();
		DWORD type;
		LONG rc= RegEnumValue(hkey, i, stringptr(name), &cbName, NULL, &type, vectorptr(data), &cbData);
		if (rc==ERROR_NO_MORE_ITEMS)
			break;
		debug("value %s = %s\n", name.c_str(), ValueToString(type, data).c_str());
		i++;
	}
	return true;
}
bool Registry::DumpKey(HKEY hkey)
{
	EnumRegKeys(hkey);
	EnumRegValues(hkey);

	return true;
}

// ------------- debug conversions -------------------------------
string Registry::StringValueToString(const ByteVector& value)
{
    string str;
    if (!ValueToString(value, str))
        return "error";

    return str;
}
string Registry::StringListValueToString(const ByteVector& value)
{
    StringList list;
    if (!ValueToStringList(value, list))
        return "error";

    string result; result.reserve(list.size()*4+value.size());
    for (StringList::iterator i= list.begin() ; i!=list.end(); ++i)
    {
        if (result.size())
            result += ", ";
        result += stringformat("'%s'", (*i).c_str());
    }
    return result;
}
string Registry::DwordValueToString(const ByteVector& value)
{
    DWORD dw;
    if (!ValueToDword(value, dw))
        return "error";
    return stringformat("%08lx", dw);
}
string Registry::ValueToString(DWORD dwType, const ByteVector& value)
{
    switch(dwType)
    {
    case REG_SZ: return StringValueToString(value);
    case REG_MULTI_SZ: return StringListValueToString(value);
    case REG_DWORD: return DwordValueToString(value);
    default:
		return stringformat("hex(%d):%s", dwType, hexdump(vectorptr(value), (int)value.size(), 1).c_str());
    }
}
