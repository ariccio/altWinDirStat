#include "stdafx.h"
#ifndef _WINDOWS_
#include "windows.h"
#endif
#include "commctrl.h"
#include "FixList.h"

// HEAPHeap
PHEAPBLOCK currentBlock = NULL;

//NONRESIDENT_ATTRIBUTE ERROR_ATTRIBUTE = {1,2,3,4,5};
#define CLUSTERSPERREAD 1024
PDISKHANDLE OpenDisk(WCHAR DosDevice)
{

#ifdef TRACING
	TRACE(_T("Opening disk (by DosDevice name)\r\n") );
#endif
	WCHAR     path[7];
	path[0] = L'\\';
	path[1] = L'\\';
	path[2] = L'.';
	path[3] = L'\\';
	path[4] = DosDevice;
	path[5] = L':';
	path[6] = L'\0';
	PDISKHANDLE disk;
	disk = OpenDisk( path );
	if ( disk != NULL ) {
		disk->DosDevice = DosDevice;
		return disk;
		}
	return NULL;
}

PDISKHANDLE OpenDisk(LPCTSTR disk)
{
	/*
	  This function, if passed the name of an NTFS volume, returns a pointer to a DISKHANDLE struct with fields ['fileHandle', 'type', 'NTFS.BytesPerCluster', 'NTFS.BytesPerFileRecord', 'NTFS.complete', 'NTFS.MFTLocation.QuadPart', 'NTFS.MFT', 'NTFS.sizeMFT', 'heapBlock', 'IsLong'] filled.

	  If this function fails to open a file handle, it returns NULL.

	  If this function fails to read the boot sector, it returns an empty DISKHANDLE structure.

	  If this function finds a volume that is identified as anything OTHER than NTFS, it returns a pointer to a DISKHANDLE struct with 'type' set to UNKNOWN, and lFiles set to NULL

	  Finding the start address of the Master File Table is not easy. First we need to find the number of bytes per cluster (by multiplying the number of sectors per cluster by the size of those sectors).
	*/


	#ifdef TRACING
	TRACE( _T( "Opening disk (by LPCTSTR)\r\n" ) );
	#endif

	PDISKHANDLE tmpDisk = new DISKHANDLE;
	DWORD       read;
	tmpDisk->DosDevice = NULL;
	tmpDisk->FAT.FAT = NULL;
	tmpDisk->fFiles = NULL;
	tmpDisk->fileHandle = INVALID_HANDLE_VALUE;
	tmpDisk->filesSize = NULL;
	tmpDisk->heapBlock = NULL;
	tmpDisk->IsLong = NULL;
	tmpDisk->lFiles = NULL;
	tmpDisk->NTFS.Bitmap = NULL;
	tmpDisk->NTFS.bootSector.BootSectors = NULL;
	tmpDisk->NTFS.bootSector.BytesPerSector = NULL;
	tmpDisk->NTFS.bootSector.ClustersPerFileRecord = NULL;
	tmpDisk->NTFS.bootSector.ClustersPerIndexBlock = NULL;
	tmpDisk->NTFS.bootSector.EndOfSectorMarker = NULL;
	tmpDisk->NTFS.bootSector.Jump_Instruction[0] = NULL;
	tmpDisk->NTFS.bootSector.Jump_Instruction[1] = NULL;
	tmpDisk->NTFS.bootSector.Jump_Instruction[2] = NULL;
	tmpDisk->NTFS.bootSector.Mbz1 = NULL;
	tmpDisk->NTFS.bootSector.Mbz2 = NULL;
	tmpDisk->NTFS.bootSector.Mbz3 = NULL;
	tmpDisk->NTFS.bootSector.Media_Descriptor = NULL;
	tmpDisk->NTFS.bootSector.Mft2StartLcn = NULL;
	tmpDisk->NTFS.bootSector.MftStartLcn = NULL;
	tmpDisk->NTFS.bootSector.NotUsedByNTFS = NULL;
	tmpDisk->NTFS.bootSector.NumberOfHeads = NULL;
	tmpDisk->NTFS.bootSector.PartitionOffset = NULL;
	tmpDisk->NTFS.bootSector.Reserved1 = NULL;
	tmpDisk->NTFS.bootSector.SectorsPerCluster = NULL;
	tmpDisk->NTFS.bootSector.SectorsPerTrack = NULL;
	tmpDisk->NTFS.bootSector.TotalSectors = NULL;
	tmpDisk->NTFS.bootSector.VolumeSerialNumber = NULL;
	tmpDisk->NTFS.BytesPerCluster = NULL;
	tmpDisk->NTFS.BytesPerFileRecord = NULL;
	tmpDisk->NTFS.complete = NULL;
	tmpDisk->NTFS.entryCount = NULL;
	tmpDisk->NTFS.MFT = NULL;
	tmpDisk->NTFS.MFT2Location.QuadPart = NULL;
	tmpDisk->NTFS.MFTLocation.QuadPart = NULL;
	tmpDisk->NTFS.sizeMFT = NULL;
	tmpDisk->realFiles = NULL;
	tmpDisk->sFiles = NULL;
	tmpDisk->type = NULL;

	tmpDisk->fileHandle = CreateFile( disk, GENERIC_READ| GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
	if ( tmpDisk->fileHandle != INVALID_HANDLE_VALUE ) {
		BOOL couldRead = ReadFile( tmpDisk->fileHandle,                  &tmpDisk->NTFS.bootSector, sizeof( BOOT_BLOCK ), &read, NULL );
		if ( read == sizeof( BOOT_BLOCK ) && couldRead ) {
#ifdef TRACING
			TRACE( _T( "bootSector.MftStartLcn: %llu\r\n" ), tmpDisk->NTFS.bootSector.MftStartLcn );
#endif
			if ( strncmp( "NTFS", ( const char* )       &tmpDisk->NTFS.bootSector.OEM_ID, 4 ) == 0 ) {
				  tmpDisk->type = NTFSDISK;
				  tmpDisk->NTFS.BytesPerCluster      =   tmpDisk->NTFS.bootSector.BytesPerSector * tmpDisk->NTFS.bootSector.SectorsPerCluster;
				  tmpDisk->NTFS.BytesPerFileRecord   = ( tmpDisk->NTFS.bootSector.ClustersPerFileRecord < 0x80 ) ? ( tmpDisk->NTFS.bootSector.ClustersPerFileRecord * tmpDisk->NTFS.BytesPerCluster ) : ( 1 <<(0x100 - tmpDisk->NTFS.bootSector.ClustersPerFileRecord) );
				
				  tmpDisk->NTFS.complete             = FALSE;
				  tmpDisk->NTFS.MFTLocation.QuadPart =   tmpDisk->NTFS.bootSector.MftStartLcn * tmpDisk->NTFS.BytesPerCluster;
				  //tmpDisk->NTFS.MFTLocation.QuadPart = tmpDisk->NTFS.bootSector.MftStartLcn;
				  tmpDisk->NTFS.MFT2Location.QuadPart = tmpDisk->NTFS.bootSector.Mft2StartLcn;
				  tmpDisk->NTFS.MFT                  = NULL;
				  tmpDisk->heapBlock                 = NULL;
				  tmpDisk->IsLong                    = FALSE;
				  tmpDisk->NTFS.sizeMFT              = 0;
#ifdef TRACING
				  TRACE( _T( "tmpDisk->bootSector.MftStartLcn * tmpDisk->NTFS.BytesPerCluster: %llu\r\n" ), ( tmpDisk->NTFS.bootSector.MftStartLcn ) * ( ( ULONGLONG ) tmpDisk->NTFS.BytesPerCluster ) );
#endif
				  NTFS_VOLUME_DATA_BUFFER ntfsVolData;
				  DWORD returnSize;
				  DWORD sizeofNtfsVolData = sizeof( ntfsVolData );
				  OVERLAPPED overlapped_unused;
				  BOOL devioRes = DeviceIoControl( tmpDisk->fileHandle, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0, (LPVOID) &ntfsVolData, sizeofNtfsVolData, &returnSize, &overlapped_unused );
				  if ( devioRes != 0 ) {
					  assert( tmpDisk->NTFS.BytesPerCluster == ntfsVolData.BytesPerCluster );
					  assert( tmpDisk->NTFS.BytesPerFileRecord == ntfsVolData.BytesPerFileRecordSegment );
					  assert( tmpDisk->NTFS.MFTLocation.QuadPart == ntfsVolData.MftStartLcn.QuadPart );
					  assert( tmpDisk->NTFS.MFT2Location.QuadPart == ntfsVolData.Mft2StartLcn.QuadPart );
					  }
				  else {
					  LPVOID lpMsgBuf;
					  LPVOID lpDisplayBuf;
					  DWORD err = GetLastError( );
					  FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
					  LPCTSTR msg = (LPCTSTR)lpMsgBuf;

					  //lpDisplayBuf = ( LPVOID ) LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR ) lpMsgBuf )* sizeof( TCHAR ) ) );
					  MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
#ifdef TRACING
					  TRACE(_T("Error: %s\r\n"), msg);
#endif
					  }
				}
			else {
				  tmpDisk->type                      = UNKNOWN;
				  tmpDisk->lFiles                    = NULL;
				}
			}
		else if (couldRead == 0) {
			LPVOID lpMsgBuf;
			LPVOID lpDisplayBuf;
			DWORD err = GetLastError( );
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
			LPCTSTR msg = (LPCTSTR)lpMsgBuf;

			//lpDisplayBuf = ( LPVOID ) LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR ) lpMsgBuf )* sizeof( TCHAR ) ) );
			MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
	#ifdef TRACING
			TRACE(_T("Error: %s\r\n"), msg);
	#endif

			}
		return tmpDisk;
		}
	else {
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD err = GetLastError( );
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
		LPCTSTR msg = (LPCTSTR)lpMsgBuf;

		//lpDisplayBuf = ( LPVOID ) LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR ) lpMsgBuf )* sizeof( TCHAR ) ) );
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
#ifdef TRACING
		TRACE(_T("Error: %s\r\n"), msg);
#endif
		}
	delete tmpDisk;
	return NULL;
};

BOOL CloseDisk(PDISKHANDLE disk)
{
	if ( disk != NULL ) {
	#ifdef TRACING
		TRACE( _T( "Closing disk\r\n" ) );
	#endif
		if ( disk->fileHandle > INVALID_HANDLE_VALUE ) {
			CloseHandle( disk->fileHandle );
			}

		if ( disk->type == NTFSDISK ) {
			if ( disk->NTFS.MFT != NULL ) {
				delete disk->NTFS.MFT;
				}
			
			disk->NTFS.MFT = NULL;

			if ( disk->NTFS.Bitmap != NULL ) {
				delete disk->NTFS.Bitmap;
				}

			disk->NTFS.Bitmap = NULL;
			}
		if ( disk->heapBlock != NULL ) {
			FreeHeap( disk->heapBlock );
			disk->heapBlock = NULL;
			}

		if ( disk->IsLong ) {
			if ( disk->lFiles != NULL ) {
				//if ( disk->lFiles->FileName != NULL ) {
				//	delete disk->lFiles->FileName;
				//	}
				disk->lFiles = NULL;
				}
			
			if ( disk->fFiles != NULL ) {
				delete disk->fFiles;
				disk->fFiles = NULL;
				}
			if ( disk->sFiles != NULL) {
				delete disk->sFiles;
				disk->sFiles = NULL;
				}
			}
		else {
			if ( disk->sFiles != NULL ) {
				//delete disk->sFiles;
				disk->sFiles = NULL;
				}
			if ( disk->lFiles != NULL ) {
				delete disk->lFiles;
				disk->lFiles = NULL;
				}
			
			if ( disk->fFiles != NULL ) {
				delete disk->fFiles;
				disk->fFiles = NULL;
				}
			}
		delete disk;
		return TRUE;
		}
#ifdef TRACING
	TRACE( _T( "No disk to close!\r\n" ) );
#endif
	return FALSE;
};


ULONGLONG LoadMFT( PDISKHANDLE disk, BOOL complete )
{
#ifdef TRACING
	TRACE(_T("LoadMFT (disk: %s, BOOL: %i)\r\n"), CString(disk->DosDevice), complete );
#endif

	DWORD                  read;
	LARGE_INTEGER         offset;
	UCHAR                  *buf;
	PFILE_RECORD_HEADER    file;
	PNONRESIDENT_ATTRIBUTE nattr;
	PNONRESIDENT_ATTRIBUTE nattr2;

	read = 0;

	offset.QuadPart = 0;
	offset.HighPart = 0;
	offset.LowPart = 0;
	
	buf = NULL;
	file = NULL;
	nattr = NULL;
	nattr2 = NULL;

	//char VALID_magic_string[5] = "FILE";

	char VALID_magic_string[ 4 ] = { 'F', 'I', 'L', 'E' };

	//char VALID_magic_string[ 4 ];
	//VALID_magic_string[ 0 ] = 'F';
	//VALID_magic_string[ 1 ] = 'I';
	//VALID_magic_string[ 2 ] = 'L';
	//VALID_magic_string[ 3 ] = 'E';

	
	//file->AllocatedBytesThisRecord = 0;
	//file->Flags = 0;
	//file->HardLink_Count = 0;
	//file->NextAttributeID = 0;
	//file->Ntfs.LogFileSequenceNumber = 0;
	//file->Ntfs.magic_number_MFT_record_header = 0;
	//file->Ntfs.UpdateSequenceArray_Offset = 0;
	//file->Ntfs.UpdateSequenceArray_Size = 0;
	//file->OffsetToFirstAttribute = 0;
	//file->ReferenceBaseFileRecord.HighPart = 0;
	//file->ReferenceBaseFileRecord.LowPart = 0;
	//file->ReferenceBaseFileRecord.QuadPart = 0;
	//file->SequenceNumber = 0;
	//file->UsedSizeOfThisRecord = 0;

	//nattr->AllocatedSize = 0;
	//nattr->CompressedSize = 0;
	//nattr->DataSize = 0;
	//nattr->InitializedSize = 0;
	


	//nattr2->AllocatedSize = 0;
	//nattr2->CompressedSize = 0;
	//nattr2->DataSize = 0;
	//nattr2->InitializedSize = 0;

	if ( disk == NULL ) {
		return 0;
		}

	if ( disk->type == NTFSDISK ) {
		offset = disk->NTFS.MFTLocation;
		/*
		  on MY computer, according to `fsutil fsinfo ntfsinfo C:`:
						MFT start LCN is 786432 (hex:c0000)
						MFT valid data length is 1,638,662,144 (hex: 61ac0000)
						MFT2 start LCN is  2
						MFT zone start is 43,262,592 (hex: 2942280)
						MFT zone end is   43,287,680 (hex: 2948480)

						512 bytes per sector
						4096 bytes per physical sector
						4096 bytes per cluster
						1024 bytes per FileRecord segment

		  on MY computer, according to `nfi C: 0`
						Logical sector 0  -> $Boot -> file #7
							$STANDARD_INFORMATION  -> resident
							$FILE_NAME             -> resident
							$SECURITY_DESCRIPTOR   -> resident
							$DATA                  -> nonresident
								logical sectors 0-15 (0x0-0xf)

		  on MY computer, according to `nfi C: 16`
						Logical sector 16 (0x10)   -> $MftMirr -> file #1
							$STANDARD_INFORMATION  -> resident
							$FILE_NAME             -> resident
							$DATA                  -> nonresident
								logical sectors 16-23 (0x10-0x17)

		  on MY computer, according to `nfi C: 24`
						Logical sector 24  -> $UpCase -> file #10
							$STANDARD_INFORMATION  -> resident
							$FILE_NAME             -> resident
							$DATA                  -> nonresident
								Logical sectors 24-279 (0x18-0x117)
							$DATA $Info            -> resident
		*/
		SetFilePointer ( disk->fileHandle, offset.LowPart, ( PLONG ) &offset.HighPart, FILE_BEGIN );
		buf = new UCHAR[ disk->NTFS.BytesPerCluster ];
		BOOL succ = ReadFile      ( disk->fileHandle, buf, disk->NTFS.BytesPerCluster, &read, NULL );
		if ( !succ ) {
			return (ULONGLONG)-1;
			}
		file = ( PFILE_RECORD_HEADER ) ( buf );
		BOOL couldFix = FixFileRecord( file );
		if ( file->Ntfs.magic_number_MFT_record_header[0] == VALID_magic_string[0] && file->Ntfs.magic_number_MFT_record_header[1] == VALID_magic_string[1] && file->Ntfs.magic_number_MFT_record_header[2] == VALID_magic_string[2] && file->Ntfs.magic_number_MFT_record_header[3] == VALID_magic_string[3] && couldFix) {//why are we breaking type safety??
#ifdef TRACING
			TRACE( _T( "Ntfs.type: %u\r\n" ),file->Ntfs.magic_number_MFT_record_header);
#endif
			PFILENAME_ATTRIBUTE fn;
				PLONGFILEINFO data = ( PLONGFILEINFO ) buf;
			PATTRIBUTE    attr = ( PATTRIBUTE ) ( ( PUCHAR ) ( file ) + file->OffsetToFirstAttribute );
			int stop = min( 8, file->NextAttributeID );
			data->Flags = file->Flags;
			for ( int i = 0; i < stop; i++ ) {//CANNOT be vectorized!
				if ( ( attr->AttributeType < 0 ) || ( attr->AttributeType > 0x100 ) ) {
					break;
					}
				switch ( attr->AttributeType )
				{
					case AttributeList:
						/*
						  Now it gets tricky!
							we have to rebuild the data attribute
							walk down the list to find all runarrays
							use ReadAttribute to get the list
						  I think, the right order is important			
						  find out how to walk down the list !!!!
						  the only solution for now
						*/
						return 3;
						break;
					case Data:
						nattr  = ( PNONRESIDENT_ATTRIBUTE ) attr;
						break;
					case Bitmap:
						nattr2 = ( PNONRESIDENT_ATTRIBUTE ) attr;
					default:
						break;
				};

				if ( attr->Length > 0 && attr->Length < file->UsedSizeOfThisRecord ) {
					attr = PATTRIBUTE( PUCHAR( attr ) + attr->Length );
					}
				else {
					if ( attr->Nonresident == TRUE ) {
						attr = PATTRIBUTE( PUCHAR( attr ) + sizeof( NONRESIDENT_ATTRIBUTE ) );
						}
					}
				data = NULL;
				}
			if ( nattr  == NULL ) {
				return NULL;
				}
			if ( nattr2 == NULL ) {
				return NULL;
				}
			}
		else {
			return NULL;
			}
		disk->NTFS.sizeMFT    = ( DWORD ) nattr->DataSize;
		disk->NTFS.MFT        = buf;
		disk->NTFS.entryCount = disk->NTFS.sizeMFT / disk->NTFS.BytesPerFileRecord;
		delete buf;
		buf = NULL;
		
		return nattr->DataSize;
	}
	return 0;
};

PATTRIBUTE FindAttribute(PFILE_RECORD_HEADER file, ATTRIBUTE_TYPE type)
{
	if ( file == NULL || type == NULL ) {
		return NULL;
		}
	PATTRIBUTE attr = ( PATTRIBUTE ) ( ( PUCHAR ) ( file ) +file->OffsetToFirstAttribute );

	for ( int i = 1; i < file->NextAttributeID; i++ ) {//CANNOT be vectorized!
		if ( attr->AttributeType == type ) {
			return attr;
			}
		
		if ( attr->AttributeType<1 || attr->AttributeType>0x100 ) {
			break;
			}
		if ( attr->Length>0 && attr->Length < file->UsedSizeOfThisRecord ) {
			attr = PATTRIBUTE( PUCHAR( attr ) + attr->Length );
			}
		else {
			if ( attr->Nonresident == TRUE ) {
				attr = PATTRIBUTE( PUCHAR( attr ) + sizeof( NONRESIDENT_ATTRIBUTE ) );
				}
			}
		}
	return NULL;
}

DWORD ParseMFT( PDISKHANDLE disk, UINT option, PSTATUSINFO info ) {//disk->NTFS.MFT not set properly!
	
	PUCHAR                 buffer;
	PUCHAR                 end;
	PFILE_RECORD_HEADER    fh;
	PRESIDENT_ATTRIBUTE    attr;
	PNONRESIDENT_ATTRIBUTE nattr;;
	LONGFILEINFO           *data;
	DWORD index = 0;

	FETCHPROC fetch;
	//fetch = FetchFileInfo;

	if ( disk == NULL ) {
		return 0;
		}	
	if ( disk->type == NTFSDISK ) {
#ifdef TRACING
		TRACE( _T( "ParseMFT\r\n" ) );
#endif
		CreateFixList();
		fh = PFILE_RECORD_HEADER( disk->NTFS.MFT );//null filehandle?
		FixFileRecord( fh );
		disk->IsLong = sizeof( SEARCHFILEINFO );

#ifdef TRACING
		TRACE(_T("Disk IsLong: %lld\r\n"), disk->IsLong );
#endif
		if ( disk->heapBlock == NULL ) {
			 disk->heapBlock = CreateHeap( 0x100000 );
#ifdef TRACING
			TRACE(_T("disk->heapBlock == NULL!!!!!!\r\n") );
#endif

			}
		nattr = (PNONRESIDENT_ATTRIBUTE) FindAttribute(fh, Data); 
		
		if (nattr != NULL) {
			buffer = new UCHAR[ CLUSTERSPERREAD*disk->NTFS.BytesPerCluster ];
			ReadMFTParse( disk, nattr, 0, ULONG( nattr->HighVcn ) + 1, buffer, NULL, info );
			delete[] buffer;
			}	
		ProcessFixList(disk);
		}
	return 0;
}

DWORD ReadMFTParse(PDISKHANDLE disk, PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, ULONG count, PVOID buffer, FETCHPROC fetch, PSTATUSINFO info)
{
	ULONGLONG lcn;
	ULONGLONG runcount;
	ULONG     readcount;
	ULONG     left;
	DWORD     ret = 0;
	PUCHAR    bytes = PUCHAR( buffer );
	PUCHAR    data;

	int x        = ( disk->NTFS.entryCount + 16 )*sizeof( SEARCHFILEINFO );
	data         = new UCHAR[ x ];
	disk->fFiles = ( PSEARCHFILEINFO ) data;
	memset( data, 0, x );

#ifdef TRACING
	TRACE( _T( "ReadMFTParse \r\n" ) );
	//TRACE( _T("Count: %lu\r\n"), count );
	//TRACE( _T("vcn: %llu\r\n"), vcn );
	//TRACE( _T( "Disk->filesSize: %lld\r\n" ), disk->filesSize );
	//TRACE( _T( "Disk->realFiles: %lld\r\n" ), disk->realFiles );
#endif
	for (left = count; left > 0; left -= readcount) {
		FindRun( attr, vcn, &lcn, &runcount );
		readcount = ULONG( min( runcount, left ) );
		ULONG n = readcount * disk->NTFS.BytesPerCluster;
		if ( lcn == 0 ) {
			// spares file?//sparse?
			memset( bytes, 0, n );
			}
		else {
			ret += ReadMFTLCN( disk, lcn, readcount, buffer, fetch, info );
			}
		vcn += readcount;
		bytes += n;
	}
#ifdef TRACING
	TRACE( _T( "ReadMFTParse: count: %lu, vcn: %llu\r\n" ), count, vcn);
	//TRACE( _T( "vcn: %llu\r\n" ), vcn );
	//TRACE( _T( "Disk->filesSize: %lld\r\n" ), disk->filesSize );
	//TRACE( _T( "Disk->realFiles: %lld\r\n" ), disk->realFiles );
	//TRACE( _T("bytes: "));
	TRACE( _T( "ReadMFTParse: ret: %lld\r\n" ), ret );
#endif
	return ret;
}

ULONG RunLength(PUCHAR run)
{
	// I guess it must be this way
	return ( *run & 0xf ) + ( ( *run >> 4 ) & 0xf ) + 1;
}

LONGLONG RunLCN(PUCHAR run)
{
#ifdef TRACING
	//TRACE( _T( "RunLCN returning lcn: %llu\r\n" ), lcn );
	TRACE( _T( "RunLCN\r\n" ) );
#endif

	UCHAR n1     =   *run & 0xf;
	UCHAR n2     = ( *run >> 4 ) & 0xf;
	LONGLONG lcn = ( (n2 == 0) ? ( 0 ) : ( CHAR( run[ n1 + n2 ] ) ) );

	for ( LONG i = n1 + n2 - 1; i > n1; i-- ) {//CANNOT be vectorized; could be if i was incremented instead of decremented
		lcn = ( lcn << 8 ) + run[ i ];
		}

	return lcn;
}

ULONGLONG RunCount(PUCHAR run)
{

#ifdef TRACING
	//TRACE( _T( "RunCount returning count: %llu\r\n" ), count );
	TRACE( _T( "Runcount\r\n" ) );
#endif

	// count the runs we have to process
	UCHAR k         = *run & 0xf;
	ULONGLONG count = 0;

	for ( ULONG i = k; i > 0; i-- ) {//cannot be vectorized? Could be if i was incremented instead of decremented
		count = ( count << 8 ) + run[ i ];
		}
	return count;
}

BOOL FindRun(PNONRESIDENT_ATTRIBUTE attr, ULONGLONG vcn, PULONGLONG lcn, PULONGLONG count)
{
	if ( vcn < attr->LowVcn || vcn > attr->HighVcn ) {
		return FALSE;
		}
	*lcn = 0;

#ifdef TRACING
	TRACE( _T( "FindRun\r\n" ) );
#endif

	ULONGLONG base = attr->LowVcn;

	for ( PUCHAR run = PUCHAR( PUCHAR( attr ) + attr->RunArrayOffset ); *run != 0; run += RunLength( run ) ) {//CANNOT be vectorized!
		*lcn  += RunLCN  ( run );
		*count = RunCount( run );
		if (base <= vcn && vcn < base + *count) {
			*lcn    = ( ( RunLCN( run ) == 0 ) ? 0 : ( *lcn + vcn - base ) );
			*count -= ULONG( vcn - base );
#ifdef TRACING
			//TRACE( _T( "FindRun lcn: %llu, count: %llu, vcn: %llu, base: %llu; RETURNING TRUE!\r\n" ), *lcn, *count, vcn, base );
			TRACE( _T( "FindRun: RETURNING TRUE!\r\n" ) );
#endif
			return TRUE;
			}
		else {
			base += *count;
			}
		}
#ifdef TRACING
	//TRACE( _T( "FindRun: lcn: %llu, count: %llu, vcn: %llu, base: %llu\r\n" ), *lcn, *count, vcn, base );
	TRACE( _T( "FindRun: returning FALSE!\r\n" ) );
#endif

	return FALSE;
}

DWORD ReadMFTLCN(PDISKHANDLE disk, ULONGLONG lcn, ULONG count, PVOID buffer, FETCHPROC fetch, PSTATUSINFO info)
{
	LARGE_INTEGER offset;
	DWORD         read = 0;
	DWORD         ret  = 0;
	DWORD         cnt  = 0;
	DWORD         c    = 0;
	DWORD         pos  = 0;
#ifdef TRACING
	TRACE( _T( "ReadMFTLCN\r\n" ) );
#endif
	offset.QuadPart = lcn*disk->NTFS.BytesPerCluster;
	SetFilePointer( disk->fileHandle, offset.LowPart, &offset.HighPart, FILE_BEGIN );

	cnt = count / CLUSTERSPERREAD;

	for ( unsigned int i = 1; i <= cnt; i++ ) {
		BOOL readRes = ReadFile( disk->fileHandle, buffer, CLUSTERSPERREAD*disk->NTFS.BytesPerCluster, &read, NULL );
		if ( readRes == 0 ) {
			LPVOID lpMsgBuf;
			LPVOID lpDisplayBuf;
			DWORD err = GetLastError( );
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
			LPCTSTR msg = (LPCTSTR)lpMsgBuf;

			//lpDisplayBuf = ( LPVOID ) LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR ) lpMsgBuf )* sizeof( TCHAR ) ) );
			MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
	#ifdef TRACING
			TRACE(_T("Error: %s\r\n"), msg);
	#endif
		}

		c   += CLUSTERSPERREAD;
		pos += read;
		ProcessBuffer( disk, ( PUCHAR ) buffer, read, fetch );
		CallMe( info, disk->filesSize );
		}

	BOOL readres = ReadFile     ( disk->fileHandle, buffer, ( count - c )*disk->NTFS.BytesPerCluster, &read, NULL );
	if ( readres == 0 ) {
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD err = GetLastError( );
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL );
		LPCTSTR msg = (LPCTSTR)lpMsgBuf;

		//lpDisplayBuf = ( LPVOID ) LocalAlloc( LMEM_ZEROINIT, ( lstrlen( ( LPCTSTR ) lpMsgBuf )* sizeof( TCHAR ) ) );
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);
#ifdef TRACING
		TRACE(_T("Error: %s\r\n"), msg);
#endif

		}
	ProcessBuffer( disk,  ( PUCHAR ) buffer, read, fetch );
	CallMe( info, disk->filesSize );
	
	pos += read;
	return pos;
}

