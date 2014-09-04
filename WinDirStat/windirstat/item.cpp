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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	CString GetFreeSpaceItemName( )  { return LoadString( IDS_FREESPACE_ITEM ); }
	CString GetUnknownItemName  ( )  { return LoadString( IDS_UNKNOWN_ITEM   ); }
	const unsigned char INVALID_m_attributes = 0x80; // File attribute packing
	}

CItemBranch::CItemBranch( ITEMTYPE type, _In_z_ LPCTSTR name, bool dontFollow, bool isRootItem ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( 0 ), m_files( 0 ), m_subdirs( 0 ), m_done( false ), m_ticksWorked( 0 ), m_readJobs( 0 ), m_attributes( 0 ), m_rect( 0, 0, 0, 0 ), m_isRootItem( isRootItem ) {
	auto thisItem_type = GetType( );
	if ( thisItem_type == IT_FILE || dontFollow || thisItem_type == IT_FREESPACE || thisItem_type == IT_UNKNOWN || thisItem_type == IT_MYCOMPUTER ) {
		SetReadJobDone( true );
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_DRIVE || thisItem_type == IT_FILESFOLDER ) {
		SetReadJobDone( false );
		}
	if ( thisItem_type == IT_DRIVE ) {
		m_name = FormatVolumeNameOfRootPath( m_name );
		}
	zeroDate( m_lastChange );
	}

CItemBranch::CItemBranch( ITEMTYPE type, _In_z_ LPCTSTR name, LONGLONG mySize, bool done, bool isRootItem ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( mySize ), m_files( 0 ), m_subdirs( 0 ), m_done( done ), m_ticksWorked( 0 ), m_readJobs( 0 ), m_attributes( 0 ), m_rect( 0, 0, 0, 0 ), m_isRootItem( isRootItem ) {
	auto thisItem_type = GetType( );
	if ( thisItem_type == IT_FILE || false || thisItem_type == IT_FREESPACE || thisItem_type == IT_UNKNOWN || thisItem_type == IT_MYCOMPUTER ) {
		SetReadJobDone( true );
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_DRIVE || thisItem_type == IT_FILESFOLDER ) {
		SetReadJobDone( false );
		}
	if ( thisItem_type == IT_DRIVE ) {
		m_name = FormatVolumeNameOfRootPath( m_name );
		}
	zeroDate( m_lastChange );
	}

CItemBranch::CItemBranch( ITEMTYPE type, _In_z_ LPCTSTR name, std::uint64_t size, FILETIME time, DWORD attr, bool done, bool isRootItem ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( size ), m_files( 0 ), m_subdirs( 0 ), m_ticksWorked( 0 ), m_readJobs( 0 ), m_rect( 0, 0, 0, 0 ), m_lastChange( time ), m_done ( done ), m_isRootItem( isRootItem ) {
	auto thisItem_type = GetType( );
	ASSERT( thisItem_type != IT_DRIVE );
	if ( thisItem_type == IT_FILE || thisItem_type == IT_FREESPACE || thisItem_type == IT_UNKNOWN || thisItem_type == IT_MYCOMPUTER ) {
		SetReadJobDone( true );
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_FILESFOLDER ) {
		SetReadJobDone( false );
		}
	
	SetAttributes( attr );
	}

CItemBranch::~CItemBranch( ) {
	auto childrenSize = m_children.size( );
	//ASSERT( m_children_v.size( ) == childrenSize );
	for ( size_t i = 0; i < childrenSize; ++i ) {
		ASSERT( m_children polyAt( i ) != NULL );
		//ASSERT( m_children_v.at( i ) != NULL );
		if ( m_children polyAt( i ) != NULL ) {
			delete m_children polyAt( i );
			m_children polyAt( i ) = NULL;
			//m_children_v.at( i ) = NULL ;
			}
		}
	}

#ifdef ITEM_DRAW_SUBITEM
bool CItem::DrawSubitem( _In_ _In_range_( 0, INT32_MAX ) const INT subitem, _In_ CDC* pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );
	
	if ( subitem == COL_NAME ) {
		return CTreeListItem::DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
		}
	if ( subitem != COL_SUBTREEPERCENTAGE ) {
		return false;
		}
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
#endif

CString CItemBranch::GetTextCOL_SUBTREEPERCENTAGE( ) const {
	if ( IsDone( ) ) {
		//ASSERT( m_readJobs == 0 );//s = "ok";
		return CString( "" );
		}
	else {
		if ( m_readJobs == 1 ) {
			return CString( "[1 Read Job]" );
			}
		else {
			//CString s;
			std::wstring a;
			a += L"[";
			a += FormatCount( m_readJobs );
			a += L" Read Jobs]";
			//s.FormatMessage( IDS_sREADJOBS, a.c_str( ) );//"[%1!s! Read Jobs]"
			return a.c_str( );
			}
		}
	}

CString CItemBranch::GetTextCOL_PERCENTAGE( ) const {
	CString s;
	if ( GetOptions( )->IsShowTimeSpent( ) && MustShowReadJobs( ) || IsRootItem( ) ) {
		//s.Format( _T( "[%s s]" ), FormatMilliseconds( GetTicksWorked( ) ).GetString( ) );
		//return s;
		return CString( "" );
		}
	s.Format( _T( "%s%%" ), FormatDouble( GetFraction( ) * 100 ).GetString( ) );
	return s;
	}


bool CItemBranch::IsNotFileFreeSpaceOrUnknown( ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		ASSERT( !TmiIsLeaf( ) );
		return true;
		}
	return false;
	}

CString CItemBranch::GetTextCOL_ITEMS( ) const {
	if ( IsNotFileFreeSpaceOrUnknown( ) ) {
		return FormatCount( GetItemsCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_FILES( ) const {
	if ( IsNotFileFreeSpaceOrUnknown( ) ) {
		return FormatCount( GetFilesCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_SUBDIRS( ) const { 
	if ( IsNotFileFreeSpaceOrUnknown( ) ) {
		return FormatCount( GetSubdirsCount( ) );
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_LASTCHANGE( ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
#ifdef C_STYLE_STRINGS
		wchar_t psz_formatted_datetime[ 73 ] = { 0 };
		CStyle_FormatFileTime( m_lastChange, psz_formatted_datetime, 73 );
		return psz_formatted_datetime;
#else
		return FormatFileTime( m_lastChange );//FIXME
#endif
		}
	return CString("");
	}

CString CItemBranch::GetTextCOL_ATTRIBUTES( ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FREESPACE && typeOfItem != IT_FILESFOLDER && typeOfItem != IT_UNKNOWN && typeOfItem != IT_MYCOMPUTER ) {
#ifdef C_STYLE_STRINGS
		wchar_t attributes[ 8 ] = { 0 };
		auto res = CStyle_FormatAttributes( GetAttributes( ), attributes, 6 );
		if ( res == 0 ) {
			return attributes;
			}
		return _T( "BAD_FMT" );
#else
		return FormatAttributes( GetAttributes( ) );
#endif
		}
	return CString("");
	}


CString CItemBranch::GetText( _In_ const INT subitem ) const {
	switch (subitem)
	{
		case COL_NAME:
			return m_name;
		case COL_SUBTREEPERCENTAGE:
			return GetTextCOL_SUBTREEPERCENTAGE( );
		case COL_PERCENTAGE:
			return GetTextCOL_PERCENTAGE( );
		case COL_SUBTREETOTAL:
			return FormatBytes( GetSize( ) );
		case COL_ITEMS:
			return GetTextCOL_ITEMS( );
		case COL_FILES:
			return GetTextCOL_FILES( );
		case COL_SUBDIRS:
			return GetTextCOL_SUBDIRS( );
		case COL_LASTCHANGE:
			return GetTextCOL_LASTCHANGE( );
		case COL_ATTRIBUTES:
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
		//return GetApp( )->AltColor( );
		return RGB( 0x00, 0x00, 0xFF );
		}
	else if ( attr & FILE_ATTRIBUTE_ENCRYPTED ) {
		return GetApp( )->AltEncryptionColor( );
		}
	return CTreeListItem::GetItemTextColor( ); // The rest is not colored
	}

INT CItemBranch::CompareName( _In_ const CItemBranch* other ) const {
	if ( GetType( ) == IT_DRIVE ) {
		ASSERT( other->GetType( ) == IT_DRIVE );
		return signum( GetPath( ).CompareNoCase( other->GetPath( ) ) );
		}	
	return signum( m_name.CompareNoCase( other->m_name ) );
	}

INT CItemBranch::CompareSubTreePercentage( _In_ const CItemBranch* other ) const {
	if ( MustShowReadJobs( ) ) {
		return signum( m_readJobs - other->m_readJobs );//TODO BUGBUG FIXME: pointless comparison of unsigned integer with zero!
		}
	return signum( GetFraction( ) - other->GetFraction( ) );
	}

INT CItemBranch::CompareLastChange( _In_ const CItemBranch* other ) const {
	if ( m_lastChange < other->m_lastChange ) {
		return -1;
		}
	else if ( m_lastChange == other->m_lastChange ) {
		return 0;
		}
	return 1;
	}


INT CItemBranch::CompareSibling( _In_ const CTreeListItem *tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
	auto other = static_cast< const CItemBranch * >( tlib );
	switch ( subitem )
	{
		case COL_NAME:
			return CompareName( other );
		case COL_SUBTREEPERCENTAGE:
			return CompareSubTreePercentage( other );
		case COL_PERCENTAGE:
			return signum( GetFraction( )       - other->GetFraction( ) );
		case COL_SUBTREETOTAL:
			return signum( std::int64_t( GetSize( ) ) - std::int64_t( other->GetSize( ) ) );
		case COL_ITEMS:
			return signum( GetItemsCount( )     - other->GetItemsCount( ) );
		case COL_FILES:
			return signum( GetFilesCount( )     - other->GetFilesCount( ) );
		case COL_SUBDIRS:
			return signum( GetSubdirsCount( )   - other->GetSubdirsCount( ) );
		case COL_LASTCHANGE:
			return CompareLastChange( other );
		case COL_ATTRIBUTES:
			return signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
		default:
			ASSERT( false );
			return 666;
	}
	}

_Must_inspect_result_ CTreeListItem *CItemBranch::GetTreeListChild( _In_ _In_range_( 0, INT32_MAX ) const INT i ) const {
	ASSERT( !( m_children.polyEmpty( ) ) && ( i < m_children.polySize( ) ) );
	//ASSERT( m_children polyAt( i ) == m_children_v.at( i ) );
	return m_children polyAt( i );
	}

INT CItemBranch::GetImageToCache( ) const { // (Caching is done in CTreeListItem::m_vi.)
	auto type_theItem = GetType( );
	if ( type_theItem == IT_MYCOMPUTER ) {
		return GetMyImageList( )->GetMyComputerImage( );
		}
	else if ( type_theItem == IT_FILESFOLDER ) {
		return GetMyImageList( )->GetFilesFolderImage( );
		}
	else if ( type_theItem == IT_FREESPACE ) {
		return GetMyImageList( )->GetFreeSpaceImage( );
		}
	else if ( type_theItem == IT_UNKNOWN ) {
		return GetMyImageList( )->GetUnknownImage( );
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

void CItemBranch::DrawAdditionalState( _In_ CDC* pdc, _In_ const CRect& rcLabel ) const {
	ASSERT_VALID( pdc );
	auto thisDocument = GetDocument( );
	if ( !IsRootItem( ) && this == thisDocument->GetZoomItem( ) ) {
		auto rc = rcLabel;
		rc.InflateRect( 1, 0 );
		rc.bottom++;

		CSelectStockObject sobrush { pdc, NULL_BRUSH };
		CPen pen                   { PS_SOLID, 2, thisDocument->GetZoomColor( ) };
		CSelectObject sopen        { pdc, &pen };

		pdc->Rectangle( rc );
		}
	}

_Must_inspect_result_ CItemBranch* CItemBranch::FindCommonAncestor( _In_ CItemBranch* item1, _In_ const CItemBranch* item2 ) {
	auto parent = item1;
	while ( !parent->IsAncestorOf( item2 ) ) {
		parent = parent->GetParent( );
		}
	ASSERT( parent != NULL );
	return parent;
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
	switch ( GetType( ) )
	{
		case IT_MYCOMPUTER:
			return GetProgressRangeMyComputer( );
		case IT_DRIVE:
			return GetProgressRangeDrive( );
		case IT_DIRECTORY:
		case IT_FILESFOLDER:
		case IT_FILE:
			return 0;
		case IT_FREESPACE:
		case IT_UNKNOWN:
		default:
			ASSERT( false );
			return 0;
	}
	}

LONGLONG CItemBranch::GetProgressPos( ) const {
	switch ( GetType( ) )
	{
		case IT_MYCOMPUTER:
			return GetProgressPosMyComputer( );
		case IT_DRIVE:
			return GetProgressPosDrive( );
		case IT_DIRECTORY:
			return m_files + m_subdirs;
		case IT_FILE:
		case IT_FILESFOLDER:
		case IT_FREESPACE:
		case IT_UNKNOWN:
		default:
			ASSERT( false );
			return 0;
	}
	}

_Must_inspect_result_ const CItemBranch *CItemBranch::UpwardGetRoot( ) const {
	auto myParent = GetParent( );
	if ( myParent == NULL ) {
		return this;
		}
	return myParent->UpwardGetRoot( );
	}

void CItemBranch::UpdateLastChange( ) {
	zeroDate( m_lastChange );
	auto typeOf_thisItem = GetType( );

	if ( typeOf_thisItem == IT_DIRECTORY || typeOf_thisItem == IT_FILE ) {
		auto path = GetPath( );
		auto i = path.ReverseFind( _T( '\\' ) );
		auto basename = path.Mid( i + 1 );
		CString pattern;
		pattern.Format( _T( "%s\\..\\%s" ), path.GetString( ), basename.GetString( ) );
		CFileFindWDS finder;
		BOOL b = finder.FindFile( pattern );
		if ( !b ) {
			return; // no chance
			}
		finder.FindNextFile( );
		finder.GetLastWriteTime( &m_lastChange );
		SetAttributes( finder.GetAttributes( ) );
		}
	}

_Success_( return != NULL ) CItemBranch* CItemBranch::GetChildGuaranteedValid( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i ) const {
	ASSERT( !( m_children.polyEmpty( ) ) && ( i < m_children.polySize( ) ) );
	if ( m_children.at( i ) != NULL ) {
		//TRACE( _T( "%i m_children: %s, m_children_v: %s\r\n" ), i, m_children polyAt( i )->GetName( ), m_children_v.at( i )->GetName( ) );
		//ASSERT( m_children polyAt( i ) == m_children_v.at( i ) );
		return m_children[ i ];
		}
	else {
		AfxCheckMemory( );//freak out
		ASSERT( false );
		MessageBox( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
		throw 666;
		std::terminate( );
		}

	MessageBox( NULL, _T( "GetChildGuaranteedValid couldn't find a valid child! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	throw 666;
	std::terminate( );
	}

INT_PTR CItemBranch::FindChildIndex( _In_ const CItemBranch* child ) const {
	auto childCount = GetChildrenCount( );	
	for ( INT i = 0; i < childCount; i++ ) {
		if ( child == m_children polyAt( i ) ) {
			//ASSERT( child == m_children_v.at( i ) );
			return i;
			}
		}
	ASSERT( false );
	return childCount;
	}

void CItemBranch::AddChild( _In_ CItemBranch* child ) {
	ASSERT( !IsDone( ) );// SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItemBranch::GetFraction() could ASSERT.
	UpwardAddSize         ( child->GetSize( ) );
	UpwardAddReadJobs     ( child->GetReadJobs( ) );
	UpwardUpdateLastChange( child->GetLastChange( ) );
	m_children.polyAdd( child );
	//m_children_v.push_back( child );
	//ASSERT( m_children polyAt( m_children.GetSize( ) - 1 ) == m_children_v.at( m_children_v.size( ) - 1 ) );

	child->SetParent( this );
	ASSERT( child->GetParent( ) == this );
	ASSERT( !( child->IsRootItem( ) ) );
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnChildAdded( this, child, IsDone( ) );
		}
	ASSERT( TreeListControl != NULL );
	}

void CItemBranch::RemoveChild(_In_ const INT_PTR i) {
	ASSERT( !( m_children.polyEmpty( ) ) && ( i < m_children.polySize( ) ) );
	//ASSERT( !( m_children_v.empty( ) ) && ( i < m_children_v.size( ) ) );
	if ( i >= 0 && ( i <= ( m_children.polySize( ) - 1 ) ) ) {
			auto child = GetChildGuaranteedValid( i );
		auto TreeListControl = GetTreeListControl( );
		if ( TreeListControl != NULL ) {
			ASSERT( m_children polyAt( i ) != NULL );
			//ASSERT( m_children polyAt( i ) == m_children_v.at( i ) );
			m_children.erase( m_children.begin( ) + i );
			TreeListControl->OnChildRemoved( this, child );
			delete child;
			child = NULL;
			}
		}
	}

void CItemBranch::RemoveAllChildren() {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnRemovingAllChildren( this );
		}
	auto childCount = GetChildrenCount( );
	for ( auto i = 0; i < childCount; i++ ) {
		ASSERT( ( i >= 0 ) && ( i <= GetChildrenCount( ) - 1 ));
		if ( m_children polyAt( i ) != NULL ) {
			delete m_children polyAt( i );
			//ASSERT( m_children polyAt( i ) == m_children_v.at( i ) );
			m_children polyAt( i ) = NULL;
			//m_children_v.at( i ) = NULL;
			}
		}

	m_children.polyClear( );
	//m_children_v.clear( );
	ASSERT( m_children.polyEmpty( ) );
	//ASSERT( m_children_v.empty( ) );
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
	ASSERT( ( bytes >= 0 ) || ( bytes == -std::int64_t( GetSize( ) ) ) );
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
	/*
	  This method increases the last change
	*/
	if ( m_lastChange < t ) {
		m_lastChange = t;
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardUpdateLastChange( t );
			}
		//else `this` may be the root item.
		}
	}


void CItemBranch::UpwardRecalcLastChange() {
	/*
	  This method may also decrease the last change
	*/
	UpdateLastChange( );
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		auto child = GetChildGuaranteedValid( i );
		auto receivedLastChange = child->GetLastChange( );
		if ( m_lastChange < receivedLastChange ) {
			m_lastChange = receivedLastChange;
			}
		}
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		myParent ->UpwardRecalcLastChange( );
		}
	//else `this` may be the root item.
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
		m_attributes = ( unsigned char ) INVALID_m_attributes;
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
		ASSERT( IsRootItem( ) );
		return 1.0;//root item? must be whole!
		}
	auto parentSize = myParent->GetSize( );
	if ( parentSize == 0){
		return 1.0;
		}
	return DOUBLE( GetSize( ) ) / DOUBLE( parentSize );
	}

CString CItemBranch::GetPath( )  const {
	auto path        = UpwardGetPathWithoutBackslash( );
	auto typeOfThisItem = GetType( );
	if ( ( typeOfThisItem == IT_DRIVE ) || ( typeOfThisItem == IT_FILESFOLDER ) ) {
		path += _T( "\\" );
		}
	return path;
	}

bool CItemBranch::HasUncPath( ) const {
	auto path = GetPath( );
	return ( path.GetLength( ) >= 2 && path.Left( 2 ) == _T( "\\\\" ) );
	}

CString CItemBranch::GetFindPattern( ) const {
	auto path = GetPath( );
	if ( path.Right( 1 ) != _T( '\\' ) ) {
		return CString( path + _T( "\\*.*" ) );
		}
	else {
		return CString( path + _T( "*.*" ) );//Yeah, if you're wondering, `*.*` works for files WITHOUT extensions.
		}
	}

CString CItemBranch::GetFolderPath( ) const {
	/*
	  Returns the path for "Explorer here" or "Command Prompt here"
	*/
	auto typeOfThisItem = GetType( );
	if ( typeOfThisItem == IT_MYCOMPUTER ) {
		return GetParseNameOfMyComputer( );
		}
	auto path = GetPath( );
	if ( typeOfThisItem == IT_FILE ) {
		auto i = path.ReverseFind( _T( '\\' ) );
		ASSERT( i != -1 );
		path = path.Left( i + 1 );
		}
	return path;
	}

CString CItemBranch::GetExtension( ) const {
	//INSIDE this function, CAfxStringMgr::Allocate	(f:\dd\vctools\vc7libs\ship\atlmfc\src\mfc\strcore.cpp:141) DOMINATES execution!!//TODO: FIXME: BUGBUG!
	switch ( GetType( ) )
	{
		case IT_FILE:
			{
				LPWSTR resultPtrStr = PathFindExtension( static_cast<LPCTSTR>( m_name ) );
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
		case IT_FREESPACE:
		case IT_UNKNOWN:
			return m_name;

		default:
			ASSERT( false );
			return CString( "" );
	}
	}

void CItemBranch::SetReadJobDone( _In_ const bool done ) {
	if ( !IsReadJobDone( ) && done ) {
		UpwardAddReadJobs( -1 );
		}
	else {
		UpwardAddReadJobs( 1 - m_readJobs );
		}
	m_readJobDone = done;
	}

void CItemBranch::SetDone( ) {
	if ( IsDone() ) {
		return;
		}
	if ( GetType( ) == IT_DRIVE ) {
		//UpdateFreeSpaceItem();
		if ( GetDocument( )->OptionShowUnknown( ) ) {
			const auto unknown = FindUnknownItem( );
			if ( unknown != NULL ) {
				if ( !( unknown->GetType( ) == IT_DIRECTORY ) ) {
					LONGLONG total = 0;
					LONGLONG free  = 0;
					auto thisPath  = GetPath( );
					MyGetDiskFreeSpace( thisPath, total, free );//redundant?

					auto unknownspace = total - GetSize( );

					if ( !GetDocument( )->OptionShowFreeSpace( ) ) {
						unknownspace -= free;
						}

					// For CDs, the GetDiskFreeSpaceEx()-function is not correct.
					if ( ( LONGLONG( unknownspace ) < 0 ) || ( free < 0 ) || ( total < 0 ) ) {
						TRACE( _T( "GetDiskFreeSpace(%s), (unknownspace: %lld), (free: %lld), (total: %lld) incorrect.\r\n" ), thisPath, unknownspace, free, total );
						unknownspace = 0;
						}
					unknown->SetSize( unknownspace );
					UpwardAddSize( unknownspace );
					}
				}
			}
		}
	qsort( m_children.data( ), static_cast< size_t >( m_children.size( ) ), sizeof( CItemBranch *), &_compareBySize );
	m_rect.bottom = NULL;
	m_rect.left   = NULL;
	m_rect.right  = NULL;
	m_rect.top    = NULL;
	m_done = true;
	}

void CItemBranch::FindFilesLoop( _In_ const std::uint64_t ticks, _In_ std::uint64_t start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files ) {
	ASSERT( GetType( ) != IT_FILE );
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetFindPattern( ) );
	bool didUpdateHack = false;
	while ( b ) {
		b = finder.FindNextFile( );
		if ( finder.IsDots( ) ) {
			continue;//Skip the rest of the block. No point in operating on ourselves!
			}
		if ( finder.IsDirectory( ) ) {
			dirCount++;
			AddDirectory( finder );
			}
		else {
			fileCount++;
			FILEINFO fi;
			fi.name = finder.GetFileName( );
			fi.attributes = finder.GetAttributes( );
			if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
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
		if ( ( GetTickCount64( ) - start ) > ticks && ( !didUpdateHack ) ) {
			DriveVisualUpdateDuringWork( );
			didUpdateHack = true;
			}
		}	

	}
void CItemBranch::readJobNotDoneWork( _In_ const std::uint64_t ticks, _In_ std::uint64_t start ) {
	LONGLONG dirCount  = 0;
	LONGLONG fileCount = 0;
	std::vector<FILEINFO> vecFiles;
	CItemBranch* filesFolder = NULL;

	vecFiles.reserve( 50 );//pseudo-arbitrary number

	FindFilesLoop( ticks, start, dirCount, fileCount, vecFiles );

	if ( dirCount > 0 && fileCount > 1 ) {
		filesFolder = new CItemBranch { IT_FILESFOLDER, _T( "<Files>" ) };
		filesFolder->SetReadJobDone( );
		AddChild( filesFolder );
		}
	else if ( fileCount > 0 ) {
		filesFolder = this;
		}
	//ASSERT( filesFolder != NULL );
	if ( filesFolder != NULL ) {
		for ( const auto& aFile : vecFiles ) {
			filesFolder->AddFile( aFile );
			}
		filesFolder->UpwardAddFiles( fileCount );
		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder->SetDone( );
			}
		}
	UpwardAddSubdirs( dirCount );
	SetReadJobDone( );
	AddTicksWorked( GetTickCount64( ) - start );
	}

void CItemBranch::StillHaveTimeToWork( _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks, _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t start ) {
	while ( GetTickCount64( ) - start < ticks ) {
		unsigned long long minticks = UINT_MAX;
		CItemBranch* minchild = NULL;
		auto countOfChildren = GetChildrenCount( );
		for ( INT i = 0; i < countOfChildren; i++ ) {
			auto child = GetChildGuaranteedValid( i );
			if ( child->IsDone( ) ) {
				continue;
				}
			//if ( !( child->IsDone( ) ) ) {
			//	child->DoSomeWork( ticks );
			//	}
			if ( child->GetTicksWorked( ) < minticks ) {
				minticks = child->GetTicksWorked( );
				minchild = child;
				}
			}
		if ( minchild == NULL ) {
			SetDone( );
			break;
			}
		auto tickssofar = GetTickCount64( ) - start;
		if ( ticks > tickssofar ) {
			minchild->DoSomeWork( ticks - tickssofar );
			}
		}
	}

void CItemBranch::DoSomeWork( _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks ) {
	if ( IsDone( ) ) {
		return;
		}

	auto start = GetTickCount64( );
	auto typeOfThisItem = GetType( );
	if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY ) {
		if ( !m_readJobDone ) {
			readJobNotDoneWork( ticks, start );
			}
		if ( GetTickCount64( ) - start > ticks ) {
			if ( typeOfThisItem == IT_DRIVE && IsReadJobDone( ) ) {
				UpdateFreeSpaceItem( );
				}
			return;
			}
		}
	if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY || typeOfThisItem == IT_MYCOMPUTER ) {
		ASSERT( IsReadJobDone( ) );
		if ( GetChildrenCount( ) == 0 ) {
			SetDone( );
			return;
			}
		auto startChildren = GetTickCount64( );
		StillHaveTimeToWork( ticks, start );
		AddTicksWorked( GetTickCount64( ) - startChildren );
		}
	else {
		SetDone( );
		}
	}

//bool CItemBranch::StartRefreshIT_MYCOMPUTER( ) {
//	zeroDate( m_lastChange );
//	auto childCount = GetChildrenCount( );
//	for ( INT i = 0; i < childCount; i++ ) {
//		auto Child = GetChildGuaranteedValid( i );
//		Child->StartRefresh( );
//		}
//	return true;
//	}

//bool CItemBranch::StartRefreshIT_FILESFOLDER( _In_ bool wasExpanded ) {
//	CFileFindWDS finder;
//	BOOL b = finder.FindFile( GetFindPattern( ) );
//	while (b) {
//		b = finder.FindNextFile( );
//		if ( finder.IsDirectory( ) ) {
//			continue;
//			}
//
//		FILEINFO fi;
//		fi.name = finder.GetFileName( );
//		fi.attributes = finder.GetAttributes( );
//
//		// Retrieve file size
//		fi.length = finder.GetCompressedLength( );
//		finder.GetLastWriteTime( &fi.lastWriteTime );
//
//		AddFile( std::move( fi ) );
//		UpwardAddFiles( 1 );
//		}
//	SetDone();
//	if ( wasExpanded ) {
//		auto TreeListControl = GetTreeListControl( );
//		if ( TreeListControl != NULL ) {
//			TreeListControl->ExpandItem( this );
//			}
//		}
//	return true;
//	}

//bool CItemBranch::StartRefreshIT_FILE( ) {
//	CFileFindWDS finder;
//	BOOL b = finder.FindFile( GetPath( ) );
//	if ( b ) {
//		finder.FindNextFile( );
//		if (!finder.IsDirectory()) {
//			FILEINFO fi;
//			fi.name = finder.GetFileName( );
//			fi.attributes = finder.GetAttributes( );
//
//			// Retrieve file size
//			fi.length = finder.GetCompressedLength( );
//			finder.GetLastWriteTime( &fi.lastWriteTime );
//
//			SetLastChange( fi.lastWriteTime );
//
//			UpwardAddSize( fi.length );
//			UpwardUpdateLastChange( GetLastChange( ) );
//			auto Parent = GetParent( );
//			if ( Parent != NULL ) {
//				Parent->UpwardAddFiles( 1 );
//				}
//			}
//		}
//	SetDone( );
//	return true;
//	}

//bool CItemBranch::StartRefreshIsDeleted( _In_ ITEMTYPE typeOf_thisItem ) {
//	bool deleted = false;
//	if ( typeOf_thisItem == IT_DRIVE ) {
//		deleted = !DriveExists( GetPath( ) );
//		}
//	else if ( typeOf_thisItem == IT_FILE ) {
//		deleted = !FileExists( GetPath( ) );
//		}
//	else if ( typeOf_thisItem == IT_DIRECTORY ) {
//		deleted = !FolderExists( GetPath( ) );
//		}
//	return deleted;
//	}

//void CItemBranch::StartRefreshHandleDeletedItem( ) {
//	auto myParent_here = GetParent( );
//	if ( myParent_here == NULL ) {
//		return GetDocument( )->UnlinkRoot( );
//		}
//	myParent_here->UpwardRecalcLastChange( );
//	myParent_here->RemoveChild( myParent_here->FindChildIndex( this ) );// --> delete this
//	}

void CItemBranch::StartRefreshRecreateFSandUnknw( ) {
	/*
	  Re-create <free space> and <unknown>
	*/
	auto Document = GetDocument( );
	if ( Document != NULL ) {
		if ( Document->OptionShowFreeSpace( ) ) {
			CreateFreeSpaceItem( );
			}
		if ( Document->OptionShowUnknown( ) ) {
			CreateUnknownItem( );
			}
		}
	else {
		//Fall back to values that I like :)
		CreateFreeSpaceItem( );
		CreateUnknownItem( );
		}
	}

void CItemBranch::StartRefreshHandleWasExpanded( ) {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->ExpandItem( this );
		return;
		}
	ASSERT( TreeListControl != NULL );//What the fuck would this even mean??
	}

void CItemBranch::StartRefreshUpwardClearItem( _In_ ITEMTYPE typeOf_thisItem ) {
	UpwardAddReadJobs( -GetReadJobs( ) );
	ASSERT( GetReadJobs( ) == 0 );

	if ( typeOf_thisItem == IT_FILE ) {
		auto Parent = GetParent( );
		if ( Parent != NULL ) {
			Parent->UpwardAddFiles( -1 );
			}
		}
	else {
		UpwardAddFiles( -GetFilesCount( ) );
		}
	ASSERT( GetFilesCount( ) == 0 );

	if ( typeOf_thisItem == IT_DIRECTORY || typeOf_thisItem == IT_DRIVE ) {
		UpwardAddSubdirs( -GetSubdirsCount( ) );
		}
	ASSERT( GetSubdirsCount( ) == 0 );
	UpwardAddSize( -std::int64_t( GetSize( ) ) );
	ASSERT( GetSize( ) == 0 );
	}

_Must_inspect_result_ bool CItemBranch::StartRefreshIsMountOrJunction( _In_ ITEMTYPE typeOf_thisItem ) {
	/*
	  Was refactored from LARGER function. A return true from this function indicates that the caller should return true
	*/
	auto Options = GetOptions( );
	auto App = GetApp( );
	if ( typeOf_thisItem == IT_DIRECTORY ) {
		if ( !IsRootItem( ) ) {
			if ( App->IsMountPoint( GetPath( ) ) && !Options->IsFollowMountPoints( ) ) {
				return true;
				}
			if ( App->IsJunctionPoint( GetPath( ) ) && !Options->IsFollowJunctionPoints( ) ) {
				return true;
				}
			}
		}
	return false;
	}

//bool CItemBranch::StartRefresh( ) {
//	/*
//	  Returns false if deleted
//	*/
//	m_ticksWorked = 0;
//
//	auto typeOf_thisItem = GetType( );
//
//	ASSERT( ( typeOf_thisItem != IT_FREESPACE ) && ( typeOf_thisItem != IT_UNKNOWN ) || ( typeOf_thisItem == IT_FILE ) || ( typeOf_thisItem == IT_DRIVE ) || ( typeOf_thisItem == IT_DIRECTORY ) || ( typeOf_thisItem == IT_FILESFOLDER ) );
//
//	if ( typeOf_thisItem == IT_MYCOMPUTER ) {// Special case IT_MYCOMPUTER
//		return StartRefreshIT_MYCOMPUTER( );
//		}
//
//	bool wasExpanded = IsVisible( ) && IsExpanded( );
//
//	auto oldScrollPosition = 0;
//	if ( IsVisible( ) ) {
//		oldScrollPosition = GetScrollPosition( );
//		ASSERT( oldScrollPosition >= 0 );
//		}
//#ifdef DRAW_ICONS
//	UncacheImage( );
//#endif
//
//	UpdateLastChange( );
//	UpwardSetUndone( );
//	StartRefreshUpwardClearItem( typeOf_thisItem );
//	RemoveAllChildren( );
//	UpwardRecalcLastChange( );
//
//	if ( typeOf_thisItem == IT_FILESFOLDER ) {// Special case IT_FILESFOLDER
//		return StartRefreshIT_FILESFOLDER( wasExpanded );
//		}
//
//	if ( StartRefreshIsDeleted( typeOf_thisItem ) ) {
//		StartRefreshHandleDeletedItem( );
//		return false;
//		}
//
//	if ( typeOf_thisItem == IT_FILE ) {
//		return StartRefreshIT_FILE( );
//		}
//	
//	if ( StartRefreshIsMountOrJunction( typeOf_thisItem ) ) {
//		return true;//bubble the return up
//		}
//	
//	TRACE( _T( "Initiating re-read!\r\n" ) );
//	SetReadJobDone( false );
//
//	if ( typeOf_thisItem == IT_DRIVE ) {// Re-create <free space> and <unknown>
//		StartRefreshRecreateFSandUnknw( );
//		}
//	DoSomeWork( 999 );
//	if ( wasExpanded ) {
//		StartRefreshHandleWasExpanded( );
//		}
//	if ( IsVisible( ) ) {
//		SetScrollPosition( oldScrollPosition );
//		}
//	return true;
//}

void CItemBranch::UpwardSetUndoneIT_DRIVE( ) {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		auto thisChild = GetChildGuaranteedValid( i );
		auto childType = thisChild->GetType( );
		if ( ( childType == IT_UNKNOWN ) || ( childType == IT_DIRECTORY ) ) {
			break;
			}
		UpwardAddSize( -std::int64_t( thisChild->GetSize( ) ) );
		thisChild->SetSize( 0 );
		}
	}

void CItemBranch::UpwardParentSetUndone( ) {
	auto Parent = GetParent( );
	if ( Parent != NULL ) {
		Parent->UpwardSetUndone( );
		}
	}

void CItemBranch::UpwardSetUndone( ) {
	auto thisItemType = GetType( );
	if ( thisItemType != IT_DIRECTORY ) {
		auto Document = GetDocument( );
		if ( Document != NULL ) {
			if ( thisItemType == IT_DRIVE && IsDone( ) && Document->OptionShowUnknown( ) ) {
				UpwardSetUndoneIT_DRIVE( );
				}
			}
		ASSERT( Document != NULL );
		}
		m_done = false;
		UpwardParentSetUndone( );
	}

void CItemBranch::CreateFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto freeSp = GetFreeDiskSpace( GetPath( ) );
	auto freespace = new CItemBranch { IT_FREESPACE, GetFreeSpaceItemName( ), freeSp, true };
	AddChild( freespace );
	}

_Success_(return != NULL) _Must_inspect_result_ CItemBranch *CItemBranch::FindFreeSpaceItem( ) const {
	auto i = FindFreeSpaceItemIndex( );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}

void CItemBranch::UpdateFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	if ( !GetDocument( )->OptionShowFreeSpace( ) ) {
		return;
		}
	auto freeSpaceItem = FindFreeSpaceItem( );
	if ( freeSpaceItem != NULL ) {
		auto free = GetFreeDiskSpace( GetPath( ) );
		auto before = freeSpaceItem->GetSize( );
		auto diff  = free - before;
		freeSpaceItem->UpwardAddSize( diff );
		ASSERT( freeSpaceItem->GetSize( ) == ULONGLONG( free ) );
		}
	}

void CItemBranch::TmiSetRectangle( _In_ const CRect& rc ) {
	ASSERT( ( rc.right + 1 ) >= rc.left );
	ASSERT( rc.bottom >= rc.top );
	ASSERT( rc.left   < 32767 );
	ASSERT( rc.top    < 32767 );
	ASSERT( rc.right  < 32767 );
	ASSERT( rc.bottom < 32767 );
	ASSERT( ( ( 0-32768 ) < rc.left   ) );
	ASSERT( ( ( 0-32768 ) < rc.top    ) );
	ASSERT( ( ( 0-32768 ) < rc.right  ) );
	ASSERT( ( ( 0-32768 ) < rc.bottom ) );
	m_rect.left		= short( rc.left   );
	m_rect.top		= short( rc.top    );
	m_rect.right	= short( rc.right  );
	m_rect.bottom	= short( rc.bottom );


	}

void CItemBranch::RemoveFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto i = FindFreeSpaceItemIndex( );
	ASSERT( i < GetChildrenCount( ) );
	if ( i < GetChildrenCount( ) ) {
		auto freespace = GetChildGuaranteedValid( i );
		UpwardAddSize( -std::int64_t( freespace->GetSize( ) ) );
		RemoveChild( i );
		}
	}

