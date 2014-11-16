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
//#include "item.h"
//#include "globalhelpers.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	const unsigned char INVALID_m_attributes = 0x80; // File attribute packing
	}

void addDIRINFO( _Inout_ std::vector<DIRINFO>& directories, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t ) {
	CFFWDS.GetLastWriteTime( &t );
	PCWSTR namePtr = CFFWDS.altGetFileName( );
	ASSERT( namePtr != NULL );
	if ( namePtr != NULL ) {
		directories.emplace_back( DIRINFO( 0, t, CFFWDS.GetAttributes( ), namePtr, CFFWDS.altGetFilePath_wstring( ) ) );
		}
	}

void addFILEINFO( _Inout_ std::vector<FILEINFO>& files, _Pre_valid_ _Post_invalid_ FILEINFO& fi, _In_ CFileFindWDS& CFFWDS ) {
	PCWSTR namePtr = CFFWDS.altGetFileName( );
	if ( namePtr != NULL ) {
		fi.name = namePtr;
		}
	else {
		fi.name = CFFWDS.GetFileName( );
		}
	fi.attributes = CFFWDS.GetAttributes( );
	//if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
	//	fi.length = CFFWDS.GetCompressedLength( );
	//	}
	//else {
	//	fi.length = CFFWDS.GetLength( );//temp
	//	}
	fi.length = CFFWDS.GetLength( );
	CFFWDS.GetLastWriteTime( &fi.lastWriteTime ); // (We don't use GetLastWriteTime(CTime&) here, because, if the file has an invalid timestamp, that function would ASSERT and throw an Exception.)
	//fi.name.FreeExtra( );
	files.emplace_back( std::move( fi ) );
	}

void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const std::wstring& path ) {
	CFileFindWDS finder;
	ASSERT( path.back( ) == _T( '*' ) );
	//BOOL b = finder.FindFile( GetFindPattern( path ) );
	BOOL b = finder.FindFile( path.c_str( ) );
	FILETIME t;
	FILEINFO fi;
	zeroFILEINFO( fi );
	while ( b ) {
		b = finder.FindNextFileW( );
		if ( finder.IsDots( ) ) {//This branches on the return of IsDirectory, then checks characters 0,1, & 2//IsDirectory calls MatchesMask, which bitwise-ANDs dwFileAttributes with FILE_ATTRIBUTE_DIRECTORY
			continue;//No point in operating on ourselves!
			}
		if ( finder.IsDirectory( ) ) {
			addDIRINFO( directories, finder, t );
			}
		else {
			addFILEINFO( files, fi, finder );
			zeroFILEINFO( fi );
			}
		}
	}

namespace {
	struct GetCompressedFileSize_functor {
		GetCompressedFileSize_functor( std::wstring&& in ) : path( std::move( in ) ) { }
		
		GetCompressedFileSize_functor( GetCompressedFileSize_functor&& in ) {
			path = std::move( in.path );
			}

		GetCompressedFileSize_functor( const GetCompressedFileSize_functor& in ) = delete;

		std::uint64_t operator()( ) {
			return GetCompressedFileSize_filename( path ); 
			}
		std::wstring path;
		};
	}
