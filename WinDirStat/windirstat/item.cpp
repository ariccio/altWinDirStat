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
	const unsigned char INVALID_m_attributes = 0x80; // File attribute packing

	}

void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring& path ) {
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
			
			
			//auto thisDirPath( path );
			
			//thisDirPath.pop_back( );
			//thisDirPath.pop_back( );
			//thisDirPath.pop_back( );
			//ASSERT( alt_path_this_dir.compare( thisDirPath ) == 0 );
			//thisDirPath += fData.cFileName;

			ASSERT( path.substr( path.length( ) - 3, 3 ).compare( L"*.*" ) == 0 );
			const auto alt_path_this_dir( path.substr( 0, path.length( ) - 3 ) + fData.cFileName );
			directories.emplace_back( DIRINFO { 0, std::move( fData.ftLastWriteTime ), std::move( fData.dwFileAttributes ), std::move( std::wstring( fData.cFileName ) ), std::move( alt_path_this_dir ) } );
			}
		else {
			fi.attributes = std::move( fData.dwFileAttributes );
			fi.lastWriteTime = std::move( fData.ftLastWriteTime );
			fi.length = std::move( ( static_cast<std::uint64_t>( fData.nFileSizeHigh ) * ( static_cast<std::uint64_t>( MAXDWORD ) + 1 ) ) + static_cast<std::uint64_t>( fData.nFileSizeLow ) );
			fi.name = std::move( std::wstring( fData.cFileName ) );
			files.emplace_back( std::move( fi ) );
			zeroFILEINFO( fi );
			}
		findNextFileRes = FindNextFileW( fDataHand, &fData );
		}
	if ( fDataHand == INVALID_HANDLE_VALUE ) {
		return;
		}
	VERIFY( FindClose( fDataHand ) );
	}

std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> addFiles_returnSizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> sizesToWorkOn_;
	sizesToWorkOn_.reserve( vecFiles.size( ) );

	ASSERT( path.back( ) != _T( '\\' ) );
	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem };
			sizesToWorkOn_.emplace_back( std::move( newChild ), std::move( std::async( GetCompressedFileSize_filename, std::move( path + _T( '\\' ) + aFile.name  ) ) ) );
#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
#endif
			}
		else {
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_FILE, std::move( aFile.name ), std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true, ThisCItem };
			}
		//detect overflows. highly unlikely.
		ASSERT( ThisCItem->m_childCount < 4294967290 );
		++( ThisCItem->m_childCount );
		}

	//ThisCItem->m_children_vector.reserve( ThisCItem->m_childCount );
	//for ( size_t i = 0; i < ThisCItem->m_childCount; ++i ) {
	//	ThisCItem->m_children_vector.emplace_back( ThisCItem->m_children + i );
	//	}
	return sizesToWorkOn_;
	}


