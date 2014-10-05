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

	bool operator< ( const FILETIME& t1, const FILETIME& t2 ) {
		const auto u1 = reinterpret_cast<const ULARGE_INTEGER&>( t1 );
		const auto u2 = reinterpret_cast<const ULARGE_INTEGER&>( t2 );
		return ( u1.QuadPart < u2.QuadPart );
		}

	}

void addDIRINFO( _Inout_ std::vector<DIRINFO>& directories, _Pre_valid_ _Post_invalid_ DIRINFO& di, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t ) {
	di.attributes    = CFFWDS.GetAttributes( );
	di.length        = 0;
	di.name          = CFFWDS.GetFileName( );
	di.path          = CFFWDS.GetFilePath( );
	CFFWDS.GetLastWriteTime( &t );
	di.lastWriteTime = t;
	directories.emplace_back( std::move( di ) );
	}

void addFILEINFO( _Inout_ std::vector<FILEINFO>& files, _Pre_valid_ _Post_invalid_ FILEINFO& fi, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t ) {
	PWSTR namePtr = CFFWDS.altGetFileName( );
	if ( namePtr != NULL ) {
		fi.name = namePtr;
		}
	else {
		fi.name = CFFWDS.GetFileName( );
		}
	fi.attributes = CFFWDS.GetAttributes( );
	if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
		fi.length = CFFWDS.GetCompressedLength( );
		}
	else {
		fi.length = CFFWDS.GetLength( );//temp
		}
	CFFWDS.GetLastWriteTime( &fi.lastWriteTime ); // (We don't use GetLastWriteTime(CTime&) here, because, if the file has an invalid timestamp, that function would ASSERT and throw an Exception.)
	files.emplace_back( std::move( fi ) );
	}

void FindFilesLoop( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, CString path ) {
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetFindPattern( path ) );
	FILETIME t;
	DIRINFO di;
	FILEINFO fi;
	zeroFILEINFO( fi );
	zeroDIRINFO( di );
	while ( b ) {
		b = finder.FindNextFile( );
		if ( finder.IsDots( ) ) {//This branches on the return of IsDirectory, then checks characters 0,1, & 2//IsDirectory calls MatchesMask, which bitwise-ANDs dwFileAttributes with FILE_ATTRIBUTE_DIRECTORY
			continue;//Skip the rest of the block. No point in operating on ourselves!
			}
		if ( finder.IsDirectory( ) ) {
			addDIRINFO( directories, di, finder, t );
			zeroDIRINFO( di );
			}
		else {
			addFILEINFO( files, fi, finder, t );
			zeroFILEINFO( fi );
			}
		}
	}

void readJobNotDoneWork( _In_ CItemBranch* ThisCItem, CString path ) {
	std::vector<FILEINFO> vecFiles;
	std::vector<DIRINFO>  vecDirs;
	CItemBranch* filesFolder = NULL;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( vecFiles, vecDirs, path );

	auto fileCount = vecFiles.size( );
	auto dirCount = vecDirs.size( );
	if ( fileCount > 0 ) {
		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder = new CItemBranch { IT_FILESFOLDER, _T( "<Files>" ), 0, zeroInitFILETIME( ), 0, false };
			ThisCItem->AddChild( filesFolder );
			}
		else {
			ASSERT( ( fileCount == 1 ) || ( dirCount == 0 ) );
			filesFolder = ThisCItem;
			}
		for ( const auto& aFile : vecFiles ) {
			filesFolder->AddChild( new CItemBranch { IT_FILE, aFile.name, aFile.length, aFile.lastWriteTime, aFile.attributes, true } );
			}
		filesFolder->UpwardAddFiles( fileCount );
		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder->SortAndSetDone( );
			}
		}

	for ( const auto& dir : vecDirs ) {
		ThisCItem->AddDirectory( dir.path, dir.attributes, dir.name, dir.lastWriteTime );
		}
	if ( dirCount != 0 ) {
		ThisCItem->UpwardAddSubdirs( dirCount );
		}
	ThisCItem->UpwardAddReadJobs( -1 );
	ThisCItem->m_readJobDone = true;
	}

