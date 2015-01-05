// item.cpp	- Implementation of CItemBranch
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
#include "item.h"
#include "globalhelpers.h"
#include "options.h"
#include "windirstat.h"

#ifdef _DEBUG
#include "dirstatdoc.h"
#endif




namespace {

	_Success_( return != UINT64_MAX )
	std::uint64_t GetCompressedFileSize_filename( const std::wstring path ) {
		ULARGE_INTEGER ret;
		ret.QuadPart = 0;//zero initializing this is critical!
		ret.LowPart = GetCompressedFileSizeW( path.c_str( ), &ret.HighPart );
	#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
	#endif
		const auto last_err = GetLastError( );
		if ( ret.QuadPart == INVALID_FILE_SIZE ) {
			if ( ret.HighPart != NULL ) {
				if ( last_err != NO_ERROR ) {
					TRACE( _T( "Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), GetLastErrorAsFormattedMessage( last_err ) );
					return UINT64_MAX;// IN case of an error return size from CFileFind object
					}
				TRACE( _T( "WTF ERROR! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), GetLastErrorAsFormattedMessage( last_err ) );
				return UINT64_MAX;
				}
			else {
				if ( last_err != NO_ERROR ) {
					TRACE( _T( "Error! Filepath: %s, Filepath length: %i, GetLastError: %s\r\n" ), path.c_str( ), path.length( ), GetLastErrorAsFormattedMessage( last_err ) );
					return UINT64_MAX;
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

	void process_vector_of_compressed_file_futures( std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>& vector_of_compressed_file_futures ) {
		const auto sizesToWorkOnCount = vector_of_compressed_file_futures.size( );
		for ( size_t i = 0; i < sizesToWorkOnCount; ++i ) {
		
			const auto sizeValue = vector_of_compressed_file_futures[ i ].second.get( );
			auto child = vector_of_compressed_file_futures[ i ].first;
			if ( sizeValue != UINT64_MAX ) {
			
				ASSERT( child != NULL );
				if ( child != NULL ) {
					child->m_size = std::move( sizeValue );
					}
				}
			else {
				TRACE( _T( "ERROR returned by GetCompressedFileSize! file: %s\r\n" ), child->m_name );
				child->m_attr.invalid = true;
				}
			}
		}

	std::vector<std::future<void>> start_workers( std::vector<std::pair<CItemBranch*, std::wstring>>& dirs_to_work_on, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in ) {
		const auto dirsToWorkOnCount = dirs_to_work_on.size( );
		std::vector<std::future<void>> workers;
		workers.reserve( dirsToWorkOnCount );
		for ( size_t i = 0; i < dirsToWorkOnCount; ++i ) {
			ASSERT( dirs_to_work_on[ i ].second.length( ) > 1 );
			ASSERT( dirs_to_work_on[ i ].second.back( ) != L'\\' );
			ASSERT( dirs_to_work_on[ i ].second.back( ) != L'*' );
			//TODO: investigate task_group
	#ifdef PERF_DEBUG_SLEEP
			Sleep( 0 );
			Sleep( 10 );
	#endif
			//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
			workers.emplace_back( std::async( DoSomeWork, std::move( dirs_to_work_on[ i ].first ), std::move( dirs_to_work_on[ i ].second ), app, sizes_to_work_on_in, std::move( false ) ) );
			}
		return workers;
		}

	void size_workers( concurrency::concurrent_vector<pair_of_item_and_path>* sizes ) {
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
	zeroFILEINFO( fi );
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
			directories.emplace_back( DIRINFO { 0,
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
	sizesToWorkOn_.reserve( vecFiles.size( ) );

	ASSERT( path.back( ) != _T( '\\' ) );
	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			const auto new_name_length = aFile.name.length( );
			PWSTR new_name_ptr = new wchar_t[ new_name_length + 1 ];
			const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1 ), aFile.name.c_str( ) );
			ENSURE( cpy_res == 0 );
			ASSERT( wcslen( new_name_ptr ) == new_name_length );
			ASSERT( wcscmp( new_name_ptr, aFile.name.c_str( ) ) == 0 );
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_FILE, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast<std::uint16_t>( new_name_length ) };

			//using std::launch::async ( instead of the default, std::launch::any ) causes WDS to hang!
			//sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( std::async( GetCompressedFileSize_filename, std::move( path + _T( '\\' ) + aFile.name  ) ) ) );
			sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( path + _T( '\\' ) + aFile.name  ) );
#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
#endif
			}
		else {
			const auto new_name_length = aFile.name.length( );
			PWSTR new_name_ptr = new wchar_t[ new_name_length + 1 ];
			const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1 ), aFile.name.c_str( ) );
			ENSURE( cpy_res == 0 );
			ASSERT( wcslen( new_name_ptr ) == new_name_length );
			ASSERT( wcscmp( new_name_ptr, aFile.name.c_str( ) ) == 0 );
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_FILE, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem, new_name_ptr, static_cast<std::uint16_t>( new_name_length ) };
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
		ThisCItem->m_children = new CItemBranch[ total_count ];
		}
	////true for 2 means DIR

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
		PWSTR new_name_ptr = new wchar_t[ new_name_length + 1 ];
		const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1 ), dir.name.c_str( ) );
		ENSURE( cpy_res == 0 );
		ASSERT( wcslen( new_name_ptr ) == new_name_length );
		ASSERT( wcscmp( new_name_ptr, dir.name.c_str( ) ) == 0 );

		const auto newitem = new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_DIRECTORY, static_cast<std::uint64_t>( 0 ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), dontFollow, ThisCItem, new_name_ptr, static_cast<std::uint16_t>( new_name_length ) };
		
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
			}
		}
	ASSERT( ( fileCount + dirCount ) == ThisCItem->m_childCount );
	//ThisCItem->m_children_vector.shrink_to_fit( );
	return std::make_pair( std::move( dirsToWorkOn ), std::move( sizesToWorkOn_ ) );
	}

