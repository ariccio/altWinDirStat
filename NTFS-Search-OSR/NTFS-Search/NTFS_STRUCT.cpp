#include "stdafx.h"
#include "windows.h"
#include "commctrl.h"
#include "FixList.h"

// HEAPHeap
PHEAPBLOCK currentBlock=NULL;

//
DWORD GetFilenameFromAttributeList(PDISKHANDLE disk, PATTRIBUTE_LIST attrlist);


//NONRESIDENT_ATTRIBUTE ERROR_ATTRIBUTE = {1,2,3,4,5};
#define CLUSTERSPERREAD 1024
PDISKHANDLE OpenDisk(WCHAR DosDevice)
{
	WCHAR path[8];
	path[0] = L'\\';
	path[1] = L'\\';
	path[2] = L'.';
	path[3] = L'\\';
	path[4] = DosDevice;
	path[5] = L':';
	path[6] = L'\0';
	PDISKHANDLE disk;
	disk = OpenDisk(path);
	if (disk!=NULL)
	{
		disk->DosDevice = DosDevice;
		return disk;
	}
	return NULL;
}

PDISKHANDLE OpenDisk(LPCTSTR disk)
{
	PDISKHANDLE tmpDisk;
	DWORD read;
	tmpDisk = new DISKHANDLE;
	memset(tmpDisk, 0, sizeof(DISKHANDLE));
	tmpDisk->fileHandle = CreateFile(disk, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (tmpDisk->fileHandle != INVALID_HANDLE_VALUE)
	{
		ReadFile(tmpDisk->fileHandle, &tmpDisk->NTFS.bootSector, sizeof(BOOT_BLOCK), &read, NULL);
		if (read==sizeof(BOOT_BLOCK))
		{
			if (strncmp("NTFS",(const char*) &tmpDisk->NTFS.bootSector.Format, 4)==0)
			{
				tmpDisk->type = NTFSDISK;
				tmpDisk->NTFS.BytesPerCluster = tmpDisk->NTFS.bootSector.BytesPerSector * tmpDisk->NTFS.bootSector.SectorsPerCluster;
				tmpDisk->NTFS.BytesPerFileRecord = tmpDisk->NTFS.bootSector.ClustersPerFileRecord < 0x80 ? tmpDisk->NTFS.bootSector.ClustersPerFileRecord * tmpDisk->NTFS.BytesPerCluster: 1 <<(0x100 - tmpDisk->NTFS.bootSector.ClustersPerFileRecord);
				
				tmpDisk->NTFS.complete = FALSE;
				tmpDisk->NTFS.MFTLocation.QuadPart = tmpDisk->NTFS.bootSector.MftStartLcn * tmpDisk->NTFS.BytesPerCluster;
				tmpDisk->NTFS.MFT = NULL;
				tmpDisk->heapBlock = NULL;
				tmpDisk->IsLong = FALSE;
				tmpDisk->NTFS.sizeMFT = 0;
			}
			else
			{
				tmpDisk->type = UNKNOWN;
				tmpDisk->lFiles = NULL;
			}
		}
		return tmpDisk;
	}

	delete tmpDisk;
	return NULL;
};

BOOL CloseDisk(PDISKHANDLE disk)
{
	if (disk!=NULL)
	{
		if (disk->fileHandle>INVALID_HANDLE_VALUE) CloseHandle(disk->fileHandle);
		if (disk->type == NTFSDISK)
		{
			if (disk->NTFS.MFT!=NULL)
				delete disk->NTFS.MFT;
			disk->NTFS.MFT = NULL;
			if (disk->NTFS.Bitmap !=NULL)
				delete disk->NTFS.Bitmap;
			disk->NTFS.Bitmap = NULL;
		}
		if (disk->heapBlock!=NULL)
		{
			FreeHeap(disk->heapBlock);
			disk->heapBlock = NULL;
		}
		if (disk->IsLong)
		{
			if (disk->lFiles!=NULL)
				delete disk->lFiles;
			disk->lFiles = NULL;
		}
		else
		{
			if (disk->sFiles!=NULL)
				delete disk->sFiles;
			disk->sFiles = NULL;
		}
		delete disk;
		return TRUE;
	}
	return FALSE;
};


ULONGLONG LoadMFT(PDISKHANDLE disk, BOOL complete)
{
	DWORD read;
	ULARGE_INTEGER offset;
	UCHAR *buffer;
	PFILE_RECORD_HEADER fh;
	PNONRESIDENT_ATTRIBUTE nattr, nattr2;

	if (disk==NULL)
		return 0;

	if (disk->type==NTFSDISK)
	{
		offset = disk->NTFS.MFTLocation;

		SetFilePointer(disk->fileHandle, offset.LowPart,(PLONG) &offset.HighPart, FILE_BEGIN);
		buffer = new UCHAR[disk->NTFS.BytesPerCluster];
		ReadFile(disk->fileHandle, buffer, disk->NTFS.BytesPerCluster, &read, NULL);

	
		fh = (PFILE_RECORD_HEADER)(buffer);
		
		FixFileRecord(fh);
/*		DWORD at;
		at = EnumAttributes(fh);
*/
		if (fh->Ntfs.Type == 'ELIF')
		{
			nattr = (PNONRESIDENT_ATTRIBUTE) FindAttribute(fh, Data); 
			if (nattr==NULL)
				return 0;
			nattr2 = (PNONRESIDENT_ATTRIBUTE) FindAttribute(fh, Bitmap); 
			if (nattr2==NULL)
				return 0;
		//	disk->NTFS.Bitmap = new UCHAR[nattr2->DataSize];
		//	ReadAttribute(disk, (PATTRIBUTE) nattr2, disk->NTFS.Bitmap);
		
		}
		disk->NTFS.sizeMFT = (DWORD) nattr->DataSize;
			
		if (complete)
		{

			offset.QuadPart = disk->NTFS.bootSector.MftStartLcn * disk->NTFS.BytesPerCluster;

			SetFilePointer(disk->fileHandle, offset.LowPart,(PLONG) &offset.HighPart, FILE_BEGIN);
			disk->NTFS.MFT = new UCHAR[nattr->DataSize];
			//ReadFile(disk->fileHandle, disk->NTFS.MFT , nattr->InitializedSize, &read, NULL);
			ReadAttribute(disk, (PATTRIBUTE) nattr, disk->NTFS.MFT);
			//ReadData(disk, (PATTRIBUTE) nattr, disk->NTFS.MFT);
			
			disk->NTFS.complete = TRUE;
			disk->realFiles = 0;
			delete buffer;
		}
		else // read the first cluster of the MFT
		{
			disk->NTFS.MFT = buffer;
		}
		FixFileRecord((PFILE_RECORD_HEADER)disk->NTFS.MFT);
		disk->NTFS.entryCount = disk->NTFS.sizeMFT / disk->NTFS.BytesPerFileRecord;
		return nattr->DataSize;
	}
	return 0;
};

inline DWORD ReadNextCluster(PDISKHANDLE disk, PVOID buffer)
{ 
	DWORD read;
	ReadFile(disk->fileHandle, buffer, disk->NTFS.BytesPerCluster, &read, NULL);
	return read;
}

PUCHAR FindAttribute(PDISKHANDLE disk, ATTRIBUTE_TYPE type)
{
	return NULL;
}


/*PATTRIBUTE FindAttribute(PFILE_RECORD_HEADER file, ATTRIBUTE_TYPE type)
{
	if (file==NULL)
		return NULL;
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	for (int i=1;
		attr->AttributeType>0 && attr->AttributeType < IndexAllocation; 
		attr = (PATTRIBUTE)((PUCHAR)attr + attr->Length))
	{
		if (attr->AttributeType==type)
			return attr;
		if (attr->Length==0)
			return NULL;
		if (i>file->NextAttributeNumber) 
			return NULL;
	}
	return NULL;
}*/

PATTRIBUTE FindAttribute(PFILE_RECORD_HEADER file, ATTRIBUTE_TYPE type)
{
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);

	for (int i=1;i<file->NextAttributeNumber;i++)
	{
		if (attr->AttributeType==type)
			return attr;
		
		if (attr->AttributeType<1 || attr->AttributeType>0x100) break;
		if (attr->Length>0 && attr->Length < file->BytesInUse)
			attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
		else
			if (attr->Nonresident == TRUE)
				attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
	}
	return NULL;
}

