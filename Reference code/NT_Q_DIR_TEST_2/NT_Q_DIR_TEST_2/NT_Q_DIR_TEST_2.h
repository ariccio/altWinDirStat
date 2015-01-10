//#define WIN32_LEAN_AND_MEAN

//We define WIN32_NO_STATUS, because there are a bunch of definitions in ntstatus.h that conflict with winnt.h; winnt.h is included first, but the definitions in ntstatus are better!
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

//#include <minwindef.h>
//#include <winternl.h>
#include <shlwapi.h>

//#include <strsafe.h>
#include <fltdefs.h>
#include <assert.h>
//#include <Fltkernel.h>
//#include <iostream>
//#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <future>
#include <regex>
#include <mutex>
#include <ntstatus.h>
#include <strsafe.h>
typedef WCHAR bufferChar;


//From `nt_kernel_stuff.hpp` in the BOOST AFIO library:

// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/FILE_INFORMATION_CLASS.html
typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
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

typedef enum {
	FileFsVolumeInformation      = 1,
	FileFsLabelInformation       = 2,
	FileFsSizeInformation        = 3,
	FileFsDeviceInformation      = 4,
	FileFsAttributeInformation   = 5,
	FileFsControlInformation     = 6,
	FileFsFullSizeInformation    = 7,
	FileFsObjectIdInformation    = 8,
	FileFsDriverPathInformation  = 9,
	FileFsVolumeFlagsInformation = 10,
	FileFsSectorSizeInformation  = 11
	} FS_INFORMATION_CLASS;
#ifndef NTSTATUS
#define NTSTATUS LONG
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(x) ((x)>=0)
#endif

#ifdef NTSTATUS
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0)
#endif
#endif

#ifndef STATUS_NO_MORE_FILES
#define STATUS_NO_MORE_FILES 0x80000006L
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#define STATUS_BUFFER_OVERFLOW 0x80000005L
#endif


#ifndef STATUS_TIMEOUT
#define STATUS_TIMEOUT               ((NTSTATUS)0x00000102L)    // winnt
#endif

#ifndef STATUS_PENDING
#define STATUS_PENDING               ((NTSTATUS)0x00000103L)    // winnt
#endif


// native bits required
#ifndef FILE_DIRECTORY_FILE
#define FILE_DIRECTORY_FILE                     0x00000001
#endif

#ifndef FILE_OPEN
#define FILE_OPEN                               0x00000001
#endif


#ifndef FILE_NON_DIRECTORY_FILE
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#endif

#ifndef FILE_DELETE_ON_CLOSE
#define FILE_DELETE_ON_CLOSE                    0x00001000
#endif


#ifndef FILE_OPEN_BY_FILE_ID
#define FILE_OPEN_BY_FILE_ID                    0x00002000
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
typedef NTSTATUS( NTAPI *NtQueryInformationFile_t )(
	/*_In_*/   HANDLE                 FileHandle,
	/*_Out_*/  PIO_STATUS_BLOCK       IoStatusBlock,
	/*_Out_*/  PVOID                  FileInformation,
	/*_In_*/   ULONG                  Length,
	/*_In_*/   FILE_INFORMATION_CLASS FileInformationClass
	);

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff567070(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtQueryVolumeInformationFile_t )(
	/*_In_*/   HANDLE               FileHandle,
	/*_Out_*/  PIO_STATUS_BLOCK     IoStatusBlock,
	/*_Out_*/  PVOID                FsInformation,
	/*_In_*/   ULONG                Length,
	/*_In_*/   FS_INFORMATION_CLASS FsInformationClass
	);

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff566492(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtOpenDirectoryObject_t )(
  /*_Out_*/  PHANDLE            DirectoryHandle,
  /*_In_*/   ACCESS_MASK        DesiredAccess,
  /*_In_*/   POBJECT_ATTRIBUTES ObjectAttributes
);


// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff567011(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtOpenFile_t )(
  /*_Out_*/  PHANDLE            FileHandle,
  /*_In_*/   ACCESS_MASK        DesiredAccess,
  /*_In_*/   POBJECT_ATTRIBUTES ObjectAttributes,
  /*_Out_*/  PIO_STATUS_BLOCK   IoStatusBlock,
  /*_In_*/   ULONG              ShareAccess,
  /*_In_*/   ULONG              OpenOptions
);

