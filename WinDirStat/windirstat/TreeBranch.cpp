#pragma once 

#include "stdafx.h"
#include "TreeBranch.h"

namespace {
	CString GetUnknownItemName() { return LoadString(IDS_UNKNOWN_ITEM); }
	CString GetFreeSpaceItemName() 	{ return LoadString(IDS_FREESPACE_ITEM); }
	}

CTreeBranch::CTreeBranch( ITEMTYPE type, LPCTSTR name, bool dontFollow ) : CItem( type, name, dontFollow )/* m_files( 0 ), m_subdirs( 0 ), m_done( false ), m_ticksWorked( 0 ), m_readJobs( 0 ) */ {
	auto thisItem_type = GetType( );

	if ( thisItem_type == IT_FILE || dontFollow || thisItem_type == IT_FREESPACE || thisItem_type == IT_UNKNOWN || thisItem_type == IT_MYCOMPUTER ) {
		SetReadJobDone( );
		m_readJobs = 0;
		}
	else if ( thisItem_type == IT_DIRECTORY || thisItem_type == IT_DRIVE || thisItem_type == IT_FILESFOLDER ) {
		m_readJobDone = false;
		}

	if ( thisItem_type == IT_DRIVE ) {
		m_name = FormatVolumeNameOfRootPath( m_name );
		}

	}

CTreeBranch::~CTreeBranch( ) {
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
				ASSERT( Document->GetZoomItem( ) != dynamic_cast<CTreeBranch*>(m_children[ i ]) );
				}
			if ( m_children[ i ] == currentRootItem ) {
				Document->clearRootItem( );
				ASSERT( Document->GetRootItem( ) != dynamic_cast<CTreeBranch*>(m_children[ i ]) );
				}
			if ( m_children[ i ] == currentlySelectedItem ) {
				Document->clearSelection( );
				ASSERT( Document->GetSelection( ) != dynamic_cast<CTreeBranch*>(m_children[ i ])  );
				}
			m_children[ i ] = NULL;//sensible?
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
		}
