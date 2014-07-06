// item.cpp	- Implementation of CItem
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
#include "windirstat.h"
#include "dirstatdoc.h"	// GetItemColor()
#include "mainframe.h"
#include "item.h"
#include "globalhelpers.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	CString GetFreeSpaceItemName() 	{ return LoadString(IDS_FREESPACE_ITEM); }
	CString GetUnknownItemName() { return LoadString(IDS_UNKNOWN_ITEM); }

	// File attribute packing
	const unsigned char INVALID_m_attributes = 0x80;
}


CItem::CItem( ITEMTYPE type, LPCTSTR name, bool dontFollow ) : m_type( std::move( type ) ), m_name( std::move( name ) ), m_size( 0 ), m_files( 0 ), m_subdirs( 0 ), m_done( false ), m_ticksWorked( 0 ), m_readJobs( 0 ), m_attributes( 0 ) {
	auto thisItem_type = GetType( );

	if ( thisItem_type == IT_FILE || dontFollow || thisItem_type == IT_FREESPACE || thisItem_type == IT_UNKNOWN || thisItem_type == IT_MYCOMPUTER ) {
		SetReadJobDone( );
		m_readJobs = 0;
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_DRIVE || thisItem_type == IT_FILESFOLDER ) {
		SetReadJobDone( false );
		}

	if ( thisItem_type == IT_DRIVE ) {
		m_name = FormatVolumeNameOfRootPath( m_name );
		}

	m_lastChange.dwHighDateTime = 0;
	m_lastChange.dwLowDateTime = 0;
	}

CItem::~CItem( ) {
	auto Document = GetDocument( );
	CItem* currentZoomItem = NULL;
	CItem* currentRootItem = NULL;
	CItem* currentlySelectedItem = NULL;
	if ( Document != NULL ) {
		currentZoomItem = Document->GetZoomItem( );
		currentRootItem = Document->GetRootItem( );
		currentlySelectedItem = Document->GetSelection( );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		return;
		}
#ifndef CHILDVEC
	auto childrenSize = m_children.GetSize( );
	for ( INT i = 0; i < childrenSize; i++ ) {
		if ( ( m_children[ i ] ) != NULL ) {
			delete m_children[ i ];
			if ( m_children[ i ] == currentZoomItem ) {
				Document->clearZoomItem( );
				ASSERT( Document->GetZoomItem( ) != m_children[ i ] );
				}
			if ( m_children[ i ] == currentRootItem ) {
				Document->clearRootItem( );
				ASSERT( Document->GetRootItem( ) != m_children[ i ] );
				}
			if ( m_children[ i ] == currentlySelectedItem ) {
				Document->clearSelection( );
				ASSERT( Document->GetSelection( ) != m_children[ i ] );
				}
			m_children[ i ] = NULL;//sensible?
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
		}
#else
	for (auto& aChild : m_vectorOfChildren){
		if (aChild != NULL){
			delete aChild;
			if (aChild == currentZoomItem){
				Document->clearZoomItem();
				ASSERT(Document->GetZoomItem() != aChild);
				}
			if (aChild == currentRootItem){
				Document->clearRootItem();
				ASSERT(Document->GetRootItem() != aChild);
				}
			if (aChild == currentlySelectedItem){
				Document->clearSelection();
				ASSERT(Document->GetSelection() != aChild);
				}
			aChild = NULL;
			}
		else{
			AfxCheckMemory();
			ASSERT(false);
			}
		}
#endif
	}

CItem::CItem( CItem&& in ) {
	m_type                 = std::move( in.m_type );
	m_name                 = std::move( in.m_name );
	m_size                 = std::move( in.m_size );
	m_files                = std::move( in.m_files );
	m_subdirs              = std::move( in.m_subdirs );
	m_lastChange           = std::move( in.m_lastChange );
	m_attributes           = std::move( in.m_attributes );
	m_readJobDone          = std::move( in.m_readJobDone );
	m_done                 = std::move( in.m_done );
	m_ticksWorked          = std::move( in.m_ticksWorked );
	m_readJobs             = std::move( in.m_readJobs );
	//m_children             = in.m_children;
#ifdef CHILDVEC
	m_vectorOfChildren     = std::move( in.m_vectorOfChildren );
#endif
	m_rect                 = std::move( in.m_rect );
	}

SRECT CItem::GetSRECT( ) const {
	SRECT rect{ m_rect };
	return std::move( rect );
	}

CRect CItem::TmiGetRectangle( ) const {
	CRect rc;
	rc.left		= m_rect.left;
	rc.top		= m_rect.top;
	rc.right	= m_rect.right;
	rc.bottom	= m_rect.bottom;
	//ASSERT( ( rc.Height( ) + rc.Width()) > 0 );
	rc.NormalizeRect( );
	return std::move( rc );
	}

LONG CItem::TmiGetRectLeft( ) const {
	return m_rect.left;
	}

void CItem::TmiSetRectangle( _In_ const CRect& rc ) {
	m_rect.left		= short( rc.left   );
	m_rect.top		= short( rc.top    );
	m_rect.right	= short( rc.right  );
	m_rect.bottom	= short( rc.bottom );
	}

bool CItem::DrawSubitem( _In_ const INT subitem, _In_ CDC* pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
	ASSERT_VALID( pdc );
	ASSERT( subitem >= 0 );
	ASSERT( &width != NULL );
	ASSERT( focusLeft != NULL );
	
	if (subitem == COL_NAME) {
		return CTreeListItem::DrawSubitem( subitem, pdc, rc, state, width, focusLeft );
		}
	if ( subitem != COL_SUBTREEPERCENTAGE ) {
		return false;
		}
	bool showReadJobs = MustShowReadJobs( );

	if ( showReadJobs ) {
		if ( IsDone( ) ) {
			return false;
			}
		}

	if ( width != NULL ) {
		*width = 105;
		return true;
		}

	DrawSelection( GetTreeListControl( ), pdc, rc, state );

	if ( showReadJobs ) {
		//rc.DeflateRect( std::move( sizeDeflatePacman ) );
		}
	else {
		rc.DeflateRect( 2, 5 );
		auto indent = GetIndent( );
		for ( INT i = 0; i < indent; i++ ) {
			rc.left += ( rc.right - rc.left ) / 10;
			}

		DrawPercentage( pdc, rc, GetFraction( ), std::move( GetPercentageColor( ) ) );
		}
	return true;
	}

void CItem::GetTextCOL_SUBTREEPERCENTAGE( _Inout_ CString& s ) const {
	if ( IsDone( ) ) {
		ASSERT( m_readJobs == 0 );//s = "ok";
		}
	else {
		if ( m_readJobs == 1 ) {
			auto ret = s.LoadString( IDS_ONEREADJOB );//TODO //IDS_ONEREADJOB == "[1 Read Job]"
			if ( ret == 0 ) {
				AfxCheckMemory( );
				ASSERT( false );
				}
			}
		else {
			std::wstring a;
			a += FormatCount( m_readJobs );
			s.FormatMessage( IDS_sREADJOBS, a.c_str( ) );
			}
		}

	}

void CItem::GetTextCOL_PERCENTAGE( _Inout_ CString& s ) const {
	if ( GetOptions( )->IsShowTimeSpent( ) && MustShowReadJobs( ) || IsRootItem( ) ) {
		s.Format( _T( "[%s s]" ), FormatMilliseconds( GetTicksWorked( ) ).GetString( ) );
		}
	else {
		s.Format( _T( "%s%%" ), FormatDouble( GetFraction( ) * 100 ).GetString( ) );
		}
	}

void CItem::GetTextCOL_ITEMS( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetItemsCount( ) );
		}
	}