PFILENAME_ATTRIBUTE FindFileName(PFILE_RECORD_HEADER file, USHORT type)
{
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	PFILENAME_ATTRIBUTE fn;

	for (int i=1;i<file->NextAttributeNumber;i++)
	{
		if (attr->AttributeType==FileName)
		{
			fn = PFILENAME_ATTRIBUTE(PUCHAR(attr) + PRESIDENT_ATTRIBUTE(attr)->ValueOffset);
			if (fn->NameType & type || fn->NameType ==0)
				return fn;
		}
		if (attr->AttributeType<1 || attr->AttributeType>0x100) break;
		if (attr->Length>0 && attr->Length < file->BytesInUse)
			attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
		else
			if (attr->Nonresident == TRUE)
				attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
	}
	return NULL;
}

DWORD EnumAttributes(PFILE_RECORD_HEADER file)
{
	int i;
	DWORD data=0;
	UCHAR *buffer = (UCHAR*) &data; 
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	int stop = min(8,file->NextAttributeNumber);
	for (i=0;i<stop;i++)
	{
		if (i%2)
			buffer[i/2] |= (attr->AttributeType & 0xf0);
		else
			buffer[i/2] = (attr->AttributeType >> 4);
		
		if (attr->Length>0 && attr->Length < file->BytesInUse)
			attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
		else
			if (attr->Nonresident == TRUE)
				attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
		if (attr->AttributeType<1 || attr->AttributeType>0x100) break;
		
	}
	return data;
}

PATTRIBUTE FindNextAttribute(PATTRIBUTE att, ATTRIBUTE_TYPE type)
{
	if (att==NULL)
		return NULL;

	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(att) + att->Length);
	for (int i=1;
		attr->AttributeType>0 && attr->AttributeType < IndexAllocation; 
		attr = (PATTRIBUTE)((PUCHAR)attr + attr->Length))
	{
		if (attr->AttributeType==type)
			return attr;
		if (attr->Length==0)
			return NULL;
	}
	return NULL;
}
LPCWSTR GetFileName(PFILE_RECORD_HEADER file)
{
	PRESIDENT_ATTRIBUTE attr;
	attr = (PRESIDENT_ATTRIBUTE) FindAttribute(file, FileName);
	if (attr!=NULL)
	{
		return (LPCWSTR) PFILENAME_ATTRIBUTE(((PUCHAR)(attr) + attr->ValueOffset))->Name;
	}
}

DWORD ParseMFT(PDISKHANDLE disk,UINT option, PSTATUSINFO info)
{
	PUCHAR buffer, end;
	PFILE_RECORD_HEADER fh;
	PRESIDENT_ATTRIBUTE attr;
	PNONRESIDENT_ATTRIBUTE nattr;;
	LONGFILEINFO *data;
	DWORD index=0;

	FETCHPROC fetch;
	fetch = FetchFileInfo;

	if (disk==NULL)
		return 0;

	if (disk->type==NTFSDISK)
	{

		CreateFixList();

		switch (option)
		{
			case LONGINFO:
				disk->IsLong = sizeof(LONGFILEINFO);
				fetch = FetchFileInfo;
				break;
			case SHORTINFO:
				disk->IsLong = sizeof(SHORTFILEINFO);
				fetch = FetchShortFileInfo;
				break;
			case SEARCHINFO:
				disk->IsLong = sizeof(LONGFILEINFO);
				fetch = FetchSearchInfo;
				break;
			case EXTRALONGINFO:
				disk->IsLong = sizeof(LONGFILEINFO);
				fetch = FetchFileInfo;
				break;
			default:
				disk->IsLong = sizeof(LONGFILEINFO);
				fetch = FetchFileInfo;
				break;
		}

		fh = PFILE_RECORD_HEADER(disk->NTFS.MFT);
		FixFileRecord(fh);

		if (disk->NTFS.complete==TRUE)
		{
			buffer	= disk->NTFS.MFT;
			end		= disk->NTFS.MFT + disk->NTFS.sizeMFT;
			
			data = new LONGFILEINFO[disk->filesSize];
			memset(data, 0, disk->filesSize*sizeof(LONGFILEINFO));
			

			DWORD tmp;
			while (buffer<=end)
			{
				
				tmp = (fetch)(disk, fh, (PUCHAR)&data[index]);
				if (tmp>0)
					disk->realFiles++;
				if (index==33624)
					Beep(2500,50);
				index++;
				buffer+= disk->NTFS.BytesPerFileRecord;
			}
			disk->lFiles = data;
		}
		else
		{
			if (disk->heapBlock==NULL)
				disk->heapBlock = CreateHeap(0x100000);
			nattr = (PNONRESIDENT_ATTRIBUTE) FindAttribute(fh, Data); 
			if (nattr!=NULL)
			{
				buffer = new UCHAR[CLUSTERSPERREAD*disk->NTFS.BytesPerCluster];
				ReadMFTParse(disk, nattr, 0, ULONG(nattr->HighVcn)+1, buffer, fetch, info);
				delete buffer;
			}	
		}

		ProcessFixList(disk);
	}


	//disk->filesSize = disk->NTFS.sizeMFT / disk->NTFS.BytesPerFileRecord+1;

	/*LPCWSTR txt;
	txt = data[88604].FileName;
	OutputDebugString(GetPath(disk, 88604));
	*/
	return 0;
}

