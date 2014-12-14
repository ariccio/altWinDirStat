/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"

#include <winioctl.h>

#include "DiskInfo.h"
#include "Registry.h"
#include "SmartHandle.h"
#include "stringutils.h"
#include "debug.h"
#include "setupapi.h"
#include "devguid.h"

#include <string>

DiskInfo::DiskInfo(char drv, const std::string& description, int disknr, const std::string& physical, DWORD sectorsize, DWORD totalsectors, const std::string& sizestring)
	: drv(drv), description(description), disknr(disknr), physical(physical), sectorsize(sectorsize), totalsectors(totalsectors), sizestring(sizestring)
{
}


bool DeviceIoControl(HANDLE fh, DWORD dwIoControlCode, const ByteVector& inbuf, ByteVector& outbuf)
{
	DWORD cb;
	if (!DeviceIoControl(fh, dwIoControlCode, 
		inbuf.size()?const_cast<BYTE*>(vectorptr(inbuf)):NULL, (DWORD)inbuf.size(), 
		outbuf.size()?vectorptr(outbuf):NULL, (DWORD)outbuf.size(), &cb, NULL))
    {
        if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
			return false;

		outbuf.resize(cb);

        if (!DeviceIoControl(fh, dwIoControlCode, 
			inbuf.size()?const_cast<BYTE*>(vectorptr(inbuf)):NULL, (DWORD)inbuf.size(), 
			outbuf.size()?vectorptr(outbuf):NULL, (DWORD)outbuf.size(), &cb, NULL))
		{
            return false;
		}
    }
	return true;
}

typedef enum _STORAGE_PROPERTY_ID {
  StorageDeviceProperty = 0,
  StorageAdapterProperty,
  StorageDeviceIdProperty
} STORAGE_PROPERTY_ID;
typedef enum _STORAGE_QUERY_TYPE {
  PropertyStandardQuery = 0,
  PropertyExistsQuery,
  PropertyMaskQuery,
  PropertyQueryMaxDefined
} STORAGE_QUERY_TYPE;
	typedef struct _STORAGE_PROPERTY_QUERY {
		STORAGE_PROPERTY_ID  PropertyId;
		STORAGE_QUERY_TYPE  QueryType;
		UCHAR  AdditionalParameters[1];
	} STORAGE_PROPERTY_QUERY;
typedef struct _STORAGE_DEVICE_DESCRIPTOR {
  ULONG  Version;
  ULONG  Size;
  UCHAR  DeviceType;
  UCHAR  DeviceTypeModifier;
  BOOLEAN  RemovableMedia;
  BOOLEAN  CommandQueueing;
  ULONG  VendorIdOffset;
  ULONG  ProductIdOffset;
  ULONG  ProductRevisionOffset;
  ULONG  SerialNumberOffset;
  STORAGE_BUS_TYPE  BusType;
  ULONG  RawPropertiesLength;
  UCHAR  RawDeviceProperties[1];
} STORAGE_DEVICE_DESCRIPTOR;
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

bool GetDeviceDescriptor(HANDLE fh, string& descriptor)
{
	ByteVector propquery; propquery.resize(sizeof(STORAGE_PROPERTY_QUERY));
	STORAGE_PROPERTY_QUERY *pq= (STORAGE_PROPERTY_QUERY*)vectorptr(propquery);
	pq->PropertyId= StorageDeviceProperty;
	pq->QueryType= PropertyStandardQuery;
	ByteVector proprpy; proprpy.resize(1024);
    if (!DeviceIoControl(fh, IOCTL_STORAGE_QUERY_PROPERTY, propquery, proprpy))
		return false;
	BYTE *p= vectorptr(proprpy);
	STORAGE_DEVICE_DESCRIPTOR *dd= (STORAGE_DEVICE_DESCRIPTOR *)vectorptr(proprpy);

	descriptor=string((char*)&p[dd->ProductIdOffset]);
	return true;
}