DWORD inline ProcessBuffer(PDISKHANDLE disk, PUCHAR buffer, DWORD size, FETCHPROC fetch)
{
	PUCHAR end;
	PUCHAR data;
	DWORD  count = 0;
	PFILE_RECORD_HEADER fh;
	end   = PUCHAR( buffer ) + size;
	data  = PUCHAR( disk->fFiles );
	data += sizeof(SEARCHFILEINFO) * disk->filesSize;
#ifdef TRACING
	TRACE( _T( "ProcessBuffer\r\n" ) );
#endif
	while ( buffer<end ){//CANNOT be vectorized (function calls, non-incrementing variable)
		fh = PFILE_RECORD_HEADER( buffer );
		FixFileRecord( fh );
		if ( FetchSearchInfo( disk, fh, data )>0 ) {
			disk->realFiles++;
			}
		buffer += disk->NTFS.BytesPerFileRecord;
		data += sizeof( SEARCHFILEINFO );
		disk->filesSize++;
		}
	return 0;
}

LPWSTR GetPath(PDISKHANDLE disk, int id)
{
	int    PathStackPos = 0;
	int    a            = id;
	int    CurrentPos   = 0;
	PUCHAR ptr          = ( PUCHAR ) disk->sFiles;
	static WCHAR glPath[0xffff];
	DWORD  pt;
	DWORD  PathStack[ 64 ];

#ifdef TRACING
	TRACE( _T( "GetPath\r\n" ) );
#endif

	//PathStackPos = 0;
	for ( int i = 0; i < 64; i++ ) {//CANNOT be vectorized
		PathStack[ PathStackPos++ ] = a;
		pt = a*disk->IsLong;
		a  = PSEARCHFILEINFO( ptr + pt )->ParentId.LowPart;
		if ( a == 0 || a == 5 ) {
			break;
			}
		}
	if ( disk->DosDevice != NULL ) {
		glPath[ 0 ] = disk->DosDevice;
		glPath[ 1 ] = L':';
		CurrentPos  = 2;
		}
	else {
		glPath[ 0 ] = L'\0';
		}
	for ( int i = PathStackPos - 1; i > 0; i-- ) {//CANNOT be vectorized; "Loop includes assignments that are of different sizes."
		pt = PathStack[ i ] * disk->IsLong;
		glPath[ CurrentPos++ ] = L'\\';
		memcpy( &glPath[ CurrentPos ], PSEARCHFILEINFO( ptr + pt )->FileName, PSEARCHFILEINFO( ptr + pt )->FileNameLength * 2 );
		CurrentPos += PSEARCHFILEINFO( ptr + pt )->FileNameLength;
		}
	glPath[ CurrentPos ]     = L'\\';
	glPath[ CurrentPos + 1 ] = L'\0';
	return glPath;
}