LPWSTR GetPath(PDISKHANDLE disk, int id)
{
	int a = id;
	int i;
	DWORD pt;
	PUCHAR ptr = (PUCHAR)disk->sFiles;
	DWORD PathStack[64];
	int PathStackPos=0;
	static WCHAR glPath[0xffff];
	int CurrentPos=0;
	
	PathStackPos = 0;
	for (int i=0;i<64;i++)
	{
		PathStack[PathStackPos++] = a;
		pt = a*disk->IsLong;
		a = PSEARCHFILEINFO(ptr+pt)->ParentId.LowPart;
		
		if (a==0 || a==5) break; 
	}
	if (disk->DosDevice !=NULL)
	{
		glPath[0] = disk->DosDevice;
		glPath[1] = L':';
		CurrentPos = 2;
	}
	else
		glPath[0]=L'\0';
	for (int i=PathStackPos-1;i>0;i--)
	{
		pt = PathStack[i]*disk->IsLong;
		glPath[CurrentPos++] = L'\\';
		memcpy(&glPath[CurrentPos], PSEARCHFILEINFO(ptr+pt)->FileName, PSEARCHFILEINFO(ptr+pt)->FileNameLength*2);
		CurrentPos+=PSEARCHFILEINFO(ptr+pt)->FileNameLength;
	}
	glPath[CurrentPos]=L'\\';
	glPath[CurrentPos+1]=L'\0';
	return glPath;
}

LPWSTR GetCompletePath(PDISKHANDLE disk, int id)
{
	int a = id;
	int i;
	DWORD pt;
	PUCHAR ptr = (PUCHAR)disk->sFiles;
	DWORD PathStack[64];
	int PathStackPos=0;
	static WCHAR glPath[0xffff];
	int CurrentPos=0;
	
	PathStackPos = 0;
	for (int i=0;i<64;i++)
	{
		PathStack[PathStackPos++] = a;
		pt = a*disk->IsLong;
		a = PSEARCHFILEINFO(ptr+pt)->ParentId.LowPart;
		
		if (a==0 || a==5) break; 
	}
	if (disk->DosDevice !=NULL)
	{
		glPath[0] = disk->DosDevice;
		glPath[1] = L':';
		CurrentPos = 2;
	}
	else
		glPath[0]=L'\0';
	for (int i=PathStackPos-1;i>=0;i--)
	{
		pt = PathStack[i]*disk->IsLong;
		glPath[CurrentPos++] = L'\\';
		memcpy(&glPath[CurrentPos], PSEARCHFILEINFO(ptr+pt)->FileName, PSEARCHFILEINFO(ptr+pt)->FileNameLength*2);
		CurrentPos+=PSEARCHFILEINFO(ptr+pt)->FileNameLength;
	}
	glPath[CurrentPos]=L'\0';
	return glPath;
}

BOOL FixFileRecord(PFILE_RECORD_HEADER file)
{
	//int sec = 2048;
	PUSHORT usa = PUSHORT(PUCHAR(file) + file->Ntfs.UsaOffset);
	PUSHORT sector = PUSHORT(file);
	
	if (file->Ntfs.UsaCount>4) 
		return FALSE;
	for (ULONG i=1;i<file->Ntfs.UsaCount;i++)
	{
		sector[255] = usa[i];
		sector+= 256;
	}
	
	return TRUE;
}


DWORD FetchShortFileInfo(PDISKHANDLE disk, PFILE_RECORD_HEADER file, PUCHAR buf)
{
	int i;
	bool hasFilename=false, hasData=false;
	LARGE_INTEGER dt;
	dt.QuadPart = 0;
	PFILENAME_ATTRIBUTE fn;
	PSHORTFILEINFO data = (PSHORTFILEINFO) buf;
	UCHAR *buffer = (UCHAR*) &dt; 
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	int stop = min(16,file->NextAttributeNumber);
	if (file->Ntfs.Type =='ELIF')
	{
		data->Flags = file->Flags;

		for (i=0;i<stop;i++)
		{
			if (attr->AttributeType<0 || attr->AttributeType>0x100) break;
			
			if (i%2)
				buffer[i/2] |= (attr->AttributeType & 0xf0);
			else
				buffer[i/2] = (attr->AttributeType >> 4);
			
			switch(attr->AttributeType)
			{
				case StandardInformation:
					/*data->AccessTime = PSTANDARD_INFORMATION(attr)->LastAccessTime;
					data->CreationTime = PSTANDARD_INFORMATION(attr)->CreationTime;
					data->WriteTime = PSTANDARD_INFORMATION(attr)->LastWriteTime;
					data->ChangeTime = PSTANDARD_INFORMATION(attr)->ChangeTime;
					*/
					//data->FileAttributes = PSTANDARD_INFORMATION(attr)->FileAttributes;
					break;
				case AttributeList:
					PATTRIBUTE_LIST al;
					al = PATTRIBUTE_LIST(attr);
					OutputDebugString(TEXT("SO we have found a Attribute list"));
					break;
				case FileName:
					fn = PFILENAME_ATTRIBUTE(PUCHAR(attr) + PRESIDENT_ATTRIBUTE(attr)->ValueOffset);
					if (fn->NameType & WIN32_NAME || fn->NameType == 0)
					{
						fn->Name[fn->NameLength] = L'\0';
						data->FileName = CopyString(fn->Name,fn->NameLength);
						data->FileNameLength = fn->NameLength;
						data->ParentId.QuadPart = fn->DirectoryFileReferenceNumber;
						data->ParentId.HighPart &= 0x0000ffff;
						hasFilename = true;
					}
					break;
				case ObjectId:
					break;
				case Data:
					if (attr->NameLength==0)
					{
						if (attr->Nonresident == TRUE)
						{
							data->FileSize.QuadPart = PNONRESIDENT_ATTRIBUTE(attr)->DataSize;
							//data->AllocatedFileSize.QuadPart = PNONRESIDENT_ATTRIBUTE(attr)->AllocatedSize;
						}
						else
						{
							data->FileSize.QuadPart = PRESIDENT_ATTRIBUTE(attr)->ValueLength;
							//data->AllocatedFileSize.QuadPart = PRESIDENT_ATTRIBUTE(attr)->ValueLength;						
						}
						hasData = true;
					}
					break;
				case IndexRoot:
					break;
				case IndexAllocation:
					break;
				case Bitmap:
					break;
				case ReparsePoint:
					break;
				case EAInformation:
					break;
				case EA:
					break;
				default:
					break;
			};


			if (attr->Length>0 && attr->Length < file->BytesInUse)
				attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
			else
				if (attr->Nonresident == TRUE)
					attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
			
		}
	}
	else if (file->Ntfs.Type == 'DAAB')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{BAAD}");
	}
	else if (file->Ntfs.Type == 'XDNI')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{INDEX}");
	}
	else if (file->Ntfs.Type == 'ELOH')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{HOLE}");
	}
	else if (file->Ntfs.Type == 'DKHC')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{CHECKED}");
	}
	else if (file->Ntfs.Type == 0)
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{FREE}");
	}
	else
	{
		data->ParentId.QuadPart = 0;
		data->FileName = TEXT("{NOT A FILE}");
		return FALSE;
		
	}
	if (!hasFilename) data->FileName = TEXT("{NO_NAME}");
	//if (!hasData) data->FileSize.LowPart = 123456789;
	data->Flags = file->Flags;
	//data->Attributes = dt.LowPart;
	return sizeof(SHORTFILEINFO);
}

