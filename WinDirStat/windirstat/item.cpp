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
#ifndef ARRAYTEST
#define new DEBUG_NEW
#endif
#endif

namespace {
	const unsigned char INVALID_m_attributes = 0x80; // File attribute packing

	//struct compDirPair {

	//	bool operator()( const std::pair<DIRINFO, bool>& l, const std::pair<DIRINFO, bool>& r ) {
	//		return ( l.first.length < r.first.length );
	//		}
	//	} myobject_compDirPair;
	}

void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring& path ) {
	ASSERT( path.back( ) == L'*' );
	WIN32_FIND_DATA fData;
	HANDLE fDataHand = NULL;
	fDataHand = FindFirstFileExW( path.c_str( ), FindExInfoBasic, &fData, FindExSearchNameMatch, NULL, 0 );
	FILETIME t;
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
			
			auto thisDirPath = path;
			thisDirPath.pop_back( );
			thisDirPath.pop_back( );
			thisDirPath.pop_back( );
			thisDirPath += fData.cFileName;

			directories.emplace_back( DIRINFO { 0, fData.ftLastWriteTime, fData.dwFileAttributes, std::wstring( fData.cFileName ), std::move( thisDirPath ) } );
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
	}

std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> addFiles_returnSizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>> sizesToWorkOn_;
	for ( const auto& aFile : vecFiles ) {
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
#ifdef ARRAYTEST
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch{ IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true };
			++( ThisCItem->m_childCount );
			newChild->m_parent = ThisCItem;
			
#else
			auto newChild = ThisCItem->AddChild( new CItemBranch { IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true } );
#endif
			if ( path.back( ) != _T( '\\' ) ) {
				std::wstring newPath( path + _T( '\\' ) + aFile.name );
				sizesToWorkOn_.emplace_back( std::move( newChild ), std::async( GetCompressedFileSize_filename, std::move( newPath ) ) );
				}
			}
		else {
#ifdef ARRAYTEST
			auto newChild = ::new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_FILE, std::move( aFile.name ), std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true };
			++( ThisCItem->m_childCount );
			newChild->m_parent = ThisCItem;
			
#else
			auto newChild = ThisCItem->AddChild( new CItemBranch { IT_FILE, std::move( aFile.name ), std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true } );
#endif
			}
		}
	return sizesToWorkOn_;
	}


