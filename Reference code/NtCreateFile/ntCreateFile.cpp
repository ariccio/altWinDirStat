#include <Windows.h>
#include <stdio.h>
#include <string>


    // From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/FILE_INFORMATION_CLASS.html
    typedef enum _FILE_INFORMATION_CLASS {
        FileDirectoryInformation                 = 1,
        FileFullDirectoryInformation,
        FileBothDirectoryInformation,
        FileBasicInformation,
        FileStandardInformation,
        FileInternalInformation,
        FileEaInformation,
        FileAccessInformation,
        FileNameInformation,
        FileRenameInformation,
        FileLinkInformation,
        FileNamesInformation,
        FileDispositionInformation,
        FilePositionInformation,
        FileFullEaInformation,
        FileModeInformation,
        FileAlignmentInformation,
        FileAllInformation,
        FileAllocationInformation,
        FileEndOfFileInformation,
        FileAlternateNameInformation,
        FileStreamInformation,
        FilePipeInformation,
        FilePipeLocalInformation,
        FilePipeRemoteInformation,
        FileMailslotQueryInformation,
        FileMailslotSetInformation,
        FileCompressionInformation,
        FileObjectIdInformation,
        FileCompletionInformation,
        FileMoveClusterInformation,
        FileQuotaInformation,
        FileReparsePointInformation,
        FileNetworkOpenInformation,
        FileAttributeTagInformation,
        FileTrackingInformation,
        FileIdBothDirectoryInformation,
        FileIdFullDirectoryInformation,
        FileValidDataLengthInformation,
        FileShortNameInformation,
        FileIoCompletionNotificationInformation,
        FileIoStatusBlockRangeInformation,
        FileIoPriorityHintInformation,
        FileSfioReserveInformation,
        FileSfioVolumeInformation,
        FileHardLinkInformation,
        FileProcessIdsUsingFileInformation,
        FileNormalizedNameInformation,
        FileNetworkPhysicalNameInformation,
        FileIdGlobalTxDirectoryInformation,
        FileIsRemoteDeviceInformation,
        FileAttributeCacheInformation,
        FileNumaNodeInformation,
        FileStandardLinkInformation,
        FileRemoteProtocolInformation,
        FileMaximumInformation
    } FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

    typedef enum  { 
      FileFsVolumeInformation       = 1,
      FileFsLabelInformation        = 2,
      FileFsSizeInformation         = 3,
      FileFsDeviceInformation       = 4,
      FileFsAttributeInformation    = 5,
      FileFsControlInformation      = 6,
      FileFsFullSizeInformation     = 7,
      FileFsObjectIdInformation     = 8,
      FileFsDriverPathInformation   = 9,
      FileFsVolumeFlagsInformation  = 10,
      FileFsSectorSizeInformation   = 11
    } FS_INFORMATION_CLASS;
#ifndef NTSTATUS
#define NTSTATUS LONG
#endif
#ifndef STATUS_TIMEOUT
#define STATUS_TIMEOUT ((NTSTATUS) 0x00000102)
#endif
#ifndef STATUS_PENDING
#define STATUS_PENDING ((NTSTATUS) 0x00000103)
#endif
#ifndef STATUS_BUFFER_OVERFLOW
#define STATUS_BUFFER_OVERFLOW ((NTSTATUS) 0x80000005)
#endif

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff550671(v=vs.85).aspx
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// From http://msdn.microsoft.com/en-us/library/windows/desktop/aa380518(v=vs.85).aspx
typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff557749(v=vs.85).aspx
typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
}  OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;


// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/NtQueryInformationFile.html
// and http://msdn.microsoft.com/en-us/library/windows/hardware/ff567052(v=vs.85).aspx
typedef NTSTATUS (NTAPI *NtQueryInformationFile_t)( _In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_ PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass );


// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff567070(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtQueryVolumeInformationFile_t )( _In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_ PVOID FsInformation, _In_ ULONG Length, _In_ FS_INFORMATION_CLASS FsInformationClass );

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff566492(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtOpenDirectoryObject_t )(
	_Out_  PHANDLE DirectoryHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes );


// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff567011(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtOpenFile_t )( _Out_  PHANDLE FileHandle, _In_   ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _In_ ULONG ShareAccess, _In_ ULONG OpenOptions );

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff566424(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtCreateFile_t )( _Out_     PHANDLE FileHandle, _In_      ACCESS_MASK DesiredAccess, _In_      POBJECT_ATTRIBUTES ObjectAttributes, _Out_     PIO_STATUS_BLOCK IoStatusBlock, _In_opt_  PLARGE_INTEGER AllocationSize, _In_    ULONG FileAttributes, _In_      ULONG ShareAccess, _In_      ULONG CreateDisposition, _In_      ULONG CreateOptions, _In_opt_  PVOID EaBuffer, _In_      ULONG EaLength );

typedef NTSTATUS( NTAPI *NtClose_t )( _Out_  HANDLE FileHandle );

// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/NtQueryDirectoryFile.html
// and http://msdn.microsoft.com/en-us/library/windows/hardware/ff567047(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtQueryDirectoryFile_t )( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ void *ApcRoutine, _In_opt_ PVOID ApcContext, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_ PVOID FileInformation, _In_ ULONG Length, _In_      FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan );

// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/NtSetInformationFile.html
// and http://msdn.microsoft.com/en-us/library/windows/hardware/ff567096(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtSetInformationFile_t )( _In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _In_ PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass );

// From http://msdn.microsoft.com/en-us/library/ms648412(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtWaitForSingleObject_t )( _In_ HANDLE Handle, _In_ BOOLEAN Alertable, _In_ PLARGE_INTEGER Timeout );

typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	ULONG         FileAttributes;
	} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
	LARGE_INTEGER AllocationSize;
	LARGE_INTEGER EndOfFile;
	ULONG         NumberOfLinks;
	BOOLEAN       DeletePending;
	BOOLEAN       Directory;
	} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
	LARGE_INTEGER IndexNumber;
	} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
	ULONG EaSize;
	} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
	ACCESS_MASK AccessFlags;
	} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {
	LARGE_INTEGER CurrentByteOffset;
	} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
	ULONG Mode;
	} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {
	ULONG AlignmentRequirement;
	} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
	ULONG FileNameLength;
	WCHAR FileName[ 1 ];
	} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
	FILE_BASIC_INFORMATION     BasicInformation;
	FILE_STANDARD_INFORMATION  StandardInformation;
	FILE_INTERNAL_INFORMATION  InternalInformation;
	FILE_EA_INFORMATION        EaInformation;
	FILE_ACCESS_INFORMATION    AccessInformation;
	FILE_POSITION_INFORMATION  PositionInformation;
	FILE_MODE_INFORMATION      ModeInformation;
	FILE_ALIGNMENT_INFORMATION AlignmentInformation;
	FILE_NAME_INFORMATION      NameInformation;
	} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_FS_SECTOR_SIZE_INFORMATION {
	ULONG LogicalBytesPerSector;
	ULONG PhysicalBytesPerSectorForAtomicity;
	ULONG PhysicalBytesPerSectorForPerformance;
	ULONG FileSystemEffectivePhysicalBytesPerSectorForAtomicity;
	ULONG Flags;
	ULONG ByteOffsetForSectorAlignment;
	ULONG ByteOffsetForPartitionAlignment;
	} FILE_FS_SECTOR_SIZE_INFORMATION, *PFILE_FS_SECTOR_SIZE_INFORMATION;

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff540310(v=vs.85).aspx
typedef struct _FILE_ID_FULL_DIR_INFORMATION {
	ULONG         NextEntryOffset;
	ULONG         FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG         FileAttributes;
	ULONG         FileNameLength;
	ULONG         EaSize;
	LARGE_INTEGER FileId;
	WCHAR         FileName[ 1 ];
	} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff552012(v=vs.85).aspx
typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[ 1 ];
			} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR  PathBuffer[ 1 ];
			} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[ 1 ];
			} GenericReparseBuffer;
		};
	} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

static NtQueryInformationFile_t NtQueryInformationFile;
static NtQueryVolumeInformationFile_t NtQueryVolumeInformationFile;
static NtOpenDirectoryObject_t NtOpenDirectoryObject;
static NtOpenFile_t NtOpenFile;
static NtCreateFile_t NtCreateFile;
static NtClose_t NtClose;
static NtQueryDirectoryFile_t NtQueryDirectoryFile;
static NtSetInformationFile_t NtSetInformationFile;
static NtWaitForSingleObject_t NtWaitForSingleObject;