#else
	for (auto& aChild : m_children){
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


//CTreeBranch::CTreeBranch( CTreeBranch&& in ) {
//	m_type                 = std::move( in.m_type );
//	m_name                 = std::move( in.m_name );
//	m_size                 = std::move( in.m_size );
//	m_files                = std::move( in.m_files );
//	m_subdirs              = std::move( in.m_subdirs );
//	m_lastChange           = std::move( in.m_lastChange );
//	m_attributes           = std::move( in.m_attributes );
//	m_readJobDone          = in.m_readJobDone;
//	m_done                 = in.m_done;
//	m_ticksWorked          = std::move( in.m_ticksWorked );
//	m_readJobs             = std::move( in.m_readJobs );
//	//m_children             = in.m_children;
//#ifdef CHILDVEC
//	m_vectorOfChildren     = std::move( in.m_vectorOfChildren );
//#endif
//	m_rect                 = std::move( in.m_rect );
//	}
bool CTreeBranch::DrawSubitem( _In_ const INT subitem, _In_ CDC* pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const {
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

void CTreeBranch::GetTextCOL_SUBTREEPERCENTAGE( _Inout_ CString& s ) const {
	if ( IsDone( ) ) {
		ASSERT( m_readJobs == 0 );//s = "ok";
		auto Type_ThisItem = GetType( );
		ASSERT( m_readJobDone == ( m_readJobs == 0 ) || ( Type_ThisItem == IT_DIRECTORY ) || ( Type_ThisItem == IT_FILESFOLDER ) || ( Type_ThisItem == IT_DRIVE ) );
		}
	else {
		if ( m_readJobs == 1 ) {
			ASSERT( !m_readJobDone );
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
void CTreeBranch::GetTextCOL_PERCENTAGE( _Inout_ CString& s ) const {
	if ( GetOptions( )->IsShowTimeSpent( ) && MustShowReadJobs( ) || IsRootItem( ) ) {
		s.Format( _T( "[%s s]" ), FormatMilliseconds( GetTicksWorked( ) ).GetString( ) );
		}
	else {
		s.Format( _T( "%s%%" ), FormatDouble( GetFraction( ) * 100 ).GetString( ) );
		}
	}

void CTreeBranch::GetTextCOL_ITEMS( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetItemsCount( ) );
		}
	}

void CTreeBranch::GetTextCOL_FILES( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetFilesCount( ) );
		}
	}

void CTreeBranch::GetTextCOL_SUBDIRS( _Inout_ CString& s ) const {
	auto typeOfItem = GetType( );
	if ( typeOfItem != IT_FILE && typeOfItem != IT_FREESPACE && typeOfItem != IT_UNKNOWN ) {
		s = FormatCount( GetSubdirsCount( ) );
		}
	}

CString CTreeBranch::GetText( _In_ const INT subitem ) const {
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


INT CTreeBranch::CompareSubTreePercentage( _In_ const CTreeBranch* other ) const {
	if ( MustShowReadJobs( ) ) {
		return signum( m_readJobs - other->m_readJobs );
		}

	else {
		return signum( GetFraction( ) - other->GetFraction( ) );
		}
	}

INT CTreeBranch::CompareSibling( _In_ const CTreeListItem *tlib, _In_ const INT subitem ) const {
	const auto other = dynamic_cast< const CTreeBranch* >( tlib );
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


INT_PTR CTreeBranch::GetChildrenCount( ) const {
	return m_children.GetSize();//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
	}


#ifdef CHILDVEC
size_t CTreeBranch::GetChildVecCount( ) const {
	return m_vectorOfChildren.size( );
	}
#endif

_Must_inspect_result_ CTreeListItem *CTreeBranch::GetTreeListChild( _In_ const INT i ) const {
#ifdef CHILDVEC
	ASSERT( ! (m_children.size() == 0));
#else
	ASSERT( !( m_children.IsEmpty( ) ) );
#endif
	ASSERT( i >= 0 );
	return m_children[ i ];
	}


LONGLONG CTreeBranch::GetProgressRange( ) const {
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

LONGLONG CTreeBranch::GetProgressRange( ) const {
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

_Success_(return != NULL) _Must_inspect_result_ CItem *CTreeBranch::GetChild(_In_ const INT i) const {
	/*
	  Returns CItem* to child if passed a valid index. Returns NULL if `i` is NOT a valid index. 
	*/
#ifdef CHILDVEC
	ASSERT( ! (m_children.size() == 0));
#else

	ASSERT( !( m_children.IsEmpty( ) ) );
#endif
	ASSERT( i >= 0 && i <= ( m_children.GetSize( ) - 1 ) );
	if ( i >= 0 && i <= ( m_children.GetSize( ) -1 ) ) {
		return m_children[ i ];
		}
	else {
		return NULL;
		}
	}

_Success_( return != NULL ) CItem* CTreeBranch::GetChildGuaranteedValid( _In_ const INT_PTR i ) const {
#ifdef CHILDVEC
	ASSERT( ! (m_children.size() == 0));
#else
	ASSERT( !( m_children.IsEmpty( ) ) );
#endif
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

INT_PTR CTreeBranch::FindChildIndex( _In_ const CItem* child ) const {
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

void CTreeBranch::AddChild( _In_ CItem* child ) {
	ASSERT( child != NULL );

	ASSERT( !IsDone( ) ); // SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItem::GetFraction() could ASSERT.
	UpwardAddSize         ( child->GetSize( ) );
	UpwardUpdateLastChange( child->GetLastChange( ) );

#ifndef CHILDVEC
	m_children.Add( child );
#else
	m_children.emplace_back(child);
#endif
	ASSERT( this != NULL );
	child->SetParent( this );
	ASSERT( dynamic_cast<CTreeBranch*>(child->GetParent( )) == this );
	auto TreeListControl = GetTreeListControl( );
	if ( TreeListControl != NULL ) {
		TreeListControl->OnChildAdded( this, child, IsDone( ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CTreeBranch::AddChild( _In_ CTreeBranch* child ) {
	ASSERT( child != NULL );

	ASSERT( !IsDone( ) ); // SetDone() computed m_childrenBySize

	// This sequence is essential: First add numbers, then CTreeListControl::OnChildAdded(), because the treelist will display it immediately. If we did it the other way round, CItem::GetFraction() could ASSERT.
	UpwardAddSize         ( child->GetSize( ) );
	UpwardAddReadJobs     ( child->GetReadJobs( ) );
	UpwardUpdateLastChange( child->GetLastChange( ) );

#ifndef CHILDVEC
	m_children.Add( child );
#else
	m_children.emplace_back(child);
#endif
	ASSERT( this != NULL );
	child->SetParent( this );
	ASSERT( child->GetParent( ) == this );
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
void CTreeBranch::AddChildToVec( _In_ CItem& child ) {
	m_children.emplace_back( std::move( child ) );
	}
#endif

void CTreeBranch::RemoveChild(_In_ const INT_PTR i) {
#ifdef CHILDVEC
	ASSERT( ! (m_children.size() == 0));
#else
	ASSERT( !( m_children.IsEmpty( ) ) );
#endif
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
void CTreeBranch::RemoveAllChildren() {
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

void CTreeBranch::UpwardAddSubdirs( _In_ const std::int64_t dirCount ) {
	if ( dirCount < 0 ) {
		if ( ( dirCount + m_subdirs ) < 0 ) {
			m_subdirs = 0;
			}
		else {
			m_subdirs -= std::uint32_t( dirCount * ( -1 ) );
			}
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddSubdirs( dirCount );
			}
		}
	else {
		m_subdirs += std::uint32_t( dirCount );
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddSubdirs( dirCount );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}


void CTreeBranch::UpwardAddFiles( _In_ const std::int64_t fileCount ) {
	if ( fileCount < 0 ) {
		if ( ( m_files + fileCount ) < 0 ) {
			m_files = 0;
			}
		else {
			m_files -= std::uint32_t( fileCount * ( -1 ) );
			}
		auto theParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( theParent != NULL ) {
			theParent->UpwardAddFiles( fileCount );
			}
		}
	else {
		m_files += std::uint32_t( fileCount );
		auto theParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( theParent != NULL ) {
			theParent->UpwardAddFiles( fileCount );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CTreeBranch::UpwardAddSize( _In_ const std::int64_t bytes ) {
	ASSERT( bytes >= 0 || bytes == -GetSize( ) || bytes >= ( -1 * ( GetTotalDiskSpace( this->UpwardGetPathWithoutBackslash( ) ) ) ) );
	if ( bytes < 0 ) {
		if ( ( bytes + m_size ) < 0 ) {
			//std::int64_t newCount = std::int64_t( ( m_size + bytes ) * ( -1 ) );
			m_size = 0;
			}
		else {
			m_size -= std::uint32_t( bytes * ( -1 ) );
			}
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		}
	else {
		m_size += std::uint32_t( bytes );
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddSize( bytes );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}

void CTreeBranch::UpwardAddReadJobs( _In_ const /* signed */std::int64_t count ) {
	if ( count < 0 ) {
		if ( count < 0 ) {
			ASSERT( m_readJobs == 0 );
			}
		if ( ( m_readJobs + count ) < 0 ) {
			//std::int64_t newCount = std::int64_t( ( std::int64_t( m_readJobs ) + count ) * ( -1 ) );
			m_readJobs = 0;
			}
		else {
			m_readJobs -= std::uint32_t( count * ( -1 ) );

			}
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddReadJobs( count );
			}
		}
	else {
		m_readJobs += std::uint32_t( count );

		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardAddReadJobs( count );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	if ( m_readJobs <= 0 ) {
		ASSERT( m_readJobs == 0 );
		ASSERT( m_readJobDone || ( GetType( ) == IT_DIRECTORY ) || ( GetType( ) == IT_FILESFOLDER ) || ( GetType( ) == IT_DRIVE ) );
		}
	}

void CTreeBranch::UpwardUpdateLastChange(_In_ const FILETIME& t) {
	/*
	  This method increases the last change
	*/
	if ( m_lastChange < t ) {
		m_lastChange = t;
		auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
		if ( myParent != NULL ) {
			myParent->UpwardUpdateLastChange( t );
			}
		else {
			//Valid condition? `this` may be the root item.
			}
		}
	}


void CTreeBranch::UpwardRecalcLastChange() {
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
	auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
	if ( myParent != NULL ) {
		myParent ->UpwardRecalcLastChange( );
		}
	else {
		//Valid condition? `this` may be the root item.
		}
	}

LONGLONG CTreeBranch::GetReadJobs() const {
	if ( m_readJobs == 0 ) {
		ASSERT( m_readJobDone || ( GetType( ) == IT_DIRECTORY ) || ( GetType( ) == IT_FILESFOLDER ) || ( GetType( ) == IT_DRIVE ) );
		}
	ASSERT( m_readJobs >= 0 );
	return m_readJobs;
	}

bool CTreeBranch::IsRootItem( ) const {
	return ( ( m_type & ITF_ROOTITEM ) != 0 );
	}

LONGLONG CTreeBranch::GetFilesCount( ) const {
	return m_files;
	}

LONGLONG CTreeBranch::GetSubdirsCount( ) const {
	return m_subdirs;
	}

LONGLONG CTreeBranch::GetItemsCount( ) const {
	if ( GetType( ) == IT_FILE ) {
		ASSERT( m_subdirs == 0 );
		}
	return m_files + m_subdirs;
	}

inline bool CTreeBranch::IsReadJobDone( ) const {
#ifdef _DEBUG
	auto Type_ThisItem = GetType( );
	ASSERT( ( m_readJobs == 0 ) || ( Type_ThisItem == IT_DIRECTORY ) || ( Type_ThisItem == IT_FILESFOLDER ) || ( Type_ThisItem == IT_DRIVE ) );
	if ( Type_ThisItem == IT_FILE ) {
		ASSERT( m_readJobs == 0 );
		ASSERT( m_subdirs == 0 );
		}
#endif
	return m_readJobDone;
	}


void CTreeBranch::SetReadJobDone( _In_ const bool done /* in header, defaults to true */ ) {
	if ( m_readJobs == 0 ) {
		ASSERT( m_readJobDone || done );
		}
#ifdef DO_NOT_STORE_READJOBDONE_BOOL
	if ( !( IsReadJobDone( ) ) && done ) {
		m_readJobs = 0;
		UpwardAddReadJobs( -1 );
		}
	else {
		UpwardAddReadJobs( 1 - m_readJobs );
		}
#else
	if ( !m_readJobDone && done ) {
		m_readJobs = 0;
		UpwardAddReadJobs( -1 );
		}
	else {
		if ( done ) {
			ASSERT( m_readJobDone );
			ASSERT( m_readJobs == 0 );
			}
		if ( !m_readJobDone ) {
			ASSERT( done );
			}
		if ( m_readJobDone && done ) {
			ASSERT( m_readJobs == 0 );
			}
		UpwardAddReadJobs( 1 - m_readJobs );
		}
	m_readJobDone = done;

	if ( m_readJobs == 0 ) {
		ASSERT( m_readJobDone );
		}
#endif
	}

bool CTreeBranch::IsDone( ) const {
	return m_done; 
	}

void CTreeBranch::SetDone( ) {
	if ( m_done ) {
		ASSERT( IsReadJobDone() && ( m_readJobs == 0 ) );
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
	std::sort( m_children.begin( ), m_children.end( ) );
#endif
	m_rect.bottom = NULL;
	m_rect.left   = NULL;
	m_rect.right  = NULL;
	m_rect.top    = NULL;
	m_done = true;
	}

std::uint64_t CTreeBranch::GetTicksWorked( ) const {
	ASSERT( GetType( ) != IT_FILE );
	return m_ticksWorked; 
	}

void CTreeBranch::AddTicksWorked( _In_ const std::uint64_t more ) {
	ASSERT( ( more >= 0 ) && ( more < UINT32_MAX ) && ( ( more + m_ticksWorked ) < UINT32_MAX ) );
	if ( ( more + std::uint64_t( m_ticksWorked ) ) > UINT32_MAX ) {
		MessageBox( NULL, NULL, TEXT( "This item has been working for MORE ticks than can be represented by an unsigned 32 bit integer! Something is very wrong!" ), MB_OK );
		ASSERT( false );
		}
	m_ticksWorked += std::uint32_t( more );;
	}

void CTreeBranch::FindFilesLoop( _In_ const unsigned long long ticks, _In_ unsigned long long start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files ) {
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
			GetApp( )->PeriodicalUpdateRamUsage( );
			}
		}	

	}

void CTreeBranch::readJobNotDoneWork( _In_ const unsigned long long ticks, _In_ unsigned long long start ) {
	LONGLONG dirCount  = 0;
	LONGLONG fileCount = 0;
	std::vector<FILEINFO> vecFiles;

	FindFilesLoop( ticks, start, dirCount, fileCount, vecFiles );
	CTreeBranch* filesFolder = NULL;
	if ( dirCount > 0 && fileCount > 1 ) {
		filesFolder = new CTreeBranch { IT_FILESFOLDER, _T( "<Files>" ) };
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


void CTreeBranch::StillHaveTimeToWork( _In_ const unsigned long long ticks, _In_ unsigned long long start ) {
	while ( GetTickCount64( ) - start < ticks ) {
		unsigned long long minticks = UINT_MAX;
		CItem *minchild = NULL;
		auto countOfChildren = GetChildrenCount( );
		for ( INT i = 0; i < countOfChildren; i++ ) {
			auto child = GetChildGuaranteedValid( i );
			if ( !( child->TmiIsLeaf( ) ) ) {
				auto BranchChild = dynamic_cast< CTreeBranch* >( child );
				if ( BranchChild->IsDone( ) ) {
					continue;
					}
				if ( BranchChild->GetTicksWorked( ) < minticks ) {
					minticks = BranchChild->GetTicksWorked( );
					minchild = child;
					}
				}
			}
		if ( minchild == NULL ) {
			SetDone( );
			break;
			}
		auto tickssofar = GetTickCount64( ) - start;
		if ( ticks > tickssofar ) {
			if ( !( minchild->TmiIsLeaf( ) ) ) {
				dynamic_cast< CTreeBranch* >( minchild )->DoSomeWork( ticks - tickssofar );
				}
			}
		}
	}

void CTreeBranch::DoSomeWork( _In_ const unsigned long long ticks ) {
	ASSERT( ticks >= 0 );
	if ( m_done ) {
		return;
		}

	auto start = GetTickCount64( );
	auto typeOfThisItem = GetType( );
	if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY ) {
		if ( !(IsReadJobDone()) ) {
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

bool CTreeBranch::StartRefreshIT_MYCOMPUTER( ) {
	m_lastChange.dwHighDateTime = 0;
	m_lastChange.dwLowDateTime = 0;
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		auto Child = GetChildGuaranteedValid( i );
		if ( !( Child->TmiIsLeaf( ) ) ) {
			dynamic_cast< CTreeBranch* >( Child )->StartRefresh( );
			}
		}
	return true;
	}

bool CTreeBranch::StartRefreshIT_FILESFOLDER( _In_ bool wasExpanded ) {
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

//bool CTreeBranch::StartRefreshIsDeleted( _In_ ITEMTYPE typeOf_thisItem ) {
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

void CTreeBranch::StartRefreshHandleDeletedItem( ) {
	auto myParent_here = dynamic_cast< CTreeBranch* >( GetParent( ) );
	if ( myParent_here == NULL ) {
		GetDocument( )->UnlinkRoot( );
		}
	else {
		myParent_here->UpwardRecalcLastChange( );
		auto myParent_IndexOfME = myParent_here->FindChildIndex( this );
		myParent_here->RemoveChild( myParent_IndexOfME );// --> delete this
		}

	}

void CTreeBranch::StartRefreshRecreateFSandUnknw( ) {
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

void CTreeBranch::StartRefreshUpwardClearItem( _In_ ITEMTYPE typeOf_thisItem ) {
	UpwardAddReadJobs( -GetReadJobs( ) );
	ASSERT( GetReadJobs( ) == 0 );

	if ( typeOf_thisItem == IT_FILE ) {
		auto Parent = dynamic_cast< CTreeBranch* >( GetParent( ) );
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

_Must_inspect_result_ bool CTreeBranch::StartRefreshIsMountOrJunction( _In_ ITEMTYPE typeOf_thisItem ) {
	/*
	  Was refactored from LARGER function. A `return true` from this function indicates that the caller should return true
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

bool CTreeBranch::StartRefresh( ) {
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


void CTreeBranch::UpwardSetUndoneIT_DRIVE( ) {
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

void CTreeBranch::UpwardParentSetUndone( ) {
	auto Parent = dynamic_cast< CTreeBranch* >( GetParent( ) );;
	if ( Parent != NULL ) {
		Parent->UpwardSetUndone( );
		}
	}

void CTreeBranch::UpwardSetUndone( ) {
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


void CTreeBranch::CreateFreeSpaceItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	LONGLONG total  = 0;
	LONGLONG freeSp = 0;

	TRACE( _T( "MyGetDiskFreeSpace\r\n" ) );
	MyGetDiskFreeSpace( GetPath( ), total, freeSp );
	auto freespace = new CItem( IT_FREESPACE, GetFreeSpaceItemName( ) );
	freespace->SetSize( freeSp );
	AddChild( freespace );
	}

_Success_(return != NULL) _Must_inspect_result_ CItem *CTreeBranch::FindFreeSpaceItem( ) const {
	auto i = FindFreeSpaceItemIndex( );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}
void CTreeBranch::UpdateFreeSpaceItem( ) {
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
		UpwardAddSize( diff );
		ASSERT( freeSpaceItem->GetSize( ) == free );
		}
	}

void CTreeBranch::RemoveFreeSpaceItem( ) {
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

void CTreeBranch::CreateUnknownItem( ) {
	ASSERT( GetType( ) == IT_DRIVE );
	UpwardSetUndone( );
	auto unknown = new CItem { IT_UNKNOWN, GetUnknownItemName( ) };
	AddChild( unknown );
	}

_Success_(return != NULL) _Must_inspect_result_ CItem* CTreeBranch::FindUnknownItem( ) const {
	auto i = FindUnknownItemIndex( );
	ASSERT( i >= 0 );
	if ( i < GetChildrenCount( ) ) {
		return GetChildGuaranteedValid( i );
		}
	else {
		return NULL;
		}
	}

void CTreeBranch::RemoveUnknownItem( ) {
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

_Success_(return != NULL) _Must_inspect_result_ CTreeBranch* CTreeBranch::FindDirectoryByPath( _In_ const CString& path ) {
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
		if ( !( Child->TmiIsLeaf( ) ) ) {
			auto item = dynamic_cast< CTreeBranch* >( Child )->FindDirectoryByPath( path );
			if ( item != NULL ) {
				return item;
				}
			}
		}
	return NULL;
	}

void CTreeBranch::stdRecurseCollectExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords ) {
	auto typeOfItem = GetType( );
	if ( IsLeaf( typeOfItem ) ) {
		if ( typeOfItem == IT_FILE ) {
			ASSERT( false );
			}
		}
	else {
		auto childCount = GetChildrenCount( );
		for ( INT i = 0; i < childCount; ++i ) {
			auto Child = GetChildGuaranteedValid( i );
			if ( !( Child->TmiIsLeaf( ) ) ) {
				dynamic_cast< CTreeBranch* >( Child )->stdRecurseCollectExtensionData( extensionRecords );
				}
			Child->stdRecurseCollectExtensionData( extensionRecords );
			}
		}

	}

LONGLONG CTreeBranch::GetProgressRangeMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG range = 0;
	auto childCountHere = GetChildrenCount( );
	for ( INT i = 0; i < childCountHere; i++ ) {
		auto Child = GetChildGuaranteedValid( i );
		if ( !( Child->TmiIsLeaf( ) ) ) {
			range += dynamic_cast< CTreeBranch* >( Child )->GetProgressRangeDrive( );
			}
		}
	return range;
	}

LONGLONG CTreeBranch::GetProgressPosMyComputer( ) const {
	ASSERT( GetType( ) == IT_MYCOMPUTER );
	LONGLONG pos = 0;
	auto childCountHere = GetChildrenCount( );
	for (INT i = 0; i < childCountHere; i++) {
		auto Child = GetChildGuaranteedValid( i );
		if ( !( Child->TmiIsLeaf( ) ) ) {
			pos += dynamic_cast< CTreeBranch* >( Child )->GetProgressPosDrive( );
			}
		}
	return pos;
	}

LONGLONG CTreeBranch::GetProgressPosDrive( ) const {
	auto pos = GetSize( );
	auto fs = FindFreeSpaceItem( );
	if ( fs != NULL ) {
		pos -= fs->GetSize( );
		}
	return pos;
	}

INT_PTR CTreeBranch::FindFreeSpaceItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChild( i )->GetType( ) == IT_FREESPACE ) {
			//break;
			return i; // maybe == GetChildrenCount() (=> not found)
			}
		}
	return childCount;
	}

INT_PTR CTreeBranch::FindUnknownItemIndex( ) const {
	auto childCount = GetChildrenCount( );
	for ( INT i = 0; i < childCount; i++ ) {
		if ( GetChild( i )->GetType( ) == IT_UNKNOWN ) {
			//break;
			return i; // maybe == GetChildrenCount() (=> not found)
			}	
		}
	return childCount;
	}


void CTreeBranch::AddDirectory( _In_ const CFileFindWDS& finder ) {
	ASSERT( &finder != NULL );
	auto thisApp      = GetApp( );
	auto thisFilePath = finder.GetFilePath( );
	auto thisOptions  = GetOptions( );

	//TODO IsJunctionPoint calls IsMountPoint deep in IsJunctionPoint's bowels. This means triplicated calls.
	bool dontFollow   = thisApp->IsMountPoint( thisFilePath ) && !thisOptions->IsFollowMountPoints( );
	
	dontFollow       |= thisApp->IsJunctionPoint( thisFilePath, finder.GetAttributes( ) ) && !thisOptions->IsFollowJunctionPoints( );
	auto child      = new CTreeBranch{ IT_DIRECTORY, finder.GetFileName( ), dontFollow };
	
	FILETIME t;
	finder.GetLastWriteTime( &t );
	child->SetLastChange( t );
	child->SetAttributes( finder.GetAttributes( ) );
	AddChild( child );
	}

void CTreeBranch::AddFile( _In_ const FILEINFO& fi ) {
	ASSERT( &fi != NULL );
	CItem* child = new CItem { IT_FILE, fi.name };
	child->SetSize( fi.length );
	child->SetLastChange( fi.lastWriteTime );
	child->SetAttributes( fi.attributes );
	AddChild( child );
	}
bool CTreeBranch::MustShowReadJobs( ) const {
	auto myParent = dynamic_cast< CTreeBranch* >( GetParent( ) );
	if ( myParent != NULL ) {
		return !myParent->IsDone( );
		}
	else {
		return !IsDone( );
		}
	}
