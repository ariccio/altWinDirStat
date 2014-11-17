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
#include ".\typeview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


bool CExtensionListControl::CListItem::DrawSubitem( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
	//ASSERT_VALID( pdc );
	if ( subitem == COL_EXTENSION ) {
		DrawLabel( m_list, nullptr, pdc, rc, state, width, focusLeft );
		}
	else if ( subitem == COL_COLOR ) {
		DrawColor( pdc, rc, state, width );
		}	
	else {
		if ( width != NULL ) {
			//Should never happen?
			*width = rc.Width( );
			}
		return false;
		}
	return true;
	}

void CExtensionListControl::CListItem::DrawColor( _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* width ) const {
	//ASSERT_VALID( pdc );
	if ( width != NULL ) {
		*width = 40;
		return;
		}

	DrawSelection( m_list, pdc, rc, state );

	rc.DeflateRect( 2, 3 );

	if ( rc.right <= rc.left || rc.bottom <= rc.top ) {
		return;
		}

	CTreemap treemap;//BUGBUG FIXME TODO
	treemap.DrawColorPreview( pdc, rc, m_record.color, &( GetOptions( )->m_treemapOptions ) );
	}

std::wstring CExtensionListControl::CListItem::GetText( _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const {
	switch (subitem)
	{
		case COL_EXTENSION:
			return m_extension;

		case COL_COLOR:
			return _T( "(color)" );

		case COL_BYTES:
			return FormatBytes( m_record.bytes );

		case COL_FILES:
			return FormatCount( m_record.files );

		case COL_DESCRIPTION:
			return _T( "" );//DRAW_ICONS
		case COL_BYTESPERCENT:
			return GetBytesPercent( );

		default:
			ASSERT( false );
			return _T("");
	}
	}

std::wstring CExtensionListControl::CListItem::GetBytesPercent( ) const {//TODO, C-style string!
	auto theDouble =  GetBytesFraction( ) * 100;
	const size_t bufSize = 12;
	wchar_t buffer[ bufSize ] = { 0 };
	auto res = CStyle_FormatDouble( theDouble, buffer, bufSize );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( buffer );
		}
	else {
		wchar_t percentage[ 2 ] = { '%', 0 };
		StringCchCatW( buffer, bufSize, percentage );
		}
	return buffer;
	}

DOUBLE CExtensionListControl::CListItem::GetBytesFraction( ) const {
	if ( m_list->m_rootSize == 0 ) {
		return 0;
		}
	return DOUBLE( m_record.bytes ) / DOUBLE( m_list->m_rootSize );
	}

INT CExtensionListControl::CListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, _In_ _In_range_( 0, 7 ) const INT subitem ) const {
	auto other = static_cast< const CListItem * >( baseOther );

	switch ( subitem )
	{
		case COL_EXTENSION:
			return signum( m_extension.compare( other->m_extension ) );

		case COL_COLOR:
		case COL_BYTES:
			return signum( std::int64_t( m_record.bytes ) - std::int64_t( other->m_record.bytes ) );

		case COL_FILES:
			return signum( std::int64_t( m_record.files ) - std::int64_t( other->m_record.files ) );

		case COL_DESCRIPTION:
			return 0;//DRAW_ICONS
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

	InsertColumn(COL_EXTENSION,    _T( "Extension" ),   LVCFMT_LEFT,  60, COL_EXTENSION);
	InsertColumn(COL_COLOR,        _T( "Color" ),       LVCFMT_LEFT,  40, COL_COLOR);
	InsertColumn(COL_BYTES,        _T( "Bytes" ),       LVCFMT_RIGHT, 60, COL_BYTES);
	InsertColumn(COL_BYTESPERCENT, _T( "% Bytes" ),     LVCFMT_RIGHT, 50, COL_BYTESPERCENT);
	InsertColumn(COL_FILES,        _T( "Files" ),       LVCFMT_RIGHT, 50, COL_FILES);
	InsertColumn(COL_DESCRIPTION,  _T( "Description" ), LVCFMT_LEFT, 170, COL_DESCRIPTION);

	OnColumnsInserted( );

	// We don't use the list control's image list, but attaching an image list to the control ensures a proper line height.
	//SetImageList( NULL, LVSIL_SMALL );
	}

void CExtensionListControl::OnDestroy( ) {
	//SetImageList( NULL, LVSIL_SMALL );//Invalid parameter value!
	COwnerDrawnListControl::OnDestroy();
	}

void CExtensionListControl::SetExtensionData( _In_ const std::vector<SExtensionRecord>* extData ) {
	DeleteAllItems( );
	LARGE_INTEGER frequency = help_QueryPerformanceFrequency( );
	auto startTime = help_QueryPerformanceCounter( );

	SetItemCount( static_cast<int>( extData->size( ) + 1 ) );
	extensionItems.clear( );
	extensionItems.reserve( extData->size( ) + 1 );
	for ( const auto& anExt : *extData ) {
		extensionItems.emplace_back( std::make_unique<CListItem>( this, anExt.ext, anExt ) );
		}
	INT_PTR count = 0;
	std::uint64_t totalSizeExtensionNameLength = 0;
	SetItemCount( static_cast<int>( extensionItems.size( ) + 1 ) );
	TRACE( _T( "Built vector of extension records, inserting....\r\n" ) );
#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif

	SetRedraw( FALSE );
	for ( auto& anExt : extensionItems ) {
		totalSizeExtensionNameLength += std::uint64_t( anExt->m_extension.length( ) );
		InsertListItem( count++, anExt.get( ) ); //InsertItem slows quadratically/exponentially with number of items in list! Seems to be dominated by UpdateScrollBars!
		}
	SetRedraw( TRUE );
	auto doneTime = help_QueryPerformanceCounter( );
	const DOUBLE adjustedTimingFrequency = ( ( DOUBLE )1.00 ) / frequency.QuadPart;
	adjustedTiming = ( doneTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;

	averageExtensionNameLength = DOUBLE( totalSizeExtensionNameLength ) / DOUBLE( count );
	SortItems( );
	}

void CExtensionListControl::SelectExtension( _In_ const std::wstring ext ) {
	auto countItems = this->GetItemCount( );
	SetRedraw( FALSE );
	for ( INT i = 0; i < countItems; i++ ) {
		if ( ( GetListItem( i )->m_extension.compare( ext ) == 0 ) && ( i >= 0 ) ) {
			TRACE(_T("Selecting extension %s (item #%i)...\r\n"), ext, i );
			SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );//Unreachable code?
			EnsureVisible( i, false );
			break;
			}

		}
	SetRedraw( TRUE );
	}

const CString CExtensionListControl::GetSelectedExtension( ) const {
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return _T( "" );
		}
	auto i = GetNextSelectedItem( pos );//SIX CYCLES PER INSTRUCTION!!!!
	auto item = GetListItem( i );
	return CString( item->m_extension.c_str( ) );
	}