DWORD FetchSearchInfo(PDISKHANDLE disk, PFILE_RECORD_HEADER file, PUCHAR buf)
{
	int i;
	
	PFILENAME_ATTRIBUTE fn;
	PLONGFILEINFO data = (PLONGFILEINFO) buf;
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	PATTRIBUTE attrlist = NULL;
	int stop = min(8,file->NextAttributeNumber);
	
	if (file->Ntfs.Type =='ELIF')
	{
		data->Flags = file->Flags;
	
		for (i=0;i<stop;i++)
		{
			if (attr->AttributeType<0 || attr->AttributeType>0x100) break;
			
			switch(attr->AttributeType)
			{
				case AttributeList:
					attrlist = attr;
					break;
				case FileName:
					fn = PFILENAME_ATTRIBUTE(PUCHAR(attr) + PRESIDENT_ATTRIBUTE(attr)->ValueOffset);
					if (fn->NameType & WIN32_NAME || fn->NameType == 0)
					{
						fn->Name[fn->NameLength] = L'\0';
						data->FileName = AllocAndCopyString(disk->heapBlock, fn->Name,fn->NameLength);
						data->FileNameLength = fn->NameLength;
						data->ParentId.QuadPart = fn->DirectoryFileReferenceNumber;
						data->ParentId.HighPart &= 0x0000ffff;
						if (file->BaseFileRecord.LowPart!=0)
						{
							AddToFixList(file->BaseFileRecord.LowPart, disk->filesSize);
						}
						return sizeof(SEARCHINFO);
					}
					break;
				default:
					break;
			};


			if (attr->Length>0 && attr->Length < file->BytesInUse)
				attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
			else
				if (attr->Nonresident == TRUE)
					attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
			
		}


		/*if (attrlist!=NULL)
		{
			data->FileName = TEXT("{MUST_BE_FIXED}");
			data->ParentId.LowPart = 5;
			DWORD res;
			static UCHAR attrbuf[4096];
			ReadAttribute(disk, attrlist, &attrbuf[0]);
			
			if ((res=GetFilenameFromAttributeList(disk, (PATTRIBUTE_LIST)&attrbuf[0]))>0)
				AddToFixList(disk->filesSize, res);
		}
*/
	}
	return 0;
}