LPWSTR GetCompletePath(PDISKHANDLE disk, int id)
{
	int    i;
	int    a            = id;
	int    PathStackPos = 0;
	int    CurrentPos   = 0;
	PUCHAR ptr          = ( PUCHAR ) disk->sFiles;
	static WCHAR glPath[0xffff];
	DWORD  pt;
	DWORD  PathStack[ 64 ];

#ifdef TRACING
	TRACE( _T( "GetCompletePath\r\n" ) );
#endif

	for ( int i = 0; i < 64; i++ ) {
		PathStack[ PathStackPos++ ] = a;
		pt = a*disk->IsLong;
		a  = PSEARCHFILEINFO( ptr + pt )->ParentId.LowPart;
		
		if ( a == 0 || a == 5 ) {
			break;
			}
		}
	if ( disk->DosDevice != NULL ) {
		glPath[ 0 ] = disk->DosDevice;
		glPath[ 1 ] = L':';
		CurrentPos  = 2;
		}
	else {
		glPath[ 0 ] = L'\0';
		}
	for ( int i = PathStackPos - 1; i >= 0; i-- ) {
		pt = PathStack[ i ] * disk->IsLong;
		glPath[ CurrentPos++ ] = L'\\';
		memcpy( &glPath[ CurrentPos ], PSEARCHFILEINFO( ptr + pt )->FileName, PSEARCHFILEINFO( ptr + pt )->FileNameLength * 2 );
		CurrentPos += PSEARCHFILEINFO( ptr + pt )->FileNameLength;
		}
	glPath[ CurrentPos ] = L'\0';
	return glPath;
}