//std::pair<std::vector<std::pair<CItemBranch*, CString>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>
//std::vector<std::pair<CItemBranch*, CString>>
_Pre_satisfies_( !ThisCItem->m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>> readJobNotDoneWork( _In_ CItemBranch* const ThisCItem, std::wstring path ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	std::vector<FILEINFO> vecFiles;
	std::vector<DIRINFO>  vecDirs;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( vecFiles, vecDirs, std::move( path + _T( "\\*.*" ) ) );

	//std::sort( vecFiles.begin( ), vecFiles.end( ) );

	const auto fileCount = vecFiles.size( );
	const auto dirCount  = vecDirs.size( );
	
	if ( ( fileCount + dirCount ) > 0 ) {
#ifdef ARRAYTEST
		//ThisCItem->m_children.ThisCItem->m_children.size( ) + fileCount + dirCount );
#else
		ThisCItem->m_children.reserve( ThisCItem->m_children.size( ) + fileCount + dirCount );
#endif
		
		}
#ifdef ARRAYTEST
	ASSERT( ThisCItem->m_childCount == 0 );
	if ( ( fileCount + dirCount ) > 0 ) {
		ThisCItem->m_children = new CItemBranch[ fileCount + dirCount ];
		}
#endif
	////true for 2 means DIR
	//std::vector< std::pair<DIRINFO, bool>> items;
	//items.reserve( fileCount + dirCount );
	//for ( const auto& item : vecDirs ) {
	//	items.emplace_back( std::make_pair( item, true ) );
	//	}
	//for ( const auto& item : vecFiles ) {
	//	items.emplace_back( std::make_pair<DIRINFO, bool>( DIRINFO { item.length, item.lastWriteTime, item.attributes, item.name, L"" }, false ) );
	//	}
	////( const std::pair<DIRINFO, bool>& l, const std::pair<DIRINFO, bool>& r )
	////std::sort( items.begin( ), items.end( ), myobject_compDirPair );
	//std::sort( items.begin( ), items.end( ), [] ( const std::pair<DIRINFO, bool>& l, const std::pair<DIRINFO, bool>& r ) { return l.first.length < r.first.length; } );


	auto sizesToWorkOn_ = addFiles_returnSizesToWorkOn( ThisCItem, vecFiles, path );
	std::vector<std::pair<CItemBranch*, std::wstring>> dirsToWorkOn;
	dirsToWorkOn.reserve( vecDirs.size( ) );
	const auto thisApp = GetApp( );
	const auto thisOptions = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	for ( const auto& dir : vecDirs ) {
#ifdef ARRAYTEST
		bool dontFollow = ( thisApp->m_mountPoints.IsJunctionPoint( dir.path, dir.attributes ) && !thisOptions->m_followJunctionPoints ) || ( thisApp->m_mountPoints.IsMountPoint( dir.path ) && !thisOptions->m_followMountPoints );
		auto newitem = new ( &( ThisCItem->m_children[ ThisCItem->m_childCount ] ) ) CItemBranch { IT_DIRECTORY, std::move( dir.name ), static_cast<std::uint64_t>( 0 ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), false || dontFollow };
		++( ThisCItem->m_childCount );
		newitem->m_parent = ThisCItem;
#else
		auto newitem = ThisCItem->AddDirectory( dir.path, std::move( dir.attributes ), std::move( dir.name ), dir.lastWriteTime );
		//auto newitem = ThisCItem->AddChild( new CItemBranch { IT_FILE, std::move( dir.name ), std::move( dir.length ), std::move( dir.lastWriteTime ), std::move( dir.attributes ), true } );
#endif

		if ( !newitem->m_done ) {
			dirsToWorkOn.emplace_back( std::move( std::make_pair( std::move( newitem ), std::move( dir.path ) ) ) );
			//dirsToWorkOn.emplace_back( std::move( newitem ), std::move( dir.path ) );
			}
		}
	return std::make_pair( std::move( dirsToWorkOn ), std::move( sizesToWorkOn_ ) );
	}

void CItemBranch::SortAndSetDone( ) {
#ifdef ARRAYTEST
	qsort( m_children, static_cast< size_t >( m_childCount ), sizeof( CItemBranch ), &CItem_compareBySize );
#else
	qsort( m_children.data( ), static_cast< size_t >( m_children.size( ) ), sizeof( CItemBranch* ), &CItem_compareBySize );
	m_children.shrink_to_fit( );
#endif
	//GetTreeListControl( )->OnChildAdded( this )
	m_done = true;
	}

_Pre_satisfies_( this->m_parent == NULL ) void CItemBranch::AddChildren( ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_done );
	if ( m_parent == NULL ) {
		GetTreeListControl( )->OnChildAdded( NULL, this, false );
		}
	}

CItemBranch* CItemBranch::AddChild( _In_ _Post_satisfies_( child->m_parent == this ) CItemBranch* const child ) {
	
#ifdef ARRAYTEST
	ASSERT( false );
	displayWindowsMsgBoxWithMessage( std::wstring( L"AddChild not valid in ARRAYTEST!" ) );
	_CrtDbgBreak( );
	std::terminate( );
	//++m_childCount;
	//m_children[ m_childCount ] = new ( m_children[ m_childCount ] ) ( )
#else
	m_children.emplace_back( child );
#endif
	child->m_parent = this;
	return child;
	}