DWORD FetchFileInfo(PDISKHANDLE disk, PFILE_RECORD_HEADER file, PUCHAR buf)
{
	int i;
	bool hasFilename=false, hasData=false;
	LARGE_INTEGER dt;
	dt.QuadPart = 0;
	PFILENAME_ATTRIBUTE fn;
	PLONGFILEINFO data = (PLONGFILEINFO) buf;
	UCHAR *buffer = (UCHAR*) &dt; 
	PATTRIBUTE attr = (PATTRIBUTE)((PUCHAR)(file) + file->AttributesOffset);
	int stop = min(16,file->NextAttributeNumber)-1;
	if (file->Ntfs.Type =='ELIF')
	{
		data->Flags = file->Flags;

		for (i=0;i<stop;i++)
		{
			if (attr->AttributeType<0 || attr->AttributeType>0x100) break;
			
			if (i%2)
				buffer[i/2] |= (attr->AttributeType & 0xf0);
			else
				buffer[i/2] = (attr->AttributeType >> 4);
			
			switch(attr->AttributeType)
			{
				case StandardInformation:
					data->AccessTime = PSTANDARD_INFORMATION(attr)->LastAccessTime;
					data->CreationTime = PSTANDARD_INFORMATION(attr)->CreationTime;
					data->WriteTime = PSTANDARD_INFORMATION(attr)->LastWriteTime;
					data->ChangeTime = PSTANDARD_INFORMATION(attr)->ChangeTime;
					
					data->FileAttributes = PSTANDARD_INFORMATION(attr)->FileAttributes;
					break;
				case AttributeList:
					PATTRIBUTE_LIST al;
					al = PATTRIBUTE_LIST(attr);
					OutputDebugString(TEXT("SO we have found a Attribute list"));
					break;
				case FileName:
					fn = PFILENAME_ATTRIBUTE(PUCHAR(attr) + PRESIDENT_ATTRIBUTE(attr)->ValueOffset);
					if (fn->NameType & WIN32_NAME || fn->NameType == 0)
					{
						fn->Name[fn->NameLength] = L'\0';
						data->FileName = CopyString(fn->Name,fn->NameLength);
						data->FileNameLength = fn->NameLength;
						data->ParentId.QuadPart = fn->DirectoryFileReferenceNumber;
						data->ParentId.HighPart &= 0x0000ffff;
						hasFilename = true;
					
						if (file->BaseFileRecord.LowPart!=0)// && file->BaseFileRecord.HighPart !=0x10000)
						{
							AddToFixList(disk->filesSize, file->BaseFileRecord.LowPart);
						}


					}
					break;
				case ObjectId:
					break;
				case Data:
					if (attr->NameLength==0)
					{
						if (attr->Nonresident == TRUE)
						{
							data->FileSize.QuadPart = PNONRESIDENT_ATTRIBUTE(attr)->DataSize;
							data->AllocatedFileSize.QuadPart = PNONRESIDENT_ATTRIBUTE(attr)->AllocatedSize;
						}
						else
						{
							data->FileSize.QuadPart = PRESIDENT_ATTRIBUTE(attr)->ValueLength;
							data->AllocatedFileSize.QuadPart = PRESIDENT_ATTRIBUTE(attr)->ValueLength;						
						}
						hasData = true;
					}
					break;
				case IndexRoot:
					break;
				case IndexAllocation:
					break;
				case Bitmap:
					break;
				case ReparsePoint:
					break;
				case EAInformation:
					break;
				case EA:
					break;
				default:
					break;
			};


			if (attr->Length>0 && attr->Length < file->BytesInUse)
				attr = PATTRIBUTE(PUCHAR(attr) + attr->Length);
			else
				if (attr->Nonresident == TRUE)
					attr = PATTRIBUTE(PUCHAR(attr) + sizeof(NONRESIDENT_ATTRIBUTE));	
			
		}
	}
	else if (file->Ntfs.Type == 'DAAB')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{BAAD}");
	}
	else if (file->Ntfs.Type == 'XDNI')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{INDEX}");
	}
	else if (file->Ntfs.Type == 'ELOH')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{HOLE}");
	}
	else if (file->Ntfs.Type == 'DKHC')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{CHECKED}");
	}
	else if (file->Ntfs.Type == 0)
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{FREE}");
	}
	else
	{
		data->ParentId.QuadPart = 0;
		data->FileName = TEXT("{NOT A FILE}");
		return FALSE;
		
	}
	if (!hasFilename) data->FileName = TEXT("{NO_NAME}");
	if (!hasData) data->FileSize.LowPart = 4242424242;
	data->Flags = file->Flags;
	data->Attributes = dt.LowPart;
	return sizeof(LONGFILEINFO);
}



BOOL inline OldFetchShortFileInfo(PFILE_RECORD_HEADER fh, PSHORTFILEINFO data)
{
	PFILENAME_ATTRIBUTE file;
	PRESIDENT_ATTRIBUTE attr;
	PNONRESIDENT_ATTRIBUTE nattr;

	if (fh->Ntfs.Type=='ELIF')
	{
		file = FindFileName(fh, WIN32_NAME);
		if (file!=NULL)
		{		
			file->Name[file->NameLength] = L'\0';
			data->FileName = file->Name;
			data->FileNameLength = file->NameLength;
			
			data->ParentId.QuadPart = file->DirectoryFileReferenceNumber;
			data->ParentId.HighPart &= 0x0000ffff;
			data->Flags = fh->Flags;

			//if (fh->Flags & 0x0001)
			//{
				nattr = (PNONRESIDENT_ATTRIBUTE) FindAttribute(fh, Data);
				if (nattr!=NULL)
				{
					if (nattr->Attribute.NameLength!=0)
					{
						data->FileSize.QuadPart = 0xDEADDEADDEADDEAD;
					}
					if (nattr->Attribute.Nonresident!=FALSE)
					{
						data->FileSize.QuadPart = nattr->DataSize;
					}
					else
					{
						attr = (PRESIDENT_ATTRIBUTE) nattr;
						data->FileSize.QuadPart = attr->ValueLength;
					}
				}
				else
				{
					if (fh->Flags & 0x0002)
					{

					}
					else
						data->FileSize.QuadPart = file->DataSize;
				}
			//}
			/*else
				data->FileSize.QuadPart = file->DataSize;*/
		}
		else
		{
lastchance:
			data->ParentId.QuadPart = 5;
			data->ParentId.HighPart &= 0x0000ffff;
			if (fh->Flags & 0x1) data->FileName = NULL;
			else data->FileName = NULL;
		}
	}
	/*else if (fh->Ntfs.Type == 'DAAB')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{BAAD}");
	}
	else if (fh->Ntfs.Type == 'XDNI')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{INDEX}");
	}
	else if (fh->Ntfs.Type == 'ELOH')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{HOLE}");
	}
	else if (fh->Ntfs.Type == 'DKHC')
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{CHECKED}");
	}
	else if (fh->Ntfs.Type == 0)
	{
		data->ParentId.QuadPart = 5;
		data->FileName = TEXT("{FREE}");
	}
	else
	{
		data->ParentId.QuadPart = 0;
		CHAR text[6];
		text[5] = '\0';
		memcpy(text, &fh->Ntfs.Type,5);
		data->FileName = TEXT("{NOT A FILE}");
		return FALSE;
		
	}*/
	return TRUE;
}


// Directly from book Native Api NT/2000



ULONG RunLength(PUCHAR run)
{
	return (*run & 0xf) + ((*run >> 4) & 0xf)+1;
}

LONGLONG RunLCN(PUCHAR run)
{
	UCHAR n1 = *run & 0xf;
	UCHAR n2 = (*run >> 4) & 0xf;
	LONGLONG lcn = n2 == 0 ? 0 : CHAR(run[n1+n2]);

	for (LONG i = n1 + n2 - 1;i>n1; i--)
		lcn = (lcn << 8) + run[i];
	return lcn;
}

ULONGLONG RunCount(PUCHAR run)
{
	UCHAR n = *run & 0xf;
	ULONGLONG count = 0;

	for (ULONG i=n;i > 0; i--)
		count = (count << 8) + run[i];

	return count;
}

BOOL FindRun(PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, PULONGLONG lcn, PULONGLONG count)
{
	if (vcn < attr->LowVcn || vcn > attr->HighVcn)
		return FALSE;
	*lcn = 0;
	
	ULONGLONG base=attr->LowVcn;

	for (PUCHAR run = PUCHAR(PUCHAR(attr) +  attr->RunArrayOffset);*run!=0;run+=RunLength(run))
	{
		*lcn += RunLCN(run);
		*count = RunCount(run);
		if (base <= vcn && vcn < base + *count)
		{
			*lcn = RunLCN(run) == 0 ? 0 : *lcn + vcn - base;
			*count -= ULONG(vcn - base);
			return TRUE;
		}
		else
		{
			base += *count;
		}

	}

	return FALSE;
}