/*
bool DiskInfo::GetPhysicalDrive(const std::string& logicaldevice, DWORD& disknr)
{
    DWORD cb;
    SmartHandle fh= CreateFile(logicaldevice.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (fh==NULL || fh==INVALID_HANDLE_VALUE)
    {
        return false;
    }
    STORAGE_DEVICE_NUMBER devnr;

    if (!DeviceIoControl(fh, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &devnr, sizeof(STORAGE_DEVICE_NUMBER), &cb, NULL))
		return false;
	if (devnr.DeviceType!=FILE_DEVICE_DISK)
		return false;
	disknr= devnr.DeviceNumber;
	return true;
}
*/
bool DiskInfo::GetPhysicalName(DWORD disknr, std::string& name)
{
	name= stringformat("\\\\.\\PhysicalDrive%d", disknr);
	return true;
}
bool DiskInfo::GetLogicalName(char drv, std::string& name)
{
	name= stringformat("\\\\.\\%c:", drv);
	return true;
}
/*
bool DiskInfo::GetDiskInfo(int disknr, DISK_GEOMETRY& geometry, string& description)
{
	string devnam;
	GetDeviceName(disknr, devnam);
    SmartHandle fh= CreateFile(devnam.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (fh==NULL || fh==INVALID_HANDLE_VALUE)
        return false;

	if (!GetDeviceDescriptor(fh, description))
		description= devnam;

	DWORD cb;
	return FALSE!=DeviceIoControl(fh, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geometry, sizeof(geometry), &cb, NULL);
}
*/
void DiskInfo::GetDiskList(DiskList& list)
{
	DWORD cb;
	for (char drv='A' ; drv<='Z' ; drv++)
	{
		string logical;
		GetLogicalName(drv, logical);
		SmartHandle fh= CreateFile(logical.c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		if (fh==NULL || fh==INVALID_HANDLE_VALUE)
			continue;

		string description;
		if (!GetDeviceDescriptor(fh, description))
			continue;

		STORAGE_DEVICE_NUMBER devnr;
		if (!DeviceIoControl(fh, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &devnr, sizeof(STORAGE_DEVICE_NUMBER), &cb, NULL))
			continue;
		if (devnr.DeviceType!=FILE_DEVICE_DISK)
			continue;
		DWORD disknr= devnr.DeviceNumber;
		string physical;
		GetPhysicalName(disknr, physical);
		DISK_GEOMETRY geometry;
		if (!DeviceIoControl(fh, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geometry, sizeof(geometry), &cb, NULL))
			continue;
		if (geometry.MediaType!=RemovableMedia)
			continue;

		double size= (double)geometry.BytesPerSector*geometry.Cylinders.QuadPart*geometry.SectorsPerTrack*geometry.TracksPerCylinder;
		string sizestring= 
			(size>1024.0*1024*1024*1024)? stringformat("%6.2fT", size/(1024.0*1024*1024*1024))
			: (size>1024.0*1024*1024)? stringformat("%6.2fG", size/(1024.0*1024*1024))
			: (size>1024.0*1024)? stringformat("%6.2fM", size/(1024.0*1024))
			: (size>1024.0)? stringformat("%6.2fk", size/1024.0)
			: stringformat("%6.2f", size);

		list.push_back(new DiskInfo(drv, description, disknr, physical, 
			geometry.BytesPerSector, 
			size>4e9?0xffffffff:(DWORD)(geometry.Cylinders.QuadPart*geometry.SectorsPerTrack*geometry.TracksPerCylinder), 
			sizestring));
	}
}

#if 0
bool DiskInfo::DumpRegistry()
{
	debug("--------------disks from registry\n");
	DWORD count;
	if (!GetNumberOfDisks(count))
		return false;
	for (DWORD regid=0 ; regid<count ; regid++)
	{
		string enumkey;
		Registry::GetString(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", stringformat("%d", regid), enumkey);
		string friendlyname;
		Registry::GetString(HKEY_LOCAL_MACHINE, stringformat("SYSTEM\\CurrentControlSet\\Enum\\%s", enumkey.c_str()), "FriendlyName", friendlyname);
		string driver;
		Registry::GetString(HKEY_LOCAL_MACHINE, stringformat("SYSTEM\\CurrentControlSet\\Enum\\%s", enumkey.c_str()), "Driver", driver);

		debug("%2d : %s : %s\n", regid, driver.substr(driver.size()-4).c_str(), friendlyname.c_str());
	}
	return true;
}
bool DiskInfo::DumpPhysicalDrives()
{
	debug("--------------disks from geometry\n");
	for (int drvid=0 ; drvid<16 ; drvid++)
	{
		DISK_GEOMETRY geom;
		if (GetGeometry(drvid, geom))
		{
			debug("%2d : %d\n", drvid, (geom.BytesPerSector/512)*geom.Cylinders.QuadPart*geom.SectorsPerTrack*geom.TracksPerCylinder/2);
		}
	}
	return true;
}


bool DiskInfo::DumpExtents(const std::string& volume)
{
    DWORD cb;
    SmartHandle fh= CreateFile(volume.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (fh==NULL || fh==INVALID_HANDLE_VALUE)
    {
        return false;
    }
    STORAGE_DEVICE_NUMBER devnr;

    if (DeviceIoControl(fh, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &devnr, sizeof(STORAGE_DEVICE_NUMBER), &cb, NULL))
    {
		debug("drive %s: type=%08lx devnr=%08lx partnr=%08lx\n", volume.c_str(), devnr.DeviceType, devnr.DeviceNumber, devnr.PartitionNumber);

		DISK_GEOMETRY geom;
		if (devnr.DeviceType==FILE_DEVICE_DISK && GetGeometry(devnr.DeviceNumber, geom))
		{
			double size= (double)geom.BytesPerSector*geom.Cylinders.QuadPart*geom.SectorsPerTrack*geom.TracksPerCylinder;
			if (size>1024.0*1024*1024) debug("    size=%6.2fG\n", size/(1024.0*1024*1024));
			else if (size>1024.0*1024) debug("    size=%6.2fM\n", size/(1024.0*1024));
			else if (size>1024.0) debug("    size=%6.2fk\n", size/1024.0);
			else debug("   size=%6.2f", size);
		}
	}
	string desc;
	if (GetDeviceDescriptor(fh, desc))
		debug("drive %s:  %s\n", volume.c_str(), desc.c_str());

	ByteVector volext; volext.resize(sizeof(VOLUME_DISK_EXTENTS)+15*sizeof(DISK_EXTENT));
    if (DeviceIoControl(fh, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, ByteVector(), volext))
    {
		const VOLUME_DISK_EXTENTS *pVolExt= (VOLUME_DISK_EXTENTS *)vectorptr(volext);
		string extbuf; extbuf.reserve(16+pVolExt->NumberOfDiskExtents*32);
		const DISK_EXTENT *pDiskExt= pVolExt->Extents;
		for (DWORD i=0 ; i<pVolExt->NumberOfDiskExtents ; i++)
		{
			extbuf += stringformat("  disk%d %08lx (%08lx)", pDiskExt->DiskNumber, pDiskExt->StartingOffset.LowPart, pDiskExt->ExtentLength.LowPart);
			pDiskExt++;
		}
		debug("drive %s: %s\n", volume.c_str(), extbuf.c_str());
	}

	return true;
}

bool DiskInfo::DumpLogicalDrives()
{
	debug("--------------logical disks\n");
	for (char drv='A' ; drv<='Z' ; drv++)
	{
        DumpExtents(stringformat("\\\\.\\%c:", drv));
	}

	return true;
}

bool GetDeviceRegistryProperty(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevdata, DWORD dwProp, DWORD& dwType, ByteVector& value)
{
	value.resize(0);
    DWORD cb;
    if (!SetupDiGetDeviceRegistryProperty(hDevInfo, pDevdata, dwProp, &dwType, NULL, 0, &cb))
    {
		if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
			return false;

		value.resize(cb);

		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, pDevdata, dwProp, &dwType, vectorptr(value), (DWORD)value.size(), &cb))
            return false;
    }

	return true;
}
void EnumDeviceInterfaces(HDEVINFO hDevInfo, const GUID *pGuid, SP_DEVINFO_DATA* pDevdata);
void EnumDriverInfo(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevdata);

bool DumpDeviceInfo(DWORD index, HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevdata)
{
	debug("-----DumpDeviceInfo %d %08lx {\n", index, pDevdata->DevInst);

	string devid; devid.resize(256);
	DWORD cb;
	if (SetupDiGetDeviceInstanceId(hDevInfo, pDevdata, vectorptr(devid), (DWORD)devid.size(), &cb))
	{
		devid.resize(cb);
		debug("device instance id= %s\n", devid.c_str());
	}

	for (DWORD prop= 0 ; prop<SPDRP_MAXIMUM_PROPERTY ; prop++)
	{
		DWORD type;
		ByteVector value;
		if (GetDeviceRegistryProperty(hDevInfo, pDevdata, prop, type, value))
		{
			debug("%02x: %s\n", prop, Registry::ValueToString(type, value).c_str());
/*
			if (prop==SPDRP_PHYSICAL_DEVICE_OBJECT_NAME)
			{
				FILE_OBJECT fileobj;
				DEVICE_OBJECT devobj;
				wstring dosname; dosname.resize(1024);
				if (STATUS_SUCCESS==IoGetDeviceObjectPointer(ToWString(Registry::ValueToString(type, value)).c_str(), FILE_READ_DATA, &fileobj, &devobj)
					&& STATUS_SUCCESS==RtlVolumeDeviceToDosName(dev, stringptr(dosname)))
					debug("name=%ls\n", dosname.c_str());
			}
*/
		}
	}

	HKEY hkeydev=SetupDiOpenDevRegKey(hDevInfo, pDevdata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
	if (hkeydev!=INVALID_HANDLE_VALUE)
	{
		Registry::DumpKey(hkeydev);
		RegCloseKey(hkeydev);
	}

	HKEY hkeydrv=SetupDiOpenDevRegKey(hDevInfo, pDevdata, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
	if (hkeydrv!=INVALID_HANDLE_VALUE)
	{
		Registry::DumpKey(hkeydrv);
		RegCloseKey(hkeydrv);
	}

	debug(".............ifs disk\n");
	EnumDeviceInterfaces(hDevInfo, &GUID_DEVINTERFACE_DISK, pDevdata);
	debug(".............ifs vol\n");
	EnumDeviceInterfaces(hDevInfo, &GUID_DEVINTERFACE_VOLUME, pDevdata);

	EnumDriverInfo(hDevInfo, pDevdata);

	debug("}\n");
	return true;
}

void EnumDeviceInfo(HDEVINFO hDevInfo)
{
	debug("----------EnumDeviceInfo {\n");
	SP_DEVINFO_DATA devdata;
	int i=0;
	while (true)
	{
		devdata.cbSize= sizeof(SP_DEVINFO_DATA);
		if (!SetupDiEnumDeviceInfo(hDevInfo, i, &devdata))
		{
			if (GetLastError()==ERROR_NO_MORE_ITEMS)
				break;
		}
		else {
			DumpDeviceInfo(i, hDevInfo, &devdata);
		}

		i++;
	}
	debug("}\n");
}
bool DumpDeviceInterface(DWORD index, HDEVINFO hDevInfo, SP_DEVICE_INTERFACE_DATA* pIfdata)
{
	debug("DumpDeviceInterface %d %08lx {\n", index, pIfdata->Flags);

	ByteVector detail; detail.resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA)+1024);
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetail= (SP_DEVICE_INTERFACE_DETAIL_DATA*)vectorptr(detail);
	pDetail->cbSize= (DWORD)detail.size();

	DWORD cb;
	//SP_DEVINFO_DATA devinfo;
	if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, pIfdata, (SP_DEVICE_INTERFACE_DETAIL_DATA*)vectorptr(detail), (DWORD)detail.size(), &cb, NULL))
	{
		error("SetupDiGetDeviceInterfaceDetail(%d)", index);
		debug("}\n");
		return false;
	}
	debug("   ifdetail: %ls\n", pDetail->DevicePath);

	HKEY hkey= SetupDiOpenDeviceInterfaceRegKey(hDevInfo, pIfdata, 0, KEY_READ);
	if (hkey!=INVALID_HANDLE_VALUE)
		Registry::DumpKey(hkey);
    debug("}\n");
	return true;
}