int main( ) {

	HANDLE res = CreateFileW( L"\\\\?\\C:\\Users\\Alexander Riccio\\Downloads\\XTU-Setup-exe.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( res == INVALID_HANDLE_VALUE ) {
		return 666;
		}
	if ( !NtQueryInformationFile )
		if ( !( NtQueryInformationFile = ( NtQueryInformationFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtQueryInformationFile" ) ) )
			abort( );
	if ( !NtQueryVolumeInformationFile )
		if ( !( NtQueryVolumeInformationFile = ( NtQueryVolumeInformationFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtQueryVolumeInformationFile" ) ) )
			abort( );
	if ( !NtOpenDirectoryObject )
		if ( !( NtOpenDirectoryObject = ( NtOpenDirectoryObject_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtOpenDirectoryObject" ) ) )
			abort( );
	if ( !NtOpenFile )
		if ( !( NtOpenFile = ( NtOpenFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtOpenFile" ) ) )
			abort( );
	if ( !NtCreateFile )
		if ( !( NtCreateFile = ( NtCreateFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtCreateFile" ) ) )
			abort( );
	if ( !NtClose )
		if ( !( NtClose = ( NtClose_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtClose" ) ) )
			abort( );
	if ( !NtQueryDirectoryFile )
		if ( !( NtQueryDirectoryFile = ( NtQueryDirectoryFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtQueryDirectoryFile" ) ) )
			abort( );
	if ( !NtSetInformationFile )
		if ( !( NtSetInformationFile = ( NtSetInformationFile_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtSetInformationFile" ) ) )
			abort( );
	if ( !NtWaitForSingleObject )
		if ( !( NtWaitForSingleObject = ( NtWaitForSingleObject_t ) GetProcAddress( GetModuleHandleA( "C:\\Windows\\System32\\ntdll.dll" ), "NtWaitForSingleObject" ) ) )
			abort( );


	//HANDLE h = nullptr;
	//IO_STATUS_BLOCK isb = { 0 };
	//OBJECT_ATTRIBUTES oa = { sizeof( oa ), 0 };
	//UNICODE_STRING _path;
	//LARGE_INTEGER AllocationSize = { 0 };
	std::wstring thePath( L"\\\\?\\C:\\Users\\Alexander Riccio\\Downloads\\XTU-Setup-exe.exe" );

	//wprintf( L"'native' path: %s\r\n", thePath.c_str( ) );

	//_path.Buffer = const_cast< wchar_t* >( thePath.c_str( ) );
	//_path.Length = ( thePath.length( ) * sizeof( wchar_t ) );
	//_path.MaximumLength = 32767;
	//oa.ObjectName = &_path;
	//auto res_2 = NtCreateFile( &h, GENERIC_READ, &oa, &isb, &AllocationSize, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, 0x4000 | 0x00200000, NULL, 0 );
	//if ( res_2 != 0 ) {
	//	return 111;
	//	}

	IO_STATUS_BLOCK info = { 0 };
	wchar_t buffer[ sizeof( FILE_ALL_INFORMATION ) / sizeof( wchar_t ) + 32769 ];
	FILE_ALL_INFORMATION &fai = *( FILE_ALL_INFORMATION * ) buffer;
	FILE_FS_SECTOR_SIZE_INFORMATION ffssi = { 0 };

	auto ntstat = NtQueryInformationFile( res, &info, &fai, sizeof( buffer ), FileAllInformation );
	if ( ntstat != 0 ) {
		return 222;
		}

	bool isCompressed = ( ( fai.BasicInformation.FileAttributes & FILE_ATTRIBUTE_COMPRESSED ) != 0 );
	auto size = fai.StandardInformation.AllocationSize;
	auto csize = fai.EaInformation.EaSize;
	wprintf( L"file: %s\r\n\t%s\r\n", thePath.c_str( ), ( isCompressed ? ( L"compressed" ) : ( L"not compressed" ) ) );
	if ( isCompressed ) {
		wprintf( L"compressed size: %lld\r\n", size.QuadPart );
		}
	else {
		wprintf( L"non-compressed size: %llu\r\n", static_cast<ULONGLONG>( csize ) );
		}

	return CloseHandle( res );
	}