_Post_satisfies_( return->m_type == IT_DIRECTORY ) CItemBranch* CItemBranch::AddDirectory( std::wstring thisFilePath, DWORD thisFileAttributes, std::wstring thisFileName, FILETIME thisFileTime ) {

#ifdef ARRAYTEST
	ASSERT( false );
	displayWindowsMsgBoxWithMessage( std::wstring( L"AddDir not valid in ARRAYTEST!" ) );
	_CrtDbgBreak( );
	std::terminate( );
	//++m_childCount;
	//m_children[ m_childCount ] = new ( m_children[ m_childCount ] ) ( )
#endif

	const auto thisApp = GetApp( );
	const auto thisOptions = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow = ( thisApp->m_mountPoints.IsJunctionPoint( thisFilePath, thisFileAttributes ) && !thisOptions->m_followJunctionPoints ) || ( thisApp->m_mountPoints.IsMountPoint( thisFilePath ) && !thisOptions->m_followMountPoints );

	return AddChild( new CItemBranch { std::move( IT_DIRECTORY ), std::move( thisFileName ), std::move( static_cast<std::uint64_t>( 0 ) ), std::move( thisFileTime ), std::move( thisFileAttributes ), std::move( false || dontFollow ) } );
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) void DoSomeWorkShim( _In_ CItemBranch* const ThisCItem, std::wstring path, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx

	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	ASSERT( strcmp_path == 0 );
	if ( strcmp_path != 0 ) {
		auto fixedPath = L"\\\\?\\" + path;
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}

	DoSomeWork( std::move( ThisCItem ), std::move( path ), std::move( isRootRecurse ) );
	//wait for sync?
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) int DoSomeWork( _In_ CItemBranch* const ThisCItem, std::wstring path, const bool isRootRecurse ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	auto strcmp_path = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp_path != 0 ) {
		auto fixedPath = L"\\\\?\\" + path;
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}
	auto itemsToWorkOn = readJobNotDoneWork( ThisCItem, std::move( path ) );
#ifdef ARRAYTEST
	if ( ThisCItem->m_childCount == 0 ) {
#else
	if ( ThisCItem->m_children.size( ) == 0 ) {
#endif
		ASSERT( itemsToWorkOn.first.size( ) == 0 );
		ASSERT( itemsToWorkOn.second.size( ) == 0 );
		ThisCItem->SortAndSetDone( );
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
		workers.emplace_back( std::async( DoSomeWork, std::move( itemsToWorkOn.first[ i ].first ), std::move( itemsToWorkOn.first[ i ].second ), std::move( false ) ) );
		}

	const auto sizesToWorkOnCount = itemsToWorkOn.second.size( );

	for ( size_t i = 0; i < sizesToWorkOnCount; ++i ) {
		auto child = std::move( itemsToWorkOn.second[ i ].first );
		const auto sizeValue = std::move( itemsToWorkOn.second[ i ].second.get( ) );
		if ( sizeValue != UINT64_MAX ) {
			ASSERT( child != NULL );
			if ( child != NULL ) {
				child->m_size = std::move( sizeValue );
				}
			}
		}

	for ( auto& worker : workers ) {
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

CItemBranch::CItemBranch( ITEMTYPE type, _In_ std::wstring name, std::uint64_t size, FILETIME time, DWORD attr, bool done ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( size ), m_rect( 0, 0, 0, 0 ), m_lastChange( std::move( time ) ), m_done( std::move( done ) ) {
	SetAttributes( attr );
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
#ifdef ARRAYTEST
	delete[ ] m_children;
	m_children = nullptr;
	m_childCount = 0;
#else
	const auto childSize = m_children.size( );
	for ( size_t i = 0; i < childSize; ++i ) {
		delete m_children[ i ];
		m_children[ i ] = { NULL };
		}
#endif
	}

#ifdef ITEM_DRAW_SUBITEM
bool CItem::DrawSubitem( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem, _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );

	if ( subitem == COL_NAME ) {
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
		write_BAD_FMT( buffer );
		return buffer;
		}

	const wchar_t percentage[ 2 ] = { '%', 0 };
	res = StringCchCatW( buffer, bufSize, percentage );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( buffer );
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
		return FormatCount( files_recurse( ) );
		}
	return L"";
	}

std::wstring CItemBranch::GetTextCOL_LASTCHANGE( ) const {
	wchar_t psz_formatted_datetime[ 73 ] = { 0 };
	auto res = CStyle_FormatFileTime( std::move( FILETIME_recurse( ) ), psz_formatted_datetime, 73 );
	if ( res == 0 ) {
		return psz_formatted_datetime;
		}
	return L"BAD_FMT";
	}

std::wstring CItemBranch::GetTextCOL_ATTRIBUTES( ) const {
	//auto typeOfItem = m_type;
	wchar_t attributes[ 8 ] = { 0 };
	auto res = CStyle_FormatAttributes( m_attr, attributes, 6 );
	if ( res == 0 ) {
		return attributes;
		}
	return L"BAD_FMT";
	}


HRESULT CItemBranch::Text_WriteToStackBuffer( _In_range_( 0, 7 ) const INT subitem, _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_text, rsize_t strSize, rsize_t& sizeBuffNeed ) const {
	switch ( subitem )
	{
			case column::COL_NAME:
				{
				auto res = StringCchCopyW( psz_text, strSize, m_name.c_str( ) );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = ( m_name.length( ) + 2 );
					}
				return res;
				}
			case column::COL_PERCENTAGE:
				{
				auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", ( GetFraction( ) * static_cast<DOUBLE>( 100 ) ) );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 64;//Generic size needed.
					}
				return res;
				}
			case column::COL_SUBTREETOTAL:
				{
				auto res = FormatBytes( size_recurse( ), psz_text, strSize );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					sizeBuffNeed = 64;//Generic size needed.
					}
				return res;
				}
			case column::COL_ITEMS:
			case column::COL_FILES:
				{
				//ASSERT( strSize > 13 );
				//auto res = StringCchPrintfW( psz_formatted_text, strSize, L"%s", ( ( m_querying ) ? ( L"(querying...)" ) : ( L"(unavailable)" ) ) );
				//if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
				//	sizeOfBufferNeeded = 15;//Generic size needed.
				//	}
				//return res;
				displayWindowsMsgBoxWithMessage( std::wstring( L"Not implemented yet. Try normal GetText." ) );
#ifdef DEBUG
				ASSERT( false );
#else
				_CrtDbgBreak( );
#endif
				return STRSAFE_E_INVALID_PARAMETER;
				}
			case column::COL_LASTCHANGE:
				{
				auto res = CStyle_FormatFileTime( FILETIME_recurse( ), psz_text, strSize );
				if ( res == 0 ) {
					return S_OK;
					}
				else {
					_CrtDbgBreak( );//not handled yet.
					return STRSAFE_E_INVALID_PARAMETER;
					}
				}

			case column::COL_ATTRIBUTES:
				{
				auto res = CStyle_FormatAttributes( m_attr, psz_text, strSize );
				if ( res != 0 ) {
					sizeBuffNeed = 8;//Generic size needed, overkill;
					_CrtDbgBreak( );//not handled yet.
					return STRSAFE_E_INVALID_PARAMETER;
					}
				else {
					return S_OK;
					}
				}
			default:
				{
				ASSERT( strSize > 8 );
				auto res = StringCchPrintfW( psz_text, strSize, L"BAD GetText_WriteToStackBuffer - subitem" );
				if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
					if ( strSize > 8 ) {
						write_BAD_FMT( psz_text );
						}
					else {
						displayWindowsMsgBoxWithMessage( std::wstring( L"CItemBranch::GetText_WriteToStackBuffer - SERIOUS ERROR!" ) );
						}
					}
				return res;
				}
	}
	}


