// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#include "stdafx.h"

#pragma once 
#ifndef WDS_DIRECTORY_ENUMERATION_CPP
#define WDS_DIRECTORY_ENUMERATION_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "directory_enumeration.h"
#include "dirstatdoc.h"
#include "globalhelpers.h"
#include "TreeListControl.h"
#include <WinIoCtl.h>


#ifdef new
#pragma push_macro("new")
#define WDS_DIRECTORY_ENUMERATION_PUSHED_MACRO_NEW
#undef new
#endif

namespace {
	struct FILEINFO final {
		FILEINFO( ) { }
		FILEINFO( const FILEINFO& in ) = delete;
		FILEINFO& operator=( const FILEINFO& in ) = delete;

		FILEINFO& operator=( FILEINFO&& in ) {
			length = std::move( in.length );
			lastWriteTime = std::move( in.lastWriteTime );
			attributes = std::move( in.attributes );
			name = std::move( in.name );
			return ( *this );
			}


		FILEINFO( FILEINFO&& in ) {
			length = std::move( in.length );
			lastWriteTime = std::move( in.lastWriteTime );
			attributes = std::move( in.attributes );
			name = std::move( in.name );
			}


		FILEINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ] ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ) {
	#ifdef DEBUG
			if ( length > 34359738368 ) {
				_CrtDbgBreak( );
				}
		#endif
			}

		FILEINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_ std::wstring cFileName ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( std::move( cFileName ) ) {
	#ifdef DEBUG
			if ( length > 34359738368 ) {
				_CrtDbgBreak( );
				}
		#endif
			}


		void reset( ) {
			length = 0;
			lastWriteTime.dwLowDateTime  = 0;
			lastWriteTime.dwHighDateTime = 0;
			attributes = INVALID_FILE_ATTRIBUTES;
			name.clear( );
			}


		bool operator<( const FILEINFO& rhs ) const {
			return length < rhs.length;
			}

		std::uint64_t length;
		FILETIME      lastWriteTime;
		//C4820: 'FILEINFO' : '4' bytes padding added after data member 'FILEINFO::attributes'
		DWORD         attributes;
		std::wstring  name;
		};

	struct DIRINFO final {
		DIRINFO( ) { }
		DIRINFO( DIRINFO&& in ) {
			length = std::move( in.length );
			lastWriteTime = std::move( in.lastWriteTime );
			attributes = std::move( in.attributes );
			name = std::move( in.name );
			path = std::move( in.path );
			}

		DIRINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ], _In_ std::wstring path_ ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ), path( std::move( path_ ) ) { }

		std::uint64_t length;
		FILETIME      lastWriteTime;
		//C4820: 'DIRINFO' : '4' bytes padding added after data member 'DIRINFO::attributes'
		DWORD         attributes;
		std::wstring       name;
		std::wstring       path;
		};

	struct pair_of_item_and_path final {
		CTreeListItem* ptr;
		std::wstring path;
		};

	}


std::vector<std::pair<CTreeListItem*, std::wstring>> addFiles_returnSizesToWorkOn( _Inout_ CTreeListItem* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path );
//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
bool DoSomeWork ( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse = false );


WDS_DECLSPEC_NOTHROW void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring path ) {
	//ASSERT( path.back( ) == L'*' );
	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;
	fDataHand = FindFirstFileExW( path.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, 0 );
	//FILETIME t;
	//FILEINFO fi;
	//zeroFILEINFO( fi );
	//memset_zero_struct( fi );
	//fi.reset( );
	BOOL findNextFileRes = TRUE;
	while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( findNextFileRes != 0 ) ) {
		const auto scmpVal  = wcscmp( fData.cFileName, L".." );
		const auto scmpVal2 = wcscmp( fData.cFileName, L"." );
		if ( ( scmpVal == 0 ) || ( scmpVal2 == 0 ) ) {//This branches on the return of IsDirectory, then checks characters 0,1, & 2//IsDirectory calls MatchesMask, which bitwise-ANDs dwFileAttributes with FILE_ATTRIBUTE_DIRECTORY
			findNextFileRes = FindNextFileW( fDataHand, &fData );
			continue;//No point in operating on ourselves!
			}
		else if ( fData.dwFileAttributes bitand FILE_ATTRIBUTE_DIRECTORY ) {
			ASSERT( path.substr( path.length( ) - 3, 3 ).compare( L"*.*" ) == 0 );
			const auto alt_path_this_dir( path.substr( 0, path.length( ) - 3 ) + fData.cFileName );
			directories.emplace_back( DIRINFO { UINT64_ERROR,
												std::move( fData.ftLastWriteTime ),
												std::move( fData.dwFileAttributes ),
												fData.cFileName,
												std::move( alt_path_this_dir )
											  }
									);
			}
		else {
			files.emplace_back( FILEINFO {  std::move( ( static_cast<std::uint64_t>( fData.nFileSizeHigh ) * ( static_cast<std::uint64_t>( MAXDWORD ) + 1 ) ) + static_cast<std::uint64_t>( fData.nFileSizeLow ) ), 
											fData.ftLastWriteTime,
											fData.dwFileAttributes,
											fData.cFileName
										 }
							  );
			}
		findNextFileRes = FindNextFileW( fDataHand, &fData );
		}
	if ( fDataHand == INVALID_HANDLE_VALUE ) {
		return;
		}
	VERIFY( FindClose( fDataHand ) );
	}