void CItemBranch::CreateUnknownItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto unknown = new CItemBranch { IT_UNKNOWN, GetUnknownItemName( ) };//std::make_shared<CItemBranch>
	unknown->SetDone( );
	AddChild( unknown );
	}

_Success_(return != NULL) _Must_inspect_result_ CItemBranch* CItemBranch::FindUnknownItem( ) const {
	auto i = FindUnknownItemIndex( );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}

void CItemBranch::RemoveUnknownItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto i = FindUnknownItemIndex( );
	ASSERT( i < GetChildrenCount( ) );
	if ( i < GetChildrenCount( ) ) {
		auto unknown = GetChildGuaranteedValid( i );
		UpwardAddSize( -std::int64_t( unknown->GetSize( ) ) );
		RemoveChild( i );
		}
	}

_Success_( return != NULL ) _Must_inspect_result_ CItemBranch* CItemBranch::FindDirectoryByPath( _In_ const CString& path ) {
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
		return this;
		}

	auto thisChildCount = GetChildrenCount( );
	for ( INT j = 0; j < thisChildCount; j++ ) {
		auto Child = GetChildGuaranteedValid( j );
		auto item = Child->FindDirectoryByPath( path );
		if ( item != NULL ) {
			return item;
			}
		}
	return NULL;
	}

void AddFileExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<CString, SExtensionRecord>& extensionMap ) {
	ASSERT( extensionRecords.size( ) == 0 );
	extensionRecords.reserve( extensionMap.size( ) );
	for ( auto mapIterator = extensionMap.begin( ); mapIterator != extensionMap.end( ); ++mapIterator ) {
		extensionRecords.emplace_back( std::move( mapIterator->second ) );
		}
	}

DOUBLE CItemBranch::averageNameLength( ) const {
	int myLength = m_name.GetLength( );
	DOUBLE childrenTotal = 0;
	if ( GetType( ) != IT_FILE ) {
		for ( INT_PTR i = 0; i < m_children.polySize( ); ++i ) {
			childrenTotal += m_children polyAt( i )->averageNameLength( );
			}
		}
	return ( childrenTotal + myLength ) / ( m_children.polySize( ) + 1 );
	}

void CItemBranch::stdRecurseCollectExtensionData( /*_Inout_ std::vector<SExtensionRecord>& extensionRecords,*/ _Inout_ std::map<CString, SExtensionRecord>& extensionMap ) {
	auto typeOfItem = GetType( );
	if ( IsLeaf( typeOfItem ) ) {
		if ( typeOfItem == IT_FILE ) {
			auto ext = GetExtension( );
			if ( extensionMap[ ext ].files == 0 ) {
				++( extensionMap[ ext ].files );
				extensionMap[ ext ].bytes += GetSize( );
				extensionMap[ ext ].ext = ext;
				}
			else {
				++( extensionMap[ ext ].files );
				extensionMap[ ext ].bytes += GetSize( );
				}
			}
		}
	else {
		auto childCount = GetChildrenCount( );
		for ( INT i = 0; i < childCount; ++i ) {
			auto Child = GetChildGuaranteedValid( i );
			Child->stdRecurseCollectExtensionData( /*extensionRecords,*/ extensionMap );
			}
		}
	}