std::wstring CItemBranch::Text( _In_ _In_range_( 0, 7 ) const INT subitem ) const {
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
		return CTreeListItem::GetItemTextColor( true );
		}
	if ( m_attr.compressed ) {
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( m_attr.encrypted ) {
		return GetApp( )->m_altEncryptionColor;
		}
	return CTreeListItem::GetItemTextColor( true ); // The rest is not colored
	}

INT CItemBranch::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
	auto other = static_cast< const CItemBranch* >( tlib );
	switch ( subitem ) {
			case column::COL_NAME:
				return signum( m_name.compare( other->m_name ) );
			case column::COL_PERCENTAGE:
				return signum( GetFraction( ) - other->GetFraction( ) );
			case column::COL_SUBTREETOTAL:
				return signum( std::int64_t( size_recurse( ) ) - std::int64_t( other->size_recurse( ) ) );
			case column::COL_ITEMS:
			case column::COL_FILES:
				return signum( files_recurse( ) - other->files_recurse( ) );
			case column::COL_LASTCHANGE:
				return Compare_FILETIME( FILETIME_recurse( ), other->FILETIME_recurse( ) );
			case column::COL_ATTRIBUTES:
				return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
			default:
				ASSERT( false );
				return 666;
		}
	}

bool CItemBranch::IsAncestorOf( _In_ const CItemBranch& thisItem ) const {
	auto p = &thisItem;
	while ( p != NULL ) {
		if ( p == this ) {
			break;
			}
		p = p->GetParent( );
		}
	return ( p != NULL );
	}