VOID inline CallMe(PSTATUSINFO info, DWORD value)
{
	if ( info != NULL ) {
		SendMessage( info->hWnd, PBM_SETPOS, value, 0 );
		}
}


DWORD inline FetchSearchInfo(PDISKHANDLE disk, PFILE_RECORD_HEADER file, PUCHAR buf)
{

	/*
	  THIS FUNCTION IS CALLED ALOT!
	  I can't even TRACE it, because that makes it unbearably slow!
	*/

	const char* VALID_magic_string = "FILE";
	const char* VALID_magic_string2 = "INDX";


	int i;
	
	PFILENAME_ATTRIBUTE fn;
	PLONGFILEINFO       data = ( PLONGFILEINFO ) buf;
	PATTRIBUTE          attr = ( PATTRIBUTE ) ( ( PUCHAR ) ( file ) +file->OffsetToFirstAttribute );
	//PATTRIBUTE attrlist = NULL;
	int stop = min( 8, file->NextAttributeID );
	
	if ( file->Ntfs.magic_number_MFT_record_header == VALID_magic_string || file->Ntfs.magic_number_MFT_record_header == VALID_magic_string2 ) {
		data->Flags = file->Flags;
	
		for ( i = 0; i < stop; i++ ) {//CANNOT be vectorized (return)
			if ( attr->AttributeType<0 || attr->AttributeType>0x100 ) {
				break;
				}
			switch ( attr->AttributeType )
			{
				case FileName:
					fn = PFILENAME_ATTRIBUTE( PUCHAR( attr ) + PRESIDENT_ATTRIBUTE( attr )->ValueOffset );
					if ( fn->NameType & WIN32_NAME || fn->NameType == 0 ) {
						fn->Name[ fn->NameLength ]         = L'\0';
						data->FileName                     = AllocAndCopyString( disk->heapBlock, fn->Name, fn->NameLength );
#ifdef TRACING
						TRACE( _T( "Fetched file %s\r\n" ), data->FileName );
#endif
						data->FileNameLength               = min( fn->NameLength, wcslen( data->FileName ) );
						data->ParentId.QuadPart            = fn->DirectoryFileReferenceNumber;
						data->ParentId.HighPart           &= 0x0000ffff;
						if ( file->ReferenceBaseFileRecord.LowPart != 0 ){// && file->BaseFileRecord.HighPart !=0x10000)
							AddToFixList( file->ReferenceBaseFileRecord.LowPart, disk->filesSize );
							}
						return sizeof( SEARCHINFO );
						}
					break;
				default:
					break;
			};
			if ( attr->Length > 0 && attr->Length < file->UsedSizeOfThisRecord ) {
				attr = PATTRIBUTE( PUCHAR( attr ) + attr->Length );
				}
			else {
				if ( attr->Nonresident == TRUE ) {
					attr = PATTRIBUTE( PUCHAR( attr ) + sizeof( NONRESIDENT_ATTRIBUTE ) );
					}
				}
			}
		}
	return 0;
}