void EnumDeviceInterfaces(HDEVINFO hDevInfo, const GUID *pGuid, SP_DEVINFO_DATA* pDevdata)
{
	debug("----------EnumDeviceInterfaces {\n");
	int i=0;
	SP_DEVICE_INTERFACE_DATA ifdata;
	while (true)
	{
		ifdata.cbSize= sizeof(SP_DEVICE_INTERFACE_DATA);
		if (!SetupDiEnumDeviceInterfaces(hDevInfo, pDevdata, pGuid, i, &ifdata))
		{
			if (GetLastError()==ERROR_NO_MORE_ITEMS)
				break;
		}
		else {
			DumpDeviceInterface(i, hDevInfo, &ifdata);
		}
		i++;
	}
	debug("}\n");
}
bool DumpDriverInfo(DWORD index, HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevdata, SP_DRVINFO_DATA* pDrvInfo)
{
	debug("DumpDriverInfo %d '%ls' %08lx '%ls' '%ls' {\n", index,
		pDrvInfo->Description, pDrvInfo->DriverType, pDrvInfo->MfgName,
		pDrvInfo->ProviderName);

	ByteVector detail; detail.resize(sizeof(SP_DRVINFO_DETAIL_DATA)+1024);
	SP_DRVINFO_DETAIL_DATA *pDetail= (SP_DRVINFO_DETAIL_DATA*)vectorptr(detail);
	pDetail->cbSize= (DWORD)detail.size();
	DWORD cb;
	if (!SetupDiGetDriverInfoDetail(hDevInfo, pDevdata, pDrvInfo, (SP_DRVINFO_DETAIL_DATA*)vectorptr(detail), (DWORD)detail.size(), &cb))
	{
		error("SetupDiGetDriverInfoDetail(%d)", index);
		debug("}\n");
		return false;
	}

	debug("drv detail: %08lxL%08lx %08lx %s %s %s %s\n",
		pDetail->CompatIDsOffset, pDetail->CompatIDsLength,  pDetail->Reserved,
		pDetail->SectionName, pDetail->InfFileName, pDetail->DrvDescription, pDetail->HardwareID);
	debug("}\n");
	return true;
}

