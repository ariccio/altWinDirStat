// typeview.cpp		- Implementation of CExtensionListControl and CTypeView
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
//#include "mainframe.h"
//#include ".\typeview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////

CExtensionListControl::CListItem::CListItem( CExtensionListControl* list, _In_z_ LPCTSTR extension, SExtensionRecord r ) {
	m_list      = std::move( list );
	m_extension = std::move( extension );
	m_record    = std::move( r );
	m_image     = -1;
	}

bool CExtensionListControl::CListItem::DrawSubitem( _In_ const INT subitem, _In_ CDC *pdc, _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT *width, _Inout_ INT *focusLeft ) const {
	ASSERT_VALID( pdc );
	if ( subitem == COL_EXTENSION ) {
		auto ImageList = GetMyImageList( );
		DrawLabel( m_list, ImageList, pdc, rc, state, width, focusLeft );
		}
	else if ( subitem == COL_COLOR ) {
		DrawColor( pdc, rc, state, width );
		}
	else {
		return false;
		}
	return true;
	}

void CExtensionListControl::CListItem::DrawColor( _In_ CDC *pdc, _In_ CRect rc, _In_ const UINT state, _Inout_opt_ INT *width ) const {
	ASSERT_VALID( pdc );
	if ( width != NULL ) {
		*width = 40;
		return;
		}

	DrawSelection( m_list, pdc, rc, state );

	rc.DeflateRect( 2, 3 );

	if ( rc.right <= rc.left || rc.bottom <= rc.top ) {
		return;
		}

	CTreemap treemap;
	treemap.DrawColorPreview( pdc, rc, m_record.color, GetOptions( )->GetTreemapOptions( ) );
	}

CString CExtensionListControl::CListItem::GetText( _In_ const INT subitem ) const {
	switch (subitem)
	{
		case COL_EXTENSION:
			return GetExtension( );

		case COL_COLOR:
			return _T( "(color)" );

		case COL_BYTES:
			return FormatBytes( m_record.bytes );

		case COL_FILES:
			return FormatCount( m_record.files );

		case COL_DESCRIPTION:
#ifdef DRAW_ICONS
			return GetDescription( );
#else
			return CString( "" );
#endif
		case COL_BYTESPERCENT:
			return GetBytesPercent( );

		default:
			ASSERT( false );
			return CString("");
	}
	}

CString CExtensionListControl::CListItem::GetExtension( ) const {
	return m_extension;
	}

#ifdef DRAW_ICONS
INT CExtensionListControl::CListItem::GetImage( ) const {
	if ( m_image == -1 ) {
		m_image = GetMyImageList( )->GetExtImageAndDescription( m_extension, m_description );
		}
	return m_image;
	}

CString CExtensionListControl::CListItem::GetDescription( ) const {
	if ( m_description.IsEmpty( ) ) {
		m_image = GetMyImageList( )->GetExtImageAndDescription( m_extension, m_description );
		}
	return m_description;
	}
#endif

CString CExtensionListControl::CListItem::GetBytesPercent( ) const {//TODO, C-style string!
	auto bytesFraction = GetBytesFraction( );
	auto theDouble =  bytesFraction * 100;
#ifdef C_STYLE_STRINGS
#ifdef _DEBUG
	auto d = FormatDouble( theDouble );
	CString s;
	s.Format( _T( "%s%%" ), d.GetString( ) );
#endif
	const size_t bufSize = 12;

	wchar_t buffer[ bufSize ] = { 0 };
	auto res = CStyle_FormatDouble( theDouble, buffer, bufSize );
	if ( !SUCCEEDED( res ) ) {
		buffer[ 0 ] = 'B';
		buffer[ 1 ] = 'A';
		buffer[ 2 ] = 'D';
		buffer[ 3 ] = '_';
		buffer[ 4 ] = 'F';
		buffer[ 5 ] = 'M';
		buffer[ 6 ] = 'T';
		buffer[ 7 ] = 0;
		}
#ifdef _DEBUG
	ASSERT( s.Compare( buffer ) == 0 );
#endif
	return buffer;
#else
	auto d = FormatDouble( theDouble );
	CString s;
	s.Format( _T( "%s%%" ), d );
	return s;
#endif
	}

