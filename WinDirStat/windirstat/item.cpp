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


void FindFilesLoop( _In_ CItemBranch* ThisCItem, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files ) {
	ASSERT( ThisCItem->GetType( ) != IT_FILE );
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetFindPattern( ThisCItem->GetPath( ) ) );
	FILEINFO fi;
	FILETIME t;
	while ( b ) {
		b = finder.FindNextFile( );
		if ( finder.IsDots( ) ) {
			continue;//Skip the rest of the block. No point in operating on ourselves!
			}
		if ( finder.IsDirectory( ) ) {
			dirCount++;
			finder.GetLastWriteTime( &t );
			ThisCItem->AddDirectory( finder.GetFilePath( ), finder.GetAttributes( ), finder.altGetFileName( ), t );
			}
		else {
			fileCount++;
			fi.attributes = 0;
			fi.lastWriteTime.dwHighDateTime = 0;
			fi.lastWriteTime.dwLowDateTime = 0;
			fi.length = 0;
			fi.name.Empty( );
			PWSTR namePtr = finder.altGetFileName( );
			if ( namePtr != NULL ) {
				fi.name = namePtr;
				}
			else {
				fi.name = finder.GetFileName( ); //GetString returns a const C-style string pointer, and the compiler bitches if we try to assign it to a PWSTR
				namePtr = const_cast<PWSTR>( fi.name.GetString( ) );
				}
			fi.attributes = finder.GetAttributes( );
			if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
				//fi.length = finder.GetCompressedLength( namePtr );
				fi.length = finder.GetCompressedLength( );
				}
			else {

#ifdef _DEBUG
				if ( !( finder.GetLength( ) == finder.GetCompressedLength( ) ) ) {
					static_assert( sizeof( unsigned long long ) == 8, "bad format specifiers!" );
					TRACE( _T( "GetLength: %llu != GetCompressedLength: %llu !!! Path: %s\r\n" ), finder.GetLength( ), finder.GetCompressedLength( ), finder.GetFilePath( ) );
					}
#endif
				fi.length = finder.GetLength( );//temp
				}
			finder.GetLastWriteTime( &fi.lastWriteTime ); // (We don't use GetLastWriteTime(CTime&) here, because, if the file has an invalid timestamp, that function would ASSERT and throw an Exception.)
			files.emplace_back( std::move( fi ) );
			}
		if ( ( ( fileCount + dirCount ) > 0 ) && ( ( ( fileCount + dirCount ) % 1000 ) == 0 ) /*&& ( !didUpdateHack ) */ ) {
			ThisCItem->DriveVisualUpdateDuringWork( );
			//didUpdateHack = true;
			}
		}
	}

void readJobNotDoneWork( _In_ CItemBranch* ThisCItem, _In_ const std::uint64_t start ) {
	LONGLONG dirCount  = 0;
	LONGLONG fileCount = 0;
	std::vector<FILEINFO> vecFiles;
	CItemBranch* filesFolder = NULL;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( ThisCItem, dirCount, fileCount, vecFiles );

	if ( dirCount > 0 && fileCount > 1 ) {
		filesFolder = new CItemBranch { IT_FILESFOLDER, _T( "<Files>" ), 0, zeroInitFILETIME( ), 0, false };
		filesFolder->SetReadJobDone( false );
		ThisCItem->AddChild( filesFolder );
		}
	else if ( fileCount > 0 ) {
		filesFolder = ThisCItem;
		}
	if ( filesFolder != NULL ) {
		for ( const auto& aFile : vecFiles ) {
			filesFolder->AddChild( new CItemBranch { IT_FILE, aFile.name, aFile.length, aFile.lastWriteTime, aFile.attributes, true } );
			}
		filesFolder->UpwardAddFiles( fileCount );
		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder->SortAndSetDone( );
			}
		}
	ThisCItem->UpwardAddSubdirs( dirCount );
	ThisCItem->UpwardAddReadJobs( -1 );
	ThisCItem->SetReadJobDone( true );
	auto TicksWorked = GetTickCount64( ) - start;
	ThisCItem->AddTicksWorked( TicksWorked );
	}

#ifdef _DEBUG
int CItemBranch::LongestName = 0;
#endif