INT __cdecl CItemBranch::_compareBySize( _In_ const void *p1, _In_ const void *p2 ) {
	auto item1 = *( CItemBranch ** ) p1;
	auto item2 = *( CItemBranch ** ) p2;
	auto size1 = item1->GetSize( );
	auto size2 = item2->GetSize( );
	return signum( std::int64_t( size2 ) - std::int64_t( size1 ) ); // biggest first// TODO: Use 2nd sort column (as set in our TreeListView?)
	}

LONGLONG CItemBranch::GetProgressRangeMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG range = 0;
	auto childCountHere = GetChildrenCount( );
	for ( INT i = 0; i < childCountHere; i++ ) {
		range += GetChildGuaranteedValid( i )->GetProgressRangeDrive( );
		}
	return range;
	}

LONGLONG CItemBranch::GetProgressPosMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG pos = 0;
	auto childCountHere = GetChildrenCount( );
	for ( INT i = 0; i < childCountHere; i++ ) {
		pos += GetChildGuaranteedValid( i )->GetProgressPosDrive( );
		}
	return pos;
	}

_Ret_range_( 0, INT64_MAX ) LONGLONG CItemBranch::GetProgressRangeDrive( ) const {
	auto Doc     = GetDocument( );
	auto total   = Doc->GetTotlDiskSpace( GetPath( ) );
	auto freeSp  = Doc->GetFreeDiskSpace( GetPath( ) );
	return ( total - freeSp );
	}