DOUBLE CExtensionListControl::CListItem::GetBytesFraction( ) const {
	if ( m_list->GetRootSize( ) == 0 ) {
		return 0;
		}
	auto rootSize = m_list->GetRootSize( );
	return DOUBLE( m_record.bytes ) / DOUBLE( rootSize );
	}

INT CExtensionListControl::CListItem::Compare( _In_ const CSortingListItem* baseOther, _In_ const INT subitem ) const {
	auto other = static_cast< const CListItem * >( baseOther );

	switch ( subitem )
	{
		case COL_EXTENSION:
			return signum( GetExtension( ).CompareNoCase( other->GetExtension( ) ) );

		case COL_COLOR:
		case COL_BYTES:
			return signum( std::int64_t( m_record.bytes ) - std::int64_t( other->m_record.bytes ) );

		case COL_FILES:
			return signum( std::int64_t( m_record.files ) - std::int64_t( other->m_record.files ) );

		case COL_DESCRIPTION:
#ifdef DRAW_ICONS
			return signum( GetDescription( ).CompareNoCase( other->GetDescription( ) ) );
#else
			return 0;
#endif
		case COL_BYTESPERCENT:
			return signum( GetBytesFraction( ) - other->GetBytesFraction( ) );
			
		default:
			ASSERT( false );
			return 0;
	}
	}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CExtensionListControl, COwnerDrawnListControl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_WM_SETFOCUS()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CExtensionListControl::CExtensionListControl( CTypeView *typeView ) : COwnerDrawnListControl( _T( "types" ), 19 ), m_typeView( typeView ) {
	m_rootSize = 0;
	}

bool CExtensionListControl::GetAscendingDefault( _In_ const INT column ) const {
	switch ( column )
	{
		case COL_EXTENSION:
		case COL_DESCRIPTION:
			return true;
		case COL_COLOR:
		case COL_BYTES:
		case COL_FILES:
		case COL_BYTESPERCENT:
			return false;
		default:
			ASSERT(false);
			return true;
	}
	}

// As we will not receive WM_CREATE, we must do initialization in this extra method. The counterpart is OnDestroy().
void CExtensionListControl::Initialize( ) {
	SetSorting(COL_BYTES, false);

	InsertColumn(COL_EXTENSION,		LoadString(IDS_EXTCOL_EXTENSION),	LVCFMT_LEFT, 60, COL_EXTENSION);
	InsertColumn(COL_COLOR,			LoadString(IDS_EXTCOL_COLOR),		LVCFMT_LEFT, 40, COL_COLOR);
	InsertColumn(COL_BYTES,			LoadString(IDS_EXTCOL_BYTES),		LVCFMT_RIGHT, 60, COL_BYTES);
	InsertColumn(COL_BYTESPERCENT,	_T("% ") + LoadString(IDS_EXTCOL_BYTES), LVCFMT_RIGHT, 50, COL_BYTESPERCENT);
	InsertColumn(COL_FILES,			LoadString(IDS_EXTCOL_FILES),		LVCFMT_RIGHT, 50, COL_FILES);
	InsertColumn(COL_DESCRIPTION,	LoadString(IDS_EXTCOL_DESCRIPTION), LVCFMT_LEFT, 170, COL_DESCRIPTION);

	OnColumnsInserted( );

	// We don't use the list control's image list, but attaching an image list to the control ensures a proper line height.
#ifdef DRAW_ICONS
	SetImageList( GetMyImageList( ), LVSIL_SMALL );
#else
	SetImageList( NULL, LVSIL_SMALL );
#endif
	}

void CExtensionListControl::OnDestroy( ) {
	AfxCheckMemory( );
	SetImageList( NULL, LVSIL_SMALL );//Invalid parameter value!
	COwnerDrawnListControl::OnDestroy();
	}