void CItem::GetTextCOL_FILES( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetFilesCount( ) );
		}
	}

void CItem::GetTextCOL_SUBDIRS( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetSubdirsCount( ) );
		}
	}

void CItem::GetTextCOL_LASTCHANGE( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatFileTime( m_lastChange );//FIXME
		}
	}

void CItem::GetTextCOL_ATTRIBUTES( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FREESPACE && typeOfItem != IT_FILESFOLDER && typeOfItem != IT_UNKNOWN && typeOfItem != IT_MYCOMPUTER ) {
		s = FormatAttributes( GetAttributes( ) );
		}
	}

CString CItem::GetText( _In_ const INT subitem ) const {
	CString s;
	ASSERT( subitem >= 0 );
	switch (subitem)
	{
		case COL_NAME:
			s = m_name;
			break;

		case COL_SUBTREEPERCENTAGE:
			GetTextCOL_SUBTREEPERCENTAGE( s );
			break;

		case COL_PERCENTAGE:
			GetTextCOL_PERCENTAGE( s );
			break;

		case COL_SUBTREETOTAL:
			s = FormatBytes( GetSize( ) );
			break;

		case COL_ITEMS:
			GetTextCOL_ITEMS( s );
			break;

		case COL_FILES:
			GetTextCOL_FILES( s );
			break;

		case COL_SUBDIRS:
			GetTextCOL_SUBDIRS( s );
			break;

		case COL_LASTCHANGE:
			GetTextCOL_LASTCHANGE( s );
			break;

		case COL_ATTRIBUTES:
			GetTextCOL_ATTRIBUTES( s );
			break;

		default:
			AfxCheckMemory( );
			ASSERT(false);
			break;
	}
	return s;
	}

COLORREF CItem::GetItemTextColor( ) const {
	// Get the file/folder attributes
	DWORD attr = GetAttributes( );

	// This happens e.g. on a Unicode-capable FS when using ANSI APIs to list files with ("real") Unicode names
	if ( attr == INVALID_FILE_ATTRIBUTES ) {
		return CTreeListItem::GetItemTextColor( );
		}

	// Check for compressed flag
	if ( attr & FILE_ATTRIBUTE_COMPRESSED ) {
		return GetApp( )->AltColor( );
		}

	else if ( attr & FILE_ATTRIBUTE_ENCRYPTED ) {
		return GetApp( )->AltEncryptionColor( );
		}

	else {
		// The rest is not colored
		return CTreeListItem::GetItemTextColor( );
		}
	}

INT CItem::CompareName( _In_ const CItem* other ) const {
	if ( GetType( ) == IT_DRIVE ) {
		ASSERT( other->GetType( ) == IT_DRIVE );
		return signum( GetPath( ).CompareNoCase( other->GetPath( ) ) );
		}

	else {
		return signum( m_name.CompareNoCase( other->m_name ) );
		}

	}

INT CItem::CompareSubTreePercentage( _In_ const CItem* other ) const {
	if ( MustShowReadJobs( ) ) {
		return signum( m_readJobs - other->m_readJobs );
		}

	else {
		return signum( GetFraction( ) - other->GetFraction( ) );
		}

	}

INT CItem::CompareLastChange( _In_ const CItem* other ) const {
	if ( m_lastChange < other->m_lastChange ) {
		return -1;
		}

	else if ( m_lastChange == other->m_lastChange ) {
		return 0;
		}
	return 1;
	}


INT CItem::CompareSibling( _In_ const CTreeListItem *tlib, _In_ const INT subitem ) const {
	CItem *other = ( CItem * ) tlib;
	ASSERT( subitem >= 0 );
	INT r = 0;
	switch (subitem)
	{
		case COL_NAME:
			r = CompareName( other );
			break;

		case COL_SUBTREEPERCENTAGE:
			r = CompareSubTreePercentage( other );
			break;

		case COL_PERCENTAGE:
			r = signum( GetFraction( ) - other->GetFraction( ) );
			break;

		case COL_SUBTREETOTAL:
			r = signum( GetSize( ) - other->GetSize( ) );
			break;

		case COL_ITEMS:
			r = signum( GetItemsCount( ) - other->GetItemsCount( ) );
			break;

		case COL_FILES:
			r = signum( GetFilesCount( ) - other->GetFilesCount( ) );
			break;

		case COL_SUBDIRS:
			r = signum( GetSubdirsCount( ) - other->GetSubdirsCount( ) );
			break;

		case COL_LASTCHANGE:
			r = CompareLastChange( other );
			break;

		case COL_ATTRIBUTES:
			r = signum( GetSortAttributes( ) - other->GetSortAttributes( ) );
			break;

		default:
			AfxCheckMemory( );
			ASSERT(false);
			break;
	}
	return r;
	}

INT CItem::GetChildrenCount( ) const {
	return m_children.GetSize();
	}

#ifdef CHILDVEC
size_t CItem::GetChildVecCount( ) const {
	return m_vectorOfChildren.size( );
	}
#endif

_Must_inspect_result_ CTreeListItem *CItem::GetTreeListChild( _In_ const INT i ) const {
	ASSERT( !( m_children.IsEmpty( ) ) );
	ASSERT( i >= 0 );
	return m_children[ i ];
	}