BOOL FixFileRecord(PFILE_RECORD_HEADER file)
{
	/*
	  THIS FUNCTION IS CALLED ALOT!
	  I can't even TRACE it, because that makes it unbearably slow!
	*/

	//int sec = 2048;
	if ( file == NULL ) {
		return FALSE;
		}
	PUSHORT UpdateSequenceArray    = PUSHORT( PUCHAR( file ) + file->Ntfs.UpdateSequenceArray_Offset );
	PUSHORT sector = PUSHORT( file );

	if ( file->Ntfs.UpdateSequenceArray_Size > 4 ) {
		return FALSE;
		}
	for ( ULONG i = 1; i < file->Ntfs.UpdateSequenceArray_Size; i++ ) {//CANNOT be vectorized -> induction variable not local or upper bound is not loop-invariant. Will Ntfs.UpdateSequenceArray_Size change?
		sector[ 255 ] = UpdateSequenceArray[ i ];
		sector       += 256;
		}
	return TRUE;
}

BOOL ReparseDisk(PDISKHANDLE disk, UINT option, PSTATUSINFO info)
{
	if ( disk != NULL ) {

#ifdef TRACING
		TRACE( _T( "ReparseDisk\r\n" ) );
#endif
		
		if ( disk->type == NTFSDISK ) {
			if ( disk->NTFS.MFT != NULL ) {
				delete disk->NTFS.MFT;
				}
			disk->NTFS.MFT = NULL;
			if ( disk->NTFS.Bitmap != NULL ) {
				delete disk->NTFS.Bitmap;
				}
			disk->NTFS.Bitmap = NULL;
			}
		if ( disk->heapBlock != NULL ) {
			ReUseBlocks( disk->heapBlock, FALSE );
			}
		if ( disk->sFiles != NULL ) {
			delete disk->sFiles;
			}
		disk->sFiles    = NULL;
		disk->filesSize = 0;
		disk->realFiles = 0;

		if ( LoadMFT( disk, FALSE ) != 0 ) {
			ParseMFT( disk, option, info );
			}
		return TRUE;
		}
	return FALSE;
};

/*PUCHAR AllocData(PHEAPBLOCK block, DWORD size)
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

	if (tmp->next !=NULL)
	{
		tmp = tmp->next;
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
			//goto here;
		}

	}
	else
	{
here:	tmp = (PHEAPBLOCK) malloc(sizeof(HEAPBLOCK));
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
	}
	return NULL;

}
*/