LONGLONG CItemBranch::GetProgressPosDrive( ) const {
	auto pos = GetSize( );
	auto fs = FindFreeSpaceItem( );
	if ( fs != NULL ) {
		pos -= fs->GetSize( );
		}
	return pos;
	}

COLORREF CItemBranch::GetGraphColor( ) const {
	switch ( GetType( ) )
	{
		case IT_UNKNOWN:
			return ( RGB( 255, 255, 0   ) | CTreemap::COLORFLAG_LIGHTER );

		case IT_FREESPACE:
			return ( RGB( 100, 100, 100 ) | CTreemap::COLORFLAG_DARKER  );

		case IT_FILE:
			return ( GetDocument( )->GetCushionColor( GetExtension( ) ) );
		
		case IT_DIRECTORY:
			return RGB( 254, 254, 254 );

		case IT_FILESFOLDER:
			return RGB( 254, 254, 254 );

		default:
			//ASSERT( GetType( ) == IT_DIRECTORY );
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

COLORREF CItemBranch::GetPercentageColor( ) const {
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		auto i = GetIndent( ) % Options->GetTreelistColorCount( );
		return std::move( Options->GetTreelistColor( i ) );
		}
	ASSERT( false );//should never ever happen, but just in case, we'll generate a random color.
	return DWORD( rand( ) );
	}

INT_PTR CItemBranch::FindFreeSpaceItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChildGuaranteedValid( i )->GetType( ) == IT_FREESPACE ) {
			return i; // maybe == GetChildrenCount() (=> not found)
			}
		}
	return childCount;
	}

