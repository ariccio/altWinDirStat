#include "stdafx.h"

#include "directory_enumeration.h"
#include "dirstatdoc.h"
#include "globalhelpers.h"

#ifndef WDS_DIRECTORY_ENUMERATION_CPP
#define WDS_DIRECTORY_ENUMERATION_CPP



namespace {

	_Success_( return != UINT64_MAX )
	std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
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


	void compose_compressed_file_size_and_fixup_child( CItemBranch* const child, const std::wstring path ) {
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
	std::vector<std::future<void>> start_workers( std::vector<std::pair<CItemBranch*, std::wstring>> dirs_to_work_on, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in ) {
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
	void size_workers( _In_ concurrency::concurrent_vector<pair_of_item_and_path>* sizes ) {
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
	}


void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring path ) {
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

std::vector<std::pair<CItemBranch*, std::wstring>> addFiles_returnSizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CItemBranch*, std::wstring>> sizesToWorkOn_;
	std::sort( vecFiles.begin( ), vecFiles.end( ) );
	sizesToWorkOn_.reserve( vecFiles.size( ) );

	ASSERT( path.back( ) != _T( '\\' ) );


	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );

			PWSTR new_name_ptr = nullptr;
			//ThisCItem->m_name_pool.copy_name_str_into_buffer
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (compressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				//                                                                                            IT_FILE
				auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
				//sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( std::async( GetCompressedFileSize_filename, std::move( path + _T( '\\' ) + aFile.name  ) ) ) );
				sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( path + _T( '\\' ) + aFile.name  ) );
				}
			}
		else {
			const auto new_name_length = aFile.name.length( );
			ASSERT( new_name_length < UINT16_MAX );
			PWSTR new_name_ptr = nullptr;
			
			//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, aFile.name );
			const HRESULT copy_res = ThisCItem->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), aFile.name );

			if ( !SUCCEEDED( copy_res ) ) {
				displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (uncompressed) name str! (addFiles_returnSizesToWorkOn)(aborting!)" );
				displayWindowsMsgBoxWithMessage( aFile.name.c_str( ) );
				}
			else {
				//                                                                            IT_FILE
				::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
				}
			}
		//detect overflows. highly unlikely.
		ASSERT( ThisCItem->m_childCount < 4294967290 );
		++( ThisCItem->m_childCount );
		}
	return sizesToWorkOn_;
	}

_Pre_satisfies_( !ThisCItem->m_attr.m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::wstring>>> readJobNotDoneWork( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app ) {
	std::vector<FILEINFO> vecFiles;
	std::vector<DIRINFO>  vecDirs;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( vecFiles, vecDirs, std::move( path + _T( "\\*.*" ) ) );

	//std::sort( vecFiles.begin( ), vecFiles.end( ) );

	const auto fileCount = vecFiles.size( );
	const auto dirCount  = vecDirs.size( );
	
	const auto total_count = ( fileCount + dirCount );

	ASSERT( ThisCItem->m_childCount == 0 );
	if ( total_count > 0 ) {
		ThisCItem->m_children.reset( new CItemBranch[ total_count ] );
		}
	////true for 2 means DIR


	//TODO: BUGBUG: need +1 here, else ASSERT( ( m_buffer_filled + new_name_length ) < m_buffer_size ) fails!
	std::uint64_t total_length = 1u;
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
	const std::uint64_t total_size_alloc = ( total_length );

#ifdef WDS_STRING_ALLOC_DEBUGGING
	TRACE( _T( "total length of strings (plus null-terminators) of all files found: %I64u, total size of needed allocation: %I64u\r\n" ), total_length, total_size_alloc );
#endif


	ThisCItem->m_name_pool.reset( total_size_alloc );

	ASSERT( ThisCItem->m_name_pool.m_buffer_filled == 0 );

	//ASSERT( path.back( ) != _T( '\\' ) );
	//sizesToWorkOn_ CANNOT BE CONST!!
	auto sizesToWorkOn_ = addFiles_returnSizesToWorkOn( ThisCItem, vecFiles, path );
	std::vector<std::pair<CItemBranch*, std::wstring>> dirsToWorkOn;
	dirsToWorkOn.reserve( dirCount );
	const auto thisOptions = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	for ( const auto& dir : vecDirs ) {
		const bool dontFollow = ( app->m_mountPoints.IsJunctionPoint( dir.path, dir.attributes ) && !thisOptions->m_followJunctionPoints ) || ( app->m_mountPoints.IsMountPoint( dir.path ) && !thisOptions->m_followMountPoints );
		const auto new_name_length = dir.name.length( );
		ASSERT( new_name_length < UINT16_MAX );

		PWSTR new_name_ptr = nullptr;
		//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, dir.name );
		const HRESULT copy_res = ThisCItem->m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), dir.name );

		if ( !SUCCEEDED( copy_res ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy (directory) name str! (readJobNotDoneWork)(aborting!)" );
			displayWindowsMsgBoxWithMessage( dir.name.c_str( ) );
			}
		else {
			//                                                                                               IT_DIRECTORY
			const auto newitem = new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { static_cast< std::uint64_t >( UINT64_ERROR ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), dontFollow, ThisCItem, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };

			//detect overflows. highly unlikely.
			ASSERT( ThisCItem->m_childCount < 4294967290 );

			++( ThisCItem->m_childCount );
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
	ASSERT( ThisCItem->m_name_pool.m_buffer_filled == ( total_size_alloc - 1 ) );
	ASSERT( ( fileCount + dirCount ) == ThisCItem->m_childCount );
	//ThisCItem->m_children_vector.shrink_to_fit( );
	return std::make_pair( std::move( dirsToWorkOn ), std::move( sizesToWorkOn_ ) );
	}

_Pre_satisfies_( this->m_parent == NULL ) void CItemBranch::AddChildren( _In_ CTreeListControl* const tree_list_control ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_attr.m_done );
	if ( m_parent == NULL ) {
		tree_list_control->OnChildAdded( NULL, this, false );
		}
	}

DOUBLE DoSomeWorkShim( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx
	ASSERT( ThisCItem->m_childCount == 0 );
	ASSERT( ThisCItem->m_children.get( ) == nullptr );
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
void DoSomeWork( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse ) {
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
	if ( ThisCItem->m_childCount == 0 ) {
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


	std::vector<std::pair<CItemBranch*, std::wstring>>& vector_of_compressed_file_futures = itemsToWorkOn.second;

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
const std::uint64_t get_uncompressed_file_size( const CTreeListItem* const item ) {
	const auto derived_item = static_cast< const CItemBranch* const >( item );
	const auto path = derived_item->GetPath( );
	const HANDLE file_handle = CreateFileW( path.c_str( ), FILE_READ_ATTRIBUTES | FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file_handle == INVALID_HANDLE_VALUE ) {
		const rsize_t str_size = 128u;
		wchar_t str_buff[ str_size ] = { 0 };
		rsize_t chars_written = 0;
		const auto last_error = GetLastError( );
		const HRESULT res = CStyle_GetLastErrorAsFormattedMessage( str_buff, str_size, chars_written, last_error );
		if ( SUCCEEDED( res ) ) {
			TRACE( _T( "get_uncompressed_file_size failed! error message: %s\r\n" ), str_buff );
			}
		TRACE( _T( "DOUBLE FAULT! get_uncompressed_file_size failed! Error getting error message!\r\n" ) );
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

	//TODO: trace error message
	return UINT64_ERROR;
	}


#else

#endif