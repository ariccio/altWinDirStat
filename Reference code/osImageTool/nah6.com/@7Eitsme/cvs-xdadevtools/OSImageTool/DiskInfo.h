/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __DISKINFO_H__

#include <string>
#include <vector>
class DiskInfo;
typedef std::vector<DiskInfo*> DiskList;

class DiskInfo {
public:
	DiskInfo(char drv, const std::string& description, int disknr, const std::string& physical, 
		DWORD sectorsize, DWORD totalsectors, const std::string& sizestring);
	static bool GetPhysicalName(DWORD disknr, std::string& name);
	//static bool GetDiskInfo(int disknr, DISK_GEOMETRY& geometry, std::string& description);
	static bool GetLogicalName(char drv, std::string& name);
	static void GetDiskList(DiskList& list);

	char drv;
	std::string description;
	int disknr;
	std::string physical;
	DWORD sectorsize;
	DWORD totalsectors;
	std::string sizestring;

	/*
	static bool GetGeometry(DWORD disknr, DISK_GEOMETRY& geometry);
	static bool GetMediaType(DWORD disknr, MEDIA_TYPE& mediatype);
	static bool GetSectorSize(DWORD disknr, DWORD& size);

	static bool DumpSetup();
	static bool DumpRegistry();
	static bool DumpPhysicalDrives();
	static bool DumpExtents(const std::string& volume);
	static bool DumpLogicalDrives();
	static bool DumpDrives();

	static bool DumpDeviceClass(const GUID *pGuid);
	*/
};

#define __DISKINFO_H__
#endif