//CExtensionListControl::CListItem* CExtensionListControl::GetListItem( _In_ const INT i ) const {
//	return reinterpret_cast< CListItem* > ( GetItemData( i ) );
//	}

void CExtensionListControl::OnLvnDeleteitem( NMHDR *pNMHDR, LRESULT *pResult ) {
	ASSERT( pNMHDR != NULL );
	ASSERT( pResult != NULL );
	if ( pNMHDR != NULL ) {
		auto lv = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
		lv->lParam = { NULL };
		}

	if ( pResult != NULL ) {
		*pResult = 0;
		}
	}

void CExtensionListControl::MeasureItem( PMEASUREITEMSTRUCT mis ) {
	mis->itemHeight = m_rowHeight;
	}

void CExtensionListControl::OnSetFocus( CWnd* pOldWnd ) {
	COwnerDrawnListControl::OnSetFocus( pOldWnd );
	GetMainFrame( )->SetLogicalFocus( LF_EXTENSIONLIST );
	}

void CExtensionListControl::OnLvnItemchanged( NMHDR *pNMHDR, LRESULT *pResult ) {
	LPNMLISTVIEW pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	if ( ( pNMLV->uNewState bitand LVIS_SELECTED ) != 0 ) {
		m_typeView->SetHighlightExtension( GetSelectedExtension( ) );
		}
	*pResult = 0;
	}

void CExtensionListControl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	if ( nChar == VK_TAB ) {
		if ( GetMainFrame( )->GetDirstatView( ) != NULL ) {
			TRACE( _T( "TAB pressed! Focusing on directory list!\r\n" ) );
			GetMainFrame( )->MoveFocus( LF_DIRECTORYLIST );
			}
		else {
			TRACE( _T( "TAB pressed! No directory list! Null focus!\r\n" ) );
			GetMainFrame( )->MoveFocus( LF_NONE );
			}
		}
	else if ( nChar == VK_ESCAPE ) {
		TRACE( _T( "ESCAPE pressed! Null focus!\r\n" ) );
		GetMainFrame( )->MoveFocus( LF_NONE );
		}
	COwnerDrawnListControl::OnKeyDown( nChar, nRepCnt, nFlags );
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CTypeView, CView)