// From http://msdn.microsoft.com/en-us/library/windows/hardware/ff566424(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtCreateFile_t )(
  /*_Out_*/     PHANDLE            FileHandle,
  /*_In_*/      ACCESS_MASK        DesiredAccess,
  /*_In_*/      POBJECT_ATTRIBUTES ObjectAttributes,
  /*_Out_*/     PIO_STATUS_BLOCK   IoStatusBlock,
  /*_In_opt_*/  PLARGE_INTEGER     AllocationSize,
  /*_In_*/      ULONG              FileAttributes,
  /*_In_*/      ULONG              ShareAccess,
  /*_In_*/      ULONG              CreateDisposition,
  /*_In_*/      ULONG              CreateOptions,
  /*_In_opt_*/  PVOID              EaBuffer,
  /*_In_*/      ULONG              EaLength
);

typedef NTSTATUS( NTAPI *NtClose_t )(
  /*_Out_*/  HANDLE FileHandle
);

// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/NtQueryDirectoryFile.html
// and http://msdn.microsoft.com/en-us/library/windows/hardware/ff567047(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtQueryDirectoryFile_t )(
	/*_In_*/      HANDLE                 FileHandle,
	/*_In_opt_*/  HANDLE                 Event,
	/*_In_opt_*/  PVOID                  ApcRoutine,
	/*_In_opt_*/  PVOID                  ApcContext,
	/*_Out_*/     PIO_STATUS_BLOCK       IoStatusBlock,
	/*_Out_*/     PVOID                  FileInformation,
	/*_In_*/      ULONG                  Length,
	/*_In_*/      FILE_INFORMATION_CLASS FileInformationClass,
	/*_In_*/      BOOLEAN                ReturnSingleEntry,
	/*_In_opt_*/  PUNICODE_STRING        FileName,
	/*_In_*/      BOOLEAN                RestartScan
	);

// From http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/NT%20Objects/File/NtSetInformationFile.html
// and http://msdn.microsoft.com/en-us/library/windows/hardware/ff567096(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtSetInformationFile_t )(
	/*_In_*/   HANDLE                 FileHandle,
	/*_Out_*/  PIO_STATUS_BLOCK       IoStatusBlock,
	/*_In_*/   PVOID                  FileInformation,
	/*_In_*/   ULONG                  Length,
	/*_In_*/   FILE_INFORMATION_CLASS FileInformationClass
	);

// From http://msdn.microsoft.com/en-us/library/ms648412(v=vs.85).aspx
typedef NTSTATUS( NTAPI *NtWaitForSingleObject_t )(
	/*_In_*/  HANDLE         Handle,
	/*_In_*/  BOOLEAN        Alertable,
	/*_In_*/  PLARGE_INTEGER Timeout
	);
//
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



// Adapted from http://www.cprogramming.com/snippets/source-code/convert-ntstatus-win32-error
// Could use RtlNtStatusToDosError() instead
static inline void SetWin32LastErrorFromNtStatus( NTSTATUS ntstatus ) {
	DWORD br;
	OVERLAPPED o;

	o.Internal = ntstatus;
	o.InternalHigh = 0;
	o.Offset = 0;
	o.OffsetHigh = 0;
	o.hEvent = 0;
	GetOverlappedResult( NULL, &o, &br, FALSE );
	}
//END boost