void StillHaveTimeToWork( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t start ) {
	bool timeExpired = true;
	while ( ( GetTickCount64( ) - start < ticks ) && (!ThisCItem->IsDone( ) ) ) {
		unsigned long long minticks = UINT_MAX;
		CItemBranch* minchild = NULL;
		
		
		//Interestingly, the old-style, non-ranged loop is faster here ( in debug mode )
		auto sizeOf_m_children = ThisCItem->m_children.size( );
		for ( size_t i = 0; i < sizeOf_m_children; ++i ) {
			if ( !ThisCItem->m_children.at( i )->IsDone( ) ) {
				minticks = ThisCItem->m_children[ i ]->GetTicksWorked( );
				minchild = ThisCItem->m_children[ i ];
				}
			}

		if ( minchild == NULL ) {
			//Either no children ( a file ) or all children are done!
			ThisCItem->SortAndSetDone( );
			ASSERT( ( ThisCItem->m_children.size( ) == 0 ) || ( ThisCItem->IsDone( ) ) );
			timeExpired = false;
			break;
			}
		auto tickssofar = GetTickCount64( ) - start;
		if ( ticks > tickssofar ) {
			if ( !minchild->IsDone( ) ) {
				DoSomeWork( minchild, ticks - tickssofar );
				}
			}
		}
	if ( timeExpired ) {
		ThisCItem->DriveVisualUpdateDuringWork( );
		}
	}

void DoSomeWork( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks ) {
	auto start = GetTickCount64( );
	auto typeOfThisItem = ThisCItem->m_type;
	if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY ) {
		if ( !ThisCItem->IsReadJobDone( ) ) {
			readJobNotDoneWork( ThisCItem, start );
			}
		if ( GetTickCount64( ) - start > ticks ) {
			return;
			}
		}
	if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY || typeOfThisItem == IT_MYCOMPUTER ) {
		ASSERT( ThisCItem->IsReadJobDone( ) );
		if ( ThisCItem->GetChildrenCount( ) == 0 ) {
			ASSERT( !ThisCItem->IsDone( ) );
			ThisCItem->SortAndSetDone( );
			return;
			}
		auto startChildren = GetTickCount64( );
		StillHaveTimeToWork( ThisCItem, ticks, start );
		auto TicksWorked = GetTickCount64( ) - startChildren;
		ThisCItem->AddTicksWorked( TicksWorked );
		}
	else {
		ASSERT( !ThisCItem->IsDone( ) );
		ThisCItem->SortAndSetDone( );
		}
	}

CString GetFindPattern( _In_ const CString path ) {
	if ( path.Right( 1 ) != _T( '\\' ) ) {
		return CString( path + _T( "\\*.*" ) );
		}
	else {
		return CString( path + _T( "*.*" ) );//Yeah, if you're wondering, `*.*` works for files WITHOUT extensions.
		}
	}

void AddFileExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) {
	ASSERT( extensionRecords.size( ) == 0 );
	extensionRecords.reserve( extensionMap.size( ) + 1 );
	for ( auto& anExt : extensionMap ) {
		extensionRecords.emplace_back( std::move( anExt.second ) );
		}
	}