INT CItem::GetImageToCache( ) const {
	// (Caching is done in CTreeListItem::m_vi.)

	INT image;
	auto type_theItem = GetType( );
	if ( type_theItem == IT_MYCOMPUTER ) {
		image = GetMyImageList( )->GetMyComputerImage( );
		}
	else if ( type_theItem == IT_FILESFOLDER ) {
		image = GetMyImageList( )->GetFilesFolderImage( );
		}
	else if ( type_theItem == IT_FREESPACE ) {
		image = GetMyImageList( )->GetFreeSpaceImage( );
		}
	else if ( type_theItem == IT_UNKNOWN ) {
		image = GetMyImageList( )->GetUnknownImage( );
		}
	else {
		CString path = GetPath();
		auto attrib = GetAttributes( );
		auto MyImageList = GetMyImageList( );
		if ( type_theItem == IT_DIRECTORY && GetApp( )->IsMountPoint( path ) ) {
			image = MyImageList->GetMountPointImage( );
			}
		else if ( type_theItem == IT_DIRECTORY && GetApp( )->IsJunctionPoint( path, attrib ) ) {
			image = MyImageList->GetJunctionImage( );
			}
		else {
			image = MyImageList->GetFileImage( path );
			}
		}
	return image; 
	}

void CItem::DrawAdditionalState( _In_ CDC* pdc, _In_ const CRect& rcLabel ) const {
	ASSERT_VALID( pdc );
	auto thisDocument = GetDocument( );
	if ( !IsRootItem( ) && this == thisDocument->GetZoomItem( ) ) {
		CRect rc = rcLabel;
		rc.InflateRect( 1, 0 );
		rc.bottom++;

		CSelectStockObject sobrush { pdc, NULL_BRUSH };
		CPen pen                   { PS_SOLID, 2, thisDocument->GetZoomColor( ) };
		CSelectObject sopen        { pdc, &pen };

		pdc->Rectangle( rc );
		}
	}

_Must_inspect_result_ CItem *CItem::FindCommonAncestor( _In_ const CItem* item1, _In_ const CItem* item2 ) {
	ASSERT( ( item1 != NULL ) && ( item2 != NULL ) );
	auto parent = item1;
	while ( !parent->IsAncestorOf( item2 ) ) {
		parent = parent->GetParent( );
		}
	ASSERT( parent != NULL );
	return const_cast< CItem * >( parent );
	}

bool CItem::IsAncestorOf( _In_ const CItem* thisItem ) const {
	ASSERT( thisItem != NULL );
	auto p = thisItem;
	while ( p != NULL ) {
		if ( p == this ) {
			break;
			}
		p = p->GetParent( );
		}
	return ( p != NULL );
	}

LONGLONG CItem::GetProgressRange( ) const {
	switch ( GetType( ) )
	{
		case IT_MYCOMPUTER:
			return GetProgressRangeMyComputer();

		case IT_DRIVE:
			return GetProgressRangeDrive();

		case IT_DIRECTORY:
		case IT_FILESFOLDER:
		case IT_FILE:
			return 0;

		case IT_FREESPACE:
		case IT_UNKNOWN:
		default:
			AfxCheckMemory( );
			ASSERT( false );
			return 0;
	}
	}

LONGLONG CItem::GetProgressPos( ) const {
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
			AfxCheckMemory( );
			ASSERT( false );
			return 0;
	}
	}

_Must_inspect_result_ const CItem *CItem::UpwardGetRoot( ) const {
	AfxCheckMemory( );
	auto myParent = GetParent( );
	if ( myParent == NULL ) {
		return this;
		}
	else {
		return myParent->UpwardGetRoot( );
		}
	}