//PATTRIBUTE FindAttribtue

VOID FixupUpdateSequenceArray(PFILE_RECORD_HEADER file)
{
	PUSHORT usa = PUSHORT(PUCHAR(file) + file->Ntfs.UsaOffset);
	PUSHORT sector = PUSHORT(file);

	
	for (ULONG i=1;i<file->Ntfs.UsaCount;i++)
	{
		sector[255] = usa[i];
		sector+= 256;
	}
}

VOID ReadSector(ULONGLONG sector, ULONG count, PVOID buffer)
{
	ULARGE_INTEGER offset;
	OVERLAPPED overlap={0};
	ULONG n;

	//offset.QuadPart = sector*
}

DWORD ReadLCN(PDISKHANDLE disk, ULONGLONG lcn, ULONG count, PVOID buffer)
{
	LARGE_INTEGER offset;
	DWORD read=0;

	offset.QuadPart = lcn*disk->NTFS.BytesPerCluster;
	
	SetFilePointer(disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	count *=disk->NTFS.BytesPerCluster;
	
	ReadFile(disk->fileHandle, buffer, count, &read, NULL);
	
	return read;
}

VOID ReadExternalAttribute(PDISKHANDLE disk, PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, ULONG count, PVOID buffer)
{
	ULONGLONG lcn, runcount;
	ULONG readcount, left;

	PUCHAR bytes=PUCHAR(buffer);
	
	for (left = count; left > 0; left -= readcount)
	{
		FindRun(attr, vcn, &lcn, &runcount);
		readcount = ULONG(min(runcount, left));
		ULONG n = readcount * disk->NTFS.BytesPerCluster;
		if (lcn==0)
			memset(bytes, 0, n);
		else
			ReadLCN(disk, lcn, readcount, bytes);
		vcn += readcount;
		bytes += n;
	}

}



ULONG AttributeLength(PATTRIBUTE attr)
{
	return attr->Nonresident == FALSE ? PRESIDENT_ATTRIBUTE(attr)->ValueLength : ULONG(PNONRESIDENT_ATTRIBUTE(attr)->DataSize);
}

ULONG AttributeLengthAllocated(PATTRIBUTE attr)
{
	return attr->Nonresident == FALSE ? PRESIDENT_ATTRIBUTE(attr)->ValueLength : ULONG(PNONRESIDENT_ATTRIBUTE(attr)->AllocatedSize);
}

VOID ReadAttribute(PDISKHANDLE disk, PATTRIBUTE attr, PVOID buffer)
{
	if (attr->Nonresident == FALSE)
	{
		PRESIDENT_ATTRIBUTE rattr = PRESIDENT_ATTRIBUTE(attr);
		memcpy(buffer, PUCHAR(rattr)+  rattr->ValueOffset, rattr->ValueLength);
	}
	else
	{
		PNONRESIDENT_ATTRIBUTE nattr = PNONRESIDENT_ATTRIBUTE(attr);
		ReadExternalAttribute(disk, nattr, 0, ULONG(nattr->HighVcn)+1, buffer);
	}
}

VOID ReadVCN(PDISKHANDLE disk, PFILE_RECORD_HEADER file, ATTRIBUTE_TYPE type, ULONGLONG vcn, ULONG count, PVOID buffer)
{
	PATTRIBUTE attr;
	PNONRESIDENT_ATTRIBUTE nattr = PNONRESIDENT_ATTRIBUTE(attr);
	
	if (nattr==NULL || (vcn<nattr->LowVcn || vcn>nattr->HighVcn))
	{
		// support for huge files
		//PATTRIBUTE attrlist = FindAttribute();
		//DebugBReak();
	}

	ReadExternalAttribute(disk, nattr, vcn, count, buffer);
}

VOID ReadFileRecord(PDISKHANDLE disk, ULONG index, PFILE_RECORD_HEADER file)
{
	ULONG clusters = disk->NTFS.bootSector.ClustersPerFileRecord;

	if (clusters > 0x80)
		clusters = 1;

	PUCHAR p = new UCHAR[disk->NTFS.BytesPerCluster*clusters];

	//ULONGLONG vcn = ULONGLONG(index) * disk->NTFS.BytesPerFileRecord 

	// missing

}

BOOL bitset(PUCHAR bitmap, ULONG index)
{
	return (bitmap[index >> 3] & (1 << (index & 7)))!=0;
}

//VOID DumpData(PDISKHANDLE disk, ULONG index, PCSTR FileName


DWORD ReadData(PDISKHANDLE disk, PATTRIBUTE attr, PUCHAR buffer)
{
	if (attr->Nonresident == FALSE)
	{
		PRESIDENT_ATTRIBUTE rattr = PRESIDENT_ATTRIBUTE(attr);
		memcpy(buffer, PUCHAR(rattr)+  rattr->ValueOffset, rattr->ValueLength);
	}
	else
	{
		PNONRESIDENT_ATTRIBUTE nattr = PNONRESIDENT_ATTRIBUTE(attr);
		return ReadExternalData(disk, nattr, 0, ULONG(nattr->HighVcn)+1, buffer);
	}
}

DWORD ReadExternalData(PDISKHANDLE disk, PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, ULONG count, PUCHAR buffer)
{
	ULONGLONG lcn, runcount;
	ULONG readcount, left;
	DWORD ret=0;
	PUCHAR bytes=PUCHAR(buffer);
	
	for (left = count; left > 0; left -= readcount)
	{
		FindRun(attr, vcn, &lcn, &runcount);
		readcount = ULONG(min(runcount, left));
		ULONG n = readcount * disk->NTFS.BytesPerCluster;
		if (lcn==0)
			memset(bytes, 0, n);
		else
			ret += ReadDataLCN(disk, lcn, readcount, bytes);
		vcn += readcount;
		bytes += n;
	}
	return ret;
}

DWORD ReadDataLCN(PDISKHANDLE disk, ULONGLONG lcn, ULONG count, PUCHAR buffer)
{
	LARGE_INTEGER offset;
	DWORD read=0;
	DWORD cnt=0,c=0,pos=0;

	offset.QuadPart = lcn*disk->NTFS.BytesPerCluster;
	SetFilePointer(disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	cnt = count / CLUSTERSPERREAD;

	for (int i=1;i<=cnt;i++)
	{
		ReadFile(disk->fileHandle, &buffer[pos], CLUSTERSPERREAD*disk->NTFS.BytesPerCluster, &read, NULL);
		c += CLUSTERSPERREAD;
		pos+=read;
	}
	
	ReadFile(disk->fileHandle, &buffer[pos], (count-c)*disk->NTFS.BytesPerCluster, &read, NULL);
	pos+=read;
	return pos;
}


DWORD ReadMFTParse(PDISKHANDLE disk, PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, ULONG count, PVOID buffer, FETCHPROC fetch, PSTATUSINFO info)
{
	ULONGLONG lcn, runcount;
	ULONG readcount, left;
	DWORD ret=0;
	PUCHAR bytes=PUCHAR(buffer);
	PUCHAR data;

	data = new UCHAR[disk->NTFS.entryCount*disk->IsLong];
	memset(data, 0, disk->NTFS.entryCount*disk->IsLong);
	disk->lFiles = (PLONGFILEINFO)data;

	for (left = count; left > 0; left -= readcount)
	{
		FindRun(attr, vcn, &lcn, &runcount);
		readcount = ULONG(min(runcount, left));
		ULONG n = readcount * disk->NTFS.BytesPerCluster;
		if (lcn==0)
			memset(bytes, 0, n);
		else
		{
			ret += ReadMFTLCN(disk, lcn, readcount, buffer, fetch, info);
		}
		vcn += readcount;
		bytes += n;
	}
	return ret;
}

/*DWORD ReadMFTLCN(PDISKHANDLE disk, ULONGLONG lcn, ULONG count, PVOID buffer, FETCHPROC fetch, PSTATUSINFO info)
{
	LARGE_INTEGER offset;
	DWORD read=0;
	DWORD ret=0;
	DWORD cnt=0, c=0, pos=0;
	
	offset.QuadPart = lcn*disk->NTFS.BytesPerCluster;
	SetFilePointer(disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	cnt = count / CLUSTERSPERREAD;

	for (int i=1;i<=cnt;i++)
	{
		ReadFile(disk->fileHandle, buffer, CLUSTERSPERREAD*disk->NTFS.BytesPerCluster, &read, NULL);
		c += CLUSTERSPERREAD;
		pos+=read;	
		ProcessBuffer(disk, (PUCHAR)buffer, read, fetch );
		CallMe(info, disk->filesSize);
	}
	
	ReadFile(disk->fileHandle, buffer, (count-c)*disk->NTFS.BytesPerCluster, &read, NULL);
	ProcessBuffer(disk, (PUCHAR)buffer, read, fetch );
	CallMe(info, disk->filesSize);
	
	pos+=read;
	return pos;
}*/

DWORD ReadMFTLCN(PDISKHANDLE disk, ULONGLONG lcn, ULONG count, PVOID buffer, FETCHPROC fetch, PSTATUSINFO info)
{
	LARGE_INTEGER offset;
	DWORD read=0;
	DWORD ret=0;
	DWORD cnt=0, c=0, pos=0;
	
	offset.QuadPart = lcn*disk->NTFS.BytesPerCluster;
	SetFilePointer(disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	cnt = count / CLUSTERSPERREAD;

	for (int i=1;i<=cnt;i++)
	{

		ReadFile(disk->fileHandle, buffer, CLUSTERSPERREAD*disk->NTFS.BytesPerCluster, &read, NULL);
		c += CLUSTERSPERREAD;
		pos+=read;
		offset.HighPart = NULL;
		offset.LowPart = SetFilePointer(disk->fileHandle, NULL, &offset.HighPart,FILE_CURRENT);

		ProcessBuffer(disk, (PUCHAR)buffer, read, fetch );
		CallMe(info, disk->filesSize);

		SetFilePointer(disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN);

	}

	ReadFile(disk->fileHandle, buffer, (count-c)*disk->NTFS.BytesPerCluster, &read, NULL);
	ProcessBuffer(disk, (PUCHAR)buffer, read, fetch );
	CallMe(info, disk->filesSize);
	
	pos+=read;
	return pos;
}


VOID inline CallMe(PSTATUSINFO info, DWORD value)
{
	if (info!=NULL)
		SendMessage(info->hWnd, PBM_SETPOS, value, 0);
	
}

DWORD ProcessBuffer(PDISKHANDLE disk, PUCHAR buffer, DWORD size, FETCHPROC fetch)
{
	PUCHAR end;
	PUCHAR data;
	DWORD count=0;
	PFILE_RECORD_HEADER fh;
	end	= PUCHAR(buffer) + size;
	data = PUCHAR(disk->lFiles);
	data += disk->IsLong * disk->filesSize;

	while (buffer<end)
	{
		fh = PFILE_RECORD_HEADER(buffer);
		FixFileRecord(fh);
		if ((fetch)(disk, fh, data)>0)
			disk->realFiles++;
		buffer+= disk->NTFS.BytesPerFileRecord;
		data += disk->IsLong;
		disk->filesSize++;
	}
	return 0;
}

/*	
	switch (option)
	{
	case LONGINFO:
		size = sizeof(LONGFILEINFO);
		break;
	case SHORTINFO:
		size = sizeof(SHORTFILEINFO);
		break;
	case SEARCHINFO:
		size = sizeof(LONGFILEINFO);
		break;
	case EXTRALONGINFO:
		size = sizeof(LONGFILEINFO);
		break;
	default:
		size = sizeof(LONGFILEINFO);
		break;
	}*/

inline LPTSTR CopyString(LPTSTR string, USHORT length)
{
	LPTSTR tmp;
	tmp = new TCHAR[length+1];
	RtlCopyMemory(tmp, string, length*sizeof(TCHAR));
	tmp[length] = L'\0';
	return tmp;

}

/*BOOL ReleaseFilenames(PDISKHANLDE disk)
{
	for (int i=0;i<disk->sizeFiles;i++)
	{
		if (
	}
}*/




// Heaps


PHEAPBLOCK CreateHeap(DWORD size)
{
	PHEAPBLOCK tmp;
	tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
	tmp->current = 0;
	tmp->size = size;
	tmp->next = NULL;
	tmp->data = (PUCHAR) malloc(size);
	if (tmp->data != NULL)
	{
		//currentBlock = tmp;
		tmp->end = tmp;
		return tmp;
	}
	else
	{
		free(tmp);
		return NULL;
	}
}

BOOL FreeHeap(PHEAPBLOCK block)
{
	if (block != NULL)
	{
		FreeAllBlocks(block);		
		return TRUE;
	}
	return FALSE;
}

LPTSTR AllocString(PHEAPBLOCK block, DWORD size)
{
	PHEAPBLOCK tmp, back;
	PUCHAR ret=NULL;
	size = ((size*sizeof(TCHAR)) & 0xfffffff8) + 8;

	tmp = block->end;
	if(tmp!=NULL)
	{
		if ((int)(tmp->current - tmp->size) > (int)size)
		{
			ret = &tmp->data[tmp->current];
			tmp->current += size;
			return (LPTSTR)ret;
		}
		back = tmp;
	}
	tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
	tmp->data =	(PUCHAR) malloc(block->size);
	if (tmp->data!=NULL)
	{
		ret = &tmp->data[0];
		tmp->current = size;
		tmp->size = block->size;
		tmp->next = NULL;

		back = block->end;
		tmp->end = block;
		back->next = tmp;
		block->end = tmp;
		return (LPTSTR)ret;
	}
	else
		free(tmp);
	return NULL;
}

LPTSTR AllocAndCopyString(PHEAPBLOCK block, LPTSTR string, DWORD size)
{
	PHEAPBLOCK tmp, back;
	PUCHAR ret=NULL;
	int t;
	DWORD rsize = (size+1)*sizeof(TCHAR);
	size = ((rsize) & 0xfffffff8) + 8;

	tmp = block->end;
	if(tmp!=NULL)
	{
		t = tmp->size - tmp->current+8;
		if (t > (int)size)
		{
			ret = &tmp->data[tmp->current];
			memcpy(ret, string, rsize);
			ret[rsize] = 0;
			ret[rsize+1] = 0;
			tmp->current += size;
			return (LPTSTR)ret;
		}
		back = tmp;
	}
	tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
	tmp->data =	(PUCHAR) malloc(block->size);
	if (tmp->data!=NULL)
	{
		ret = &tmp->data[0];	
		tmp->current = size;
		tmp->size = block->size;
		tmp->next = NULL;
		memcpy(ret, string, rsize);

		back = block->end;
		tmp->end = block;
		back->next = tmp;
		block->end = tmp;
		
		ret[rsize] = 0;
		ret[rsize+1] = 0;

		return (LPTSTR)ret;
	}
	else
		free(tmp);
	return NULL;
}


BOOL FreeAllBlocks(PHEAPBLOCK block)
{
	PHEAPBLOCK tmp, back;
	tmp = block;

	while (tmp!=NULL)
	{
		free(tmp->data);	
		back = tmp;
		tmp = tmp->next;
		free(back);
	}
	
	return TRUE;
}

BOOL ReUseBlocks(PHEAPBLOCK block, BOOL clear)
{
	if (block!=NULL)
	{
		PHEAPBLOCK tmp, back;
		tmp = block;
		while (tmp!=NULL)
		{
			tmp->current = 0;	
			tmp = tmp->next;
			if (clear) memset(tmp->data, 0, tmp->size*sizeof(TCHAR));
		}
	}
	return TRUE;
}




BOOL StringRegCompare(LPTSTR str1, DWORD sSize, LPTSTR pattern, DWORD pSize)
{
	int p=0, s=0;
	int bp=0;
	if (sSize < pSize)
		return FALSE;
	
	while (p < pSize)
	{
		if (pattern[p] == str1[s])
		{
			p++;
		}
		else if (pattern[p] == '?')
		{
			p++;
		}
		else if (pattern[p] == '*')
		{
			TCHAR* found;
			found = wcschr(&str1[s],pattern[p]);		
			if (found==NULL) return FALSE;
			else
			{
				bp = p;
				s += found - &str1[s]-1;
				p++;
			}
		}
		else
		{
			
			return FALSE;
		}
		s++;
		
	}
	if (s<sSize) 
		return FALSE;
	return TRUE;
}


BOOL ReparseDisk(PDISKHANDLE disk, UINT option, PSTATUSINFO info)
{
	if (disk!=NULL)
	{
		if (disk->type == NTFSDISK)
		{
			if (disk->NTFS.MFT!=NULL)
				delete disk->NTFS.MFT;
			disk->NTFS.MFT = NULL;
			if (disk->NTFS.Bitmap !=NULL)
				delete disk->NTFS.Bitmap;
			disk->NTFS.Bitmap = NULL;
		}
		if (disk->heapBlock!=NULL)
		{
			ReUseBlocks(disk->heapBlock, FALSE);
		}
		if (disk->IsLong)
		{
			if (disk->lFiles!=NULL)
				delete disk->lFiles;
			disk->lFiles = NULL;
		}
		else
		{
			if (disk->sFiles!=NULL)
				delete disk->sFiles;
			disk->sFiles = NULL;
		}

		disk->filesSize = 0;
		disk->realFiles = 0;

		if (LoadMFT(disk, FALSE)!=0)
			ParseMFT(disk, option, info);
		return TRUE;
	}
	return FALSE;
};

PUCHAR AllocData(PHEAPBLOCK block, DWORD size)
{
	PHEAPBLOCK tmp, back;
	PUCHAR ret=NULL;
	int t;

	tmp = block->end;
	if(tmp!=NULL)
	{
		t = tmp->size - tmp->current;
		if (t > (int)size)
		{
			ret = &tmp->data[tmp->current];
			tmp->current += size;
			return ret;
		}
		back = tmp;
	}
	tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
	tmp->data =	(PUCHAR) malloc(block->size);
	if (tmp->data!=NULL)
	{	
		tmp->current = size;
		tmp->size = block->size;
		tmp->next = NULL;

		back = block->end;
		tmp->end = block;
		back->next = tmp;
		block->end = tmp;

		return tmp->data;
	}
	else
		free(tmp);
	return NULL;

}

DWORD GetFilenameFromAttributeList(PDISKHANDLE disk, PATTRIBUTE_LIST attrlist)
{
	while(attrlist->Attribute!=-1)
	{
		if (attrlist->Attribute == FileName)
			return (DWORD)attrlist->FileReferenceNumber;
		else if (attrlist->Attribute == AttributeList)
		{
			UCHAR buf[4096];
			
		}
		attrlist = (PATTRIBUTE_LIST)(PUCHAR(attrlist) + attrlist->Length);
	}

	return 0;
}