namespace {





	//end copied & pasted


	PCWSTR const NTFS_SPECIAL_FILES[] = {
		/*L"$MFT",*/ //Will query manually with: FSCTL_GET_NTFS_VOLUME_DATA
		L"$MFTMirr",
		L"$LogFile",
		L"$Volume",
		L"$AttrDef",
		L"$Bitmap",
		L"$Boot",
		L"$BadClus",
		L"$Secure",
		L"$Upcase",
		L"$Extend"
	};


	//returning false means that we were denied CreateFileW access to the volume
	HRESULT get_NTFS_volume_data( const std::wstring& raw_dir_path, _Inout_ std::vector<FILEINFO>& files, _Always_( _Out_ ) DWORD* const result_code ) {
		const HANDLE root_volume_handle = CreateFileW( raw_dir_path.c_str( ), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0 );
		if ( root_volume_handle == INVALID_HANDLE_VALUE ) {
			const DWORD last_err = ::GetLastError( );
			TRACE( L"Failed to open \"file\" `%s`, Error: %lu\r\n", raw_dir_path.c_str( ), last_err );
			(*result_code) = last_err;
			return E_FAIL;
			}
		
		//const rsize_t size_of_base_struct = sizeof( NTFS_VOLUME_DATA_BUFFER );
		NTFS_VOLUME_DATA_BUFFER data_buf = { 0 };

		DWORD bytes_returned = 0u;

		const BOOL data_res = DeviceIoControl( root_volume_handle, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0u, &data_buf, sizeof( NTFS_VOLUME_DATA_BUFFER ), &bytes_returned, NULL );
		if ( data_res == 0 ) {
			const DWORD last_err = ::GetLastError( );
			TRACE( _T( "FSCTL_GET_NTFS_VOLUME_DATA failed! Error: %lu\r\n" ), last_err );
			close_handle( root_volume_handle );
			(*result_code) = last_err;

			
			return E_FAIL;
			}

		TRACE( _T( "FSCTL_GET_NTFS_VOLUME_DATA succeeded!\r\n" ) );
		files.emplace_back( FILEINFO {  static_cast<std::uint64_t>( data_buf.MftValidDataLength.QuadPart ), 
										FILETIME{ 0 }, //fData.ftLastWriteTime, - GetFileTime/GetFileInformationByHandle
										static_cast<DWORD>( FILE_ATTRIBUTE_HIDDEN bitor FILE_ATTRIBUTE_SYSTEM bitor FILE_ATTRIBUTE_READONLY ),//fData.dwFileAttributes,
										L"$MFT"
										}
							);

		close_handle( root_volume_handle );
		
		(*result_code) = NO_ERROR;
		return S_OK;
		}

