/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include "SdCardDevice.h"
#include "debug.h"

SdCardDevice::~SdCardDevice()
{
    // ... calling virtual function does not seem to work from
    // ... destructor

    // Close();
}


std::string SdCardDevice::GetFriendlyName()
{
	std::string name;
	GetFriendlyName(name);
	return name;
}