void EnumDriverInfo(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pDevdata)
{
	debug("----------EnumDriverInfo {\n");
	int i=0;
	SP_DRVINFO_DATA drvinfo;
	DWORD dwType= (pDevdata==NULL) ? SPDIT_CLASSDRIVER : SPDIT_COMPATDRIVER;
	while (true)
	{
		drvinfo.cbSize= sizeof(SP_DRVINFO_DATA);
		if (!SetupDiEnumDriverInfo(hDevInfo, pDevdata, dwType, i, &drvinfo))
		{
			if (GetLastError()==ERROR_NO_MORE_ITEMS)
				break;
		}
		else {
			DumpDriverInfo(i, hDevInfo, pDevdata, &drvinfo);
		}
		i++;
	}
	debug("}\n");
}
bool DumpDeviceInfoListDetail(HDEVINFO hDevInfo, SP_DEVINFO_LIST_DETAIL_DATA  *pDetail)
{
	debug("DumpDeviceInfoListDetail - not implemented yet\n");

	return true;
}
bool DiskInfo::DumpDeviceClass(const GUID *pGuid)
{
	debug("--------------------dump via setupapi\n");

    HDEVINFO hDevs= SetupDiGetClassDevsEx(pGuid, NULL, NULL, DIGCF_PRESENT, NULL, NULL, NULL);
	SP_DEVINFO_LIST_DETAIL_DATA  detail;
	if (SetupDiGetDeviceInfoListDetail(hDevs, &detail))
		DumpDeviceInfoListDetail(hDevs, &detail);
	debug("-----------------------------------------EnumDeviceInfo\n");
	EnumDeviceInfo(hDevs);

	debug("-----------------------------------------GUID_DEVINTERFACE_DISK\n");
	EnumDeviceInterfaces(hDevs, &GUID_DEVINTERFACE_DISK, NULL);
	debug("-----------------------------------------GUID_DEVINTERFACE_VOLUME\n");
	EnumDeviceInterfaces(hDevs, &GUID_DEVINTERFACE_VOLUME, NULL);

	debug("-----------------------------------------\n");
	EnumDriverInfo(hDevs, NULL);

	SetupDiDestroyDeviceInfoList(hDevs);

	return true;
}
bool DiskInfo::DumpSetup()
{
	DumpDeviceClass(&GUID_DEVCLASS_VOLUME);
	DumpDeviceClass(&GUID_DEVCLASS_DISKDRIVE);

	return true;
}
#endif