std::vector<std::pair<CItemBranch*, concurrency::task<std::uint64_t>>> sizesToWorkOn( _In_ CItemBranch* const ThisCItem, std::vector<FILEINFO>& vecFiles, const std::wstring& path ) {
	std::vector<std::pair<CItemBranch*, concurrency::task<std::uint64_t>>> sizesToWorkOn_;
	for ( const auto& aFile : vecFiles ) {
		auto newChild = ThisCItem->AddChild( new CItemBranch { IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true } );
		if ( ( aFile.attributes bitand FILE_ATTRIBUTE_COMPRESSED ) != 0 ) {
			//auto newChild = ThisCItem->AddChild( new CItemBranch { IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true } );
			//ASSERT( path.Right( 1 ).Compare( _T( "\\" ) ) != 0 );
			//if ( path.Right( 1 ).Compare( _T( "\\" ) ) != 0 ) {
			if ( path.back( ) != _T( '\\' ) ) {
				std::wstring newPath( path + _T( '\\' ) + aFile.name );
				//GetCompressedFileSize_functor functor( std::move( newPath ) );
				sizesToWorkOn_.emplace_back( newChild, concurrency::create_task( [ = ] ( ) { return GetCompressedFileSize_filename( newPath ); } ) );
				//sizesToWorkOn_.emplace_back( newChild, concurrency::create_task( std::move( functor ) ) );
				}
			}
		//else {
		//	auto newChild = ThisCItem->AddChild( new CItemBranch { IT_FILE, aFile.name, std::move( aFile.length ), std::move( aFile.lastWriteTime ), std::move( aFile.attributes ), true } );
		//	}
		}
	return sizesToWorkOn_;
	}


//std::pair<std::vector<std::pair<CItemBranch*, CString>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>
//std::vector<std::pair<CItemBranch*, CString>>
_Pre_satisfies_( !ThisCItem->m_done ) std::pair<std::vector<std::pair<CItemBranch*, std::wstring>>,std::vector<std::pair<CItemBranch*, concurrency::task<std::uint64_t>>>> readJobNotDoneWork( _In_ CItemBranch* const ThisCItem, const std::wstring path ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	std::vector<FILEINFO> vecFiles;
	std::vector<DIRINFO>  vecDirs;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( vecFiles, vecDirs, path + _T( "\\*.*" ) );

	const auto fileCount = vecFiles.size( );
	const auto dirCount  = vecDirs.size( );
	
	if ( ( fileCount + dirCount ) > 0 ) {
		ThisCItem->m_children.reserve( ThisCItem->m_children.size( ) + fileCount + dirCount );
		}

	auto sizesToWorkOn_ = sizesToWorkOn( ThisCItem, vecFiles, path );
	std::vector<std::pair<CItemBranch*, std::wstring>> dirsToWorkOn;
	dirsToWorkOn.reserve( vecDirs.size( ) );
	for ( const auto& dir : vecDirs ) {
		auto newitem = ThisCItem->AddDirectory( dir.path, dir.attributes, dir.name, dir.lastWriteTime );
		if ( !newitem->m_done ) {
			
			dirsToWorkOn.emplace_back( std::move( std::make_pair( std::move( newitem ), std::move( dir.path ) ) ) );
			//dirsToWorkOn.emplace_back( std::move( newitem ), std::move( dir.path ) );
			}
		}
	return std::move( std::make_pair( dirsToWorkOn, std::move( sizesToWorkOn_ ) ) );
	}

void CItemBranch::SortAndSetDone( ) {
	qsort( m_children.data( ), static_cast< size_t >( m_children.size( ) ), sizeof( CItemBranch * ), &CItem_compareBySize );
	m_children.shrink_to_fit( );
	//GetTreeListControl( )->OnChildAdded( this )
	m_done = true;
	}

void CItemBranch::AddChildren( ) {
	ASSERT( GetDocument( )->IsRootDone( ) );
	ASSERT( m_done );
	if ( m_parent == NULL ) {
		GetTreeListControl( )->OnChildAdded( NULL, this, false );
		}
	//for ( auto& child : m_children ) {
	//	GetTreeListControl( )->OnChildAdded( this, child, false );
	//	}
	//for ( auto& child : m_children ) {
	//	child->AddChildren( );
	//	}
	}

CItemBranch* CItemBranch::AddChild( _In_ _Post_satisfies_( child->m_parent == this ) CItemBranch* const child ) {
	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItemBranch::GetFraction() could ASSERT.
	m_children.emplace_back( child );
	child->m_parent = this;
	//GetTreeListControl( )->OnChildAdded( this, child, false );
	return child;
	}