CItemBranch::CItemBranch( ITEMTYPE type, _In_z_ PCTSTR name, std::uint64_t size, FILETIME time, DWORD attr, bool done, bool dontFollow ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( size ), m_files( 0 ), m_subdirs( 0 ), m_ticksWorked( 0 ), m_readJobs( 0 ), m_rect( 0, 0, 0, 0 ), m_lastChange( time ), m_done ( done ) {
	auto thisItem_type = m_type;
	if ( thisItem_type == IT_FILE || dontFollow || thisItem_type == IT_MYCOMPUTER ) {
		ASSERT( TmiIsLeaf( ) /*|| IsRootItem( )*/ || dontFollow );
		//UpwardAddReadJobs( -1 );
		m_readJobDone = true;
		}
	else if ( thisItem_type == IT_DIRECTORY ) {
		UpwardAddReadJobs( 1 );
		m_readJobDone = false;
		}

	SetAttributes( attr );
#ifdef _DEBUG
	if ( m_name.GetLength( ) > LongestName ) {
		LongestName = m_name.GetLength( );
		ASSERT( LongestName < ( MAX_PATH + 1 ) );
		TRACE( _T( "Found new longest name! (%i characters), name: %s\r\n" ), LongestName, m_name );
		}
#endif
	m_name.FreeExtra( );
	}

CItemBranch::~CItemBranch( ) {
	for ( auto& aChild : m_children ) {
		if ( aChild != NULL ) {
			delete aChild;
			}
		m_children.clear( );
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
		if ( IsDone( ) ) {
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
		auto i = GetIndent( ) % Options->GetTreelistColorCount( );
		return std::move( Options->GetTreelistColor( i ) );
		}
	ASSERT( false );//should never ever happen, but just in case, we'll generate a random color.
	return DWORD( rand( ) );
	}

#endif

CString CItemBranch::GetTextCOL_PERCENTAGE( ) const {
	if ( GetOptions( )->IsShowTimeSpent( ) && MustShowReadJobs( ) /* || IsRootItem( ) */ ) {
		const size_t bufSize = 24;
		wchar_t buffer[ bufSize ] = { 0 };
		if ( IsDone( ) ) {
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
			//BAD_FMT
			buffer[ 0 ] = 'B';
			buffer[ 1 ] = 'A';
			buffer[ 2 ] = 'D';
			buffer[ 3 ] = '_';
			buffer[ 4 ] = 'F';
			buffer[ 5 ] = 'M';
			buffer[ 6 ] = 'T';
			buffer[ 7 ] = 0;
			}
		return buffer;
		}
	
#ifdef _DEBUG
	CString s;
	s.Format( _T( "%s%%" ), FormatDouble( GetFraction( ) * DOUBLE( 100 ) ).GetString( ) );
#endif
	const size_t bufSize = 12;

	wchar_t buffer[ bufSize ] = { 0 };
	auto res = CStyle_FormatDouble( GetFraction( ) * DOUBLE( 100 ), buffer, bufSize );
	if ( !SUCCEEDED( res ) ) {
		//BAD_FMT
		buffer[ 0 ] = 'B';
		buffer[ 1 ] = 'A';
		buffer[ 2 ] = 'D';
		buffer[ 3 ] = '_';
		buffer[ 4 ] = 'F';
		buffer[ 5 ] = 'M';
		buffer[ 6 ] = 'T';
		buffer[ 7 ] = 0;
		return buffer;
		}

	wchar_t percentage[ 2 ] = { '%', 0 };
	res = StringCchCat( buffer, bufSize, percentage );
	if ( !SUCCEEDED( res ) ) {
		//BAD_FMT
		buffer[ 0 ] = 'B';
		buffer[ 1 ] = 'A';
		buffer[ 2 ] = 'D';
		buffer[ 3 ] = '_';
		buffer[ 4 ] = 'F';
		buffer[ 5 ] = 'M';
		buffer[ 6 ] = 'T';
		buffer[ 7 ] = 0;
		return buffer;
		}
#ifdef _DEBUG
	ASSERT( s.Compare( buffer ) == 0 );
#endif
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
	//auto typeOfItem = GetType( );
#ifdef C_STYLE_STRINGS
		wchar_t psz_formatted_datetime[ 73 ] = { 0 };
		auto res = CStyle_FormatFileTime( m_lastChange, psz_formatted_datetime, 73 );
		if ( res == 0 ) {
			return psz_formatted_datetime;
			}
		else {
			return _T( "BAD_FMT" );
			}
#else
		return FormatFileTime( m_lastChange );//FIXME
#endif
	return CString("");
	}

CString CItemBranch::GetTextCOL_ATTRIBUTES( ) const {
	auto typeOfItem = m_type;
	if ( typeOfItem != IT_FILESFOLDER && typeOfItem != IT_MYCOMPUTER ) {
#ifdef C_STYLE_STRINGS
		wchar_t attributes[ 8 ] = { 0 };
		auto res = CStyle_FormatAttributes( GetAttributes( ), attributes, 6 );
		if ( res == 0 ) {
			ASSERT( FormatAttributes( GetAttributes( ) ).Compare( attributes ) == 0 );
			return attributes;
			}
		return _T( "BAD_FMT" );
#else
		return FormatAttributes( GetAttributes( ) );
#endif
		}
	return CString("");
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
			return CString( " " );
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
		return GetApp( )->AltEncryptionColor( );
		}
	return CTreeListItem::GetItemTextColor( ); // The rest is not colored
	}

INT CItemBranch::CompareName( _In_ const CItemBranch* other ) const {
	if ( m_type == IT_DRIVE ) {
		ASSERT( other->m_type == IT_DRIVE );
		return signum( GetPath( ).CompareNoCase( other->GetPath( ) ) );
		}	
	return signum( m_name.CompareNoCase( other->m_name ) );
	}

INT CItemBranch::CompareLastChange( _In_ const CItemBranch* other ) const {
	if ( m_lastChange < other->m_lastChange ) {
		return -1;
		}
	else if ( ( m_lastChange.dwLowDateTime == other->m_lastChange.dwLowDateTime ) && ( m_lastChange.dwHighDateTime  == other->m_lastChange.dwHighDateTime ) ) {
		return 0;
		}
	return 1;
	}


INT CItemBranch::CompareSibling( _In_ const CTreeListItem* tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
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
	if ( type_theItem == IT_MYCOMPUTER ) {
		return GetMyImageList( )->GetMyComputerImage( );
		}
	else if ( type_theItem == IT_FILESFOLDER ) {
		return GetMyImageList( )->GetFilesFolderImage( );
		}
	auto path = GetPath();
	auto MyImageList = GetMyImageList( );
	if ( type_theItem == IT_DIRECTORY && GetApp( )->IsMountPoint( path ) ) {
		return MyImageList->GetMountPointImage( );
		}
	else if ( type_theItem == IT_DIRECTORY && GetApp( )->IsJunctionPoint( path, GetAttributes( ) ) ) {
		return MyImageList->GetJunctionImage( );
		}
	return MyImageList->GetFileImage( path );
	}
#endif

_Must_inspect_result_ CItemBranch* CItemBranch::FindCommonAncestor( _In_ const CItemBranch* item1, _In_ const CItemBranch* item2 ) {
	auto parent = item1;
	while ( !parent->IsAncestorOf( item2 ) ) {
		parent = parent->GetParent( );
		}
	ASSERT( parent != NULL );
	return const_cast< CItemBranch* >( parent );
	}

bool CItemBranch::IsAncestorOf( _In_ const CItemBranch* thisItem ) const {
	auto p = thisItem;
	while ( p != NULL ) {
		if ( p == this ) {
			break;
			}
		p = p->GetParent( );
		}
	return ( p != NULL );
	}

LONGLONG CItemBranch::GetProgressRange( ) const {
	switch ( m_type )
	{
		case IT_MYCOMPUTER:
			return GetProgressRangeMyComputer( );
		case IT_DRIVE:
			return GetProgressRangeDrive( );
		case IT_DIRECTORY:
		case IT_FILESFOLDER:
		case IT_FILE:
			return 0;
		default:
			ASSERT( false );
			return 0;
	}
	}

LONGLONG CItemBranch::GetProgressPos( ) const {
	switch ( m_type )
	{
		case IT_MYCOMPUTER:
			return GetProgressPosMyComputer( );
		case IT_DRIVE:
			return m_size;
		case IT_DIRECTORY:
			return m_files + m_subdirs;
		case IT_FILE:
		case IT_FILESFOLDER:
		default:
			ASSERT( false );
			return 0;
	}
	}

_Success_( return != NULL ) CItemBranch* CItemBranch::GetChildGuaranteedValid( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const {
	if ( m_children.at( i ) != NULL ) {
		return m_children[ i ];
		}
	AfxCheckMemory( );//freak out
	ASSERT( false );
	MessageBox( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	throw std::logic_error( "GetChildGuaranteedValid couldn't find a valid child! This should never happen!" );
	std::terminate( );
	}

void CItemBranch::AddChild( _In_ CItemBranch* child ) {
	ASSERT( !IsDone( ) );// SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItemBranch::GetFraction() could ASSERT.
	UpwardAddSize         ( child->m_size );
	
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
		TreeListControl->OnChildAdded( this, child, IsDone( ) );
		}
	ASSERT( TreeListControl != NULL );
	}

void CItemBranch::UpwardAddSubdirs( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t dirCount ) {
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
	//if ( count == 0 ) {
	//	return;
	//	}
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
	auto path        = UpwardGetPathWithoutBackslash( );
	auto typeOfThisItem = m_type;
	if ( ( typeOfThisItem == IT_DRIVE ) || ( typeOfThisItem == IT_FILESFOLDER ) ) {
		path += _T( "\\" );
		}
	return path;
	}

CString CItemBranch::GetFolderPath( ) const {
	/*
	  Returns the path for "Explorer here" or "Command Prompt here"
	*/
	auto typeOfThisItem = m_type;
	if ( typeOfThisItem == IT_MYCOMPUTER ) {
		return GetParseNameOfMyComputer( );
		}
	auto path = GetPath( );
	if ( typeOfThisItem == IT_FILE ) {
		auto i = path.ReverseFind( _T( '\\' ) );
		ASSERT( i != -1 );
		return path.Left( i + 1 );
		}
	return path;
	}

CString CItemBranch::UpwardGetPathWithoutBackslash( ) const {
	if ( m_type == IT_DRIVE ) {
		return PathFromVolumeName( m_name );
		}
	CString path;
	path.Preallocate( MAX_PATH );
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		path = myParent->UpwardGetPathWithoutBackslash( );
		}
	switch ( m_type )
	{
		case IT_DIRECTORY:
			if ( !path.IsEmpty( ) ) {
				path += _T( "\\" );
				}
			return ( path + m_name );
		case IT_FILE:
			return ( path + _T( "\\" ) + m_name );
		case IT_MYCOMPUTER:
		case IT_FILESFOLDER:
			return path;
		default:
			ASSERT( false );
			return path;
	}
	}

PWSTR CItemBranch::CStyle_GetExtensionStrPtr( ) const {
	ASSERT( m_type == IT_FILE );
	ASSERT( m_name.GetLength( ) < ( MAX_PATH + 1 ) );
	PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
	ASSERT( resultPtrStr != '\0' );
	return resultPtrStr;
	}

_Success_( SUCCEEDED( return ) ) HRESULT CItemBranch::CStyle_GetExtension( _Out_writes_z_( strSize ) PWSTR psz_extension, const size_t strSize ) const {
	if ( m_type == IT_FILE ) {
		PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
		ASSERT( resultPtrStr != '\0' );
		if ( resultPtrStr != '\0' ) {
			auto res = StringCchCopy( psz_extension, strSize, resultPtrStr );
			return res;
			}
		psz_extension[ 0 ] = 0;
		return ERROR_FUNCTION_FAILED;
		}
	ASSERT( false );
	psz_extension[ 0 ] = 0;
	return ERROR_FUNCTION_FAILED;
	}

CString CItemBranch::GetExtension( ) const {
	//INSIDE this function, CAfxStringMgr::Allocate	(f:\dd\vctools\vc7libs\ship\atlmfc\src\mfc\strcore.cpp:141) DOMINATES execution!!//TODO: FIXME: BUGBUG!
	switch ( m_type )
	{
		case IT_FILE:
			{
				PWSTR resultPtrStr = PathFindExtension( m_name.GetString( ) );
				ASSERT( resultPtrStr != '\0' );
				if ( resultPtrStr != '\0' ) {
					return resultPtrStr;
					}
				INT i = m_name.ReverseFind( _T( '.' ) );
				
				if ( i == -1 ) {
					return _T( "." );
					}
				else {
					return m_name.Mid( i ).MakeLower( );//slower part?
					}
			}
		default:
			ASSERT( false );
			return CString( "" );
	}
	}

void CItemBranch::SortAndSetDone( ) {
	ASSERT( !IsDone( ) );
	if ( IsDone( ) ) {
		return;
		}
	qsort( m_children.data( ), static_cast< size_t >( m_children.size( ) ), sizeof( CItemBranch *), &_compareBySize );
	m_children.shrink_to_fit( );
	m_rect.bottom = NULL;
	m_rect.left   = NULL;
	m_rect.right  = NULL;
	m_rect.top    = NULL;
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

_Success_( return != NULL ) _Must_inspect_result_ CItemBranch* CItemBranch::FindDirectoryByPath( _In_ const CString& path ) const {
	ASSERT( path != _T( "" ) );
	auto myPath = GetPath( );
	myPath.MakeLower( );

	INT i = 0;
	auto myPath_GetLength = myPath.GetLength( );
	auto path_GetLength = path.GetLength( );
	while ( i < myPath_GetLength && i < path_GetLength && myPath[ i ] == path[ i ] ) {
		i++;
		}

	if ( i < myPath_GetLength ) {
		return NULL;
		}

	if ( i >= path_GetLength ) {
		ASSERT( myPath == path );
		return const_cast<CItemBranch*>( this );
		}

	for ( auto& Child : m_children ) {
		auto item = Child->FindDirectoryByPath( path );
		if ( item != NULL ) {
			return item;
			}
		}
	return NULL;
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

void CItemBranch::stdRecurseCollectExtensionData( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) {
	auto typeOfItem = GetType( );
	if ( typeOfItem == IT_FILE) {
		wchar_t extensionPsz[ MAX_PATH ] = { 0 };
		auto res = CStyle_GetExtension( extensionPsz, MAX_PATH );
#ifdef _DEBUG
		auto ext = GetExtension( );
		if ( SUCCEEDED( res ) ) {
			ASSERT( ext.Compare( extensionPsz ) == 0 );
			}
		else {
			ASSERT( false );
			}
#endif
		if ( SUCCEEDED( res ) ) {
			if ( extensionMap[ extensionPsz ].files == 0 ) {
				++( extensionMap[ extensionPsz ].files );
				extensionMap[ extensionPsz ].bytes += m_size;
				extensionMap[ extensionPsz ].ext = extensionPsz;
				}
			else {
				++( extensionMap[ extensionPsz ].files );
				extensionMap[ extensionPsz ].bytes += m_size;
				}
			}
		else {
			//use an underscore to avoid name conflict with _DEBUG build
			auto ext_ = std::wstring( GetExtension( ).GetString( ) );
			if ( extensionMap[ ext_ ].files == 0 ) {
				++( extensionMap[ ext_ ].files );
				extensionMap[ ext_ ].bytes += m_size;
				extensionMap[ ext_ ].ext = ext_;
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

INT __cdecl CItemBranch::_compareBySize( _In_ const void* p1, _In_ const void* p2 ) {
	const auto size1 = ( *( const CItemBranch ** ) p1 )->m_size;
	const auto size2 = ( *( const CItemBranch ** ) p2 )->m_size;
	return signum( std::int64_t( size2 ) - std::int64_t( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}

LONGLONG CItemBranch::GetProgressRangeMyComputer( ) const {
	ASSERT( m_type == IT_MYCOMPUTER );
	LONGLONG range = 0;
	for ( auto& child : m_children ) {
		range += child->GetProgressRangeDrive( );
		}
	return range;
	}

LONGLONG CItemBranch::GetProgressPosMyComputer( ) const {
	ASSERT( m_type == IT_MYCOMPUTER );
	LONGLONG pos = 0;
	for ( const auto& child : m_children ) {
		pos += child->m_size;
		}
	return pos;
	}

_Ret_range_( 0, INT64_MAX ) std::uint64_t CItemBranch::GetProgressRangeDrive( ) const {
	auto path    = GetPath( );
	std::uint64_t total = 0;
	std::uint64_t freeSp = 0;

	MyGetDiskFreeSpace( path, total, freeSp );

	return ( total - freeSp );
	}

COLORREF CItemBranch::GetGraphColor( ) const {
	switch ( m_type )
	{
		case IT_FILE:
			return ( GetDocument( )->GetCushionColor( CStyle_GetExtensionStrPtr( ) ) );
		case IT_DIRECTORY:
			return RGB( 254, 254, 254 );
		case IT_FILESFOLDER:
			return RGB( 254, 254, 254 );
		default:
			return RGB( 0, 0, 0 );
	}
	}

bool CItemBranch::MustShowReadJobs( ) const {
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		return !myParent->IsDone( );
		}
	return !IsDone( );
	}

//CString thisFilePath, DWORD& thisFileAttributes, CString thisFileName, FILETIME& thisFileTime
void CItemBranch::AddDirectory( CString thisFilePath, DWORD thisFileAttributes, CString thisFileName, FILETIME& thisFileTime ) {
	auto thisApp      = GetApp( );
	auto thisOptions  = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow = ( thisApp->IsJunctionPoint( thisFilePath, thisFileAttributes ) && !thisOptions->m_followJunctionPoints ) || ( thisApp->IsMountPoint( thisFilePath ) && !thisOptions->m_followMountPoints );
	
	AddChild( new CItemBranch{ IT_DIRECTORY, thisFileName, 0, thisFileTime, thisFileAttributes, false, dontFollow } );
	}

void CItemBranch::DriveVisualUpdateDuringWork( ) {
	//TRACE( _T( "Exceeding number of ticks!\r\npumping messages - this is a dirty hack to ensure responsiveness while single-threaded.\r\n" ) );
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
