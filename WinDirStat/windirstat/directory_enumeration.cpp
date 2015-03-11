#include "stdafx.h"

#pragma once 
#ifndef WDS_DIRECTORY_ENUMERATION_CPP
#define WDS_DIRECTORY_ENUMERATION_CPP

#pragma message( "Including `" __FILE__ "`..." )

#include "directory_enumeration.h"
#include "dirstatdoc.h"
#include "globalhelpers.h"
#include "TreeListControl.h"


#ifdef new
#pragma push_macro("new")
#define WDS_DIRECTORY_ENUMERATION_PUSHED_MACRO_NEW
#undef new
#endif

//struct FILEINFO;

//forward declaration
//void FindFilesLoop ( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring path );

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


std::vector<std::pair<CTreeListItem*, std::wstring>> addFiles_returnSizesToWorkOn( _In_ CTreeListItem* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path );
//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
void DoSomeWork ( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse = false );


WDS_DECLSPEC_NOTHROW void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring path ) {
	ASSERT( path.back( ) == L'*' );
	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;
	fDataHand = FindFirstFileExW( path.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, 0 );
	//FILETIME t;
	FILEINFO fi;
	//zeroFILEINFO( fi );
	//memset_zero_struct( fi );
	fi.reset( );
	BOOL findNextFileRes = TRUE;
	while ( ( fDataHand != INVALID_HANDLE_VALUE ) && ( findNextFileRes != 0 ) ) {
		auto scmpVal  = wcscmp( fData.cFileName, L".." );
		auto scmpVal2 = wcscmp( fData.cFileName, L"." );
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


	
	

	_Success_( return != UINT64_MAX )//using string here means that we pay for 'free' on return
	__forceinline WDS_DECLSPEC_NOTHROW std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
		ULARGE_INTEGER ret;
		ret.QuadPart = 0;//zero initializing this is critical!
		ret.LowPart = GetCompressedFileSizeW( path.c_str( ), &ret.HighPart );
		const auto last_err = GetLastError( );
#ifdef DEBUG
		const rsize_t error_message_buffer_size = 128;
		_Null_terminated_ wchar_t error_message_buffer[ error_message_buffer_size ] = { 0 };
		rsize_t chars_written = 0;
		const HRESULT fmt_res = CStyle_GetLastErrorAsFormattedMessage( error_message_buffer, error_message_buffer_size, chars_written, last_err );
#endif
		if ( ret.QuadPart == INVALID_FILE_SIZE ) {
			if ( ret.HighPart != NULL ) {
				if ( last_err != NO_ERROR ) {
#ifdef DEBUG
					if ( SUCCEEDED( fmt_res ) ) {
						TRACE( _T( "Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
						}
					else {
						TRACE( _T( "Error! Filepath: %s, Filepath length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
						}
#endif
					return UINT64_ERROR;// IN case of an error return size from CFileFind object
					}
#ifdef DEBUG
				if ( SUCCEEDED( fmt_res ) ) {
					TRACE( _T( "WTF ERROR! File path: %s, File path length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
					}
				else {
					TRACE( _T( "WTF ERROR! File path: %s, File path length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
					}
#endif
				return UINT64_ERROR;
				}
			else {
				if ( last_err != NO_ERROR ) {
#ifdef DEBUG
					if ( SUCCEEDED( fmt_res ) ) {
						TRACE( _T( "Error! File path: %s, File path length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), error_message_buffer );
						}
					else {
						TRACE( _T( "Error! File path: %s, File path length: %i. Failed to get error message for error code: %u\r\n" ), path.c_str( ), path.length( ), last_err );
						}
#endif
					return UINT64_ERROR;
					}
				return ret.QuadPart;
				}
			}
		return ret.QuadPart;
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
	WDS_DECLSPEC_NOTHROW std::vector<std::future<void>> start_workers( std::vector<std::pair<CTreeListItem*, std::wstring>> dirs_to_work_on, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in ) {
		const auto dirsToWorkOnCount = dirs_to_work_on.size( );
		std::vector<std::future<void>> workers;
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



	_Pre_satisfies_( !ThisCItem->m_attr.m_done ) WDS_DECLSPEC_NOTHROW std::pair<std::vector<std::pair<CTreeListItem*, std::wstring>>,std::vector<std::pair<CTreeListItem*, std::wstring>>> readJobNotDoneWork( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app ) {
		std::vector<FILEINFO> vecFiles;
		std::vector<DIRINFO>  vecDirs;

		vecFiles.reserve( 50 );//pseudo-arbitrary number

		FindFilesLoop( vecFiles, vecDirs, std::move( path + _T( "\\*.*" ) ) );

		//std::sort( vecFiles.begin( ), vecFiles.end( ) );

		const auto fileCount = vecFiles.size( );
		const auto dirCount  = vecDirs.size( );
	
		const auto total_count = ( fileCount + dirCount );


		if ( total_count == 0 ) {
			return std::make_pair( std::vector<std::pair<CTreeListItem*, std::wstring>>( ), std::vector<std::pair<CTreeListItem*, std::wstring>>( ) );
			}


		//ASSERT( ThisCItem->m_childCount == 0 );


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

		static_assert( sizeof( std::wstring::value_type ) == sizeof( wchar_t ), "WTF" );
		const std::wstring::size_type total_size_alloc = ( total_length );

	#ifdef WDS_STRING_ALLOC_DEBUGGING
		TRACE( _T( "total length of strings (plus null-terminators) of all files found: %I64u, total size of needed allocation: %I64u\r\n" ), total_length, total_size_alloc );
	#endif

		ThisCItem->m_child_info = std::make_unique<child_info>( );
		ThisCItem->m_child_info->m_children.reset( new CTreeListItem[ total_count ] );

		//ThisCItem->m_name_pool.reset( total_size_alloc );
		
		ThisCItem->m_child_info->m_name_pool.reset( total_size_alloc );

		//ASSERT( ThisCItem->m_name_pool.m_buffer_filled == 0 );
		ASSERT( ThisCItem->m_child_info->m_name_pool.m_buffer_filled == 0 );

		//ASSERT( path.back( ) != _T( '\\' ) );
		//sizesToWorkOn_ CANNOT BE CONST!!
		auto sizesToWorkOn_ = addFiles_returnSizesToWorkOn( ThisCItem, vecFiles, path );
		std::vector<std::pair<CTreeListItem*, std::wstring>> dirsToWorkOn;
		dirsToWorkOn.reserve( dirCount );
		const auto thisOptions = GetOptions( );

		//ASSERT( static_cast< size_t >( ThisCItem->m_childCount ) == vecFiles.size( ) );

		std::uint32_t total_so_far = static_cast<std::uint32_t>( vecFiles.size( ) );

		//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
		for ( const auto& dir : vecDirs ) {
			const bool dontFollow = ( app->m_mountPoints.IsJunctionPoint( dir.path, dir.attributes ) && !thisOptions->m_followJunctionPoints ) || ( app->m_mountPoints.IsMountPoint( dir.path ) && !thisOptions->m_followMountPoints );
			const auto new_name_length = dir.name.length( );
			ASSERT( new_name_length < UINT16_MAX );


			ASSERT( ThisCItem->m_child_info != nullptr );

			PWSTR new_name_ptr = nullptr;
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, dir.name );
			const HRESULT copy_res = ThisCItem->m_child_info->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), dir.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (directory) name str! (readJobNotDoneWork)(aborting!)" );
				displayWindowsMsgBoxWithMessage( dir.name.c_str( ) );
				}
			else {
				//                                                                                               IT_DIRECTORY
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				const auto newitem = new ( &( ThisCItem->m_child_info->m_children[ total_so_far ] ) ) CTreeListItem { static_cast< std::uint64_t >( UINT64_ERROR ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), dontFollow, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };

				//detect overflows. highly unlikely.
				//ASSERT( ThisCItem->m_childCount < 4294967290 );

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
		ASSERT( ThisCItem->m_child_info != nullptr );
		ThisCItem->m_child_info->m_childCount = total_so_far;
		ASSERT( ThisCItem->m_child_info->m_name_pool.m_buffer_filled == ( total_size_alloc - 1 ) );
		//ASSERT( ( fileCount + dirCount ) == ThisCItem->m_childCount );
		//ThisCItem->m_children_vector.shrink_to_fit( );
		return std::make_pair( std::move( dirsToWorkOn ), std::move( sizesToWorkOn_ ) );
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






WDS_DECLSPEC_NOTHROW std::vector<std::pair<CTreeListItem*, std::wstring>> addFiles_returnSizesToWorkOn( _In_ CTreeListItem* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CTreeListItem*, std::wstring>> sizesToWorkOn_;
	std::sort( vecFiles.begin( ), vecFiles.end( ) );
	sizesToWorkOn_.reserve( vecFiles.size( ) );

	ASSERT( path.back( ) != _T( '\\' ) );

	std::uint32_t total_so_far = 0u;

	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );

			ASSERT( ThisCItem->m_child_info != nullptr );

			PWSTR new_name_ptr = nullptr;
			//ThisCItem->m_name_pool.copy_name_str_into_buffer
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_child_info->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (compressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				//                                                                                            IT_FILE
				auto newChild = ::new ( &( ThisCItem->m_child_info->m_children[ total_so_far ] ) ) CTreeListItem { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
				//sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( std::async( GetCompressedFileSize_filename, std::move( path + _T( '\\' ) + aFile.name  ) ) ) );
				sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( path + _T( '\\' ) + aFile.name  ) );
				}
			}
		else {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );
			PWSTR new_name_ptr = nullptr;
			
			ASSERT( ThisCItem->m_child_info != nullptr );
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_child_info->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (uncompressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				ASSERT( ThisCItem->m_child_info != nullptr );
				//ASSERT( total_so_far == ThisCItem->m_childCount );
				//                                                                            IT_FILE
				::new ( &( ThisCItem->m_child_info->m_children[ total_so_far ] ) ) CTreeListItem { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				}
			}
		//detect overflows. highly unlikely.
		//ASSERT( ThisCItem->m_childCount < 4294967290 );
		//++( ThisCItem->m_childCount );
		++total_so_far;
		}
	ASSERT( ThisCItem->m_child_info != nullptr );
	ThisCItem->m_child_info->m_childCount = total_so_far;
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

WDS_DECLSPEC_NOTHROW DOUBLE DoSomeWorkShim( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx
	//ASSERT( ThisCItem->m_childCount == 0 );
	ASSERT( ThisCItem->m_child_info == nullptr );
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
	
	DoSomeWork( std::move( ThisCItem ), std::move( path ), app, &sizes_to_work_on, std::move( isRootRecurse ) );
	
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

	return timing_2;
	//wait for sync?
	}

//sizes_to_work_on_in NEEDS to be passed as a pointer, else bad things happen!
WDS_DECLSPEC_NOTHROW void DoSomeWork( _In_ CTreeListItem* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse ) {
	//This is temporary.
	UNREFERENCED_PARAMETER( isRootRecurse );

	ASSERT( wcscmp( L"\\\\?\\", path.substr( 0, 4 ).c_str( ) ) == 0 );
	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp_path != 0 ) {
		auto fixedPath( L"\\\\?\\" + path );
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}
	auto itemsToWorkOn = readJobNotDoneWork( ThisCItem, std::move( path ), app );

	//if ( ThisCItem->m_child_info == nullptr ) {
	//	ASSERT( ThisCItem->m_childCount == 0 );
	//	}

	//if ( ThisCItem->m_childCount == 0 ) {
	//	ASSERT( ThisCItem->m_child_info == nullptr );
	//	}

	if ( ThisCItem->m_child_info == nullptr ) {
		//ASSERT( ThisCItem->m_childCount == 0 );
		ASSERT( ThisCItem->m_child_info == nullptr );
		ASSERT( itemsToWorkOn.first.size( ) == 0 );
		ASSERT( itemsToWorkOn.second.size( ) == 0 );
		ThisCItem->m_attr.m_done = true;
		ThisCItem->m_size = 0;
		return;
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
		worker.get( );
		}

	ThisCItem->m_attr.m_done = true;
	return;
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

//FILEINFO::FILEINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ] ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ) {
//#ifdef DEBUG
//	if ( length > 34359738368 ) {
//		_CrtDbgBreak( );
//		}
//#endif
//	}


//FILEINFO& FILEINFO::operator=( FILEINFO&& in ) {
//	length = std::move( in.length );
//	lastWriteTime = std::move( in.lastWriteTime );
//	attributes = std::move( in.attributes );
//	name = std::move( in.name );
//	return ( *this );
//	}

//FILEINFO::FILEINFO( FILEINFO&& in ) {
//	length = std::move( in.length );
//	lastWriteTime = std::move( in.lastWriteTime );
//	attributes = std::move( in.attributes );
//	name = std::move( in.name );
//	}

//void FILEINFO::reset( ) {
//	length = 0;
//	lastWriteTime.dwLowDateTime  = 0;
//	lastWriteTime.dwHighDateTime = 0;
//	attributes = INVALID_FILE_ATTRIBUTES;
//	name.clear( );
//	}

//Yes, this is used!
SExtensionRecord::SExtensionRecord( SExtensionRecord&& in ) {
	ext = std::move( in.ext );
	files = std::move( in.files );
	bytes = std::move( in.bytes );
	color = std::move( in.color );
	}


//DIRINFO::DIRINFO( _In_ std::uint64_t length_, _In_ FILETIME lastWriteTime_, _In_ DWORD attributes_, _In_z_ wchar_t( &cFileName )[ MAX_PATH ], _In_ std::wstring path_ ) : length { std::move( length_ ) }, lastWriteTime( std::move( lastWriteTime_ ) ), attributes { std::move( attributes_ ) }, name( cFileName ), path( std::move( path_ ) ) { }


//DIRINFO::DIRINFO( DIRINFO&& in ) {
//	length = std::move( in.length );
//	lastWriteTime = std::move( in.lastWriteTime );
//	attributes = std::move( in.attributes );
//	name = std::move( in.name );
//	path = std::move( in.path );
//	}


#else

#endif