	__forceinline WDS_DECLSPEC_NOTHROW void GetCompressedFileSize_failure_handler( const std::wstring& path, const ULARGE_INTEGER ret, const DWORD last_err ) {
#ifdef DEBUG
		const rsize_t error_message_buffer_size = 128;
		_Null_terminated_ wchar_t error_message_buffer[ error_message_buffer_size ] = { 0 };
		rsize_t chars_written = 0;
		const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( error_message_buffer, error_message_buffer_size, chars_written, last_err );
		if ( ret.HighPart != NULL ) {
			if ( last_err != NO_ERROR ) {
				if ( SUCCEEDED( fmt_res ) ) {
					TRACE( _T( "Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
					return;
					}
				TRACE( _T( "Error! Filepath: %s, Filepath length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
				return;
				}
			if ( SUCCEEDED( fmt_res ) ) {
				TRACE( _T( "WTF ERROR! File path: %s, File path length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
				return;
				}
			TRACE( _T( "WTF ERROR! File path: %s, File path length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
			return;
			}
		if ( last_err != NO_ERROR ) {
			if ( SUCCEEDED( fmt_res ) ) {
				TRACE( _T( "Error! File path: %s, File path length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
				return;
				}
			TRACE( _T( "Error! File path: %s, File path length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
			return;
			}
#endif
		return;
		}


	_Success_( return != UINT64_MAX )//using string here means that we pay for 'free' on return
	__forceinline WDS_DECLSPEC_NOTHROW std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
		ULARGE_INTEGER ret;
		ret.QuadPart = 0;//zero initializing this is critical!
		ret.LowPart = GetCompressedFileSizeW( path.c_str( ), &ret.HighPart );
		const DWORD last_err = GetLastError( );
		if ( ret.QuadPart == INVALID_FILE_SIZE ) {
			if ( ret.HighPart != NULL ) {
				GetCompressedFileSize_failure_handler( path, ret, last_err );
				if ( last_err != NO_ERROR ) {
					return UINT64_ERROR;// IN case of an error return size from CFileFind object
					}
				return UINT64_ERROR;
				}
			else {
				if ( last_err != NO_ERROR ) {
					GetCompressedFileSize_failure_handler( path, ret, last_err );
					return UINT64_ERROR;
					}
				return ret.QuadPart;
				}
			}
		return ret.QuadPart;
		}

	void query_special_file_fallback( const std::wstring& path, _Inout_ std::vector<FILEINFO>& files, _In_z_ PCWSTR const special_file_name ) { 
		TRACE( L"Trying fallback method (GetCompressedFileSize) for \"file\" `%s`\r\n", path.c_str( ) );
		const std::uint64_t file_size = GetCompressedFileSize_filename( path );
		if ( file_size == UINT64_ERROR ) {
			const DWORD last_err = ::GetLastError( );
			TRACE( L"Fallback method for \"file\" `%s` failed!, Error: %lu\r\n", path.c_str( ), last_err );
			
			const rsize_t err_buff_size = 512u;
			wchar_t err_buff[ err_buff_size ] = { 0 };
			rsize_t unused_chars_written;
			const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, unused_chars_written, last_err );
			if ( SUCCEEDED( fmt_res ) ) {
				OutputDebugStringW( L"WinDirStat: " );
				OutputDebugStringW( err_buff );
				OutputDebugStringW( L"\r\n" );
				TRACE( L"%s\r\n", err_buff );
				}
			else {
				TRACE( L"Failed to format error message for error encountered when trying to open `%s`\r\n", path.c_str( ) );
				}
			return;
			}



		files.emplace_back( FILEINFO {  static_cast<std::uint64_t>( file_size ), 
										FILETIME{ 0 }, //fData.ftLastWriteTime, - GetFileTime/GetFileInformationByHandle
										static_cast<DWORD>( FILE_ATTRIBUTE_HIDDEN bitor FILE_ATTRIBUTE_SYSTEM bitor FILE_ATTRIBUTE_READONLY ),//fData.dwFileAttributes,
										special_file_name
										}
							);

		}

	void query_single_special_file_failed_to_open( const std::wstring& path, _Inout_ std::vector<FILEINFO>& files, _In_z_ PCWSTR const special_file_name ) {
			const DWORD last_err = ::GetLastError( );
			TRACE( L"Failed to open \"file\" `%s`, Error: %lu\r\n", path.c_str( ), last_err );
			
			const rsize_t err_buff_size = 512u;
			wchar_t err_buff[ err_buff_size ] = { 0 };
			rsize_t unused_chars_written;
			const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, unused_chars_written, last_err );
			if ( SUCCEEDED( fmt_res ) ) {
				OutputDebugStringW( L"WinDirStat: " );
				OutputDebugStringW( err_buff );
				OutputDebugStringW( L"\r\n" );
				TRACE( L"%s\r\n", err_buff );
				}
			else {
				TRACE( L"Failed to format error message for error encountered when trying to open `%s`\r\n", path.c_str( ) );
				}
			return query_special_file_fallback( path, files, special_file_name );
		}

	void query_single_special_file( const std::wstring& path, _Inout_ std::vector<FILEINFO>& files, _In_z_ PCWSTR const special_file_name ) {
		const HANDLE special_file_handle = CreateFileW( path.c_str( ), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0 );
		if ( special_file_handle == INVALID_HANDLE_VALUE ) {
			return query_single_special_file_failed_to_open( path, files, special_file_name );
			}


		const rsize_t file_full_dir_info_size_with_name = ( sizeof( FILE_FULL_DIR_INFO ) + ( sizeof( std::declval<FILE_FULL_DIR_INFO>( ).FileName[ 0 ] ) * MAX_PATH ) );
		
		
		
		//DO NOT ACCESS DIRECTLY!
		char file_full_dir_info_buffer[ file_full_dir_info_size_with_name ] = { 0 };


		FILE_FULL_DIR_INFO* const file_info = reinterpret_cast< FILE_FULL_DIR_INFO* >( &( file_full_dir_info_buffer[ 0 ] ) );

		static_assert( __alignof( decltype(  *file_info ) ) == __alignof( FILE_FULL_DIR_INFO ) == __alignof( file_full_dir_info_buffer ), "bad casting/alignment!" );

		const BOOL file_info_success = ::GetFileInformationByHandleEx( special_file_handle, FileFullDirectoryInfo, file_info, file_full_dir_info_size_with_name );
		
		if ( file_info_success == 0 ) {
			const DWORD last_err = ::GetLastError( );
			TRACE( L"Failed to get file information by handle for \"file\": `%s`, Error: %lu\r\n", path.c_str( ), last_err );
			
			const rsize_t err_buff_size = 512u;
			wchar_t err_buff[ err_buff_size ] = { 0 };
			rsize_t unused_chars_written;
			const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( err_buff, err_buff_size, unused_chars_written, last_err );
			if ( SUCCEEDED( fmt_res ) ) {
				OutputDebugStringW( L"WinDirStat: " );
				OutputDebugStringW( err_buff );
				OutputDebugStringW( L"\r\n" );
				TRACE( L"%s\r\n", err_buff );
				}
			else {
				TRACE( L"Failed to format error message for error encountered when trying to get information for `%s`\r\n", path.c_str( ) );
				}
			return;
			}

		(void)files;//temporarily suppress warning

		close_handle( special_file_handle );
		}

	void query_special_files( const std::wstring& path, _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories ) {
		for ( size_t i = 0u; i < _countof( NTFS_SPECIAL_FILES ); ++i ) {
			std::wstring path_to_special_file( path );
			path_to_special_file.append( L"\\" );
			path_to_special_file.append( NTFS_SPECIAL_FILES[ i ] );
			TRACE( L"Trying to query special file `%s`...\r\n", path_to_special_file.c_str( ) );
			query_single_special_file( path_to_special_file, files, NTFS_SPECIAL_FILES[ i ] );
			FindFilesLoop( files, directories, path_to_special_file );
			}
		}

	std::wstring fixup_path_to_MFT( _In_ const std::wstring& path ) {
		std::wstring raw_dir_path( path );
		ASSERT( path.length( ) > 5 );
		if ( path.length( ) > 5 ) {
			ASSERT( path.compare( 0, 4, L"\\\\?\\", 4 ) == 0 );
			raw_dir_path = std::wstring( path.cbegin( ) + 4, path.cbegin( ) + 6 );
			TRACE( L"raw_dir_path: `%s`\r\n", raw_dir_path.c_str( ) );
			}
		return raw_dir_path;
		}

	
	//return indicates whether we should ask user to elevate
	bool handle_NTFS_special_files( _In_ const std::wstring& path, _In_ const CDirstatApp* const app, _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories ) {
		std::wstring raw_dir_path( fixup_path_to_MFT( path ) );


		const bool is_mount_point = app->m_mountPoints.IsVolume( raw_dir_path + L"\\" );
		ASSERT( is_mount_point == app->m_mountPoints.IsVolume( raw_dir_path ) );
		if ( !is_mount_point ) {
			TRACE( _T( "Path: `%s` is NOT a volume, so it can't have NTFS special files in it's first level child directory!\r\n" ), raw_dir_path.c_str( ) );
			return false;
			}

		//path + _T( "\\*.*" )
		//

		ASSERT( raw_dir_path.length( ) > 0 );

		if ( raw_dir_path.back( ) == L'\\' ) {
			raw_dir_path.pop_back( );
			}

		ASSERT( raw_dir_path.length( ) > 0 );
		ASSERT( raw_dir_path.back( ) == L':' );

		raw_dir_path = ( L"\\\\.\\" + raw_dir_path );


		DWORD result_code = 0;
		const HRESULT query_volume_data = get_NTFS_volume_data( raw_dir_path, files, &result_code );

		//if we got ERROR_ACCESS_DENIED, assume that the problem will be fixed by elevating.
		if ( FAILED( query_volume_data ) && ( result_code == ERROR_ACCESS_DENIED ) ) {
			return true;
			}

		query_special_files( raw_dir_path, files, directories );

		return false;
		}
	



	WDS_DECLSPEC_NOTHROW void compose_compressed_file_size_and_fixup_child( CTreeListItem* const child, const std::wstring path ) {
		const auto size_child = GetCompressedFileSize_filename( path );
		if ( size_child != UINT64_MAX ) {
			ASSERT( child != NULL );
			if ( child != NULL ) {
				child->m_size = std::move( size_child );
				}
			}
		else {
			TRACE( _T( "ERROR returned by GetCompressedFileSize! file: %s\r\n" ), child->m_name );
			child->m_attr.invalid = true;
			}
		}

	//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
	WDS_DECLSPEC_NOTHROW std::vector<std::future<bool>> start_workers( std::vector<std::pair<CTreeListItem*, std::wstring>> dirs_to_work_on, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in ) {
		const auto dirsToWorkOnCount = dirs_to_work_on.size( );
		std::vector<std::future<bool>> workers;
		workers.reserve( dirsToWorkOnCount );
		for ( size_t i = 0; i < dirsToWorkOnCount; ++i ) {
			ASSERT( dirs_to_work_on[ i ].second.length( ) > 1 );
			ASSERT( dirs_to_work_on[ i ].second.back( ) != L'\\' );
			ASSERT( dirs_to_work_on[ i ].second.back( ) != L'*' );
			//TODO: investigate task_group
			//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
			workers.emplace_back( std::async( DoSomeWork, std::move( dirs_to_work_on[ i ].first ), std::move( dirs_to_work_on[ i ].second ), app, sizes_to_work_on_in, std::move( false ) ) );
			}
		return workers;
		}

	//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
	WDS_DECLSPEC_NOTHROW void size_workers( _In_ concurrency::concurrent_vector<pair_of_item_and_path>* sizes ) {
		//std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> sizesToWorkOn_;
		std::vector<std::future<void>> sizesToWorkOn_;
		TRACE( _T( "need to get the compressed size for %I64u files!\r\n" ), std::uint64_t( sizes->size( ) ) );
		sizesToWorkOn_.reserve( sizes->size( ) + 1 );
		const auto number_sizes = sizes->size( );
		for ( size_t i = 0; i < number_sizes; ++i ) {
			sizesToWorkOn_.emplace_back( std::async( compose_compressed_file_size_and_fixup_child, ( *sizes )[ i ].ptr, ( *sizes )[ i ].path ) );
			}

		for ( size_t i = 0; i < number_sizes; ++i ) {
			sizesToWorkOn_[ i ].get( );
			}

		//process_vector_of_compressed_file_futures( sizesToWorkOn_ );
		}


	std::wstring::size_type WDS_DECLSPEC_NOTHROW sum_string_length( _In_ const std::vector<FILEINFO>& vecFiles, _In_ const std::vector<DIRINFO>& vecDirs ) {
		//TODO: BUGBUG: need +1 here, else ASSERT( ( m_buffer_filled + new_name_length ) < m_buffer_size ) fails!
		std::wstring::size_type total_length = 1u;
		for ( const auto& aFile : vecFiles ) {
			total_length += static_cast<std::uint64_t>( aFile.name.length( ) );
		
			//for the null terminator
			++total_length;
			}
		for ( const auto& aDir : vecDirs ) {
			total_length += static_cast<std::uint64_t>( aDir.name.length( ) );
		
			//for the null terminator
			++total_length;
			}
		return total_length;
		}

	std::vector<std::pair<CTreeListItem*, std::wstring>> launch_directory_workers( _Inout_ CTreeListItem* const ThisCItem, _In_ const CDirstatApp* const app, _In_ const rsize_t dirCount, _In_ const rsize_t vecFiles_size, _In_ const std::vector<DIRINFO>& vecDirs ) {
		std::vector<std::pair<CTreeListItem*, std::wstring>> dirsToWorkOn;
		dirsToWorkOn.reserve( dirCount );
		const auto thisOptions = GetOptions( );

		//ASSERT( static_cast< size_t >( ThisCItem->m_childCount ) == vecFiles.size( ) );

		std::uint32_t total_so_far = static_cast<std::uint32_t>( vecFiles_size );

		//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
		for ( const auto& dir : vecDirs ) {
			const bool dontFollow = ( app->m_mountPoints.IsJunctionPoint( dir.path, dir.attributes ) && !thisOptions->m_followJunctionPoints ) || ( app->m_mountPoints.IsMountPoint( dir.path ) && !thisOptions->m_followMountPoints );
			const auto new_name_length = dir.name.length( );
			ASSERT( new_name_length < UINT16_MAX );


			ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );

			PWSTR new_name_ptr = nullptr;
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, dir.name );
			const HRESULT copy_res = ThisCItem->m_child_info.m_child_info_ptr->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), dir.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (directory) name str! (readJobNotDoneWork)(aborting!)" );
				displayWindowsMsgBoxWithMessage( dir.name.c_str( ) );
				}
			else {
				//                                                                                               IT_DIRECTORY
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				const auto newitem = new ( &( ThisCItem->m_child_info.m_child_info_ptr->m_children[ total_so_far ] ) ) CTreeListItem { static_cast< std::uint64_t >( UINT64_ERROR ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), dontFollow, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };

				//++( ThisCItem->m_childCount );
				++total_so_far;
				//ThisCItem->m_children_vector.emplace_back( newitem );

				if ( !newitem->m_attr.m_done ) {
					ASSERT( !dontFollow );
					dirsToWorkOn.emplace_back( std::move( std::make_pair( std::move( newitem ), std::move( dir.path ) ) ) );
					}
				else {
					ASSERT( dontFollow );
					newitem->m_size = 0;
					}
				}
			}
		ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );
		ThisCItem->m_child_info.m_child_info_ptr->m_childCount = total_so_far;
		return dirsToWorkOn;
		}


	_Pre_satisfies_( !ThisCItem->m_attr.m_done ) WDS_DECLSPEC_NOTHROW std::pair<std::pair<std::vector<std::pair<CTreeListItem*, std::wstring>>,std::vector<std::pair<CTreeListItem*, std::wstring>>>, bool> readJobNotDoneWork( _Inout_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* const app ) {
		std::vector<FILEINFO> vecFiles;
		std::vector<DIRINFO>  vecDirs;

		vecFiles.reserve( 50 );//pseudo-arbitrary number

		bool should_we_elevate_temp = false;
		if ( ThisCItem->m_parent == NULL ) {
			should_we_elevate_temp = handle_NTFS_special_files( path, app, vecFiles, vecDirs );
			}

		const bool should_we_elevate = should_we_elevate_temp;

		FindFilesLoop( vecFiles, vecDirs, std::move( path + _T( "\\*.*" ) ) );

		//std::sort( vecFiles.begin( ), vecFiles.end( ) );

		const auto fileCount = vecFiles.size( );
		const auto dirCount  = vecDirs.size( );
	
		const auto total_count = ( fileCount + dirCount );


		if ( total_count == 0 ) {
			return std::make_pair( std::make_pair( std::vector<std::pair<CTreeListItem*, std::wstring>>( ), std::vector<std::pair<CTreeListItem*, std::wstring>>( ) ), should_we_elevate );
			}

		static_assert( sizeof( std::wstring::value_type ) == sizeof( wchar_t ), "WTF" );
		const std::wstring::size_type total_size_alloc = sum_string_length( vecFiles, vecDirs );

		ThisCItem->m_child_info.~child_info_block_manager( );
		new ( &( ThisCItem->m_child_info ) ) child_info_block_manager( total_size_alloc, total_count );
		//ThisCItem->m_child_info = std::make_unique<child_info>( total_size_alloc, total_count );
		//ThisCItem->m_child_info->m_children.reset( new CTreeListItem[ total_count ] );

		//ThisCItem->m_name_pool.reset( total_size_alloc );
		
		//ThisCItem->m_child_info->m_name_pool.reset( total_size_alloc );

		//ASSERT( ThisCItem->m_name_pool.m_buffer_filled == 0 );
		ASSERT( ThisCItem->m_child_info.m_child_info_ptr->m_name_pool.m_buffer_filled == 0 );

		//ASSERT( path.back( ) != _T( '\\' ) );
		//sizesToWorkOn_ CANNOT BE CONST!!
		auto sizesToWorkOn_ = addFiles_returnSizesToWorkOn( ThisCItem, vecFiles, path );
		auto dirsToWorkOn = launch_directory_workers( ThisCItem, app, dirCount, fileCount, vecDirs );


		ASSERT( ThisCItem->m_child_info.m_child_info_ptr->m_name_pool.m_buffer_filled == ( total_size_alloc - 1 ) );
		//ASSERT( ( fileCount + dirCount ) == ThisCItem->m_childCount );
		//ThisCItem->m_children_vector.shrink_to_fit( );
		return std::make_pair( std::make_pair( std::move( dirsToWorkOn ), std::move( sizesToWorkOn_ ) ), should_we_elevate );
		}

	_Success_( return < UINT64_ERROR )
	const std::uint64_t get_uncompressed_file_size( const std::wstring path ) {
		const HANDLE file_handle = CreateFileW( path.c_str( ), ( FILE_READ_ATTRIBUTES bitor FILE_READ_EA ), ( FILE_SHARE_READ bitor FILE_SHARE_WRITE ), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( file_handle == INVALID_HANDLE_VALUE ) {
		
			const rsize_t str_size = 128u;
			wchar_t str_buff[ str_size ] = { 0 };
			rsize_t chars_written = 0;
			const auto last_error = GetLastError( );
			const HRESULT err_msg_fmt_res = CStyle_GetLastErrorAsFormattedMessage( str_buff, str_size, chars_written, last_error );
			if ( SUCCEEDED( err_msg_fmt_res ) ) {
				TRACE( _T( "get_uncompressed_file_size (%s) failed, as we failed to open the file! File open error message:\r\n\t%s\r\n" ), path.c_str( ), str_buff );
				return UINT64_ERROR;
				}
			TRACE( _T( "DOUBLE FAULT! get_uncompressed_file_size (%s) failed! Error getting error message!\r\n" ), path.c_str( ) );

			return UINT64_ERROR;
			}

		LARGE_INTEGER large_integer = { 0 };


		//If [GetFileSizeEx] fails, the return value is zero. To get extended error information, call GetLastError.
		const BOOL file_size_result = GetFileSizeEx( file_handle, &large_integer );
		close_handle( file_handle );

		if ( file_size_result != 0 ) {
			ASSERT( large_integer.QuadPart >= 0 );
			return static_cast< const std::uint64_t >( large_integer.QuadPart );
			}
#ifdef DEBUG
		//If [GetFileAttributes] succeeds, the return value contains the attributes of the specified file or directory. If [GetFileAttributes] fails, the return value is INVALID_FILE_ATTRIBUTES. To get extended error information, call GetLastError.
		const DWORD attrib = GetFileAttributesW( path.c_str( ) );
		ASSERT( attrib != INVALID_FILE_ATTRIBUTES );
		if ( attrib != INVALID_FILE_ATTRIBUTES ) {
			if ( attrib == FILE_ATTRIBUTE_DIRECTORY ) {
				TRACE( _T( "Couldn't get uncompressed size for `%s`, which is expected, as it's a directory!\r\n" ), path.c_str( ) );
				return UINT64_ERROR;
				}
			TRACE( _T( "get_uncompressed_file_size (%s) failed!! This doesn't make sense, because it's not a directory.\r\n" ), path.c_str( ) );
			return UINT64_ERROR;
			}
		TRACE( _T( "get_uncompressed_file_size (%s) failed!! What really doesn't make sense, is that we failed to get attributes for that file.\r\n" ), path.c_str( ) );
#endif

		//TODO: trace error message
		return UINT64_ERROR;
		}

	}






WDS_DECLSPEC_NOTHROW std::vector<std::pair<CTreeListItem*, std::wstring>> addFiles_returnSizesToWorkOn( _Inout_ CTreeListItem* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CTreeListItem*, std::wstring>> sizesToWorkOn_;
	std::sort( vecFiles.begin( ), vecFiles.end( ) );
	sizesToWorkOn_.reserve( vecFiles.size( ) );

	ASSERT( path.back( ) != _T( '\\' ) );

	std::uint32_t total_so_far = 0u;

	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );

			ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );

			PWSTR new_name_ptr = nullptr;
			//ThisCItem->m_name_pool.copy_name_str_into_buffer
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_child_info.m_child_info_ptr->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (compressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				//                                                                                            IT_FILE
				auto newChild = ::new ( &( ThisCItem->m_child_info.m_child_info_ptr->m_children[ total_so_far ] ) ) CTreeListItem { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
				//sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( std::async( GetCompressedFileSize_filename, std::move( path + _T( '\\' ) + aFile.name  ) ) ) );
				sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( path + _T( '\\' ) + aFile.name  ) );
				}
			}
		else {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );
			PWSTR new_name_ptr = nullptr;
			
			ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );
			if ( ThisCItem->m_child_info.m_child_info_ptr == nullptr ) {
				displayWindowsMsgBoxWithMessage( L"ThisCItem->m_child_info.m_child_info_ptr is NULL!! THIS SHOULD NEVER HAPPEN! (terminating)" );
				std::terminate( );

				//so /analyze understands
				abort( );
				return sizesToWorkOn_;

				}
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_child_info.m_child_info_ptr->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (uncompressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				//                                                                            IT_FILE
				::new ( &( ThisCItem->m_child_info.m_child_info_ptr->m_children[ total_so_far ] ) ) CTreeListItem { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				}
			}
		//detect overflows. highly unlikely.
		//ASSERT( ThisCItem->m_childCount < 4294967290 );
		//++( ThisCItem->m_childCount );
		++total_so_far;
		}
	ASSERT( ThisCItem->m_child_info.m_child_info_ptr != nullptr );
	ThisCItem->m_child_info.m_child_info_ptr->m_childCount = total_so_far;
	return sizesToWorkOn_;
	}