void CExtensionListControl::SetExtensionData( _In_ const std::vector<SExtensionRecord>* extData ) {
	DeleteAllItems( );
	LARGE_INTEGER frequency;
	if ( !(QueryPerformanceFrequency( &frequency ) ) ) {
		frequency.QuadPart = -1;
		}
	auto startTime = help_QueryPerformanceCounter( );

	SetItemCount( static_cast<int>( extData->size( ) + 1 ) );
	extensionItems.clear( );
	extensionItems.reserve( extData->size( ) + 1 );
	for ( auto& anExt : *extData ) {
		extensionItems.emplace_back( CListItem ( this, anExt.ext, anExt ) );
		}
	INT_PTR count = 0;
	SetItemCount( extensionItems.size( ) + 1 );
	for ( auto& anExt : extensionItems ) {
		InsertListItem( count++, &anExt ); //InsertItem slows quadratically/exponentially with number of items in list! Seems to be dominated by UpdateScrollBars!
		
		}
	auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE adjustedTimingFrequency = ( ( DOUBLE )1.00 ) / frequency.QuadPart;
	adjustedTiming = ( doneTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

	SortItems( );

	}

void CExtensionListControl::SetRootSize( _In_ const LONGLONG totalBytes ) {
	m_rootSize = totalBytes;
	}

LONGLONG CExtensionListControl::GetRootSize( ) const {
	return m_rootSize;
	}

void CExtensionListControl::SelectExtension( _In_z_ const LPCTSTR ext ) {
	auto countItems = this->GetItemCount( );
	for ( INT i = 0; i < countItems; i++ ) {
		/*SLOW*/
		if ( ( GetListItem( i )->GetExtension( ).CompareNoCase( ext ) == 0 ) && ( i >= 0 ) ) {
			TRACE(_T("Selecting extension %s (item #%i)...\r\n"), ext, i );
			SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );//Unreachable code?
			EnsureVisible( i, false );
			break;
			}

		}
	}

CString CExtensionListControl::GetSelectedExtension( ) {
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return _T( "" );
		}
	auto i = GetNextSelectedItem( pos );//SIX CYCLES PER INSTRUCTION!!!!
	auto item = GetListItem( i );
	return item->GetExtension( );
	}

CExtensionListControl::CListItem* CExtensionListControl::GetListItem( _In_ const INT i ) {
	return reinterpret_cast< CListItem* > ( GetItemData( i ) );
	}

void CExtensionListControl::OnLvnDeleteitem( NMHDR *pNMHDR, LRESULT *pResult ) {
	ASSERT( pNMHDR != NULL );
	ASSERT( pResult != NULL );
	if ( pNMHDR != NULL ) {
		auto lv = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );

		//delete[] ( CListItem * ) ( lv->lParam ); // “scalar deleting destructor.” (see http://blog.aaronballman.com/2011/11/destructors/ for more)

		if ( lv->lParam != NULL ) {
			//delete ( CListItem * ) ( lv->lParam );//occasional heap corruption??!?
			}
		lv->lParam = NULL;
		}

	if ( pResult != NULL ) {
		//if ( static_cast< void* >( pResult ) != NULL ) {
			*pResult = 0;
			//}
		}
	}

void CExtensionListControl::MeasureItem( LPMEASUREITEMSTRUCT mis ) {
	mis->itemHeight = UINT( GetRowHeight( ) );
	}

void CExtensionListControl::OnSetFocus( CWnd* pOldWnd ) {
	COwnerDrawnListControl::OnSetFocus( pOldWnd );
	GetMainFrame( )->SetLogicalFocus( LF_EXTENSIONLIST );
	}

void CExtensionListControl::OnLvnItemchanged( NMHDR *pNMHDR, LRESULT *pResult ) {
	LPNMLISTVIEW pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	if ( ( pNMLV->uNewState & LVIS_SELECTED ) != 0 ) {
		m_typeView->SetHighlightExtension( GetSelectedExtension( ) );
		}
	*pResult = 0;
	}

void CExtensionListControl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	if ( nChar == VK_TAB ) {
		GetMainFrame( )->MoveFocus( LF_DIRECTORYLIST );
		}
	else if ( nChar == VK_ESCAPE ) {
		GetMainFrame( )->MoveFocus( LF_NONE );
		}
	COwnerDrawnListControl::OnKeyDown( nChar, nRepCnt, nFlags );
	}