_Pre_satisfies_( this->m_parent == NULL ) void CItemBranch::AddChildren( ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_attr.m_done );
	if ( m_parent == NULL ) {
		GetTreeListControl( )->OnChildAdded( NULL, this, false );
		}
	}

double DoSomeWorkShim( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx
	ASSERT( ThisCItem->m_childCount == 0 );
	ASSERT( ThisCItem->m_children == nullptr );
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
	wchar_t debug_buf[ debug_buf_size ] = { 0 };
	_snwprintf_s( debug_buf, debug_buf_size, L"WDS: enum timing: %f\r\n", timing );
	
	OutputDebugStringW( debug_buf );


	const auto qpc_3 = help_QueryPerformanceCounter( );

	size_workers( &sizes_to_work_on );

	const auto qpc_4 = help_QueryPerformanceCounter( );
	const auto qpf_2 = help_QueryPerformanceFrequency( );
	const auto timing_2 = ( static_cast<double>( qpc_4.QuadPart - qpc_3.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf_2.QuadPart ) ) );

	
	wchar_t debug_buf_2[ debug_buf_size ] = { 0 };
	_snwprintf_s( debug_buf_2, debug_buf_size, L"WDS: compressed file timing: %f\r\n", timing_2 );
	
	OutputDebugStringW( debug_buf_2 );

	return timing_2;
	//wait for sync?
	}

void DoSomeWork( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, concurrency::concurrent_vector<pair_of_item_and_path>* sizes_to_work_on_in, const bool isRootRecurse ) {
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
		return;
		}

	std::vector<std::pair<CItemBranch*, std::wstring>>& dirs_to_work_on = itemsToWorkOn.first;

	auto workers = start_workers( dirs_to_work_on, app, sizes_to_work_on_in );

	//std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>& vector_of_compressed_file_futures = itemsToWorkOn.second;

	//process_vector_of_compressed_file_futures( vector_of_compressed_file_futures );


	std::vector<std::pair<CItemBranch*, std::wstring>>& vector_of_compressed_file_futures = itemsToWorkOn.second;

	for ( auto& a_pair : vector_of_compressed_file_futures ) {
		pair_of_item_and_path the_pair;
		the_pair.ptr  = a_pair.first;
		the_pair.path = a_pair.second;
		sizes_to_work_on_in->push_back( the_pair );
		}

	for ( auto& worker : workers ) {
#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
#endif
		worker.get( );
		}

	ThisCItem->m_attr.m_done = true;
	return;
	}