BEGIN_MESSAGE_MAP(CTypeView, CView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


//void CTypeView::SysColorChanged( ) {
//	m_extensionListControl.SysColorChanged( );
//	}

//void CTypeView::ShowTypes( _In_ const bool show ) {
//	m_showTypes = show;
//	OnUpdate( NULL, 0, NULL );
//	}

void CTypeView::SetHighlightExtension( _In_z_ const PCWSTR ext ) {
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

//BOOL CTypeView::PreCreateWindow( CREATESTRUCT& cs ) {
//	return CView::PreCreateWindow( cs );
//	}

INT CTypeView::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( CView::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}

	RECT rect = { 0, 0, 0, 0 };
	VERIFY( m_extensionListControl.CreateEx( 0, LVS_SINGLESEL | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT, rect, this, _N_ID_EXTENSION_LIST_CONTROL ) );
	m_extensionListControl.SetExtendedStyle( m_extensionListControl.GetExtendedStyle( ) | LVS_EX_HEADERDRAGDROP );
	auto Options = GetOptions( );
	ASSERT( Options != NULL );
	if ( Options != NULL ) {
		m_extensionListControl.ShowGrid( Options->m_listGrid );
		m_extensionListControl.ShowStripes( Options->m_listStripes );
		m_extensionListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
		}
	//else {
	//	ASSERT( Options != NULL );
	//	//Fall back to defaults that I like :)
	//	m_extensionListControl.ShowGrid( true );
	//	m_extensionListControl.ShowStripes( true );
	//	m_extensionListControl.ShowFullRowSelection( true );
	//	}
	m_extensionListControl.Initialize( );
	return 0;
	}

//void CTypeView::OnInitialUpdate( ) {
//	CView::OnInitialUpdate();
//	}

void CTypeView::OnUpdate0( ) {
	auto theDocument = GetDocument( );
	if ( theDocument != NULL ) {
		if ( m_showTypes && theDocument->IsRootDone( ) ) {
			m_extensionListControl.SetRootSize( theDocument->GetRootSize( ) );
#ifdef PERF_DEBUG_SLEEP
			Sleep( 1000 );
#endif
			TRACE( _T( "Populating extension list...\r\n" ) );
			m_extensionListControl.SetExtensionData( theDocument->GetExtensionRecords( ) );
			TRACE( _T( "Finished populating extension list...\r\n" ) );
#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif
			// If there is no vertical scroll bar, the header control doesn't repaint correctly. Don't know why. But this helps:
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		}
	else {
		if ( m_showTypes ) {
			m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		AfxCheckMemory( );
		ASSERT( false );
		}

	}

void CTypeView::OnUpdateHINT_LISTSTYLECHANGED( ) {
	auto thisOptions = GetOptions( );
	ASSERT( thisOptions != NULL );
	if ( thisOptions != NULL ) {
		m_extensionListControl.ShowGrid( thisOptions->m_listGrid );
		m_extensionListControl.ShowStripes( thisOptions->m_listStripes );
		m_extensionListControl.ShowFullRowSelection( thisOptions->m_listFullRowSelection );
		}
	//else {
	//	//Fall back to defaults that I like :)
	//	m_extensionListControl.ShowGrid( true );
	//	m_extensionListControl.ShowStripes( true );
	//	m_extensionListControl.ShowFullRowSelection( true );
	//	}
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
			if ( m_showTypes ) {
				SetSelection( );
				}
			break;

		case HINT_REDRAWWINDOW:
			m_extensionListControl.RedrawWindow();
			break;

		case HINT_TREEMAPSTYLECHANGED:
			return OnUpdateHINT_TREEMAPSTYLECHANGED( );

		case HINT_LISTSTYLECHANGED:
			return OnUpdateHINT_LISTSTYLECHANGED( );

		case HINT_ZOOMCHANGED:
		default:
			break;
	}
	}

void CTypeView::SetSelection( ) {
	auto Document = GetDocument( );
	if ( Document != NULL ) {
		auto item = Document->GetSelection( );
		if ( item != NULL && item->m_type == IT_FILE ) {
			auto selectedExt = m_extensionListControl.GetSelectedExtension( );
			ASSERT( item->GetExtension( ).compare( item->CStyle_GetExtensionStrPtr( ) ) == 0 );
			if ( selectedExt.CompareNoCase( item->CStyle_GetExtensionStrPtr( ) ) != 0 ) {
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
	//return static_cast<CDirstatDoc*>( m_pDocument );
	return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	}
#endif //_DEBUG

//void CTypeView::OnDraw( CDC* pDC ) {
//	ASSERT_VALID( pDC );
//	CView::OnDraw( pDC );
//	}

//BOOL CTypeView::OnEraseBkgnd( CDC* pDC ) {
//	ASSERT_VALID( pDC );
//	return CView::OnEraseBkgnd( pDC );
//	}


void CTypeView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize(nType, cx, cy);
	if ( IsWindow( m_extensionListControl.m_hWnd ) ) {
		CRect rc( 0, 0, cx, cy );
		m_extensionListControl.MoveWindow( rc );
		}
	}

//void CTypeView::OnSetFocus( CWnd* /*pOldWnd*/ ) {
//	m_extensionListControl.SetFocus();
//	}


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
