/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __CONFIGURATION_H__

#include <string>

class Configuration {
public:
    Configuration();
    ~Configuration();

	bool RegisterSelf();

    std::string ProgrammeApath;
    std::string ApplicationRoot;
    std::string BootloaderName;
    std::string ProgrammeAcommand;

	bool bCheckFilesize;
};

extern Configuration g_cfg;
#define __CONFIGURATION_H__
#endif