//

CItemBranch::CItemBranch( ITEMTYPE type, std::uint64_t size, FILETIME time, DWORD attr, bool done, CItemBranch* parent, _In_z_ PCWSTR name, const std::uint16_t length ) : m_size( size ), m_rect( 0, 0, 0, 0 ), m_lastChange( std::move( time ) ), m_childCount( 0 ), m_children( nullptr ), CTreeListItem( std::move( name ), std::move( length ) ) {
	m_parent = std::move( parent );
	m_vi = NULL;
	SetAttributes( attr );
	m_attr.m_done = done;
	//m_name = std::move( name );
	}

CItemBranch::~CItemBranch( ) {
	delete[ ] m_children;
	m_children = nullptr;
	m_childCount = 0;
	//m_children_vector.clear( );
	}

#ifdef ITEM_DRAW_SUBITEM
bool CItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );

	if ( subitem == column::COL_NAME ) {
		return CTreeListItem::DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
		}
	if ( width != NULL ) {
		*width = 105;
		return true;
		}
	DrawSelection( GetTreeListControl( ), pdc, rc, state );
	rc.DeflateRect( 2, 5 );
	auto indent = GetIndent( );
	for ( INT i = 0; i < indent; i++ ) {
		rc.left += ( rc.Width( ) ) / 10;
		}
	DrawPercentage( pdc, rc, GetFraction( ), std::move( GetPercentageColor( ) ) );
	return true;
	}

COLORREF CItemBranch::GetPercentageColor( ) const {
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		auto i = GetIndent( ) % TREELISTCOLORCOUNT;
		return std::move( Options->GetTreelistColor( i ) );
		}
	ASSERT( false );//should never ever happen, but just in case, we'll generate a random color.
	return DWORD( rand( ) );
	}
#endif

_Pre_satisfies_( subitem == column::COL_NAME ) _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_NAME( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_NAME );
	size_t chars_remaining = 0;
	const auto res = StringCchCopyExW( psz_text, strSize, m_name, NULL, &chars_remaining, 0 );
		
	chars_written = m_name_length;
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = ( m_name_length + 2 );
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	return res;
	}

_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_PERCENTAGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_PERCENTAGE );
	size_t chars_remaining = 0;
	const auto percentage = ( GetFraction( ) * static_cast< DOUBLE >( 100 ) );
	ASSERT( percentage <= 100.00 );
	auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", percentage );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Pre_satisfies_( subitem == column::COL_SUBTREETOTAL ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_SUBTREETOTAL( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_SUBTREETOTAL );
	auto res = FormatBytes( size_recurse( ), psz_text, strSize, chars_written );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Pre_satisfies_( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_FILES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) );
	const auto number_to_format = files_recurse( );
	return CStyle_GetNumberFormatted( number_to_format, psz_text, strSize, chars_written );

	}

_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_LASTCHANGE( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_LASTCHANGE );
	const HRESULT res = CStyle_FormatFileTime( FILETIME_recurse( ), psz_text, strSize, chars_written );
	if ( SUCCEEDED( res ) ) {
		UNREFERENCED_PARAMETER( sizeBuffNeed );
		return S_OK;
		}
	chars_written = 0;
	_CrtDbgBreak( );//not handled yet.
	return STRSAFE_E_INVALID_PARAMETER;
	}

_Pre_satisfies_( subitem == column::COL_ATTRIBUTES ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_COL_ATTRIBUTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_ATTRIBUTES );
	const HRESULT res = CStyle_FormatAttributes( m_attr, psz_text, strSize, chars_written );
	if ( !SUCCEEDED( res ) ) {
		sizeBuffNeed = 8;//Generic size needed, overkill;
		chars_written = 0;
		_CrtDbgBreak( );//not handled yet.
		return res;
		}
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::WriteToStackBuffer_default( const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( strSize > 8 );
	sizeBuffNeed = SIZE_T_ERROR;
	//auto res = StringCchPrintfW( psz_text, strSize, L"BAD GetText_WriteToStackBuffer - subitem" );
	size_t chars_remaining = 0;
	auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"BAD GetText_WriteToStackBuffer - subitem" );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		if ( strSize > 8 ) {
			write_BAD_FMT( psz_text, chars_written );
			}
		else {
			chars_written = strSize;
			displayWindowsMsgBoxWithMessage( std::wstring( L"CItemBranch::" ) + std::wstring( global_strings::write_to_stackbuffer_err ) );
			}
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}

	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		}

	
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}