//std::pair<std::vector<std::pair<CItemBranch*, CString>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>
//std::vector<std::pair<CItemBranch*, CString>>
_Pre_satisfies_( !ThisCItem->m_attr.m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>> readJobNotDoneWork( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	std::vector<FILEINFO> vecFiles;
	std::vector<DIRINFO>  vecDirs;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( vecFiles, vecDirs, std::move( path + _T( "\\*.*" ) ) );

	//std::sort( vecFiles.begin( ), vecFiles.end( ) );

	const auto fileCount = vecFiles.size( );
	const auto dirCount  = vecDirs.size( );
	
	ASSERT( ThisCItem->m_childCount == 0 );
	if ( ( fileCount + dirCount ) > 0 ) {
		ThisCItem->m_children = new CItemBranch[ fileCount + dirCount ];
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
		const auto newitem = new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_DIRECTORY, std::move( dir.name ), static_cast<std::uint64_t>( 0 ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), dontFollow, ThisCItem };
		
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

void CItemBranch::SortAndSetDone( ) {
	//qsort( m_children_vector.data( ), static_cast< size_t >( m_children_vector.size( ) ), sizeof( CItemBranch* ), &CItem_compareBySize );
	m_attr.m_done = true;
	}

_Pre_satisfies_( this->m_parent == NULL ) void CItemBranch::AddChildren( ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_attr.m_done );
	if ( m_parent == NULL ) {
		GetTreeListControl( )->OnChildAdded( NULL, this, false );
		}
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) void DoSomeWorkShim( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
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
	DoSomeWork( std::move( ThisCItem ), std::move( path ), app, std::move( isRootRecurse ) );
	//wait for sync?
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) int DoSomeWork( _In_ CItemBranch* const ThisCItem, std::wstring path, _In_ const CDirstatApp* app, const bool isRootRecurse ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
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
		//ASSERT( ThisCItem->m_children_vector.size( ) == 0 );
		//ThisCItem->SortAndSetDone( );
		ThisCItem->m_attr.m_done = true;
		//return;
		return 0;
		}

	//std::vector<std::future<void>>,
	//                               std::pair<
	//                                          std::vector<
	//                                                      std::pair<CItemBranch*, std::wstring>
	//                                                     >,
	//                                          std::vector<
	//                                                      std::pair<CItemBranch*, std::future<std::uint64_t>>
	//                                                     >
	//                                        >
	//OR----
	//std::tuple<
	//            std::vector<std::future<void>>, //<------DoSomeWork futures
	//            std::vector<                    //<------Folders that we need to call DoSomeWork on
	//                        std::pair<CItemBranch*, std::wstring>
	//                       >,
	//            std::vector<                    //<------Items that have a special compressed size, and therefore, we need to wait 
	//                        std::pair<CItemBranch*, std::future<std::uint64_t>>
	//                       >
	//          >

	const auto dirsToWorkOnCount = itemsToWorkOn.first.size( );
	//std::vector<std::future<void>> workers;
	std::vector<std::future<int>> workers;
	workers.reserve( dirsToWorkOnCount );
	for ( size_t i = 0; i < dirsToWorkOnCount; ++i ) {
		//DoSomeWork( dirsToWorkOn[ i ].first, dirsToWorkOn[ i ].second );
		ASSERT( itemsToWorkOn.first[ i ].second.length( ) > 1 );
		ASSERT( itemsToWorkOn.first[ i ].second.back( ) != L'\\' );
		ASSERT( itemsToWorkOn.first[ i ].second.back( ) != L'*' );
		//path += _T( "\\*.*" );
		//TODO: investigate task_group
#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
#endif
		workers.emplace_back( std::async( DoSomeWork, std::move( itemsToWorkOn.first[ i ].first ), std::move( itemsToWorkOn.first[ i ].second ), app, std::move( false ) ) );
		}

	const auto sizesToWorkOnCount = itemsToWorkOn.second.size( );

	for ( size_t i = 0; i < sizesToWorkOnCount; ++i ) {
		
		const auto sizeValue = std::move( itemsToWorkOn.second[ i ].second.get( ) );
		if ( sizeValue != UINT64_MAX ) {
			auto child = std::move( itemsToWorkOn.second[ i ].first );
			ASSERT( child != NULL );
			if ( child != NULL ) {
				child->m_size = std::move( sizeValue );
				}
			}
		}

	for ( auto& worker : workers ) {
#ifdef PERF_DEBUG_SLEEP
		Sleep( 0 );
		Sleep( 10 );
#endif
		worker.get( );
		}

	ThisCItem->SortAndSetDone( );

	//return dummy
	return 0;
	}