std::vector<CItemBranch*> StillHaveTimeToWork( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t start ) {
	while ( ( GetTickCount64( ) - start < ticks ) && (!ThisCItem->IsTreeDone( ) ) ) {
		CItemBranch* minchild = NULL;
		bool moreWorkToDo = false;
		auto sizeOf_m_children = ThisCItem->m_children.size( );
		for ( size_t i = 0; i < sizeOf_m_children; ++i ) {
			if ( !ThisCItem->m_children.at( i )->IsTreeDone( ) ) {
				moreWorkToDo = true;
				auto tickssofar = GetTickCount64( ) - start;
				if ( ticks > tickssofar ) {
					DoSomeWork( ThisCItem->m_children[ i ], ticks - tickssofar );
					}
				else {
					break;
					}
				}
			}
		if ( !moreWorkToDo ) { //Either no children or all children are done!
			return std::vector<CItemBranch*>( );
			}
		}
	std::vector<CItemBranch*> minChilds;
	auto sizeOf_m_children = ThisCItem->m_children.size( );
	minChilds.reserve( sizeOf_m_children );
	for ( size_t i = 0; i < sizeOf_m_children; ++i ) {
		if ( !ThisCItem->m_children.at( i )->IsTreeDone( ) ) {
			minChilds.emplace_back( ThisCItem->m_children[ i ] );
			}
		}
	return minChilds;
	}

void DoSomeWork( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks ) {
	auto start = GetTickCount64( );
	ASSERT( ThisCItem->m_type == IT_DIRECTORY );
	if ( !ThisCItem->m_readJobDone ) {
		readJobNotDoneWork( ThisCItem, ThisCItem->GetPath( ) );
		}
	if ( GetTickCount64( ) - start > ticks ) {
		return;
		}
	if ( ThisCItem->GetChildrenCount( ) == 0 ) {
		ThisCItem->SortAndSetDone( );
		return;
		}
	auto notDone = StillHaveTimeToWork( ThisCItem, ticks, start );
	if ( notDone.empty( ) ) {
		ThisCItem->SortAndSetDone( );
		ThisCItem->DriveVisualUpdateDuringWork( );
		}

	}

CString GetFindPattern( _In_ const CString path ) {
	if ( path.Right( 1 ) != _T( '\\' ) ) {
		return CString( path + _T( "\\*.*" ) );
		}
	return CString( path + _T( "*.*" ) );//Yeah, if you're wondering, `*.*` works for files WITHOUT extensions.
	}

void AddFileExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) {
	ASSERT( extensionRecords.size( ) == 0 );
	extensionRecords.reserve( extensionMap.size( ) + 1 );
	for ( auto& anExt : extensionMap ) {
		extensionRecords.emplace_back( std::move( anExt.second ) );
		}
	}

_Ret_range_( 0, UINT64_MAX ) std::uint64_t GetProgressRangeDrive( CString path ) {
	std::uint64_t total = 0;
	std::uint64_t freeSp = 0;

	MyGetDiskFreeSpace( path, total, freeSp );
	ASSERT( ( std::int64_t( total ) - std::int64_t( freeSp ) ) >= 0 );
	return ( total - freeSp );
	}


CItemBranch::CItemBranch( ITEMTYPE type, _In_ CString name, std::uint64_t size, FILETIME time, DWORD attr, bool done, bool dontFollow ) : m_type( std::move( type ) ), m_name( name ), m_size( size ), m_files( 0 ), m_subdirs( 0 ), m_readJobs( 0 ), m_rect( 0, 0, 0, 0 ), m_lastChange( time ), m_done ( done ) {
	auto thisItem_type = m_type;
	if ( thisItem_type == IT_FILE || dontFollow ) {
		m_readJobDone = true;
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_FILESFOLDER ) {
		UpwardAddReadJobs( 1 );
		m_readJobDone = false;
		}

	SetAttributes( attr );
	m_name.FreeExtra( );
	}

CItemBranch::~CItemBranch( ) {
	for ( auto& aChild : m_children ) {
		delete aChild;
		aChild = NULL;
		}
	}