//TODO: WTF IS THIS DOING HERE??!?
_Pre_satisfies_( this->m_parent == NULL ) void CTreeListItem::AddChildren( _In_ CTreeListControl* const tree_list_control ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_attr.m_done );
	if ( m_parent == NULL ) {
		tree_list_control->OnChildAdded( NULL, this, false );
		}
	}

WDS_DECLSPEC_NOTHROW std::pair<DOUBLE, bool> DoSomeWorkShim( _Inout_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx
	//ASSERT( ThisCItem->m_childCount == 0 );
	ASSERT( ThisCItem->m_child_info.m_child_info_ptr == nullptr );
	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	ASSERT( strcmp_path == 0 );
	if ( strcmp_path != 0 ) {
		auto fixedPath( L"\\\\?\\" + path );
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}
	if ( path.back( ) == L'\\' ) {
		path.pop_back( );
		}
	concurrency::concurrent_vector<pair_of_item_and_path> sizes_to_work_on;

	const auto qpc_1 = help_QueryPerformanceCounter( );
	
	const bool should_we_elevate = DoSomeWork( std::move( ThisCItem ), std::move( path ), app, &sizes_to_work_on, std::move( isRootRecurse ) );
	
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );

	const auto timing = ( static_cast<double>( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf.QuadPart ) ) );
	const rsize_t debug_buf_size = 96;
	_Null_terminated_ wchar_t debug_buf[ debug_buf_size ] = { 0 };
	const auto debug_buf_res_1 = _snwprintf_s( debug_buf, debug_buf_size, _TRUNCATE, L"WDS: enum timing: %f\r\n", timing );
	ASSERT( debug_buf_res_1 != -1 );
	if ( debug_buf_res_1 == -1 ) {
		//NOTE TO SELF, TODO: BUGBUG: OutputDebugString calls RtlUnicodeStringToAnsiString
		OutputDebugStringW( global_strings::output_dbg_string_error );
		//std::terminate( );
		}

	//NOTE TO SELF, TODO: BUGBUG: OutputDebugString calls RtlUnicodeStringToAnsiString
	OutputDebugStringW( debug_buf );


	const auto qpc_3 = help_QueryPerformanceCounter( );

	size_workers( &sizes_to_work_on );

	const auto qpc_4 = help_QueryPerformanceCounter( );
	const auto qpf_2 = help_QueryPerformanceFrequency( );
	const auto timing_2 = ( static_cast<double>( qpc_4.QuadPart - qpc_3.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf_2.QuadPart ) ) );

	
	_Null_terminated_ wchar_t debug_buf_2[ debug_buf_size ] = { 0 };
	const auto debug_buf_res_2 = _snwprintf_s( debug_buf_2, debug_buf_size, _TRUNCATE, L"WDS: compressed file timing: %f\r\n", timing_2 );
	ASSERT( debug_buf_res_2 != -1 );
	if ( debug_buf_res_2 == -1 ) {
		//NOTE TO SELF, TODO: BUGBUG: OutputDebugString calls RtlUnicodeStringToAnsiString
		OutputDebugStringW( global_strings::output_dbg_string_error );
		//std::terminate( );
		}
	//NOTE TO SELF, TODO: BUGBUG: OutputDebugString calls RtlUnicodeStringToAnsiString
	OutputDebugStringW( debug_buf_2 );

	return std::make_pair( timing_2, should_we_elevate );
	//wait for sync?
	}