typedef struct _FILE_DIRECTORY_INFORMATION {
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
  WCHAR         FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;



typedef NTSTATUS( NTAPI* pfnQueryDirFile )(
	HANDLE                 FileHandle,
	HANDLE                 Event,
	PVOID                  ApcRoutine,
	PVOID                  ApcContext,
	PIO_STATUS_BLOCK       IoStatusBlock,
	PVOID                  pBuffer,
	ULONG                  bufferSize,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN                ReturnSingleEntry,
	PUNICODE_STRING        FileName,
	BOOLEAN                RestartScan
);

typedef NTSTATUS( NTAPI* pfnOpenFile )(
	 PHANDLE            phFile,
	 ACCESS_MASK        amPerms,
	 OBJECT_ATTRIBUTES* pAttrs,
	 PIO_STATUS_BLOCK   ioStatus,
	 PLARGE_INTEGER     allocationSize,
	 ULONG              fileAttributes,
	 ULONG              ShareAccess,
	 ULONG              CreateDisposition,
	 ULONG              CreateOptions,
	 PVOID              EaBuffer,
	 ULONG              EaLength
);

typedef NTSTATUS( NTAPI* pfnSetInfoFile )( HANDLE hFile, IO_STATUS_BLOCK* iosb, PVOID pData, ULONG dataLen, FILE_INFORMATION_CLASS infoClass );

typedef DECLSPEC_ALIGN( 8 ) struct _FILE_ID_BOTH_DIR_INFORMATION {
	ULONG  NextEntryOffset;
	ULONG  FileIndex;
	LARGE_INTEGER  CreationTime;
	LARGE_INTEGER  LastAccessTime;
	LARGE_INTEGER  LastWriteTime;
	LARGE_INTEGER  ChangeTime;
	LARGE_INTEGER  EndOfFile;
	LARGE_INTEGER  AllocationSize;
	ULONG          FileAttributes;
	ULONG          FileNameLength;
	ULONG          EaSize;
	CCHAR          ShortNameLength;
	WCHAR          ShortName[ 12 ];
	LARGE_INTEGER  FileId;
	WCHAR          FileName[ 1 ];
	} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;




struct NtQueryDirectoryFile_f {
	typedef NTSTATUS( NTAPI* pfnQueryDirFile )( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan );

	pfnQueryDirFile ntQueryDirectoryFuncPtr = nullptr;

	NtQueryDirectoryFile_f( ) : ntQueryDirectoryFuncPtr( nullptr ) { }

	NtQueryDirectoryFile_f( _In_ FARPROC ntQueryDirectoryFuncPtr_IN ) : ntQueryDirectoryFuncPtr( reinterpret_cast<pfnQueryDirFile>( ntQueryDirectoryFuncPtr_IN ) ) {
		if ( ntQueryDirectoryFuncPtr == nullptr ) {
			throw -1;
			}
		}

	NtQueryDirectoryFile_f( NtQueryDirectoryFile_f& in ) = delete;
	bool init( FARPROC ntqfptr ) {
		if ( ntqfptr != nullptr ) {
			ntQueryDirectoryFuncPtr = reinterpret_cast< pfnQueryDirFile >( ntqfptr );
			return true;
			}
		return false;
		}

	_Success_( NT_SUCCESS( return ) )  NTSTATUS NTAPI operator()( _In_ HANDLE FileHandle, _In_opt_ HANDLE Event, _In_opt_ PVOID ApcRoutine, _In_opt_ PVOID ApcContext, _Out_  IO_STATUS_BLOCK* IoStatusBlock, _Out_  PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, _In_ BOOLEAN ReturnSingleEntry, _In_opt_ PUNICODE_STRING FileName, _In_ BOOLEAN RestartScan );

	bool operator!( ) {
		return ( ntQueryDirectoryFuncPtr == nullptr );
		}

	operator bool( ) {
		return ( ntQueryDirectoryFuncPtr != nullptr );
		}

	};

struct NtdllWrap {
	HMODULE ntdll = nullptr;
	//FARPROC ntQueryDirectoryFuncPtr = nullptr;

	NtQueryDirectoryFile_f NtQueryDirectoryFile;
	
	NtdllWrap( );
	NtdllWrap( NtdllWrap& in ) = delete;
	};


std::pair<std::uint64_t, std::uint64_t> ListDirectory( _In_ std::wstring dir, _In_ const bool writeToScreen, _In_ NtdllWrap* ntdll );

void FormatError( _Out_ _Out_writes_z_( msgSize ) PWSTR msg, size_t msgSize ) {
	// Retrieve the system error message for the last-error code

	const size_t bufSize = 256;
	wchar_t lpMsgBuf[ bufSize ] = { 0 };
	//LPTSTR lpDisplayBuf = NULL;
	DWORD dw = GetLastError( );

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), lpMsgBuf, bufSize, NULL );

	// Display the error message and exit the process

	//lpDisplayBuf = LPTSTR( LocalAlloc( LMEM_ZEROINIT, ( 40 * sizeof( TCHAR ) ) ) );
	StringCchPrintf( msg, msgSize, L"error %lu: %s", dw, lpMsgBuf );
	//MessageBox( NULL, ( LPCTSTR ) lpDisplayBuf, TEXT( "Error" ), MB_OK );


	//StringCchCopy( msg, msgSize, lpDisplayBuf );
	//LocalFree( lpMsgBuf );
	//LocalFree( lpDisplayBuf );
	}