#ifdef ITEM_DRAW_SUBITEM
bool CItem::DrawSubitem( _In_ _In_range_( 0, INT32_MAX ) const INT subitem, _In_ CDC* pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );
	
	if ( subitem == COL_NAME ) {
		return CTreeListItem::DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
		}
	//if ( subitem != COL_SUBTREEPERCENTAGE ) {
		//return false;
		//}
	if ( MustShowReadJobs( ) ) {
		if ( IsTreeDone( ) ) {
			return false;
			}
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

CString CItemBranch::GetTextCOL_PERCENTAGE( ) const {
	if ( GetOptions( )->m_showTimeSpent && MustShowReadJobs( ) ) {
		const size_t bufSize = 24;
		wchar_t buffer[ bufSize ] = { 0 };
		if ( IsTreeDone( ) ) {
			return buffer;
			}
		HRESULT res = STRSAFE_E_INVALID_PARAMETER;

		if ( m_readJobs == 1 ) {
			res = StringCchPrintf( buffer, bufSize, L"[%s Read Job]", FormatCount( m_readJobs ).GetString( ) );
			}
		else {
			res = StringCchPrintf( buffer, bufSize, L"[%s Read Jobs]", FormatCount( m_readJobs ).GetString( ) );
			}
		if ( !SUCCEEDED( res ) ) {
			write_BAD_FMT( buffer );
			}
		return buffer;
		}
	const size_t bufSize = 12;

	wchar_t buffer[ bufSize ] = { 0 };
	auto res = CStyle_FormatDouble( GetFraction( ) * DOUBLE( 100 ), buffer, bufSize );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( buffer );
		return buffer;
		}

	wchar_t percentage[ 2 ] = { '%', 0 };
	res = StringCchCat( buffer, bufSize, percentage );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( buffer );
		return buffer;
		}
	return buffer;
	}