//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
WDS_DECLSPEC_NOTHROW bool DoSomeWork( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse ) {
	//This is temporary.
	UNREFERENCED_PARAMETER( isRootRecurse );

	ASSERT( wcscmp( L"\\\\?\\", path.substr( 0, 4 ).c_str( ) ) == 0 );
	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp_path != 0 ) {
		auto fixedPath( L"\\\\?\\" + path );
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}

	auto kookyPair = readJobNotDoneWork( ThisCItem, std::move( path ), app );

	auto itemsToWorkOn = kookyPair.first;

	const bool should_we_elevate = kookyPair.second;
	//if ( ThisCItem->m_child_info == nullptr ) {
	//	ASSERT( ThisCItem->m_childCount == 0 );
	//	}

	//if ( ThisCItem->m_childCount == 0 ) {
	//	ASSERT( ThisCItem->m_child_info == nullptr );
	//	}

	if ( ThisCItem->m_child_info.m_child_info_ptr == nullptr ) {
		//ASSERT( ThisCItem->m_childCount == 0 );
		ASSERT( ThisCItem->m_child_info.m_child_info_ptr == nullptr );
		ASSERT( itemsToWorkOn.first.size( ) == 0 );
		ASSERT( itemsToWorkOn.second.size( ) == 0 );
		ThisCItem->m_attr.m_done = true;
		ThisCItem->m_size = 0;
		return should_we_elevate;
		}

	//std::vector<std::pair<CItemBranch*, std::wstring>>& dirs_to_work_on = itemsToWorkOn.first;

	//auto workers = start_workers( std::move( dirs_to_work_on ), app, sizes_to_work_on_in );

	auto workers = start_workers( std::move( itemsToWorkOn.first ), app, sizes_to_work_on_in );

	//std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>& vector_of_compressed_file_futures = itemsToWorkOn.second;

	//process_vector_of_compressed_file_futures( vector_of_compressed_file_futures );


	std::vector<std::pair<CTreeListItem*, std::wstring>>& vector_of_compressed_file_futures = itemsToWorkOn.second;

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto& a_pair : vector_of_compressed_file_futures ) {
		pair_of_item_and_path the_pair;
		the_pair.ptr  = a_pair.first;
		the_pair.path = std::move( a_pair.second );
		sizes_to_work_on_in->push_back( the_pair );
		}

	for ( auto& worker : workers ) {
		
		//purposefully ignore the bool-return-value.
		//it *should* only ever be true for the root directory, which is done syncronously.
#ifdef DEBUG
		const bool verify_bool_is_false =
#else
		(void)
#endif
		
		worker.get( );

#ifdef DEBUG
		ASSERT( verify_bool_is_false == false );
#endif

		}

	ThisCItem->m_attr.m_done = true;
	return should_we_elevate;
	}

_Success_( return < UINT64_ERROR )
WDS_DECLSPEC_NOTHROW const std::uint64_t get_uncompressed_file_size( const CTreeListItem* const item ) {
	const auto derived_item = static_cast< const CTreeListItem* const >( item );
	std::wstring path_holder( derived_item->GetPath( ) );

	auto strcmp_path = path_holder.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp_path != 0 ) {
		path_holder = ( L"\\\\?\\" + path_holder );
		}
	
	return get_uncompressed_file_size( std::move( path_holder ) );

	}

#ifdef WDS_DIRECTORY_ENUMERATION_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_DIRECTORY_ENUMERATION_PUSHED_MACRO_NEW
#endif


//Yes, this is used!
SExtensionRecord::SExtensionRecord( SExtensionRecord&& in ) {
	ext = std::move( in.ext );
	files = std::move( in.files );
	bytes = std::move( in.bytes );
	color = std::move( in.color );
	}


#else

#endif