_Must_inspect_result_ _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	switch ( subitem )
	{
			case column::COL_NAME:
				return WriteToStackBuffer_COL_NAME( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_PERCENTAGE:
				return WriteToStackBuffer_COL_PERCENTAGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_SUBTREETOTAL:
				return WriteToStackBuffer_COL_SUBTREETOTAL( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ITEMS:
			case column::COL_FILES:
				return WriteToStackBuffer_COL_FILES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_LASTCHANGE:
				return WriteToStackBuffer_COL_LASTCHANGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ATTRIBUTES:
				return WriteToStackBuffer_COL_ATTRIBUTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			default:
				return WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}


COLORREF CItemBranch::ItemTextColor( ) const {
	if ( m_attr.invalid ) {
		//return GetItemTextColor( true );
		return RGB( 0xFF, 0x00, 0x00 );
		}
	if ( m_attr.compressed ) {
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( m_attr.encrypted ) {
		return GetApp( )->m_altEncryptionColor;
		}
	//ASSERT( GetItemTextColor( true ) == default_item_text_color( ) );
	//return GetItemTextColor( true ); // The rest is not colored
	return default_item_text_color( ); // The rest is not colored
	}

INT CItemBranch::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const {
	auto other = static_cast< const CItemBranch* >( tlib );
	switch ( subitem ) {
			case column::COL_NAME:
				return signum( wcscmp( m_name, other->m_name ) );
			case column::COL_PERCENTAGE:
				return signum( GetFraction( ) - other->GetFraction( ) );
			case column::COL_SUBTREETOTAL:
				return signum( static_cast<std::int64_t>( size_recurse( ) ) - static_cast<std::int64_t>( other->size_recurse( ) ) );
			case column::COL_ITEMS:
			case column::COL_FILES:
				return signum( static_cast<std::int64_t>( files_recurse( ) ) - static_cast<std::int64_t>( other->files_recurse( ) ) );
			case column::COL_LASTCHANGE:
				return Compare_FILETIME( FILETIME_recurse( ), other->FILETIME_recurse( ) );
			case column::COL_ATTRIBUTES:
				return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
			default:
				ASSERT( false );
				return 666;
		}
	}

std::vector<CTreeListItem*> CItemBranch::size_sorted_vector_of_children( ) const {
	std::vector<CTreeListItem*> children;
	const auto child_count = m_childCount;
	children.reserve( child_count );
	if ( m_children != nullptr ) {
		for ( size_t i = 0; i < child_count; ++i ) {
			children.emplace_back( m_children + i );
			}
		}
#ifdef DEBUG
	else {
		ASSERT( m_childCount == 0 );
		}
#endif
	qsort( children.data( ), static_cast< const size_t >( children.size( ) ), sizeof( CTreeListItem* ), &CItem_compareBySize );
	return children;
	}

//Encodes the attributes to fit (in) 1 byte
void CItemBranch::SetAttributes( _In_ const DWORD attr ) {
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		m_attr.invalid = true;
		return;
		}
	m_attr.readonly   = ( ( attr bitand FILE_ATTRIBUTE_READONLY      ) != 0 );
	m_attr.hidden     = ( ( attr bitand FILE_ATTRIBUTE_HIDDEN        ) != 0 );
	m_attr.system     = ( ( attr bitand FILE_ATTRIBUTE_SYSTEM        ) != 0 );
	//m_attr.archive    = ( ( attr bitand FILE_ATTRIBUTE_ARCHIVE       ) != 0 );
	m_attr.compressed = ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED    ) != 0 );
	m_attr.encrypted  = ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED     ) != 0 );
	m_attr.reparse    = ( ( attr bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
	m_attr.invalid    = false;
	}

INT CItemBranch::GetSortAttributes( ) const {
	DWORD ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += ( m_attr.readonly   ) ? 1000000 : 0; // R
	ret += ( m_attr.hidden     ) ? 100000 : 0; // H
	ret += ( m_attr.system     ) ? 10000 : 0; // S
	//ret += ( m_attr.archive    ) ? 1000 : 0; // A
	ret += ( m_attr.compressed ) ? 100 : 0; // C
	ret += ( m_attr.encrypted  ) ? 10 : 0; // E

	return static_cast< INT >( ( m_attr.invalid ) ? 0 : ret );
	}

DOUBLE CItemBranch::GetFraction( ) const {
	const auto myParent = GetParentItem( );
	if ( myParent == NULL ) {
		return static_cast<DOUBLE>( 1.0 );//root item? must be whole!
		}
	const auto parentSize = myParent->size_recurse( );
	if ( parentSize == 0 ) {//root item?
		return static_cast<DOUBLE>( 1.0 );
		}
	const auto my_size = size_recurse( );
	ASSERT( my_size != UINT64_ERROR );
	ASSERT( my_size <= parentSize );
	return static_cast<DOUBLE>( my_size ) / static_cast<DOUBLE>( parentSize );
	}

std::wstring CItemBranch::GetPath( ) const {
	std::wstring pathBuf;
	pathBuf.reserve( MAX_PATH );
	UpwardGetPathWithoutBackslash( pathBuf );
	ASSERT( wcslen( m_name ) == m_name_length );
	ASSERT( wcslen( m_name ) < 33000 );
	ASSERT( pathBuf.length( ) < 33000 );
	return pathBuf;
	}

void CItemBranch::UpwardGetPathWithoutBackslash( std::wstring& pathBuf ) const {
	auto myParent = GetParentItem( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
	ASSERT( wcslen( m_name ) == m_name_length );
	ASSERT( wcslen( m_name ) < 33000 );
	if ( m_children == NULL ) {
		//ASSERT( m_parent != NULL );
		if ( m_parent != NULL ) {
			if ( m_parent->m_parent != NULL ) {
				pathBuf += L'\\';
				ASSERT( wcslen( m_name ) == m_name_length );
				pathBuf += m_name;
				}
			else {
				pathBuf += m_name;
				}
			return;
			}
		ASSERT( pathBuf.empty( ) );
		ASSERT( wcslen( m_name ) == m_name_length );
		pathBuf = m_name;
		return;
		//ASSERT( false );
		//return;
		}
	if ( !pathBuf.empty( ) ) {
		if ( pathBuf.back( ) != L'\\' ) {//if pathBuf is empty, it's because we don't have a parent ( we're the root ), so we already have a "\\"
			pathBuf += L'\\';
			}
		}
	pathBuf += m_name;
	return;

	}

CRect CItemBranch::TmiGetRectangle( ) const {
	return BuildCRect( m_rect );
	}

_Success_( return < SIZE_T_MAX )
size_t CItemBranch::findItemInChildren( const CItemBranch* const theItem ) const {
	const auto childrenSize = m_childCount;
	for ( size_t i = 0; i < childrenSize; ++i ) {
		if ( ( ( m_children + i ) ) == theItem ) {
			return i;
			}
		}
	return SIZE_T_MAX;
	}


void CItemBranch::refresh_sizeCache( ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == NULL ) {
		return;
		}
	if ( m_vi != NULL ) {
		if ( m_vi->sizeCache != UINT64_ERROR ) {
			m_vi->sizeCache = UINT64_ERROR;
			m_vi->sizeCache = size_recurse( );
			}
		}
	}



_Ret_range_( 0, UINT64_MAX )
std::uint64_t CItemBranch::size_recurse( ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == NULL ) {
		ASSERT( m_size < ( UINT64_ERROR / 8 ) );
		return m_size;
		}
	if ( m_vi != NULL ) {
		if ( m_vi->sizeCache != UINT64_ERROR ) {
			ASSERT( m_vi->sizeCache < ( UINT64_ERROR / 4 ) );
			return m_vi->sizeCache;
			}
		}

	std::uint64_t total = m_size;

	const auto childCount = m_childCount;
	const auto child_array = m_children;
	//ASSERT( m_childCount == childCount );

	for ( size_t i = 0; i < childCount; ++i ) {
		total += ( child_array + ( i ) )->size_recurse( );
		ASSERT( total < ( UINT64_ERROR / 4 ) );
		}
	if ( m_vi != NULL ) {
		if ( m_vi->sizeCache == UINT64_ERROR ) {
			m_vi->sizeCache = total;
			//if ( total != 0 ) {
			//	ASSERT( total < ( UINT64_ERROR / 4 ) );
			//	m_vi->sizeCache = total;
			//	}
			}
		}

	ASSERT( total < ( UINT64_ERROR / 4 ) );
	return total;
	}