//
void AddFileExtensionData( _Out_ _Pre_satisfies_( ( extensionRecords._Mylast - extensionRecords._Myfirst ) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) {
	extensionRecords.reserve( extensionMap.size( ) + 1 );
	for ( auto& anExt : extensionMap ) {
		extensionRecords.emplace_back( std::move( anExt.second ) );
		}
	}

CItemBranch::CItemBranch( ITEMTYPE type, _In_ std::wstring name, std::uint64_t size, FILETIME time, DWORD attr, bool done, CItemBranch* parent ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( size ), m_rect( 0, 0, 0, 0 ), m_lastChange( std::move( time ) ), m_childCount( 0 ), m_children( nullptr ) {
	m_parent = std::move( parent );
	m_vi = NULL;
	SetAttributes( attr );
	m_attr.m_done = done;
#ifdef PLACEMENT_NEW_DEBUGGING
	TRACE( _T( "CItem constructed at: %p\r\n" ), this );
	m_beginSentinel[ 0 ] = 'h';
	m_beginSentinel[ 1 ] = 'e';
	m_beginSentinel[ 2 ] = 'l';
	m_beginSentinel[ 3 ] = 'l';
	m_beginSentinel[ 4 ] = 'o';
	m_beginSentinel[ 5 ] =  0;

	m_bye[ 0 ] = 'b';
	m_bye[ 1 ] = 'y';
	m_bye[ 2 ] = 'e';
	m_bye[ 3 ] =  0;
#endif

	}

CItemBranch::~CItemBranch( ) {
	delete[ ] m_children;
	m_children = nullptr;
	m_childCount = 0;
	//m_children_vector.clear( );
	}

#ifdef ITEM_DRAW_SUBITEM
bool CItem::DrawSubitem( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem, _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
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

std::wstring CItemBranch::GetTextCOL_PERCENTAGE( ) const {
	const size_t bufSize = 12;

	wchar_t buffer[ bufSize ] = { 0 };
	auto res = CStyle_FormatDouble( GetFraction( ) * static_cast<DOUBLE>( 100 ), buffer, bufSize );
	if ( !SUCCEEDED( res ) ) {
		rsize_t chars_written = 0;
		write_BAD_FMT( buffer, chars_written );
		return buffer;
		}

	const wchar_t percentage[ 2 ] = { '%', 0 };
	res = StringCchCatW( buffer, bufSize, percentage );
	if ( !SUCCEEDED( res ) ) {
		rsize_t chars_written = 0;
		write_BAD_FMT( buffer, chars_written );
		return buffer;
		}
	return buffer;
	}


//does the same thing as GetTextCOL_FILES
std::wstring CItemBranch::GetTextCOL_ITEMS( ) const {
	if ( m_type != IT_FILE ) {
		return FormatCount( files_recurse( ) );
		}
	return L"";
	}

//does the same thing as GetTextCOL_ITEMS
std::wstring CItemBranch::GetTextCOL_FILES( ) const {
	if ( m_type != IT_FILE ) {
#ifdef DEBUG
		const rsize_t number_fmt_size = 48;
		wchar_t number_fmt[ number_fmt_size ] = { 0 };
		rsize_t chars_written = 0;
		const HRESULT fmt_res = CStyle_GetNumberFormatted( files_recurse( ), number_fmt, number_fmt_size, chars_written );
		ASSERT( SUCCEEDED( fmt_res ) );
		ASSERT( FormatCount( files_recurse( ) ).compare( number_fmt ) == 0 );
#endif

		return FormatCount( files_recurse( ) );
		}
	return L"";
	}

std::wstring CItemBranch::GetTextCOL_LASTCHANGE( ) const {
	const rsize_t datetime_size = 128;
	wchar_t psz_formatted_datetime[ datetime_size ] = { 0 };
	rsize_t chars_written = 0;

	const HRESULT res = CStyle_FormatFileTime( std::move( FILETIME_recurse( ) ), psz_formatted_datetime, datetime_size, chars_written );
	if ( SUCCEEDED( res ) ) {
		return psz_formatted_datetime;
		}
	return L"BAD_FMT";
	}

std::wstring CItemBranch::GetTextCOL_ATTRIBUTES( ) const {
	//auto typeOfItem = m_type;
	wchar_t attributes[ 8 ] = { 0 };
	rsize_t dummy = 0;
	auto res = CStyle_FormatAttributes( m_attr, attributes, 6, dummy );
	if ( res == 0 ) {
		return attributes;
		}
	return L"BAD_FMT";
	}

_Pre_satisfies_( subitem == column::COL_NAME ) _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_NAME( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_NAME );
	size_t chars_remaining = 0;
	auto res = StringCchCopyExW( psz_text, strSize, m_name.c_str( ), NULL, &chars_remaining, 0 );
		
	chars_written = m_name.length( );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = ( m_name.length( ) + 2 );
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			ASSERT( m_name.length( ) == wcslen( psz_text ) );
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}

_Pre_satisfies_( subitem == column::COL_PERCENTAGE ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_PERCENTAGE( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_PERCENTAGE );
	size_t chars_remaining = 0;
	auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
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
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_SUBTREETOTAL( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_FILES( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( ( subitem == column::COL_FILES ) || ( subitem == column::COL_ITEMS ) );
	const auto number_to_format = files_recurse( );
	return CStyle_GetNumberFormatted( number_to_format, psz_text, strSize, chars_written );

	//displayWindowsMsgBoxWithMessage( global_strings::write_to_stackbuffer_file );//40
	//if ( strSize > 40 ) {
	//	//res = StringCchPrintfExW( psz_formatted_LONGLONG_HUMAN, strSize, NULL, &remaining_chars, 0, L"%i Bytes", static_cast<INT>( B ) );
	//	size_t remaining_chars = strSize;
	//	HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &remaining_chars, 0, global_strings::write_to_stackbuffer_file );
	//	if ( SUCCEEDED( res ) ) {
	//		chars_written = ( strSize - remaining_chars );
	//		}
	//	return res;
	//	}
	//sizeBuffNeed = 41;
	//chars_written = 0;
	//#ifdef DEBUG
	//	ASSERT( false );
	//#endif
	//	return STRSAFE_E_INVALID_PARAMETER;
	}

_Pre_satisfies_( subitem == column::COL_LASTCHANGE ) _On_failure_( _Post_satisfies_( sizeBuffNeed == SIZE_T_ERROR ) ) _Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_LASTCHANGE( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
HRESULT CItemBranch::Text_WriteToStackBuffer_COL_ATTRIBUTES( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_ATTRIBUTES );
	auto res = CStyle_FormatAttributes( m_attr, psz_text, strSize, chars_written );
	if ( res != 0 ) {
		sizeBuffNeed = 8;//Generic size needed, overkill;
		chars_written = 0;
		_CrtDbgBreak( );//not handled yet.
		return STRSAFE_E_INVALID_PARAMETER;
		}
	ASSERT( chars_written == wcslen( psz_text ) );
	return S_OK;
	}

_Success_( SUCCEEDED( return ) )
HRESULT CItemBranch::Text_WriteToStackBuffer_default( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
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
HRESULT CItemBranch::Text_WriteToStackBuffer( _In_range_( 0, 7 ) const column::ENUM_COL subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Inout_ rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	switch ( subitem )
	{
			case column::COL_NAME:
				return Text_WriteToStackBuffer_COL_NAME( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_PERCENTAGE:
				return Text_WriteToStackBuffer_COL_PERCENTAGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_SUBTREETOTAL:
				return Text_WriteToStackBuffer_COL_SUBTREETOTAL( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ITEMS:
			case column::COL_FILES:
				return Text_WriteToStackBuffer_COL_FILES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_LASTCHANGE:
				return Text_WriteToStackBuffer_COL_LASTCHANGE( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_ATTRIBUTES:
				return Text_WriteToStackBuffer_COL_ATTRIBUTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			default:
				return Text_WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}


std::wstring CItemBranch::Text( _In_ _In_range_( 0, 7 ) const column::ENUM_COL subitem ) const {
	//wchar_t buffer[ 73 ] = { 0 };
	switch ( subitem ) {
			case column::COL_NAME:
				return m_name;
			case column::COL_PERCENTAGE:
				return GetTextCOL_PERCENTAGE( );
			case column::COL_SUBTREETOTAL:
				return FormatBytes( size_recurse( ), GetOptions( )->m_humanFormat );
			case column::COL_ITEMS://both GetTextCOL_ITEMS and GetTextCOL_FILES do same thing
			case column::COL_FILES:
				return GetTextCOL_FILES( );
			case column::COL_LASTCHANGE:
				return GetTextCOL_LASTCHANGE( );
			case column::COL_ATTRIBUTES:
				return GetTextCOL_ATTRIBUTES( );
			default:
				ASSERT( false );
				return L"";
		}
	}

COLORREF CItemBranch::ItemTextColor( ) const {
	if ( m_attr.invalid ) {
		return GetItemTextColor( true );
		}
	if ( m_attr.compressed ) {
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( m_attr.encrypted ) {
		return GetApp( )->m_altEncryptionColor;
		}
	return GetItemTextColor( true ); // The rest is not colored
	}

INT CItemBranch::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const {
	auto other = static_cast< const CItemBranch* >( tlib );
	switch ( subitem ) {
			case column::COL_NAME:
				return signum( m_name.compare( other->m_name ) );
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

//bool CItemBranch::IsAncestorOf( _In_ const CItemBranch& thisItem ) const {
//	auto p = static_cast<const CTreeListItem*>( &thisItem );
//	while ( p != NULL ) {
//		if ( p == static_cast<const CTreeListItem*>( this ) ) {
//			break;
//			}
//		p = p->m_parent;
//		}
//	return ( p != NULL );
//	}

//_Ret_notnull_ CItemBranch* CItemBranch::GetChildGuaranteedValid( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const {
//	if ( m_children != nullptr ) {
//		const auto childCount = m_childCount;
//		ASSERT( m_children_vector.size( ) == childCount );
//		ASSERT( i < childCount );
//		//return &( *( m_children + ( i ) ) );
//		return m_children_vector.at( i );
//		}
//	VERIFY( AfxCheckMemory( ) );//freak out
//	ASSERT( false );
//	TRACE( "%s Value: %I64u\r\n", global_strings::child_guaranteed_valid_err, std::uint64_t( i ) );
//	MessageBoxW( NULL, global_strings::child_guaranteed_valid_err, _T( "Hit `OK` when you're ready to abort." ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
//	std::terminate( );
//	}

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
	auto myParent = GetParentItem( );
	if ( myParent == NULL ) {
		return static_cast<DOUBLE>( 1.0 );//root item? must be whole!
		}
	auto parentSize = myParent->size_recurse( );
	if ( parentSize == 0 ) {//root item?
		return static_cast<DOUBLE>( 1.0 );
		}
	return static_cast<DOUBLE>( size_recurse( ) ) / static_cast<DOUBLE>( parentSize );
	}

std::wstring CItemBranch::GetPath( ) const {
	std::wstring pathBuf;
	pathBuf.reserve( MAX_PATH );
	UpwardGetPathWithoutBackslash( pathBuf );
	return pathBuf;
	}

void CItemBranch::UpwardGetPathWithoutBackslash( std::wstring& pathBuf ) const {
	auto myParent = GetParentItem( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
	switch ( m_type ) {
			case IT_DIRECTORY:
				if ( !pathBuf.empty( ) ) {
					if ( pathBuf.back( ) != L'\\' ) {//if pathBuf is empty, it's because we don't have a parent ( we're the root ), so we already have a "\\"
						pathBuf += L'\\';
						}
					}
				pathBuf += m_name;
				return;
			case IT_FILE:
				ASSERT( m_parent != NULL );
				if ( m_parent != NULL ) {
					if ( m_parent->m_parent != NULL ) {
						pathBuf += ( L'\\' + m_name );
						}
					else {
						pathBuf += m_name;
						}
					return;
					}
				ASSERT( false );
				return;
			default:
				ASSERT( false );
				return;
		}
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
	if ( m_type == IT_FILE ) {
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
	if ( m_type == IT_FILE ) {
		return m_size;
		}
	if ( m_vi != NULL ) {
		if ( m_vi->sizeCache != UINT64_ERROR ) {
			return m_vi->sizeCache;
			}
		}
	std::uint64_t total = m_size;
	const auto childCount = m_childCount;
	//ASSERT( m_childCount == childCount );
	for ( size_t i = 0; i < childCount; ++i ) {
		total += ( m_children + ( i ) )->size_recurse( );
		}
	if ( m_vi != NULL ) {
		if ( m_vi->sizeCache == UINT64_ERROR ) {
			if ( total != 0 ) {
				m_vi->sizeCache = total;
				}
			}
		}
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
_Pre_satisfies_( this->m_type == IT_FILE ) PCWSTR CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name.length( ) < ( MAX_PATH + 1 ) );
	PCWSTR resultPtrStr = PathFindExtensionW( m_name.c_str( ) );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}

_Pre_satisfies_( this->m_type == IT_FILE ) _Success_( SUCCEEDED( return ) ) HRESULT CItemBranch::CStyle_GetExtension( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_extension, const rsize_t strSize ) const {
	psz_extension[ 0 ] = 0;

	PWSTR resultPtrStr = PathFindExtensionW( m_name.c_str( ) );
	ASSERT( resultPtrStr != '\0' );
	if ( resultPtrStr != '\0' ) {
		size_t extLen = 0;
		auto res = StringCchLengthW( resultPtrStr, MAX_PATH, &extLen );
		if ( FAILED( res ) ) {
			psz_extension[ 0 ] = 0;
			return res;
			}
		if ( extLen > ( strSize ) ) {
			psz_extension[ 0 ] = 0;
			return STRSAFE_E_INSUFFICIENT_BUFFER;
			}
		res = StringCchCopyW( psz_extension, strSize, resultPtrStr );
#ifdef DEBUG
		if ( SUCCEEDED( res ) ) {
			ASSERT( GetExtension( ).compare( psz_extension ) == 0 );
			}
#endif
		return res;
		}

	psz_extension[ 0 ] = 0;
	return STRSAFE_E_INVALID_PARAMETER;//some generic error
	}

_Pre_satisfies_( this->m_type == IT_FILE ) const std::wstring CItemBranch::GetExtension( ) const {
	if ( m_type == IT_FILE ) {
		PWSTR resultPtrStr = PathFindExtensionW( m_name.c_str( ) );
		ASSERT( resultPtrStr != 0 );
		if ( resultPtrStr != '\0' ) {
			return resultPtrStr;
			}
		auto i = m_name.find_last_of( _T( '.' ) );

		if ( i == std::string::npos ) {
			return _T( "." );
			}
		return m_name.substr( i );
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
	const auto myLength = static_cast<DOUBLE>( m_name.length( ) );
	DOUBLE childrenTotal = 0;
	
	if ( m_type != IT_FILE ) {
		const auto childCount = m_childCount;
		for ( size_t i = 0; i < childCount; ++i ) {
			childrenTotal += ( m_children + ( i ) )->averageNameLength( );
			}
		return ( childrenTotal + myLength ) / static_cast<DOUBLE>( m_childCount + 1 );
		}
	ASSERT( m_childCount == 0 );
	return myLength;
	}

_Pre_satisfies_( this->m_type == IT_FILE )
void CItemBranch::stdRecurseCollectExtensionData_FILE( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) const {
	const size_t extensionPsz_size = 48;
	wchar_t extensionPsz[ extensionPsz_size ] = { 0 };
	HRESULT res = CStyle_GetExtension( extensionPsz, extensionPsz_size );
	if ( SUCCEEDED( res ) ) {
		if ( extensionMap[ extensionPsz ].files == 0 ) {
			extensionMap[ extensionPsz ].ext = extensionPsz;
			extensionMap[ extensionPsz ].ext.shrink_to_fit( );
			}
		++( extensionMap[ extensionPsz ].files );
		extensionMap[ extensionPsz ].bytes += m_size;
		}
	else {
		//use an underscore to avoid name conflict with _DEBUG build
		auto ext_ = GetExtension( );
		ext_.shrink_to_fit( );
		TRACE( _T( "Extension len: %i ( bigger than buffer! )\r\n\toffending extension:\r\n %s\r\n" ), ext_.length( ), ext_.c_str( ) );
		if ( extensionMap[ ext_ ].files == 0 ) {
			extensionMap[ ext_ ].ext = ext_;
			extensionMap[ ext_ ].ext.shrink_to_fit( );
			}
		++( extensionMap[ ext_ ].files );
		extensionMap[ ext_ ].bytes += m_size;
		}
	}


void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) const {
	if ( m_type == IT_FILE ) {
		stdRecurseCollectExtensionData_FILE( extensionMap );
		}
	else {
		const auto childCount = m_childCount;
		for ( size_t i = 0; i < childCount; ++i ) {
			( m_children + ( i ) )->stdRecurseCollectExtensionData( extensionMap );
			}

		}
	}

//I return a color that is visually obvious as an error, if directory, or `default`. This makes it easier to (literally) spot bugs.
//_Pre_satisfies_( this->m_type == IT_FILE ) COLORREF CItemBranch::GetGraphColor( ) const {
//	if ( m_type == IT_FILE ) {
//		return GetDocument( )->GetCushionColor( CStyle_GetExtensionStrPtr( ) );
//		}
//	ASSERT( m_type == IT_DIRECTORY );
//	return RGB( 254, 254, 254 );
//	}


//_Ret_maybenull_ CItemBranch* const FindCommonAncestor( _In_ _Pre_satisfies_( item1->m_type != IT_FILE ) const CItemBranch* const item1, _In_ const CItemBranch& item2 ) {
//	auto parent = item1;
//	while ( ( parent != NULL ) && ( !parent->IsAncestorOf( item2 ) ) ) {
//		if ( parent != NULL ) {
//			parent = parent->GetParentItem( );
//			}
//		else {
//			break;
//			}
//		}
//	ASSERT( parent != NULL );
//	return const_cast<CItemBranch*>( parent );
//	}

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