_Post_satisfies_( return->m_type == IT_DIRECTORY ) CItemBranch* CItemBranch::AddDirectory( const std::wstring& thisFilePath, const DWORD thisFileAttributes, const std::wstring& thisFileName, const FILETIME& thisFileTime ) {
	const auto thisApp = GetApp( );
	const auto thisOptions = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow = ( thisApp->m_mountPoints.IsJunctionPoint( thisFilePath, thisFileAttributes ) && !thisOptions->m_followJunctionPoints ) || ( thisApp->m_mountPoints.IsMountPoint( thisFilePath ) && !thisOptions->m_followMountPoints );

	return AddChild( new CItemBranch { IT_DIRECTORY, thisFileName, 0, thisFileTime, thisFileAttributes, false || dontFollow } );
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) void DoSomeWorkShim( _In_ CItemBranch* const ThisCItem, std::wstring path, const bool isRootRecurse ) {
	//some sync primitive
	//http://msdn.microsoft.com/en-us/library/ff398050.aspx
	DoSomeWork( ThisCItem, std::move( path ), isRootRecurse );
	//wait for sync
	}

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) int DoSomeWork( _In_ CItemBranch* const ThisCItem, std::wstring path, const bool isRootRecurse ) {
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	//auto strcmp = path.compare( 0, 4, L"" )
	auto strcmp = path.compare( 0, 4, L"\\\\?\\", 0, 4 );
	if ( strcmp != 0 ) {
		auto fixedPath = L"\\\\?\\" + path;
		TRACE( _T( "path fixed as: %s\r\n" ), fixedPath.c_str( ) );
		path = fixedPath;
		}
	//auto fixedPath = L"\\\\?\\" + path;
	auto itemsToWorkOn = readJobNotDoneWork( ThisCItem, ( path ) );
	if ( ThisCItem->m_children.size( ) == 0 ) {
		ASSERT( itemsToWorkOn.first.size( ) == 0 );
		ASSERT( itemsToWorkOn.second.size( ) == 0 );
		ThisCItem->SortAndSetDone( );
		//return;
		//return dummy
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
	//std::vector<std::pair<CItemBranch*, std::wstring>>
	//std::vector<std::future<void>> workers;
	//std::vector<concurrency::task<int>> workers;
	concurrency::task_group tasks;
	//workers.reserve( dirsToWorkOnCount );
	for ( size_t i = 0; i < dirsToWorkOnCount; ++i ) {
		//DoSomeWork( dirsToWorkOn[ i ].first, dirsToWorkOn[ i ].second );
		ASSERT( itemsToWorkOn.first[ i ].second.length( ) > 1 );
		ASSERT( itemsToWorkOn.first[ i ].second.back( ) != L'\\' );
		ASSERT( itemsToWorkOn.first[ i ].second.back( ) != L'*' );
		//path += _T( "\\*.*" );
		//TODO: investigate task_group
		//workers.emplace_back( concurrency::create_task( [ = ] { return DoSomeWork( itemsToWorkOn.first[ i ].first, itemsToWorkOn.first[ i ].second, false ); } ) );
		tasks.run( [ = ] ( ) { return DoSomeWork( itemsToWorkOn.first[ i ].first, itemsToWorkOn.first[ i ].second, false ); } );
		}

	const auto sizesToWorkOnCount = itemsToWorkOn.second.size( );

	for ( size_t i = 0; i < sizesToWorkOnCount; ++i ) {
		auto child = itemsToWorkOn.second[ i ].first;
		//const auto sizeValue = itemsToWorkOn.second[ i ].second.get( );
		//if ( sizeValue != UINT64_MAX ) {
		//	ASSERT( child != NULL );
		//	if ( child != NULL ) {
		//		child->m_size = sizeValue;
		//		}
		//	}
		itemsToWorkOn.second[ i ].second.then( [ = ]( std::uint64_t sizeValue ) {
			if ( sizeValue != UINT64_MAX ) {
				ASSERT( child != NULL );
				if ( child != NULL ) {
					child->m_size = sizeValue;
					}
				}
			} );
		}

	//for ( auto& worker : workers ) {
	//	worker.get( );
	//	}
	//auto workerTasksDone = concurrency::when_all( workers.begin( ), workers.end( ) );
	//workerTasksDone.then( [ = ] {
	//	
	//	} );
	//workerTasksDone.wait( );

	tasks.wait( );
	ThisCItem->SortAndSetDone( );
	//return dummy
	return 0;
	}

//CString GetFindPattern( _In_ const CString& path ) {
//	ASSERT( path.Right( 1 ) != _T( '*' ) );
//	if ( path.Right( 1 ) != _T( '\\' ) ) {
//		return CString( path + _T( "\\*.*" ) );
//		}
//	return CString( path + _T( "*.*" ) );//Yeah, if you're wondering, `*.*` works for files WITHOUT extensions.
//	}

//
void AddFileExtensionData( _Out_ _Pre_satisfies_( ( extensionRecords._Mylast - extensionRecords._Myfirst ) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) {
	extensionRecords.reserve( extensionMap.size( ) + 1 );
	for ( auto& anExt : extensionMap ) {
		extensionRecords.emplace_back( std::move( anExt.second ) );
		}
	}

CItemBranch::CItemBranch( ITEMTYPE type, _In_ std::wstring name, std::uint64_t size, FILETIME time, DWORD attr, bool done ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( size ), m_rect( 0, 0, 0, 0 ), m_lastChange( time ), m_done( done ) {
	SetAttributes( attr );
	//m_name.FreeExtra( );
	}

CItemBranch::~CItemBranch( ) {
	//Found the OLD style loop to be a TINY bit faster.
	const auto childSize = m_children.size( );
	for ( size_t i = 0; i < childSize; ++i ) {
		delete m_children[ i ];
		m_children[ i ] = NULL;
		}
	//for ( auto& aChild : m_children ) {
	//	delete aChild;
	//	aChild = NULL;
	//	}
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
	auto res = CStyle_FormatDouble( GetFraction( ) * DOUBLE( 100 ), buffer, bufSize );
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

std::wstring CItemBranch::GetTextCOL_ITEMS( ) const {
	if ( m_type != IT_FILE ) {
		return FormatCount( files_recurse( ) );
		}
	return L"";
	}

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


std::wstring CItemBranch::GetText( _In_ _In_range_( 0, 7 ) const INT subitem ) const {
	//wchar_t buffer[ 73 ] = { 0 };
	switch ( subitem ) {
			case column::COL_NAME:
				return m_name;
			case column::COL_PERCENTAGE:
				return GetTextCOL_PERCENTAGE( );
			case column::COL_SUBTREETOTAL:
				return FormatBytes( size_recurse( ) );
			case column::COL_ITEMS:
				return GetTextCOL_ITEMS( );
			case column::COL_FILES:
				return GetTextCOL_FILES( );
			case column::COL_LASTCHANGE:
				return GetTextCOL_LASTCHANGE( );
			case column::COL_ATTRIBUTES:
				return GetTextCOL_ATTRIBUTES( );
			default:
				ASSERT( false );
				return L" ";
		}
	}

COLORREF CItemBranch::GetItemTextColor( ) const {
	if ( m_attr.invalid ) {
		return CTreeListItem::GetItemTextColor( );
		}
	if ( m_attr.compressed ) {
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( m_attr.encrypted ) {
		return GetApp( )->m_altEncryptionColor;
		}
	return CTreeListItem::GetItemTextColor( ); // The rest is not colored
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
	if ( m_children.at( i ) != NULL ) {
		return m_children[ i ];
		}
	AfxCheckMemory( );//freak out
	ASSERT( false );
	TRACE( _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Value: %I64u\r\n" ), std::uint64_t( i ) );
	MessageBoxW( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	//throw std::logic_error( "GetChildGuaranteedValid couldn't find a valid child! This should never happen!" );
	std::terminate( );
	}



void CItemBranch::SetAttributes( _In_ const DWORD attr ) {
	/*
	Encodes the attributes to fit (in) 1 byte
	Bitmask of m_attributes:
	7 6 5 4 3 2 1 0
	^ ^ ^ ^ ^ ^ ^ ^
	| | | | | | | |__ 1 == R					(0x01)
	| | | | | | |____ 1 == H					(0x02)
	| | | | | |______ 1 == S					(0x04)
	| | | | |________ 1 == A					(0x08)
	| | | |__________ 1 == Reparse point		(0x10)
	| | |____________ 1 == C					(0x20)
	| |______________ 1 == E					(0x40)
	|________________ 1 == invalid attributes	(0x80)
	*/

	//DWORD ret = attr;

	static_assert( sizeof( unsigned char ) == 1, "this method cannot do what it advertises if an unsigned char is NOT one byte in size!" );

	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		//m_attributes = INVALID_m_attributes;
		m_attr.invalid = true;
		return;
		}

	//ret &=  FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM; // Mask out lower 3 bits

	m_attr.readonly   = ( ( attr bitand FILE_ATTRIBUTE_READONLY      ) != 0 );
	m_attr.hidden     = ( ( attr bitand FILE_ATTRIBUTE_HIDDEN        ) != 0 );
	m_attr.system     = ( ( attr bitand FILE_ATTRIBUTE_SYSTEM        ) != 0 );
	m_attr.archive    = ( ( attr bitand FILE_ATTRIBUTE_ARCHIVE       ) != 0 );
	m_attr.compressed = ( ( attr bitand FILE_ATTRIBUTE_COMPRESSED    ) != 0 );
	m_attr.encrypted  = ( ( attr bitand FILE_ATTRIBUTE_ENCRYPTED     ) != 0 );
	m_attr.reparse    = ( ( attr bitand FILE_ATTRIBUTE_REPARSE_POINT ) != 0 );
	m_attr.invalid    = false;


	//auto archiveAttr = ( attr & FILE_ATTRIBUTE_ARCHIVE ) >> 2;
	//ret |= archiveAttr;// Prepend the archive attribute
	//auto reparseCompressedAttr = ( attr & ( FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_COMPRESSED ) ) >> 6;

	//ret |= reparseCompressedAttr; // --> At this point the lower nibble is fully used. Now shift the reparse point and compressed attribute into the lower 2 bits of the high nibble.
	//auto encryptAttr = ( attr &   FILE_ATTRIBUTE_ENCRYPTED ) >> 8;
	//ret |= encryptAttr; // Shift the encrypted bit by 8 places

	//m_attributes = UCHAR( ret );
	}

INT CItemBranch::GetSortAttributes( ) const {
	DWORD ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += ( m_attr.readonly ) ? 1000000 : 0; // R
	ret += ( m_attr.hidden ) ? 100000 : 0; // H
	ret += ( m_attr.system ) ? 10000 : 0; // S
	ret += ( m_attr.archive ) ? 1000 : 0; // A
	ret += ( m_attr.compressed ) ? 100 : 0; // C
	ret += ( m_attr.encrypted ) ? 10 : 0; // E

	return static_cast< INT >( ( m_attr.invalid ) ? 0 : ret );
	}

DOUBLE CItemBranch::GetFraction( ) const {
	auto myParent = GetParent( );
	if ( myParent == NULL ) {
		return 1.0;//root item? must be whole!
		}
	auto parentSize = myParent->size_recurse( );
	if ( parentSize == 0 ) {
		return 1.0;
		}
	return DOUBLE( size_recurse( ) ) / DOUBLE( parentSize );
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

_Pre_satisfies_( this->m_type == IT_FILE ) PCWSTR CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	//Sometimes I just need to COMPARE the extension with a string. So, instead of copying/screwing with CString internals, I'll just return a pointer to the substring.
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
			return ERROR_FUNCTION_FAILED;
			}
		if ( extLen > ( strSize ) ) {
			psz_extension[ 0 ] = 0;
			return STRSAFE_E_INSUFFICIENT_BUFFER;
			}
		res = StringCchCopyW( psz_extension, strSize, resultPtrStr );
		if ( SUCCEEDED( res ) ) {
			ASSERT( GetExtension( ).compare( psz_extension ) == 0 );
			}
		return res;
		}

	psz_extension[ 0 ] = 0;
	return ERROR_FUNCTION_FAILED;
	}

_Pre_satisfies_( this->m_type == IT_FILE ) const std::wstring CItemBranch::GetExtension( ) const {
	//INSIDE this function, CAfxStringMgr::Allocate	(f:\dd\vctools\vc7libs\ship\atlmfc\src\mfc\strcore.cpp:141) DOMINATES execution!!//TODO: FIXME: BUGBUG!
	if ( m_type == IT_FILE ) {
		PWSTR resultPtrStr = PathFindExtensionW( m_name.c_str( ) );
		ASSERT( resultPtrStr != 0 );
		if ( resultPtrStr != '\0' ) {
			return resultPtrStr;
			}
		//INT i = m_name.ReverseFind( _T( '.' ) );
		auto i = m_name.find_last_of( _T( '.' ) );

		if ( i == std::string::npos ) {
			return _T( "." );
			}
		else {
			//return m_name.Mid( i ).GetString( );
			return m_name.substr( i );
			}
		}
	return std::wstring( L"" );
	}


void CItemBranch::TmiSetRectangle( _In_ const CRect& rc ) const {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	m_rect.left = short( rc.left );
	m_rect.top = short( rc.top );
	m_rect.right = short( rc.right );
	m_rect.bottom = short( rc.bottom );
	}


DOUBLE CItemBranch::averageNameLength( ) const {
	const auto myLength = m_name.length( );
	DOUBLE childrenTotal = 0;
	if ( m_type != IT_FILE ) {
		for ( const auto& aChild : m_children ) {
			childrenTotal += aChild->averageNameLength( );
			}
		}
	return ( childrenTotal + myLength ) / ( m_children.size( ) + 1 );
	}

void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) const {
	if ( m_type == IT_FILE ) {
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
			TRACE( _T( "Extension len: %i ( bigger than buffer! )\r\n" ), ext_.length( ) );
			if ( extensionMap[ ext_ ].files == 0 ) {
				extensionMap[ ext_ ].ext = ext_;
				extensionMap[ ext_ ].ext.shrink_to_fit( );
				}
			++( extensionMap[ ext_ ].files );
			extensionMap[ ext_ ].bytes += m_size;
			}
		}
	else {
		//for ( auto& Child : m_children ) {
		//	Child->stdRecurseCollectExtensionData( /*extensionRecords,*/ extensionMap );
		//	}
		const auto childCount = m_children.size( );
		for ( size_t i = 0; i < childCount; ++i ) {
			m_children[ i ]->stdRecurseCollectExtensionData( /*extensionRecords,*/ extensionMap );
			}

		}
	}

//I return a color that is visually obvious as an error, if directory, or `default`. This makes it easier to (literally) spot bugs.
_Pre_satisfies_( this->m_type == IT_FILE ) COLORREF CItemBranch::GetGraphColor( ) const {
	if ( m_type == IT_FILE ) {
		return GetDocument( )->GetCushionColor( CStyle_GetExtensionStrPtr( ) );
		}
	if ( m_type == IT_DIRECTORY ) {
		return RGB( 254, 254, 254 );
		}
	return RGB( 0, 0, 0 );
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