//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
_Ret_range_( 0, 4294967295 )
std::uint32_t CItemBranch::files_recurse( ) const {
	std::uint32_t total = 0;
	const auto childCount = m_childCount;
	for ( size_t i = 0; i < childCount; ++i ) {
		total += ( m_children + ( i ) )->files_recurse( );
		}
	total += 1;
	return total;
	}




FILETIME CItemBranch::FILETIME_recurse( ) const {
	auto ft = zeroInitFILETIME( );
	if ( Compare_FILETIME_cast( ft, m_lastChange ) ) {
		ft = m_lastChange;
		}
	
	const auto childCount = m_childCount;
	for ( size_t i = 0; i < childCount; ++i ) {
		auto ft_child = ( m_children + ( i ) )->FILETIME_recurse( );
		if ( Compare_FILETIME_cast( ft, ft_child ) ) {
			ft = ft_child;
			}
		}
	return ft;
	}



//Sometimes I just need to COMPARE the extension with a string. So, instead of copying/screwing with string internals, I'll just return a pointer to the substring.
//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children == NULL ) 
PCWSTR CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name_length < ( MAX_PATH + 1 ) );
	PCWSTR resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children == NULL ) 
_Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::CStyle_GetExtension( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_extension, const rsize_t strSize, _Out_ rsize_t& chars_written ) const {
	psz_extension[ 0 ] = 0;

	PWSTR resultPtrStr = PathFindExtensionW( m_name );
	ASSERT( resultPtrStr != '\0' );
	if ( resultPtrStr != '\0' ) {
		size_t extLen = 0;
		const HRESULT res_1 = StringCchLengthW( resultPtrStr, MAX_PATH, &extLen );
		if ( FAILED( res_1 ) ) {
			psz_extension[ 0 ] = 0;
			chars_written = 0;
			return res_1;
			}
		if ( extLen > ( strSize ) ) {
			psz_extension[ 0 ] = 0;
			chars_written = 0;
			return STRSAFE_E_INSUFFICIENT_BUFFER;
			}
		const HRESULT res_2 = StringCchCopyW( psz_extension, strSize, resultPtrStr );
		if ( SUCCEEDED( res_2 ) ){
			chars_written = extLen;
			}
#ifdef DEBUG
		if ( SUCCEEDED( res_2 ) ) {
			ASSERT( GetExtension( ).compare( psz_extension ) == 0 );
			}
#endif
		return res_2;
		}

	psz_extension[ 0 ] = 0;
	chars_written = 0;
	return STRSAFE_E_INVALID_PARAMETER;//some generic error
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children == NULL ) 
const std::wstring CItemBranch::GetExtension( ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == NULL ) {
		PWSTR resultPtrStr = PathFindExtensionW( m_name );
		ASSERT( resultPtrStr != 0 );
		if ( resultPtrStr != '\0' ) {
			return resultPtrStr;
			}
		const PCWSTR i = wcsrchr( m_name, L'.' );

		if ( i == NULL ) {
			return _T( "." );
			}
		return std::wstring( i );
		}
	return std::wstring( L"" );
	}