/////////////////////////////////////////////////////////////////////////////

static UINT _nIdExtensionListControl = 4711;


IMPLEMENT_DYNCREATE(CTypeView, CView)

BEGIN_MESSAGE_MAP(CTypeView, CView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


CTypeView::CTypeView( ) : m_extensionListControl( this ) {
	m_showTypes = true;
	}

CTypeView::~CTypeView()
{
}

void CTypeView::SysColorChanged( ) {
	m_extensionListControl.SysColorChanged( );
	}

bool CTypeView::IsShowTypes( ) const {
	return m_showTypes;
	}

void CTypeView::ShowTypes( _In_ const bool show ) {
	m_showTypes = show;
	OnUpdate( NULL, 0, NULL );
	}

void CTypeView::SetHighlightExtension( _In_z_ const LPCTSTR ext ) {
	auto Document = GetDocument( );

	if ( Document != NULL ) {
		Document->SetHighlightExtension( ext );
		if ( GetFocus( ) == &m_extensionListControl ) {
			Document->UpdateAllViews( this, HINT_EXTENSIONSELECTIONCHANGED );
			TRACE( _T( "Highlighted extension %s\r\n" ), ext );
			}
		}
	ASSERT( Document != NULL );
	}

BOOL CTypeView::PreCreateWindow( CREATESTRUCT& cs ) {
	return CView::PreCreateWindow( cs );
	}

INT CTypeView::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( CView::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}

	RECT rect = { 0, 0, 0, 0 };
	VERIFY( m_extensionListControl.CreateEx( 0, LVS_SINGLESEL | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT, rect, this, _nIdExtensionListControl ) );
	m_extensionListControl.SetExtendedStyle( m_extensionListControl.GetExtendedStyle( ) | LVS_EX_HEADERDRAGDROP );
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		m_extensionListControl.ShowGrid( Options->IsListGrid( ) );
		m_extensionListControl.ShowStripes( Options->IsListStripes( ) );
		m_extensionListControl.ShowFullRowSelection( Options->IsListFullRowSelection( ) );
		}
	else {
		ASSERT( Options != NULL );
		//Fall back to defaults that I like :)
		m_extensionListControl.ShowGrid( true );
		m_extensionListControl.ShowStripes( true );
		m_extensionListControl.ShowFullRowSelection( true );
		}
	m_extensionListControl.Initialize( );
	return 0;
	}

void CTypeView::OnInitialUpdate( ) {
	CView::OnInitialUpdate();
	}

