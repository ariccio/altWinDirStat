/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "Configuration.h"
#include "Registry.h"
#include <string>
#include "errorhandler.h"
#include "debug.h"

Configuration g_cfg;

#define TOOL_REGPATH "Software\\XDA Developers\\OsImageTool"
#define NBxTYPENAME "XDA OS Image"

char *GetAppPath()
{
    static char appname[1024];
    if (!GetModuleFileName(NULL, appname, 1024))
	{
		g_err.Set(stringformat("unable to get modulename: %s", LastError().c_str()));
        return NULL;
	}

    return appname;
}

Configuration::Configuration()
{
	DebugOutputDebugString();
    Registry::GetString(HKEY_CURRENT_USER, TOOL_REGPATH, "Programme A Path", ProgrammeApath);
    Registry::GetString(HKEY_CURRENT_USER, TOOL_REGPATH, "bootloader", BootloaderName);
    Registry::GetString(HKEY_CURRENT_USER, TOOL_REGPATH, "Programme A Command", ProgrammeAcommand);

	ApplicationRoot= std::string(GetAppPath());
	ApplicationRoot.resize(ApplicationRoot.find_last_of('\\'));

	bCheckFilesize= true;
}

Configuration::~Configuration()
{
}
/*
    Registry::GetString(HKEY_CURRENT_USER, "Software\\XDA Developers\\OsImageWriter", "Programme A Path", programmeApath);
    Registry::GetString(HKEY_CURRENT_USER, "Software\\XDA Developers\\OsImageWriter", "bootloader", m_NB0FileName);
*/
bool Configuration::RegisterSelf()
{
	// file context settings
	Registry::SetString(HKEY_CLASSES_ROOT, ".nb1", "", NBxTYPENAME);
	Registry::SetString(HKEY_CLASSES_ROOT, ".nbf", "", NBxTYPENAME);

	Registry::SetString(HKEY_CLASSES_ROOT, NBxTYPENAME "\\shell", "",
		string("Burn"));

	Registry::SetString(HKEY_CLASSES_ROOT, NBxTYPENAME "\\shell\\Burn\\command", "",
		stringformat("\"%s\" -a -r \"%%1\" -w xdamemory", GetAppPath()));
	Registry::SetString(HKEY_CLASSES_ROOT, NBxTYPENAME "\\shell\\write to SD\\command", "",
		stringformat("\"%s\" -a -r \"%%1\" -w localsdcard", GetAppPath()));

    Registry::SetString(HKEY_CURRENT_USER, TOOL_REGPATH, "Programme A Path", ApplicationRoot);
    Registry::SetString(HKEY_CURRENT_USER, TOOL_REGPATH, "bootloader", ApplicationRoot+"\\bootloader_v5_15.nb0");
    Registry::SetString(HKEY_CURRENT_USER, TOOL_REGPATH, "Programme A Command", ApplicationRoot+"\\start.bat");

	return true;
}