void CItemBranch::TmiSetRectangle( _In_ const CRect& rc ) const {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	m_rect.left   = static_cast<short>( rc.left );
	m_rect.top    = static_cast<short>( rc.top );
	m_rect.right  = static_cast<short>( rc.right );
	m_rect.bottom = static_cast<short>( rc.bottom );
	}


_Ret_range_( 0, 33000 ) DOUBLE CItemBranch::averageNameLength( ) const {
	const auto myLength = static_cast<DOUBLE>( m_name_length );
	DOUBLE childrenTotal = 0;
	
	//if ( m_type != IT_FILE ) {
	if ( m_children != NULL ) {
		const auto childCount = m_childCount;
		for ( size_t i = 0; i < childCount; ++i ) {
			childrenTotal += ( m_children + ( i ) )->averageNameLength( );
			}
		return ( childrenTotal + myLength ) / static_cast<DOUBLE>( m_childCount + 1 );
		}
	ASSERT( m_childCount == 0 );
	return myLength;
	}

//_Pre_satisfies_( this->m_type == IT_FILE )
_Pre_satisfies_( this->m_children == NULL ) 
void CItemBranch::stdRecurseCollectExtensionData_FILE( _Inout_ std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const {
	const size_t extensionPsz_size = 48;
	wchar_t extensionPsz[ extensionPsz_size ] = { 0 };
	rsize_t chars_written = 0;
	HRESULT res = CStyle_GetExtension( extensionPsz, extensionPsz_size, chars_written );
	if ( SUCCEEDED( res ) ) {
		auto& value = extensionMap[ extensionPsz ];
		if ( value.files == 0 ) {
			value.ext = extensionPsz;
			value.ext.shrink_to_fit( );
			}
		++( value.files );
		value.bytes += m_size;
		}
	else {
		//use an underscore to avoid name conflict with _DEBUG build
		auto ext_ = GetExtension( );
		ext_.shrink_to_fit( );
		TRACE( _T( "Extension len: %i ( bigger than buffer! )\r\n\toffending extension:\r\n %s\r\n" ), ext_.length( ), ext_.c_str( ) );
		auto& value = extensionMap[ ext_ ];
		if ( value.files == 0 ) {
			value.ext = ext_;
			value.ext.shrink_to_fit( );
			}
		++( value.files );
		extensionMap[ ext_ ].bytes += m_size;
		}
	}


void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::unordered_map<std::wstring, SExtensionRecord>& extensionMap ) const {
	//if ( m_type == IT_FILE ) {
	if ( m_children == NULL ) {
		stdRecurseCollectExtensionData_FILE( extensionMap );
		}
	else {
		const auto childCount = m_childCount;
		for ( size_t i = 0; i < childCount; ++i ) {
			( m_children + ( i ) )->stdRecurseCollectExtensionData( extensionMap );
			}

		}
	}

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 ) {
	const auto size1 = ( *( reinterpret_cast< const CItemBranch * const* const >( p1 ) ) )->size_recurse( );
	const auto size2 = ( *( reinterpret_cast< const CItemBranch * const* const >( p2 ) ) )->size_recurse( );
	return signum( static_cast<std::int64_t>( size2 ) - static_cast<std::int64_t>( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}


// $Log$
// Revision 1.27  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.26  2004/12/31 16:01:42  bseifert
// Bugfixes. See changelog 2004-12-31.
//
// Revision 1.25  2004/12/12 08:34:59  bseifert
// Aboutbox: added Authors-Tab. Removed license.txt from resource dlls (saves 16 KB per dll).
//
// Revision 1.24  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.23  2004/11/28 19:20:46  assarbad
// - Fixing strange behavior of logical operators by rearranging code in
//   CItem::SetAttributes() and CItem::GetAttributes()
//
// Revision 1.22  2004/11/28 15:38:42  assarbad
// - Possible sorting implementation (using bit-order in m_attributes)
//
// Revision 1.21  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.20  2004/11/25 23:07:23  assarbad
// - Derived CFileFindWDS from CFileFind to correct a problem of the ANSI version
//
// Revision 1.19  2004/11/25 21:13:38  assarbad
// - Implemented "attributes" column in the treelist
// - Adopted width in German dialog
// - Provided German, Russian and English version of IDS_TREECOL_ATTRIBUTES
//
// Revision 1.18  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.17  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.16  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.15  2004/11/10 01:03:00  assarbad
// - Style cleaning of the alternative coloring code for compressed/encrypted items
//
// Revision 1.14  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.13  2004/11/07 20:14:30  assarbad
// - Added wrapper for GetCompressedFileSize() so that by default the compressed file size will be shown.
//
// Revision 1.12  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