void CTypeView::OnUpdate0( ) {
	auto theDocument = GetDocument( );
	if ( theDocument != NULL ) {
		if ( IsShowTypes( ) && theDocument->IsRootDone( ) ) {
			m_extensionListControl.SetRootSize( theDocument->GetRootSize( ) );

			LockWindowUpdate( );
			m_extensionListControl.SetExtensionData( theDocument->GetExtensionRecords( ) );
			UnlockWindowUpdate( );

			// If there is no vertical scroll bar, the header control doesn't repaint correctly. Don't know why. But this helps:
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		}
	else {
		if ( IsShowTypes( ) ) {
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		AfxCheckMemory( );
		ASSERT( false );
		}

	}

void CTypeView::OnUpdateHINT_HIDEFREESPACE( ) {
	auto theDocument = GetDocument( );
	if ( theDocument != NULL ) {
		if ( IsShowTypes( ) && theDocument->IsRootDone( ) ) {
			//m_extensionListControl.SetRootSize( theDocument->GetRootSize( ) );

			//LockWindowUpdate( );
			//m_extensionListControl.SetExtensionData( theDocument->GetExtensionRecords( ) );
			//UnlockWindowUpdate( );

			// If there is no vertical scroll bar, the header control doesn't repaint correctly. Don't know why. But this helps:
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			//m_extensionListControl.DeleteAllItems( );
			}
		}
	else {
		if ( IsShowTypes( ) ) {
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			//m_extensionListControl.DeleteAllItems( );
			}
		AfxCheckMemory( );
		ASSERT( false );
		}

	}


void CTypeView::OnUpdateHINT_LISTSTYLECHANGED( ) {
	auto thisOptions = GetOptions( );
	if ( thisOptions != NULL ) {
		m_extensionListControl.ShowGrid( thisOptions->IsListGrid( ) );
		m_extensionListControl.ShowStripes( thisOptions->IsListStripes( ) );
		m_extensionListControl.ShowFullRowSelection( thisOptions->IsListFullRowSelection( ) );
		}
	else {
		//Fall back to defaults that I like :)
		m_extensionListControl.ShowGrid( true );
		m_extensionListControl.ShowStripes( true );
		m_extensionListControl.ShowFullRowSelection( true );
		}
	}

void CTypeView::OnUpdateHINT_TREEMAPSTYLECHANGED( ) {
	InvalidateRect( NULL );
	m_extensionListControl.InvalidateRect( NULL );
	m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
	}

void CTypeView::OnUpdate( CView * /*pSender*/, LPARAM lHint, CObject * ) {
	switch ( lHint )
	{
		case HINT_NEWROOT:
		case 0:
			OnUpdate0( );
			// fall thru

		case HINT_SELECTIONCHANGED:
		case HINT_SHOWNEWSELECTION:
			if ( IsShowTypes( ) ) {
				SetSelection( );
				}
			break;

		case HINT_ZOOMCHANGED:
			break;

		case HINT_REDRAWWINDOW:
			m_extensionListControl.RedrawWindow();
			break;

		case HINT_TREEMAPSTYLECHANGED:
			return OnUpdateHINT_TREEMAPSTYLECHANGED( );

		case HINT_LISTSTYLECHANGED:
			return OnUpdateHINT_LISTSTYLECHANGED( );

		case HINT_HIDEFREESPACE:
			return OnUpdateHINT_HIDEFREESPACE( );
		default:
			break;
	}
	}

void CTypeView::SetSelection( ) {
	auto Document = GetDocument( );
	if ( Document != NULL ) {
		auto item = Document->GetSelection( );
		if ( item == NULL || item->GetType( ) != IT_FILE ) {
			m_extensionListControl.EnsureVisible( 0, false );
			}
		else {
			ASSERT( item->GetType( ) != IT_DRIVE );
			if ( !( m_extensionListControl.GetSelectedExtension( ).CompareNoCase( item->GetExtension( ) ) ) ) {
				m_extensionListControl.SelectExtension( item->GetExtension( ) );
				}
			}
		}
	ASSERT( Document != NULL );
	}

#ifdef _DEBUG
void CTypeView::AssertValid( ) const {
	CView::AssertValid();
	}

void CTypeView::Dump( CDumpContext& dc ) const {
	CView::Dump( dc );
	}

_Must_inspect_result_ CDirstatDoc* CTypeView::GetDocument( ) const {// Nicht-Debugversion ist inline
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CDirstatDoc ) ) );
	return static_cast<CDirstatDoc*>( m_pDocument );
	}
#endif //_DEBUG

void CTypeView::OnDraw( CDC* pDC ) {
	ASSERT_VALID( pDC );
	CView::OnDraw( pDC );
	}

BOOL CTypeView::OnEraseBkgnd( CDC* pDC ) {
	ASSERT_VALID( pDC );
	return CView::OnEraseBkgnd( pDC );
	}


void CTypeView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize(nType, cx, cy);
	if ( IsWindow( m_extensionListControl.m_hWnd ) ) {
		CRect rc(0, 0, cx, cy);
		m_extensionListControl.MoveWindow( rc );
		}
	}

void CTypeView::OnSetFocus( CWnd* /*pOldWnd*/ ) {
	m_extensionListControl.SetFocus();
	}


// $Log$
// Revision 1.13  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.12  2004/12/31 16:01:42  bseifert
// Bugfixes. See changelog 2004-12-31.
//
// Revision 1.11  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.10  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.9  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.8  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