INT_PTR CItemBranch::FindUnknownItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChildGuaranteedValid( i )->GetType( ) == IT_UNKNOWN ) {
			return i; // maybe == GetChildrenCount() (=> not found)
			}	
		}
	return childCount;
	}

CString CItemBranch::UpwardGetPathWithoutBackslash( ) const {
	CString path;
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		path = myParent->UpwardGetPathWithoutBackslash( );
		}
	switch (GetType())
	{
		case IT_MYCOMPUTER:
			break;

		case IT_DRIVE:
			return PathFromVolumeName( m_name ); // (we don't use our parent's path here.)

		case IT_DIRECTORY:
			if ( !path.IsEmpty( ) ) {
				path += _T( "\\" );
				}
			path += m_name;
			break;

		case IT_FILE:
			path += _T("\\") + m_name;
			break;

		case IT_FILESFOLDER:
		case IT_FREESPACE:
		case IT_UNKNOWN:
			break;

		default:
			ASSERT(false);
	}
	return path; 
	}

void CItemBranch::AddDirectory( _In_ const CFileFindWDS& finder ) {
	ASSERT( &finder != NULL );
	auto thisApp      = GetApp( );
	auto thisFilePath = finder.GetFilePath( );
	auto thisOptions  = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow   = thisApp->IsMountPoint( thisFilePath ) && !thisOptions->IsFollowMountPoints( );
	
	dontFollow       |= thisApp->IsJunctionPoint( thisFilePath, finder.GetAttributes( ) ) && !thisOptions->IsFollowJunctionPoints( );
	auto child        = new CItemBranch{ IT_DIRECTORY, finder.GetFileName( ), dontFollow };
	
	FILETIME t;
	finder.GetLastWriteTime( &t );
	child->SetLastChange( t );
	child->SetAttributes( finder.GetAttributes( ) );
	AddChild( child );
	}

void CItemBranch::AddFile( _In_ const FILEINFO& fi ) {
	AddChild( new CItemBranch { IT_FILE, fi.name, fi.length, fi.lastWriteTime, fi.attributes, true } );
	}

void CItemBranch::DriveVisualUpdateDuringWork( ) {
	TRACE( _T( "Exceeding number of ticks!\r\npumping messages - this is a dirty hack to ensure responsiveness while single-threaded.\r\n" ) );
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