void CItem::UpdateLastChange( ) {
	m_lastChange.dwHighDateTime = NULL;
	m_lastChange.dwLowDateTime = NULL;
	auto typeOf_thisItem = GetType( );

	if ( typeOf_thisItem == IT_DIRECTORY || typeOf_thisItem == IT_FILE ) {
		auto path = GetPath( );
		INT i = path.ReverseFind( _T( '\\' ) );
		CString basename = path.Mid( i + 1 );
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

_Success_(return != NULL) _Must_inspect_result_ CItem *CItem::GetChild(_In_ const INT i) const {
	/*
	  Returns CItem* to child if passed a valid index. Returns NULL if `i` is NOT a valid index. 
	*/
	ASSERT( !( m_children.IsEmpty( ) ) );
	ASSERT( i >= 0 && i <= ( m_children.GetSize( ) - 1 ) );
	if ( i >= 0 && i <= ( m_children.GetSize( ) -1 ) ) {
		return m_children[ i ];
		}
	else {
		return NULL;
		}
	}

_Success_( return != NULL ) CItem* CItem::GetChildGuaranteedValid( _In_ const INT i ) const {
	ASSERT( !( m_children.IsEmpty( ) ) );
	ASSERT( i >= 0 && i <= ( m_children.GetSize( ) - 1 ) );
	if ( i >= 0 && i <= ( m_children.GetSize( ) -1 ) ) {
		if ( m_children[ i ] != NULL ) {
			return m_children[ i ];
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			throw 666;
			}
		}
	else {
		throw 666;
		}
	}

_Must_inspect_result_ CItem *CItem::GetParent( ) const {
	return ( CItem * ) CTreeListItem::GetParent( );
	}

INT CItem::FindChildIndex( _In_ const CItem* child ) const {
	ASSERT( child != NULL );
	auto childCount = GetChildrenCount( );	
	for ( INT i = 0; i < childCount; i++ ) {
		if ( child == m_children[ i ] ) {
			return i;
			}
		}
	AfxCheckMemory( );
	ASSERT( false );
	return childCount;
	}

void CItem::AddChild( _In_ CItem* child ) {
	ASSERT( child != NULL );

	ASSERT( !IsDone( ) ); // SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItem::GetFraction() could ASSERT.
	UpwardAddSize         ( child->GetSize( ) );
	UpwardAddReadJobs     ( child->GetReadJobs( ) );
	UpwardUpdateLastChange( child->GetLastChange( ) );

#ifndef CHILDVEC
	m_children.Add( child );
#else
	m_vectorOfChildren.emplace_back(child);
#endif
	ASSERT( this != NULL );
	child->SetParent( this );
	ASSERT( child->GetParent( ) == this );
	ASSERT( !( child->IsRootItem( ) ) );
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnChildAdded( this, child, IsDone( ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

#ifdef CHILDVEC
void CItem::AddChildToVec( _In_ CItem& child ) {
	m_vectorOfChildren.emplace_back( std::move( child ) );
	}
#endif

void CItem::RemoveChild(_In_ const INT i) {
	ASSERT( !( m_children.IsEmpty( ) ) );
	ASSERT( i >= 0 && i <= ( m_children.GetSize( ) - 1 ) );
	if ( i >= 0 && ( i <= ( m_children.GetSize( ) - 1 ) ) ) {
		auto child = GetChildGuaranteedValid( i );
		auto TreeListControl = GetTreeListControl( );
		if ( TreeListControl != NULL ) {
			m_children.RemoveAt( i );
			TreeListControl->OnChildRemoved( this, child );
			delete child;
			child = NULL;
			}
		}
	}

void CItem::RemoveAllChildren() {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnRemovingAllChildren( this );
		}
	auto childCount = GetChildrenCount( );
	for ( auto i = 0; i < childCount; i++ ) {
		ASSERT( ( i >= 0 ) && ( i <= GetChildrenCount( ) - 1 ));
		if ( m_children[ i ] != NULL ) {
			delete m_children[ i ];
			m_children[ i ] = NULL;
			AfxCheckMemory( );
			}
		}
	m_children.SetSize( 0 );
	ASSERT( m_children.IsEmpty( ) );
	AfxCheckMemory( );
	}

void CItem::UpwardAddSubdirs( _In_ const std::int64_t dirCount ) {
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
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CItem::UpwardAddFiles( _In_ const std::int64_t fileCount ) {
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
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CItem::UpwardAddSize( _In_ const std::int64_t bytes ) {
	ASSERT( bytes >= 0 || bytes == -GetSize( ) || bytes >= ( -1 * ( GetTotalDiskSpace( this->UpwardGetPathWithoutBackslash( ) ) ) ) );
	if ( bytes < 0 ) {
		if ( ( bytes + m_size ) < 0 ) {
			//std::int64_t newCount = std::int64_t( ( m_size + bytes ) * ( -1 ) );
			m_size = 0;
			}
		else {
			m_size -= std::uint32_t( bytes * ( -1 ) );
			}
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		}
	else {
		m_size += std::uint32_t( bytes );
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CItem::UpwardAddReadJobs( _In_ const /* signed */std::int64_t count ) {
	if ( count < 0 ) {
		if ( ( m_readJobs + count ) < 0 ) {
			//std::int64_t newCount = std::int64_t( ( std::int64_t( m_readJobs ) + count ) * ( -1 ) );
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
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CItem::UpwardUpdateLastChange(_In_ const FILETIME& t) {
	/*
	  This method increases the last change
	*/
	if ( m_lastChange < t ) {
		m_lastChange = t;
		auto myParent = GetParent( );
		if ( myParent != NULL ) {
			myParent->UpwardUpdateLastChange( t );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}


void CItem::UpwardRecalcLastChange() {
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
	else {
		//Valid condition? `this` may be the root item.
		}
	}

LONGLONG CItem::GetSize() const {
	return m_size;
	}

void CItem::SetSize( _In_ const LONGLONG ownSize ) {

#ifdef _DEBUG
	auto typeOf_thisItem = GetType( );
	bool leafness = IsLeaf(typeOf_thisItem);
	if ( !leafness ) {
		TRACE( _T("%s is NOT a leaf!\r\n"), m_name);
		ASSERT( IsLeaf( typeOf_thisItem ) );
		}
	ASSERT( ownSize >= 0 );
#endif

	m_size = ownSize;
	}

LONGLONG CItem::GetReadJobs() const {
	return m_readJobs;
	}

FILETIME CItem::GetLastChange() const {
	return m_lastChange;
	}

void CItem::SetLastChange( _In_ const FILETIME& t ) {
	m_lastChange = t;
	}


void CItem::SetAttributes( const DWORD attr ) {
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

	ret &=  FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;// Mask out lower 3 bits
	
	// Prepend the archive attribute
	ret |= ( attr & FILE_ATTRIBUTE_ARCHIVE ) >> 2;
	
	// --> At this point the lower nibble is fully used. Now shift the reparse point and compressed attribute into the lower 2 bits of the high nibble.
	ret |= ( attr & ( FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_COMPRESSED ) ) >> 6;
	
	// Shift the encrypted bit by 8 places
	ret |= ( attr & FILE_ATTRIBUTE_ENCRYPTED ) >> 8;

	m_attributes = ( unsigned char ) ret;
	}

// Decode the attributes encoded by SetAttributes()
DWORD CItem::GetAttributes( ) const {
	DWORD ret = m_attributes;

	if ( ret & INVALID_m_attributes ) {
		return INVALID_FILE_ATTRIBUTES;
		}

	ret &= FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;// Mask out lower 3 bits
	
	// FILE_ATTRIBUTE_ARCHIVE
	ret |= (m_attributes & 0x08) << 2;
	
	// FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_COMPRESSED
	ret |= (m_attributes & 0x30) << 6;
	
	// FILE_ATTRIBUTE_ENCRYPTED
	ret |= (m_attributes & 0x40) << 8;
	
	return ret;
	}

// Returns a value which resembles sorting of RHSACE considering gaps
INT CItem::GetSortAttributes( ) const {
	DWORD ret = 0;

	// We want to enforce the order RHSACE with R being the highest priority attribute and E being the lowest priority attribute.
	ret += (m_attributes & 0x01) ? 1000000 : 0; // R
	ret += (m_attributes & 0x02) ? 100000  : 0; // H
	ret += (m_attributes & 0x04) ? 10000   : 0; // S
	ret += (m_attributes & 0x08) ? 1000    : 0; // A
	ret += (m_attributes & 0x20) ? 100     : 0; // C
	ret += (m_attributes & 0x40) ? 10      : 0; // E

	return ( m_attributes & INVALID_m_attributes ) ? 0 : ret;
	}

DOUBLE CItem::GetFraction( ) const {
	auto myParent = GetParent( );
	if ( myParent == NULL ) {
		return 1.0;//root item? must be whole!
		}
	auto parentSize = myParent->GetSize( );
	if ( parentSize == 0){
		return 1.0;
		}
	return DOUBLE( GetSize( ) ) / DOUBLE( parentSize );
	}

ITEMTYPE CItem::GetType( ) const {
	return ( ITEMTYPE ) ( m_type & ~ITF_FLAGS );
	}

bool CItem::IsRootItem( ) const {
	return ( ( m_type & ITF_ROOTITEM ) != 0 );
	}

CString CItem::GetPath( )  const {
	if ( this == NULL ) {
		TRACE(_T("'this' hasn't been initialized yet!\r\n") );
		CString temp;
		return temp;
		}
	CString path        = UpwardGetPathWithoutBackslash( );
	auto typeOfThisItem = GetType( );
	auto Parent         = GetParent( );
	if ( Parent != NULL ) {
		if ( ( typeOfThisItem == IT_DRIVE ) || ( typeOfThisItem == IT_FILESFOLDER ) && ( Parent->GetType( ) == IT_DRIVE ) ) {
			path += _T( "\\" );
			}
		}
	else {
		if ( ( typeOfThisItem == IT_DRIVE ) || ( typeOfThisItem == IT_FILESFOLDER ) ) {//ehh TODO: does this make sense?
			path += _T( "\\" );
			}
		}
	return path;
	}

bool CItem::HasUncPath( ) const {
	CString path = GetPath( );
	return ( path.GetLength( ) >= 2 && path.Left( 2 ) == _T( "\\\\" ) );
	}

CString CItem::GetFindPattern( ) const {
	CString pattern = GetPath();
	if ( pattern.Right( 1 ) != _T( '\\' ) ) {
		pattern += _T( "\\*.*" );
		}
	else {
		pattern += _T( "*.*" );//Yeah, if you're wondering, `*.*` works for files WITHOUT extensions.
		}
	return pattern;
	}

CString CItem::GetFolderPath( ) const {
	/*
	  Returns the path for "Explorer here" or "Command Prompt here"
	*/
	CString path;
	auto typeOfThisItem = GetType( );
	if ( typeOfThisItem == IT_MYCOMPUTER ) {
		path = GetParseNameOfMyComputer( );
		}
	else {
		path = GetPath( );
		if ( typeOfThisItem == IT_FILE ) {
			INT i = path.ReverseFind( _T( '\\' ) );
			ASSERT( i != -1 );
			path = path.Left( i + 1 );
			}
		}
	return path;
	}

CString CItem::GetName( ) const {
	return m_name;
	}

CString CItem::GetExtension( ) const {
	//INSIDE this function, CAfxStringMgr::Allocate	(f:\dd\vctools\vc7libs\ship\atlmfc\src\mfc\strcore.cpp:141) DOMINATES execution!!//TODO: FIXME: BUGBUG!

	CString ext;

	switch ( GetType( ) )
	{
		case IT_FILE:
			{
				auto thePath = LPCTSTR( GetName( ) );
				auto ptstrPath = ( PTSTR( thePath ) );
				auto resultPtrStr = PathFindExtension( ptstrPath );
				if ( resultPtrStr != '\0' ) {
					ext = resultPtrStr;
					//ext.MakeLower( );
					return ext;
					}
				INT i = GetName( ).ReverseFind( _T( '.' ) );
				if ( i == -1 ) {
					ext = _T( "." );
					}
				else {
					ext = GetName( ).Mid( i );
					}
				ext.MakeLower( );//slower part?
				break;
			}
		case IT_FREESPACE:
		case IT_UNKNOWN:
			ext = GetName( );
			break;

		default:
			AfxCheckMemory( );
			ASSERT(false);
	}
	return ext;
	}

LONGLONG CItem::GetFilesCount( ) const {
	return m_files;
	}

LONGLONG CItem::GetSubdirsCount( ) const {
	return m_subdirs;
	}

LONGLONG CItem::GetItemsCount( ) const {
	return m_files + m_subdirs;
	}

bool CItem::IsReadJobDone( ) const {
	return m_readJobDone;
	}

void CItem::SetReadJobDone( _In_ const bool done ) {
	if ( !m_readJobDone && done ) {
		UpwardAddReadJobs( -1 );
		}
	else {
		UpwardAddReadJobs( 1 - m_readJobs );
		}
	m_readJobDone = done;
	}

bool CItem::IsDone( ) const {
	return m_done; 
	}

void CItem::SetDone( ) {
	if ( m_done ) {
		return;
		}
	if ( GetType( ) == IT_DRIVE ) {
		//UpdateFreeSpaceItem();
		if ( GetDocument( )->OptionShowUnknown( ) ) {
			CItem *unknown = FindUnknownItem( );
			if ( unknown != NULL ) {
				if ( !( unknown->GetType( ) == IT_DIRECTORY ) ) {
					LONGLONG total = 0;
					LONGLONG free = 0;
					auto thisPath = GetPath( );
					MyGetDiskFreeSpace( thisPath, total, free );//redundant?

					LONGLONG unknownspace = total - GetSize( );

					if ( !GetDocument( )->OptionShowFreeSpace( ) ) {
						unknownspace -= free;
						}

					// For CDs, the GetDiskFreeSpaceEx()-function is not correct.
					if ( ( unknownspace < 0 ) || ( free < 0 ) || ( total < 0 ) ) {
						TRACE( _T( "GetDiskFreeSpace(%s), (unknownspace: %lld), (free: %lld), (total: %lld) incorrect.\r\n" ), thisPath, unknownspace, free, total );
						unknownspace = 0;
						}
					unknown->SetSize( unknownspace );
					UpwardAddSize( unknownspace );
					}
				}
			}
		}

#ifndef CHILDVEC
	qsort( m_children.GetData( ), m_children.GetSize( ), sizeof( CItem * ), &_compareBySize );
#else
	std::sort( m_vectorOfChildren.begin( ), m_vectorOfChildren.end( ) );
#endif
	m_rect.bottom = NULL;
	m_rect.left   = NULL;
	m_rect.right  = NULL;
	m_rect.top    = NULL;
	m_done = true;
	}

unsigned long long CItem::GetTicksWorked( ) const {
	return m_ticksWorked; 
	}

void CItem::AddTicksWorked( _In_ const unsigned long long more ) {
	ASSERT( more >= 0 );
	m_ticksWorked += more; 
	}

void CItem::FindFilesLoop( _In_ const unsigned long long ticks, _In_ unsigned long long start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files ) {
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetFindPattern( ) );
	while ( b ) {
		b = finder.FindNextFile();
		if ( finder.IsDots( ) ) {
			continue;
			}
		if ( finder.IsDirectory( ) ) {
			dirCount++;
			AddDirectory( std::move( finder ) );
			}
		else {
			fileCount++;
			FILEINFO fi;
			fi.name = finder.GetFileName( );
			fi.attributes = finder.GetAttributes( );
			if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
				fi.length = finder.GetCompressedLength( ); // Retrieve file size //MAYBE GetFileInformationByHandleEx would be faster?
				}
			else {

#ifdef _DEBUG
				if ( !( finder.GetLength( ) == finder.GetCompressedLength( ) ) ) {
					static_assert(sizeof(unsigned long long) == 8, "bad format specifiers!");
					TRACE( _T( "GetLength: %llu != GetCompressedLength: %llu !!! Path: %s\r\n" ), finder.GetLength( ), finder.GetCompressedLength( ), finder.GetFilePath( ) );
					}
#endif
				fi.length = finder.GetLength( );//temp
				}
			finder.GetLastWriteTime( &fi.lastWriteTime );
			// (We don't use GetLastWriteTime(CTime&) here, because, if the file has an invalid timestamp, that function would ASSERT and throw an Exception.)
			
			files.emplace_back( std::move( fi ) );

			}
		if ( ( GetTickCount64( ) - start ) > ticks && ( GetTickCount64( ) % 1000 ) == 0 ) {
			DriveVisualUpdateDuringWork( );
			TRACE( _T( "Exceeding number of ticks! (%llu > %llu)\r\npumping messages - this is a dirty hack to ensure responsiveness while single-threaded.\r\n" ), (GetTickCount64() - start), ticks );
			}
		}	

	}
void CItem::readJobNotDoneWork( _In_ const unsigned long long ticks, _In_ unsigned long long start ) {
	LONGLONG dirCount  = 0;
	LONGLONG fileCount = 0;
	std::vector<FILEINFO> vecFiles;

	FindFilesLoop( ticks, start, dirCount, fileCount, vecFiles );
	CItem* filesFolder = NULL;
	if ( dirCount > 0 && fileCount > 1 ) {
		filesFolder = new CItem { IT_FILESFOLDER, _T( "<Files>" ) };
		filesFolder->SetReadJobDone( );
		AddChild( filesFolder );
		}
	else if ( fileCount > 0 ) {
		filesFolder = this;
		}
	for ( auto& aFile : vecFiles ) {
		filesFolder->AddFile( aFile );
		}
	if ( filesFolder != NULL ) {
		filesFolder->UpwardAddFiles( fileCount );
		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder->SetDone( );
			}
		}
	UpwardAddSubdirs( dirCount );
	SetReadJobDone( );
	AddTicksWorked( GetTickCount64( ) - start );
	}

void CItem::StillHaveTimeToWork( _In_ const unsigned long long ticks, _In_ unsigned long long start ) {
	while ( GetTickCount64( ) - start < ticks ) {
		unsigned long long minticks = UINT_MAX;
		CItem *minchild = NULL;
		auto countOfChildren = GetChildrenCount( );
		for ( INT i = 0; i < countOfChildren; i++ ) {
			auto child = GetChildGuaranteedValid( i );
			if ( child->IsDone( ) ) {
				continue;
				}
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

void CItem::DoSomeWork( _In_ const unsigned long long ticks ) {
	ASSERT( ticks >= 0 );
	if ( m_done ) {
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

bool CItem::StartRefreshIT_MYCOMPUTER( ) {
	m_lastChange.dwHighDateTime = 0;
	m_lastChange.dwLowDateTime = 0;
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		auto Child = GetChildGuaranteedValid( i );
		Child->StartRefresh( );
		}
	return true;
	}

bool CItem::StartRefreshIT_FILESFOLDER( _In_ bool wasExpanded ) {
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetFindPattern( ) );
	while (b) {
		b = finder.FindNextFile( );
		if ( finder.IsDirectory( ) ) {
			continue;
			}

		FILEINFO fi;
		fi.name = finder.GetFileName( );
		fi.attributes = finder.GetAttributes( );

		// Retrieve file size
		fi.length = finder.GetCompressedLength( );
		finder.GetLastWriteTime( &fi.lastWriteTime );

		AddFile( std::move( fi ) );
		UpwardAddFiles( 1 );
		}
	SetDone();
	if ( wasExpanded ) {
		auto TreeListControl = GetTreeListControl( );
		if ( TreeListControl != NULL ) {
			TreeListControl->ExpandItem( this );
			}
		}
	return true;

	}

bool CItem::StartRefreshIT_FILE( ) {
	CFileFindWDS finder;
	BOOL b = finder.FindFile( GetPath( ) );
	if ( b ) {
		finder.FindNextFile( );
		if (!finder.IsDirectory()) {
			FILEINFO fi;
			fi.name = finder.GetFileName( );
			fi.attributes = finder.GetAttributes( );

			// Retrieve file size
			fi.length = finder.GetCompressedLength( );
			finder.GetLastWriteTime( &fi.lastWriteTime );

			SetLastChange( fi.lastWriteTime );

			UpwardAddSize( fi.length );
			UpwardUpdateLastChange( GetLastChange( ) );
			auto Parent = GetParent( );
			if ( Parent != NULL ) {
				Parent->UpwardAddFiles( 1 );
				}
			}
		}
	SetDone( );
	return true;
	}

bool CItem::StartRefreshIsDeleted( _In_ ITEMTYPE typeOf_thisItem ) {
	bool deleted = false;
	if ( typeOf_thisItem == IT_DRIVE ) {
		deleted = !DriveExists( GetPath( ) );
		}
	else if ( typeOf_thisItem == IT_FILE ) {
		deleted = !FileExists( GetPath( ) );
		}
	else if ( typeOf_thisItem == IT_DIRECTORY ) {
		deleted = !FolderExists( GetPath( ) );
		}
	return deleted;
	}

void CItem::StartRefreshHandleDeletedItem( ) {
	auto myParent_here = GetParent( );
	if ( myParent_here == NULL ) {
		GetDocument( )->UnlinkRoot( );
		}
	else {
		myParent_here->UpwardRecalcLastChange( );
		auto myParent_IndexOfME = myParent_here->FindChildIndex( this );
		myParent_here->RemoveChild( myParent_IndexOfME );// --> delete this
		}

	}

void CItem::StartRefreshRecreateFSandUnknw( ) {
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

void CItem::StartRefreshHandleWasExpanded( ) {
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->ExpandItem( this );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );//What the fuck would this even mean??
		}
	}

void CItem::StartRefreshUpwardClearItem( _In_ ITEMTYPE typeOf_thisItem ) {
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

	UpwardAddSize( -GetSize( ) );
	ASSERT( GetSize( ) == 0 );

	}

_Must_inspect_result_ bool CItem::StartRefreshIsMountOrJunction( _In_ ITEMTYPE typeOf_thisItem ) {
	/*
	  Was refactored from LARGER function. A return true from this function indicates that the caller should return true
	*/
	auto Options = GetOptions( );
	auto App = GetApp( );
	if ( Options != NULL ) {
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
		}
	else {
		//Fall back to values that I like :)
		if ( typeOf_thisItem == IT_DIRECTORY ) {
			if ( !IsRootItem( ) ) {
				if ( App->IsMountPoint( GetPath( ) ) ) {
					return true;
					}
				if ( App->IsJunctionPoint( GetPath( ) ) ) {
					return true;
					}
				}
			}
		}
	return false;
	}

bool CItem::StartRefresh( ) {
	/*
	  Returns false if deleted
	*/
	m_ticksWorked = 0;

	auto typeOf_thisItem = GetType( );

	ASSERT( ( typeOf_thisItem != IT_FREESPACE ) && ( typeOf_thisItem != IT_UNKNOWN ) );
	ASSERT( typeOf_thisItem == IT_FILE || typeOf_thisItem == IT_DRIVE || typeOf_thisItem == IT_DIRECTORY || typeOf_thisItem == IT_FILESFOLDER );

	// Special case IT_MYCOMPUTER
	if ( typeOf_thisItem == IT_MYCOMPUTER ) {
		return StartRefreshIT_MYCOMPUTER( );
		}

	bool wasExpanded = IsVisible( ) && IsExpanded( );

	auto oldScrollPosition = 0;
	if ( IsVisible( ) ) {
		oldScrollPosition = GetScrollPosition( );
		ASSERT( oldScrollPosition >= 0 );
		}

	UncacheImage( );

	// Upward clear data
	UpdateLastChange( );
	UpwardSetUndone( );
	StartRefreshUpwardClearItem( typeOf_thisItem );

	RemoveAllChildren( );
	UpwardRecalcLastChange( );

	// Special case IT_FILESFOLDER
	if ( typeOf_thisItem == IT_FILESFOLDER ) {
		return StartRefreshIT_FILESFOLDER( wasExpanded );
		}

	if ( StartRefreshIsDeleted( typeOf_thisItem ) ) {
		StartRefreshHandleDeletedItem( );
		return false;
		}

	if ( typeOf_thisItem == IT_FILE ) {
		return StartRefreshIT_FILE( );
		}
	
	if ( StartRefreshIsMountOrJunction( typeOf_thisItem ) ) {
		//bubble the return up
		return true;
		}
	
	TRACE( _T( "Initiating re-read!\r\n" ) );
	SetReadJobDone( false );

	// Re-create <free space> and <unknown>
	if ( typeOf_thisItem == IT_DRIVE ) {
		StartRefreshRecreateFSandUnknw( );
		}
	DoSomeWork( 999 );
	if ( wasExpanded ) {
		StartRefreshHandleWasExpanded( );
		}
	if ( IsVisible( ) ) {
		SetScrollPosition( oldScrollPosition );
		}
	return true;
}

void CItem::UpwardSetUndoneIT_DRIVE( ) {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		auto thisChild = GetChildGuaranteedValid( i );
		auto childType = thisChild->GetType( );
		if ( ( childType == IT_UNKNOWN ) || ( childType == IT_DIRECTORY ) ) {
			break;
			}
		auto unknown = thisChild;
		UpwardAddSize( -unknown->GetSize( ) );
		unknown->SetSize( 0 );
		}
	}

void CItem::UpwardParentSetUndone( ) {
	auto Parent = GetParent( );
	if ( Parent != NULL ) {
		Parent->UpwardSetUndone( );
		}
	}

void CItem::UpwardSetUndone( ) {
	auto thisItemType = GetType( );
	if ( thisItemType == IT_DIRECTORY ) {
		}
	else {
		auto Document = GetDocument( );
		if ( Document != NULL ) {
			if ( thisItemType == IT_DRIVE && IsDone( ) && Document->OptionShowUnknown( ) ) {
				UpwardSetUndoneIT_DRIVE( );
				}
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
		}
		m_done = false;
		UpwardParentSetUndone( );
	}

void CItem::CreateFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	LONGLONG total  = 0;
	LONGLONG freeSp = 0;

	TRACE( _T( "MyGetDiskFreeSpace\r\n" ) );
	MyGetDiskFreeSpace( GetPath( ), total, freeSp );
	auto freespace = new CItem( IT_FREESPACE, GetFreeSpaceItemName( ) );//std::make_shared<CItem>
	freespace->SetSize( freeSp );
	freespace->SetDone( );
	AddChild( freespace );
	}

_Success_(return != NULL) _Must_inspect_result_ CItem *CItem::FindFreeSpaceItem( ) const {
	INT i = FindFreeSpaceItemIndex( );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}


void CItem::UpdateFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	if ( !GetDocument( )->OptionShowFreeSpace( ) ) {
		return;
		}
	auto freeSpaceItem = FindFreeSpaceItem( );
	if ( freeSpaceItem != NULL ) {
		LONGLONG total = 0;
		LONGLONG free  = 0;
		TRACE( _T( "MyGetDiskFreeSpace, path: %s\r\n" ), GetPath( ) );
		MyGetDiskFreeSpace( GetPath( ), total, free );
		LONGLONG before = freeSpaceItem->GetSize( );
		LONGLONG diff  = free - before;
		freeSpaceItem->UpwardAddSize( diff );
		ASSERT( freeSpaceItem->GetSize( ) == free );
		}
	}

void CItem::RemoveFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto i = FindFreeSpaceItemIndex( );
	ASSERT( i < GetChildrenCount( ) );
	if ( i < GetChildrenCount( ) ) {
		auto freespace = GetChildGuaranteedValid( i );
		UpwardAddSize( -freespace->GetSize( ) );
		RemoveChild( i );
		}
	}

void CItem::CreateUnknownItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto unknown = new CItem { IT_UNKNOWN, GetUnknownItemName( ) };//std::make_shared<CItem>
	unknown->SetDone( );
	AddChild( unknown );
	}

_Success_(return != NULL) _Must_inspect_result_ CItem* CItem::FindUnknownItem( ) const {
	auto i = FindUnknownItemIndex( );
	ASSERT( i >= 0 );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}

void CItem::RemoveUnknownItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );

	UpwardSetUndone( );

	auto i = FindUnknownItemIndex( );
	ASSERT( i < GetChildrenCount( ) );
	if ( i < GetChildrenCount( ) ) {
		auto unknown = GetChildGuaranteedValid( i );
		UpwardAddSize( -unknown->GetSize( ) );
		RemoveChild( i );

		}
	}

_Success_(return != NULL) _Must_inspect_result_ CItem *CItem::FindDirectoryByPath( _In_ const CString& path ) {
	AfxCheckMemory( );
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
	for ( i = 0; i < thisChildCount; i++ ) {
		auto Child = GetChildGuaranteedValid( i );
		auto item = Child->FindDirectoryByPath( path );
		if ( item != NULL ) {
			return item;
			}

		}
	return NULL;
	}

void CItem::stdRecurseCollectExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords ) {
	auto typeOfItem = GetType( );
	if ( IsLeaf( typeOfItem ) ) {
		if ( typeOfItem == IT_FILE ) {
			auto ext = GetExtension( );
			auto location = findInVec( extensionRecords, ext );
			if ( location < extensionRecords.size( ) ) {
				extensionRecords.at( location ).bytes += GetSize( );
				++( extensionRecords.at( location ).files );
				}
			else {
				extensionRecords.emplace_back( SExtensionRecord( std::uint32_t( 1 ), COLORREF( 0 ), GetSize( ), ext ) );

 				}
			}
		}
	else {
		auto childCount = GetChildrenCount( );
		for ( INT i = 0; i < childCount; ++i ) {
			auto Child = GetChildGuaranteedValid( i );
			Child->stdRecurseCollectExtensionData( extensionRecords );
			}
		}

	}

INT __cdecl CItem::_compareBySize( _In_ const void *p1, _In_ const void *p2 ) {
	CItem *item1 = *( CItem ** ) p1;
	CItem *item2 = *( CItem ** ) p2;
	LONGLONG size1 = item1->GetSize( );
	LONGLONG size2 = item2->GetSize( );

	// TODO: Use 2nd sort column (as set in our TreeListView?)
	return signum( size2 - size1 ); // biggest first
	}

LONGLONG CItem::GetProgressRangeMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG range = 0;
	auto childCountHere = GetChildrenCount( );
	for ( INT i = 0; i < childCountHere; i++ ) {
		auto Child = GetChildGuaranteedValid( i );
		range += Child->GetProgressRangeDrive( );
		}
	return range;
	}

LONGLONG CItem::GetProgressPosMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG pos = 0;
	auto childCountHere = GetChildrenCount( );
	for (INT i = 0; i < childCountHere; i++) {
		auto Child = GetChildGuaranteedValid( i );
		pos += Child->GetProgressPosDrive( );	
		}
	return pos;
	}

LONGLONG CItem::GetProgressRangeDrive( ) const {
	auto Doc     = GetDocument( );
	auto total   = Doc->GetTotlDiskSpace( GetPath( ) );
	auto freeSp  = Doc->GetFreeDiskSpace( GetPath( ) );
	auto range   = total - freeSp;
	ASSERT( range >= 0 );
	return range;
	}

LONGLONG CItem::GetProgressPosDrive( ) const {
	auto pos = GetSize( );
	auto fs = FindFreeSpaceItem( );
	if ( fs != NULL ) {
		pos -= fs->GetSize( );
		}
	return pos;
	}

COLORREF CItem::GetGraphColor( ) const {
	COLORREF color;

	switch ( GetType() )
	{
		case IT_UNKNOWN:
			color = RGB( 255, 255, 0 ) | CTreemap::COLORFLAG_LIGHTER;
			break;

		case IT_FREESPACE:
			color = RGB( 100, 100, 100 ) | CTreemap::COLORFLAG_DARKER;
			break;

		case IT_FILE:
			color = GetDocument( )->GetCushionColor( GetExtension( ) );
			break;

		default:
			color = RGB( 0, 0, 0 );
			break;
	}
	return std::move( color );
	}

bool CItem::MustShowReadJobs( ) const {
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		return !myParent->IsDone( );
		}
	else {
		return !IsDone( );
		}
	}

COLORREF CItem::GetPercentageColor( ) const {
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		auto i = GetIndent( ) % Options->GetTreelistColorCount( );
		return std::move( Options->GetTreelistColor( i ) );
		}
	else {

		ASSERT( false );//should never ever happen, but just in case, we'll generate a random color.
		DWORD fakeColor = 0;
		fakeColor = ( DWORD ) rand( );
		return std::move( fakeColor );
		}
	}

INT CItem::FindFreeSpaceItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChild( i )->GetType( ) == IT_FREESPACE ) {
			//break;
			return i; // maybe == GetChildrenCount() (=> not found)
			}
		}
	return childCount;
	}

