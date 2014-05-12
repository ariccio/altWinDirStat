// ------------------------------------------------------------------------------------------------
// FileSystem utility class used to access file system information.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------

#include "FsUtil.h"
#include "BaseTypes.h"
#include <sstream>

#define IOCTL_VOLUME_LOGICAL_TO_PHYSICAL \
        CTL_CODE( IOCTL_VOLUME_BASE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_VOLUME_PHYSICAL_TO_LOGICAL \
        CTL_CODE( IOCTL_VOLUME_BASE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------
wchar_t FsUtil::GetDriveLetter(const wchar_t* path)
{
    if (wcscspn(path, L":") == 1)
        return path[0];

    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(ARRAYSIZE(currentDir), currentDir);
    return currentDir[0];
}

// ------------------------------------------------------------------------------------------------
DWORD FsUtil::GetDriveAndPartitionNumber(const wchar_t* volumeName, unsigned& phyDrvNum, unsigned& partitionNum)
{
    Hnd volumeHandle = CreateFile(
        volumeName,                     // "\\\\.\\C:";
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (!volumeHandle.IsValid())
        return GetLastError();

    struct STORAGE_DEVICE_NUMBER 
    {
        DEVICE_TYPE DeviceType;
        ULONG       DeviceNumber;
        ULONG       PartitionNumber;
    };

    STORAGE_DEVICE_NUMBER storage_device_number;
    DWORD dwBytesReturned;

    if (!DeviceIoControl(
            volumeHandle,
            IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL,
            0,
            &storage_device_number,
            sizeof(storage_device_number),
            &dwBytesReturned,
            NULL))
    {
        return GetLastError();
    }

    phyDrvNum = storage_device_number.DeviceNumber;
    partitionNum = storage_device_number.PartitionNumber - 1;   // appears to one based, so shift down one.

    return ERROR_SUCCESS;
}

#if 0
// Not used
// ------------------------------------------------------------------------------------------------

DWORD FsUtil::GetNtfsDiskNumber(const wchar_t* volumeName, int& diskNumber)
{
    Hnd volumeHandle = CreateFile(
        volumeName,                     // "\\\\.\\C:";
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (!volumeHandle.IsValid())
        return GetLastError();

    // if (strcmp(szFileSystemName, "NTFS") == 0)
    {
        // Volume logical offset  
        struct VOLUME_LOGICAL_OFFSET 
        {
            LONGLONG    LogicalOffset;
        };

        // Volume physical offset 
        struct VOLUME_PHYSICAL_OFFSET 
        {
            ULONG       DiskNumber;
            LONGLONG    Offset;
        };

        // Volume physical offsets 
        struct VOLUME_PHYSICAL_OFFSETS 
        {
            ULONG                   NumberOfPhysicalOffsets;
            VOLUME_PHYSICAL_OFFSET  PhysicalOffset[10];  // ANYSIZE_ARRAY];
        };

        VOLUME_LOGICAL_OFFSET   volumeLogicalOffset;
        VOLUME_PHYSICAL_OFFSETS volumePhysicalOffsets;
        LONGLONG logicalOffset = 0; // lpRetrievalPointersBuffer->Extents [0].Lcn.QuadPart * dwClusterSizeInBytes;
        DWORD dwBytesReturned;
        ZeroMemory(&volumePhysicalOffsets, sizeof(volumePhysicalOffsets));
        volumePhysicalOffsets.PhysicalOffset[0].DiskNumber = 123;

        volumeLogicalOffset.LogicalOffset = logicalOffset;
        if (!DeviceIoControl(
            volumeHandle,
            IOCTL_VOLUME_LOGICAL_TO_PHYSICAL,
            &volumeLogicalOffset,
            sizeof(VOLUME_LOGICAL_OFFSET),
            &volumePhysicalOffsets,
            sizeof(volumePhysicalOffsets),
            &dwBytesReturned,
            NULL))
        {
            return GetLastError();
        }

        diskNumber = volumePhysicalOffsets.PhysicalOffset[0].DiskNumber;
        return ERROR_SUCCESS;
    }

    return ERROR_BAD_UNIT;  // not NTFS file system.
}
#endif

// ------------------------------------------------------------------------------------------------
/// This function is from vinoj kumar's article forensic in codeguru

DWORD FsUtil::GetLogicalDrives(const wchar_t* phyDrv, DiskInfoList& diskInfoList, FsBits whichFs)
{
    int patIdx, nRet;

    Hnd hDrive = CreateFile(phyDrv, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive == INVALID_HANDLE_VALUE)
        return GetLastError();

    DWORD dwBytes;
    const unsigned sSectorSize = 512;
    BYTE szSector[sSectorSize];
    nRet = ReadFile(hDrive, szSector, sSectorSize, &dwBytes, 0);
    if (!nRet)
    {
        return GetLastError();
    }

    DWORD dwMainPrevRelSector = 0;
    DWORD dwPrevRelSector     = 0;

    Partition*  pPartition = (Partition*)(szSector + 0x1BE);
    // int partSize = sizeof(Partition);
    DiskInfo diskInfo;

    for (patIdx = 0; patIdx < 4; patIdx++) /// scanning partitions in the physical disk
    {
        diskInfo.wCylinder    = pPartition->chCylinder;
        diskInfo.wHead        = pPartition->chHead;
        diskInfo.wSector      = pPartition->chSector;
        diskInfo.dwNumSectors = pPartition->dwNumberSectors;
        diskInfo.wType        = 
                ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X)) ? 
                EXTENDED_PART:BOOT_RECORD;

        if ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X))
        {
            dwMainPrevRelSector			    = pPartition->dwRelativeSector;
            diskInfo.dwNTRelativeSector	= dwMainPrevRelSector;
        }
        else
        {
            diskInfo.dwNTRelativeSector = dwMainPrevRelSector + pPartition->dwRelativeSector;
        }

        if (diskInfo.wType == EXTENDED_PART)
            break;

        if (pPartition->chType == 0)
            break;

        switch (pPartition->chType)
        {
        case PART_DOS2_FAT: // FAT12
            if ((whichFs & eFsDOS12) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_DOSX13:
        case PART_DOS4_FAT:
        case PART_DOS3_FAT:
            if ((whichFs & eFsDOS16) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_DOS32X:
        case PART_DOS32:
            if ((whichFs & eFsDOS32) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_NTFS:  
            if ((whichFs & eFsNTFS) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        default: // Unknown
            if (whichFs == eFsALL)
                diskInfoList.push_back(diskInfo);
            break;
        }

        pPartition++;
    }

    if (patIdx == 4)
        return ERROR_SUCCESS;

    for (int LogiHard = 0; LogiHard < 50; LogiHard++) // scanning extended partitions
    {
        if (diskInfo.wType == EXTENDED_PART)
        {
            LARGE_INTEGER n64Pos;

            n64Pos.QuadPart = ((LONGLONG) diskInfo.dwNTRelativeSector) * 512;

            nRet = SetFilePointer(hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
            if (nRet == 0xffffffff)
                return GetLastError();;

            dwBytes = 0;

            nRet = ReadFile(hDrive, szSector, 512, (DWORD *) &dwBytes, NULL);
            if (!nRet)
                return GetLastError();

            if (dwBytes != 512)
                return ERROR_READ_FAULT;

            pPartition = (Partition*) (szSector+0x1BE);

            for (patIdx = 0; patIdx < 4; patIdx++)
            {
                diskInfo.wCylinder = pPartition->chCylinder;
                diskInfo.wHead = pPartition->chHead;
                diskInfo.dwNumSectors = pPartition->dwNumberSectors;
                diskInfo.wSector = pPartition->chSector;
                diskInfo.dwRelativeSector = 0;
                diskInfo.wType = ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X)) ? EXTENDED_PART:BOOT_RECORD;

                if ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X))
                {
                    dwPrevRelSector = pPartition->dwRelativeSector;
                    diskInfo.dwNTRelativeSector = dwPrevRelSector + dwMainPrevRelSector;
                }
                else
                {
                    diskInfo.dwNTRelativeSector = dwMainPrevRelSector + dwPrevRelSector + pPartition->dwRelativeSector;
                }

                if (diskInfo.wType == EXTENDED_PART)
                    break;

                if (pPartition->chType == 0)
                    break;

                switch(pPartition->chType)
                {
                case PART_DOS2_FAT: // FAT12
                    if ((whichFs & eFsDOS12) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case PART_DOSX13:
                case PART_DOS4_FAT:
                case PART_DOS3_FAT:
                    if ((whichFs & eFsDOS16) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case PART_DOS32X:
                case PART_DOS32:
                    if ((whichFs & eFsDOS32) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case 7: // NTFS
                    if ((whichFs & eFsNTFS) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                default: // Unknown
                    break;
                }

                pPartition++;
            }

            if (patIdx == 4)
                break;
        }
    }

    return ERROR_SUCCESS;
}
