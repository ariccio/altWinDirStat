#include "stdafx.h"


namespace NativeAPI {
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

#ifndef STATUS_TIMEOUT
#define STATUS_TIMEOUT ((NTSTATUS) 0x00000102)
#endif

#ifndef STATUS_PENDING
#define STATUS_PENDING ((NTSTATUS) 0x00000103)
#endif

#ifndef STATUS_BUFFER_OVERFLOW
#define STATUS_BUFFER_OVERFLOW ((NTSTATUS) 0x80000005)
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


	//From `nt_kernel_stuff.hpp` in the BOOST AFIO library:
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

	/*
    static inline std::filesystem::path ntpath_from_dospath(std::filesystem::path p)
    {
        // This is pretty easy thanks to a convenient symlink in the NT kernel root directory ...
        std::filesystem::path base("\\??");
        base/=p;
        return base;
    }
	*/

	static inline std::wstring ntpath_from_dospath( std::wstring p ) {
		// This is pretty easy thanks to a convenient symlink in the NT kernel root directory ...
		std::wstring base( L"\\??" );
		//operator /= appears to just append a path separator and rhs. Ick.
		base += L'\\';
		base += p;
		//base /= p;
		return base;
		}



	/*
    static inline std::filesystem::path dospath_from_ntpath(std::filesystem::path p)
    {
        auto first=++p.begin();
        if(*first=="??")
            p=std::filesystem::path(p.native().begin()+4, p.native().end());
        return p;
    }
	*/

	static inline std::wstring dospath_from_ntpath( std::wstring p ) {
		//auto first = ++p.begin( );
		if ( ( p.at( 2 ) == L'?' ) && ( p.at( 3 ) == L'?' ) ) {
			p = std::wstring( p.begin( ) + 4, p.end( ) );
			}
		return p;
		}


	//slightly modified from the boost version.
	static inline void doinit( ) {

		HMODULE ntdll_temp;
		const BOOL mod_handle_res = GetModuleHandleExW( GET_MODULE_HANDLE_EX_FLAG_PIN, L"NTDLL.DLL", &ntdll_temp );
		if ( mod_handle_res == 0 ) {
			//failed!
			const auto last_err = GetLastError( );
			fwprintf( stderr, L"Failed to get handle to ntdll! Err: %lu\r\n", last_err );
			
			//TODO: don't abort!
			abort( );
			}
		const HMODULE ntdll = ntdll_temp;
		if ( !NtQueryInformationFile ) {
			NtQueryInformationFile = reinterpret_cast<NtQueryInformationFile_t>( GetProcAddress( ntdll, "NtQueryInformationFile" ) );
			if ( NtQueryInformationFile == NULL ) {
				abort( );
				}
			}
		if ( !NtQueryVolumeInformationFile ) {
			NtQueryVolumeInformationFile = reinterpret_cast<NtQueryVolumeInformationFile_t>( GetProcAddress( ntdll, "NtQueryVolumeInformationFile" ) );
			if ( NtQueryVolumeInformationFile == NULL ) {
				abort( );
				}
			}
		if ( !NtOpenDirectoryObject ) {
			NtOpenDirectoryObject = reinterpret_cast<NtOpenDirectoryObject_t>( GetProcAddress( ntdll, "NtOpenDirectoryObject" ) );
			if ( NtOpenDirectoryObject == NULL ) {
				abort( );
				}
			}
		if ( !NtOpenFile ) {
			NtOpenFile = reinterpret_cast<NtOpenFile_t>( GetProcAddress( ntdll, "NtOpenFile" ) );
			if ( NtOpenFile == NULL ) {
				abort( );
				}
			}
		if ( !NtCreateFile ) {
			NtCreateFile = reinterpret_cast<NtCreateFile_t>( GetProcAddress( ntdll, "NtCreateFile" ) );
			if ( NtCreateFile == NULL ) {
				abort( );
				}
			}
		if ( !NtClose ) {
			NtClose = reinterpret_cast<NtClose_t>( GetProcAddress( ntdll, "NtClose" ) );
			if ( NtClose == NULL ) {
				abort( );
				}
			}
		if ( !NtQueryDirectoryFile ) {
			NtQueryDirectoryFile = reinterpret_cast<NtQueryDirectoryFile_t>( GetProcAddress( ntdll, "NtQueryDirectoryFile" ) );
			if ( NtQueryDirectoryFile == NULL ) {
				abort( );
				}
			}
		if ( !NtSetInformationFile ) {
			NtSetInformationFile = reinterpret_cast<NtSetInformationFile_t>( GetProcAddress( ntdll, "NtSetInformationFile" ) );
			if ( NtSetInformationFile == NULL ) {
				abort( );
				}
			}
		if ( !NtWaitForSingleObject ) {
			NtWaitForSingleObject = reinterpret_cast<NtWaitForSingleObject_t>( GetProcAddress( ntdll, "NtWaitForSingleObject" ) );
			if ( NtWaitForSingleObject == NULL ) {
				abort( );
				}
			}
		}


	static inline void init( ) {
		static bool initialised = false;
		if ( !initialised ) {
			doinit( );
			initialised = true;
			}
		}



	//END boost

	}