INT CItem::FindUnknownItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChild( i )->GetType( ) == IT_UNKNOWN ) {
			//break;
			return i; // maybe == GetChildrenCount() (=> not found)
			}	
		}
	return childCount;
	}

CString CItem::UpwardGetPathWithoutBackslash( ) const {
	CString path;
	auto myParent = GetParent( );
	if ( myParent != NULL ) {
		path = myParent->UpwardGetPathWithoutBackslash( );
		}
	switch (GetType())
	{
		case IT_MYCOMPUTER:
			// empty
			break;

		case IT_DRIVE:
			// (we don't use our parent's path here.)
			path = PathFromVolumeName(m_name);
			break;

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
			break;

		case IT_FREESPACE:
		case IT_UNKNOWN:
			break;

		default:
			ASSERT(false);
	}
	return path; 
	}

void CItem::AddDirectory( _In_ const CFileFindWDS& finder ) {
	ASSERT( &finder != NULL );
	auto thisApp      = GetApp( );
	auto thisFilePath = finder.GetFilePath( );
	auto thisOptions  = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow   = thisApp->IsMountPoint( thisFilePath ) && !thisOptions->IsFollowMountPoints( );
	
	dontFollow       |= thisApp->IsJunctionPoint( thisFilePath, finder.GetAttributes( ) ) && !thisOptions->IsFollowJunctionPoints( );
	CItem *child      = new CItem{ IT_DIRECTORY, finder.GetFileName( ), dontFollow };
	
	FILETIME t;
	finder.GetLastWriteTime( &t );
	child->SetLastChange( t );
	child->SetAttributes( finder.GetAttributes( ) );
	AddChild( child );
	}

void CItem::AddFile( _In_ const FILEINFO& fi ) {
	ASSERT( &fi != NULL );
	CItem* child = new CItem { IT_FILE, fi.name };
	child->SetSize( fi.length );
	child->SetLastChange( fi.lastWriteTime );
	child->SetAttributes( fi.attributes );
	child->SetDone( );
	AddChild( child );
	}

void CItem::DriveVisualUpdateDuringWork( ) {
	MSG msg;
	while ( PeekMessage( &msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE ) ) {
		DispatchMessage( &msg );
		}
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