_Ret_notnull_ CItemBranch* CItemBranch::GetChildGuaranteedValid( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const {
#ifdef ARRAYTEST
	if ( m_children != nullptr ) {
#ifdef ARRAYTEST
		const auto childCount = m_childCount;
#else
		const auto childCount = m_children.size( );
#endif
		ASSERT( i < childCount );
		return &( *( m_children + ( i ) ) );
		}
#else
	if ( m_children.at( i ) != NULL ) {
		return m_children[ i ];
		}
#endif
	AfxCheckMemory( );//freak out
	ASSERT( false );
	TRACE( _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Value: %I64u\r\n" ), std::uint64_t( i ) );
	MessageBoxW( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	std::terminate( );
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
	m_attr.archive    = ( ( attr bitand FILE_ATTRIBUTE_ARCHIVE       ) != 0 );
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
	ret += ( m_attr.archive    ) ? 1000 : 0; // A
	ret += ( m_attr.compressed ) ? 100 : 0; // C
	ret += ( m_attr.encrypted  ) ? 10 : 0; // E

	return static_cast< INT >( ( m_attr.invalid ) ? 0 : ret );
	}

DOUBLE CItemBranch::GetFraction( ) const {
	auto myParent = GetParent( );
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
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
	switch ( m_type ) {
			case IT_DIRECTORY:
				if ( !pathBuf.empty( ) ) {//if pathBuf is empty, it's because we don't have a parent ( we're the root ), so we already have a "\\"
					pathBuf += _T( "\\" );
					}
				pathBuf += m_name;
				return;
			case IT_FILE:
				pathBuf += ( _T( "\\" ) + m_name );
				return;
			default:
				ASSERT( false );
				return;
		}
	}

CRect CItemBranch::TmiGetRectangle( ) const {
	return BuildCRect( m_rect );
	}


FILETIME CItemBranch::FILETIME_recurse( ) const {
	auto ft = zeroInitFILETIME( );
	if ( Compare_FILETIME_cast( ft, m_lastChange ) ) {
		ft = m_lastChange;
		}
	
#ifdef ARRAYTEST
	const auto childCount = m_childCount;
#else
	const auto childCount = m_children.size( );
#endif
	for ( size_t i = 0; i < childCount; ++i ) {
#ifdef ARRAYTEST
		auto ft_child = ( m_children + ( i ) )->FILETIME_recurse( );
#else
		auto ft_child = m_children[ i ]->FILETIME_recurse( );
#endif
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
#ifdef ARRAYTEST
		const auto childCount = m_childCount;
		for ( size_t i = 0; i < childCount; ++i ) {
			childrenTotal = ( m_children + ( i ) )->averageNameLength( );
			}
#else
		for ( const auto& aChild : m_children ) {
			childrenTotal += aChild->averageNameLength( );
			}
#endif
		}

#ifdef ARRAYTEST
	return ( childrenTotal + myLength ) / static_cast<DOUBLE>( m_childCount + 1 );
#else
	return ( childrenTotal + myLength ) / static_cast<DOUBLE>( m_children.size( ) + 1 );
#endif
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
#ifdef ARRAYTEST
		const auto childCount = m_childCount;
#else
		const auto childCount = m_children.size( );
#endif		//ASSERT( m_childCount == childCount );
		for ( size_t i = 0; i < childCount; ++i ) {
#ifdef ARRAYTEST
			( m_children + ( i ) )->stdRecurseCollectExtensionData( extensionMap );
#else
			m_children[ i ]->stdRecurseCollectExtensionData( extensionMap );
#endif		
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


_Ret_maybenull_ CItemBranch* const FindCommonAncestor( _In_ _Pre_satisfies_( item1->m_type != IT_FILE ) const CItemBranch* const item1, _In_ const CItemBranch& item2 ) {
	auto parent = item1;
	while ( ( parent != NULL ) && ( !parent->IsAncestorOf( item2 ) ) ) {
		if ( parent != NULL ) {
			parent = parent->GetParent( );
			}
		else {
			break;
			}
		}
	ASSERT( parent != NULL );
	return const_cast<CItemBranch*>( parent );
	}

INT __cdecl CItem_compareBySize( _In_ _Points_to_data_ const void* const p1, _In_ _Points_to_data_ const void* const p2 ) {
	const auto size1 = ( *( reinterpret_cast< const CItemBranch * const* const >( p1 ) ) )->size_recurse( );
	const auto size2 = ( *( reinterpret_cast< const CItemBranch * const* const >( p2 ) ) )->size_recurse( );
	return signum( std::int64_t( size2 ) - std::int64_t( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
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