CString CItemBranch::GetTextCOL_ITEMS( ) const {
	if ( m_type != IT_FILE ) {
		return FormatCount( GetItemsCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_FILES( ) const {
	if ( m_type != IT_FILE ) {
		return FormatCount( GetFilesCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_SUBDIRS( ) const { 
	if ( m_type != IT_FILE ) {
		return FormatCount( GetSubdirsCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_LASTCHANGE( ) const {
	wchar_t psz_formatted_datetime[ 73 ] = { 0 };
	auto res = CStyle_FormatFileTime( m_lastChange, psz_formatted_datetime, 73 );
	if ( res == 0 ) {
		return psz_formatted_datetime;
		}
	return _T( "BAD_FMT" );
	}

CString CItemBranch::GetTextCOL_ATTRIBUTES( ) const {
	auto typeOfItem = m_type;
	if ( typeOfItem != IT_FILESFOLDER ) {
		wchar_t attributes[ 8 ] = { 0 };
		auto res = CStyle_FormatAttributes( GetAttributes( ), attributes, 6 );
		if ( res == 0 ) {
#ifdef DEBUG
			ASSERT( FormatAttributes( GetAttributes( ) ).Compare( attributes ) == 0 );
#endif
			return attributes;
			}
		return _T( "BAD_FMT" );
		}
	return _T( "" );
	}


CString CItemBranch::GetText( _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
	switch (subitem)
	{
		case column::COL_NAME:
			return m_name;
		case column::COL_PERCENTAGE:
			return GetTextCOL_PERCENTAGE( );
		case column::COL_SUBTREETOTAL:
			return FormatBytes( m_size );
		case column::COL_ITEMS:
			return GetTextCOL_ITEMS( );
		case column::COL_FILES:
			return GetTextCOL_FILES( );
		case column::COL_SUBDIRS:
			return GetTextCOL_SUBDIRS( );
		case column::COL_LASTCHANGE:
			return GetTextCOL_LASTCHANGE( );
		case column::COL_ATTRIBUTES:
			return GetTextCOL_ATTRIBUTES( );
		default:
			ASSERT( false );
			return _T( " " );
	}
	}

COLORREF CItemBranch::GetItemTextColor( ) const {
	auto attr = GetAttributes( ); // Get the file/folder attributes

	if ( attr == INVALID_FILE_ATTRIBUTES ) { // This happens e.g. on a Unicode-capable FS when using ANSI APIs to list files with ("real") Unicode names
		return CTreeListItem::GetItemTextColor( );
		}
	if ( attr & FILE_ATTRIBUTE_COMPRESSED ) { // Check for compressed flag
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( attr & FILE_ATTRIBUTE_ENCRYPTED ) {
		return GetApp( )->m_altEncryptionColor;
		}
	return CTreeListItem::GetItemTextColor( ); // The rest is not colored
	}

INT CItemBranch::CompareName( _In_ const CItemBranch* const other ) const {
	return signum( m_name.CompareNoCase( other->m_name ) );
	}

INT CItemBranch::CompareLastChange( _In_ const CItemBranch* const other ) const {
	if ( m_lastChange < other->m_lastChange ) {
		return -1;
		}
	else if ( ( m_lastChange.dwLowDateTime == other->m_lastChange.dwLowDateTime ) && ( m_lastChange.dwHighDateTime  == other->m_lastChange.dwHighDateTime ) ) {
		return 0;
		}
	return 1;
	}


INT CItemBranch::CompareSibling( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
	auto other = static_cast< const CItemBranch* >( tlib );
	switch ( subitem )
	{
		case column::COL_NAME:
			return CompareName( other );
		case column::COL_PERCENTAGE:
			return signum( GetFraction( )       - other->GetFraction( ) );
		case column::COL_SUBTREETOTAL:
			return signum( std::int64_t( m_size ) - std::int64_t( other->m_size ) );
		case column::COL_ITEMS:
			return signum( GetItemsCount( )     - other->GetItemsCount( ) );
		case column::COL_FILES:
			return signum( GetFilesCount( )     - other->GetFilesCount( ) );
		case column::COL_SUBDIRS:
			return signum( GetSubdirsCount( )   - other->GetSubdirsCount( ) );
		case column::COL_LASTCHANGE:
			return CompareLastChange( other );
		case column::COL_ATTRIBUTES:
			return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
		default:
			ASSERT( false );
			return 666;
	}
	}

_Success_( return != NULL ) _Must_inspect_result_ CTreeListItem* CItemBranch::GetTreeListChild( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const {
	return m_children.at( i );
	}

#ifdef ITEM_DRAW_SUBITEM
INT CItemBranch::GetImageToCache( ) const { // (Caching is done in CTreeListItem::m_vi.)
	auto type_theItem = GetType( );
	if ( type_theItem == IT_FILESFOLDER ) {
		return GetMyImageList( )->GetFilesFolderImage( );
		}
	auto path = GetPath();
	auto MyImageList = GetMyImageList( );
	if ( GetApp( )->IsMountPoint( path ) ) {
		return MyImageList->GetMountPointImage( );
		}
	else if ( GetApp( )->IsJunctionPoint( path, GetAttributes( ) ) ) {
		return MyImageList->GetJunctionImage( );
		}
	return MyImageList->GetFileImage( path );
	}
#endif

bool CItemBranch::IsAncestorOf( _In_ const CItemBranch* const thisItem ) const {
	auto p = thisItem;
	while ( p != NULL ) {
		if ( p == this ) {
			break;
			}
		p = p->GetParent( );
		}
	return ( p != NULL );
	}

//std::uint64_t CItemBranch::GetProgressRange( ) const {
//	switch ( m_type )
//	{
//		case IT_DIRECTORY:
//		case IT_FILESFOLDER:
//		case IT_FILE:
//			return 0;
//		default:
//			ASSERT( false );
//			return 0;
//	}
//	}

//std::uint64_t CItemBranch::GetProgressPos( ) const {
//	switch ( m_type )
//	{
//		case IT_DIRECTORY:
//			return std::uint64_t( m_files ) + std::uint64_t( m_subdirs );
//		case IT_FILE:
//		case IT_FILESFOLDER:
//		default:
//			ASSERT( false );
//			return 0;
//	}
//	}

_Success_( return != NULL ) CItemBranch* CItemBranch::GetChildGuaranteedValid( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const {
	if ( m_children.at( i ) != NULL ) {
		return m_children[ i ];
		}
	AfxCheckMemory( );//freak out
	ASSERT( false );
	TRACE( _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Value: %I64u\r\n" ), std::uint64_t( i ) );
	MessageBox( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	//throw std::logic_error( "GetChildGuaranteedValid couldn't find a valid child! This should never happen!" );
	std::terminate( );
	}

CItemBranch* CItemBranch::AddChild( _In_ CItemBranch* child ) {
	ASSERT( !IsTreeDone( ) );// SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItemBranch::GetFraction() could ASSERT.
	if ( child->m_size != 0 ) {
		UpwardAddSize( child->m_size );
		}

	auto readJobs = child->GetReadJobs( );
	if ( readJobs != 0 ) {
		UpwardAddReadJobs( readJobs );
		}
	UpwardUpdateLastChange( child->m_lastChange );
	m_children.push_back( child );

	child->m_parent = this;
	ASSERT( child->GetParent( ) == this );
	
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnChildAdded( this, child, IsTreeDone( ) );
		}
	ASSERT( TreeListControl != NULL );
	return child;
	}

void CItemBranch::UpwardAddSubdirs( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t dirCount ) {
	ASSERT( dirCount != 0 );
	if ( dirCount < 0 ) {
		if ( ( dirCount + m_subdirs ) < 0 ) {
			m_subdirs = 0;
			}
		else {
			m_subdirs -= std::uint32_t( dirCount * ( -1 ) );
			}
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSubdirs( dirCount );
			}
		}
	else {
		m_subdirs += std::uint32_t( dirCount );
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSubdirs( dirCount );
			}
		//else `this` may be the root item.
		}
	}

void CItemBranch::UpwardAddFiles( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t fileCount ) {
	ASSERT( fileCount != 0 );
	if ( fileCount < 0 ) {
		if ( ( m_files + fileCount ) < 0 ) {
			m_files = 0;
			}
		else {
			m_files -= std::uint32_t( fileCount * ( -1 ) );
			}
		auto theParent = GetParent( );
		if ( theParent != NULL ) {
			theParent->UpwardAddFiles( fileCount );
			}
		}
	else {
		m_files += std::uint32_t( fileCount );
		auto theParent = GetParent( );
		if ( theParent != NULL ) {
			theParent->UpwardAddFiles( fileCount );
			}
		//else `this` may be the root item.
		}
	}

void CItemBranch::UpwardAddSize( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t bytes ) {
	ASSERT( bytes != 0 );
	if ( bytes < 0 ) {
		if ( ( bytes + std::int64_t( m_size ) ) < 0 ) {
			m_size = 0;
			}
		else {
			m_size -= std::uint64_t( bytes * ( -1 ) );
			}
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		}
	else {
		m_size += std::uint64_t( bytes );
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		//else `this` may be the root item.
		}
	}

void CItemBranch::UpwardAddReadJobs( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t count ) {
	ASSERT( count != 0 );
	if ( count < 0 ) {
		if ( ( m_readJobs + count ) < 0 ) {
			m_readJobs = 0;
			}
		else {
			m_readJobs -= std::uint32_t( count * ( -1 ) );
			}
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddReadJobs( count );
			}
		}
	else {
		m_readJobs += std::uint32_t( count );

		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddReadJobs( count );
			}
		//else `this` may be the root item.
		}
	}

void CItemBranch::UpwardUpdateLastChange(_In_ const FILETIME& t) {
	if ( m_lastChange < t ) {
		m_lastChange = t;
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardUpdateLastChange( t );
			}
		//else `this` may be the root item.
		}
	}

void CItemBranch::SetAttributes( const DWORD attr ) {
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
	
	DWORD ret = attr;
	
	static_assert( sizeof( unsigned char ) == 1, "this method cannot do what it advertises if an unsigned char is NOT one byte in size!" );

	if ( ret == INVALID_FILE_ATTRIBUTES ) {
		m_attributes = INVALID_m_attributes;
		return;
		}

	ret &=  FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM; // Mask out lower 3 bits

	ret |= ( attr &   FILE_ATTRIBUTE_ARCHIVE                                     ) >> 2; // Prepend the archive attribute
	ret |= ( attr & ( FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_COMPRESSED ) ) >> 6; // --> At this point the lower nibble is fully used. Now shift the reparse point and compressed attribute into the lower 2 bits of the high nibble.
	ret |= ( attr &   FILE_ATTRIBUTE_ENCRYPTED                                   ) >> 8; // Shift the encrypted bit by 8 places

	m_attributes = UCHAR( ret );
	}

// Decode the attributes encoded by SetAttributes()
DWORD CItemBranch::GetAttributes( ) const {
	DWORD ret = m_attributes;

	if ( ret & INVALID_m_attributes ) {
		return INVALID_FILE_ATTRIBUTES;
		}

	ret &= FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;// Mask out lower 3 bits
	
	ret |= ( m_attributes & 0x08 ) << 2; // FILE_ATTRIBUTE_ARCHIVE
	ret |= ( m_attributes & 0x30 ) << 6; // FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_COMPRESSED
	ret |= ( m_attributes & 0x40 ) << 8; // FILE_ATTRIBUTE_ENCRYPTED
	
	return ret;
	}

// Returns a value which resembles sorting of RHSACE considering gaps
INT CItemBranch::GetSortAttributes( ) const {
	DWORD ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += ( m_attributes & 0x01 ) ? 1000000 : 0; // R
	ret += ( m_attributes & 0x02 ) ? 100000  : 0; // H
	ret += ( m_attributes & 0x04 ) ? 10000   : 0; // S
	ret += ( m_attributes & 0x08 ) ? 1000    : 0; // A
	ret += ( m_attributes & 0x20 ) ? 100     : 0; // C
	ret += ( m_attributes & 0x40 ) ? 10      : 0; // E

	return ( ( m_attributes & INVALID_m_attributes ) ? 0 : ret );
	}

DOUBLE CItemBranch::GetFraction( ) const {
	auto myParent = GetParent( );
	if ( myParent == NULL ) {
		return 1.0;//root item? must be whole!
		}
	auto parentSize = myParent->m_size;
	if ( parentSize == 0){
		return 1.0;
		}
	return DOUBLE( m_size ) / DOUBLE( parentSize );
	}

CString CItemBranch::GetPath( ) const {
	CString pathBuf;
	pathBuf.Preallocate( MAX_PATH );
	UpwardGetPathWithoutBackslash( pathBuf );
	auto typeOfThisItem = m_type;
	if ( ( typeOfThisItem == IT_FILESFOLDER ) ) {
		pathBuf += _T( "\\" );
		}
	pathBuf.FreeExtra( );
	return pathBuf;
	}

CString CItemBranch::GetFolderPath( ) const {
	/*
	  Returns the path for "Explorer here" or "Command Prompt here"
	*/
	auto path = GetPath( );
	if ( m_type == IT_FILE ) {
		auto i = path.ReverseFind( _T( '\\' ) );
		ASSERT( i != -1 );
		return path.Left( i + 1 );
		}
	return path;
	}

void CItemBranch::UpwardGetPathWithoutBackslash( CString& pathBuf ) const {
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		myParent->UpwardGetPathWithoutBackslash( pathBuf );
		}
	switch ( m_type )
	{
		case IT_DIRECTORY:
			if ( !pathBuf.IsEmpty( ) ) {//if pathBuf is empty, it's because we don't have a parent ( we're the root ), so we already have a "\\"
				pathBuf += _T( "\\" );
				}
			pathBuf += m_name;
			return;
		case IT_FILE:
			pathBuf += ( _T( "\\" ) + m_name );
			return;
		case IT_FILESFOLDER:
			return;
		default:
			ASSERT( false );
			return;
	}
	}

_Pre_satisfies_( this->m_type == IT_FILE ) PCWSTR CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_name.GetLength( ) < ( MAX_PATH + 1 ) );
	PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}

_Pre_satisfies_( this->m_type == IT_FILE ) _Success_( SUCCEEDED( return ) ) HRESULT CItemBranch::CStyle_GetExtension( _Out_writes_z_( strSize ) PWSTR psz_extension, const rsize_t strSize ) const {
	psz_extension[ 0 ] = 0;

	PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
	ASSERT( resultPtrStr != '\0' );
	if ( resultPtrStr != '\0' ) {
		size_t extLen = 0;
		auto res = StringCchLength( resultPtrStr, MAX_PATH, &extLen );
		if ( FAILED( res ) ) {
			psz_extension[ 0 ] = 0;
			return ERROR_FUNCTION_FAILED;
			}
		if ( extLen > ( strSize + 1 ) ) {
			psz_extension[ 0 ] = 0;
			return STRSAFE_E_INSUFFICIENT_BUFFER;
			}
		res = StringCchCopy( psz_extension, strSize, resultPtrStr );
		if ( SUCCEEDED( res ) ) {
			ASSERT( GetExtension( ).compare( psz_extension ) == 0 );
			}
		return res;
		}

	psz_extension[ 0 ] = 0;
	return ERROR_FUNCTION_FAILED;
	}

const std::wstring CItemBranch::GetExtension( ) const {
	//INSIDE this function, CAfxStringMgr::Allocate	(f:\dd\vctools\vc7libs\ship\atlmfc\src\mfc\strcore.cpp:141) DOMINATES execution!!//TODO: FIXME: BUGBUG!
	switch ( m_type )
	{
		case IT_FILE:
			{
				PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
				ASSERT( resultPtrStr != '\0' );
				ASSERT( resultPtrStr != 0 );
				if ( resultPtrStr != '\0' ) {
					return resultPtrStr;
					}
				INT i = m_name.ReverseFind( _T( '.' ) );
				
				if ( i == -1 ) {
					return _T( "." );
					}
				else {
					return m_name.Mid( i ).GetString( );
					}
			}
		default:
			ASSERT( false );
			return std::wstring( L"" );
	}
	}

void CItemBranch::SortAndSetDone( ) {
	ASSERT( !IsTreeDone( ) );
	if ( IsTreeDone( ) ) {
		return;
		}
	qsort( m_children.data( ), static_cast< size_t >( m_children.size( ) ), sizeof( CItemBranch *), &CItem_compareBySize );
	m_children.shrink_to_fit( );
	m_done = true;
	}



void CItemBranch::TmiSetRectangle( _In_ const CRect& rc ) {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	m_rect.left		= short( rc.left   );
	m_rect.top		= short( rc.top    );
	m_rect.right	= short( rc.right  );
	m_rect.bottom	= short( rc.bottom );
	}


DOUBLE CItemBranch::averageNameLength( ) const {
	int myLength = m_name.GetLength( );
	DOUBLE childrenTotal = 0;
	if ( m_type != IT_FILE ) {
		for ( const auto& aChild : m_children ) {
			childrenTotal += aChild->averageNameLength( );
			}
		}
	return ( childrenTotal + myLength ) / ( m_children.size( ) + 1 );
	}

void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem == IT_FILE) {
		const size_t extensionPsz_size = 48;
		wchar_t extensionPsz[ extensionPsz_size ] = { 0 };
		HRESULT res = CStyle_GetExtension( extensionPsz, extensionPsz_size );
		if ( SUCCEEDED( res ) ) {
			if ( extensionMap[ extensionPsz ].files == 0 ) {
				++( extensionMap[ extensionPsz ].files );
				extensionMap[ extensionPsz ].bytes += m_size;
				extensionMap[ extensionPsz ].ext = extensionPsz;
				extensionMap[ extensionPsz ].ext.shrink_to_fit( );
				}
			else {
				++( extensionMap[ extensionPsz ].files );
				extensionMap[ extensionPsz ].bytes += m_size;
				}
			}
		else {
			//use an underscore to avoid name conflict with _DEBUG build
			auto ext_ = GetExtension( );
			ext_.shrink_to_fit( );
			TRACE( _T( "Extension len: %i ( bigger than buffer! )\r\n" ), ext_.length( ) );
			if ( extensionMap[ ext_ ].files == 0 ) {
				++( extensionMap[ ext_ ].files );
				extensionMap[ ext_ ].bytes += m_size;
				extensionMap[ ext_ ].ext = ext_;
				extensionMap[ ext_ ].ext.shrink_to_fit( );
				}
			else {
				++( extensionMap[ ext_ ].files );
				extensionMap[ ext_ ].bytes += m_size;
				}
			}
		}
	else {
		for ( auto& Child : m_children ) {
			Child->stdRecurseCollectExtensionData( /*extensionRecords,*/ extensionMap );
			}
		}
	}

COLORREF CItemBranch::GetGraphColor( ) const {
	switch ( m_type )
	{
		case IT_FILE:
			return ( GetDocument( )->GetCushionColor( CStyle_GetExtensionStrPtr( ) ) );
		case IT_FILESFOLDER:
		case IT_DIRECTORY:
			return RGB( 254, 254, 254 );
//		case IT_FILESFOLDER:
//			return RGB( 254, 254, 254 );
		default:
			return RGB( 0, 0, 0 );
	}
	}

bool CItemBranch::MustShowReadJobs( ) const {
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		return !myParent->IsTreeDone( );
		}
	return !IsTreeDone( );
	}

CItemBranch* CItemBranch::AddDirectory( const CString thisFilePath, const DWORD thisFileAttributes, const CString thisFileName, const FILETIME& thisFileTime ) {
	auto thisApp      = GetApp( );
	auto thisOptions  = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow = ( thisApp->IsJunctionPoint( thisFilePath, thisFileAttributes ) && !thisOptions->m_followJunctionPoints ) || ( thisApp->IsMountPoint( thisFilePath ) && !thisOptions->m_followMountPoints );
	
	return AddChild( new CItemBranch{ IT_DIRECTORY, thisFileName, 0, thisFileTime, thisFileAttributes, false, dontFollow } );
	}

void CItemBranch::DriveVisualUpdateDuringWork( ) {
	MSG msg;
	while ( PeekMessage( &msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE ) ) {
		DispatchMessage( &msg );
		}
	GetApp( )->PeriodicalUpdateRamUsage( );
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
