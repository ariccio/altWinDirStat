// typeview.cpp		- Implementation of CExtensionListControl and CTypeView
//
// see `file_header_text.txt` for licensing & contact info.


#pragma once


#include "stdafx.h"

#ifndef WDS_TYPEVIEW_CPP
#define WDS_TYPEVIEW_CPP


#pragma message( "Including `" __FILE__ "`..." )

#include "mainframe.h"
#include "datastructures.h"

//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
//#include "item.h"
#include "typeview.h"


#include "treemap.h"
#include "dirstatdoc.h"
//#include "ownerdrawnlistcontrol.h"
#include "windirstat.h"
#include "options.h"
#include "globalhelpers.h"


#pragma warning(suppress:4355)
CTypeView::CTypeView( ) : m_extensionListControl( this ), m_showTypes( true ) {
	m_showTypes = CPersistence::GetShowFileTypes( );
	}


//CTypeView::~CTypeView( ) { }

bool CListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const {
	//ASSERT_VALID( pdc );
	//Why are we bothering to draw this ourselves?
	if ( subitem == column::COL_EXTENSION ) {
		ASSERT( list == m_list );
		UNREFERENCED_PARAMETER( list );
		//ASSERT( width != NULL );
		CFont* const list_font = list->GetFont( );
		const bool list_has_focus = list->HasFocus( );
		const bool list_is_show_selection_always = list->IsShowSelectionAlways( );
		const COLORREF list_highlight_text_color = list->GetHighlightTextColor( );
		const COLORREF list_highlight_color = list->GetHighlightColor( );
		const bool list_is_full_row_selection = list->m_showFullRowSelection;
		//list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection

		DrawLabel( pdc, rc, state, width, focusLeft, true, list_font, list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
		return true;
		}
	else if ( subitem == column::COL_COLOR ) {
		DrawColor( pdc, rc, state, width );
		return true;
		}	
	if ( width == NULL ) {
		return false;
		}
	//Should never happen?
	*width = ( rc.right - rc.left );
	return false;

	}

void CListItem::DrawColor( _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* width ) const {
	if ( width != NULL ) {
		*width = 40;
		return;
		}

	DrawSelection( pdc, rc, state, m_list->HasFocus( ), m_list->IsShowSelectionAlways( ), m_list->GetHighlightColor( ), m_list->m_showFullRowSelection );
	VERIFY( ::InflateRect( &rc, -( 2 ), -( 3 ) ) );

	if ( ( rc.right <= rc.left ) || ( rc.bottom <= rc.top ) ) {
		return;
		}

	CTreemap treemap;
#ifdef DEBUG
	treemap.m_is_typeview = true;
#endif
	treemap.DrawColorPreview( pdc, rc, color, &( GetOptions( )->m_treemapOptions ) );
	}

_Pre_satisfies_( subitem == column::COL_BYTES ) _Success_( SUCCEEDED( return ) )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTES );
	const HRESULT res = wds_fmt::FormatBytes( m_bytes, psz_text, strSize, chars_written, sizeBuffNeed );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	//handled in FormatBytes.
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		//ASSERT( chars_written == strSize );
		//chars_written = strSize;
		//sizeBuffNeed = ( ( 64 > sizeBuffNeed ) ? 64 : sizeBuffNeed );//Generic size needed.
		return res;
		}
	return res;
	}

_Pre_satisfies_( subitem == column::COL_FILES_TYPEVIEW ) _Success_( SUCCEEDED( return ) )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_FILES_TYPEVIEW );
	size_t chars_remaining = 0;

	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%I32u", m_files );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		//chars_written = strSize;
		//sizeBuffNeed = 64;//Generic size needed.
		//ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	chars_written = 0;
	return res;
	}

_Pre_satisfies_( subitem == column::COL_DESCRIPTION ) _Success_( SUCCEEDED( return ) ) _Pre_satisfies_( strSize > 0 )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_DESCRIPTION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_DESCRIPTION );
	ASSERT( strSize > 0 );
	if ( strSize > 0 ) {
		psz_text[ 0 ] = 0;
		chars_written = 0;
		return S_OK;
		}
	chars_written = 0;
	sizeBuffNeed = 1;//Generic size needed
	return STRSAFE_E_INSUFFICIENT_BUFFER;
	}

_Pre_satisfies_( subitem == column::COL_BYTESPERCENT ) _Success_( SUCCEEDED( return ) )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTESPERCENT( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTESPERCENT );
	size_t chars_remaining = 0;
	const auto theDouble = GetBytesFraction( ) * 100;
	const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", theDouble );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 8u, sizeBuffNeed, chars_written );
		//chars_written = strSize;
		//sizeBuffNeed = 8;//Generic size needed, overkill;
		//ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}
	chars_written = 0;
	return res;
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	ASSERT( subitem != column::COL_NAME );
	if ( subitem == column::COL_NAME ) {
		displayWindowsMsgBoxWithMessage( L"GetText_WriteToStackBuffer was called for column::COL_NAME!!! This should never happen!!!!" );
		std::terminate( );
		}
	switch ( subitem )
	{
			case column::COL_NAME:
			case column::COL_ATTRIBUTES:
			case column::COL_COLOR://COL_COLOR is supposed to have a tiny, colored, treemap - NOT text.
			default:
				return WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written, L"CListItem::" );
			case column::COL_DESCRIPTION:
				return Text_WriteToStackBuffer_COL_DESCRIPTION( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTES:
				return Text_WriteToStackBuffer_COL_BYTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTESPERCENT:
				return Text_WriteToStackBuffer_COL_BYTESPERCENT( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_FILES_TYPEVIEW:
				return Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}

COLORREF CListItem::ItemTextColor( ) const {
	return default_item_text_color( );
	}

DOUBLE CListItem::GetBytesFraction( ) const {
	if ( m_list->m_rootSize == 0 ) {
		return 0;
		}
	return static_cast<DOUBLE>( m_bytes ) / static_cast<DOUBLE>( m_list->m_rootSize );
	}

INT CListItem::concrete_compare( _In_ const CListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	switch ( subitem )
	{
		case column::COL_DESCRIPTION:
		case column::COL_EXTENSION:
			//return signum( wcscmp( m_name.get( ), other->m_name.get( ) ) );
			//ASSERT( false );//not ever reached?
			return default_compare( other );

		case column::COL_COLOR:
		case column::COL_BYTES:
			return signum( static_cast<std::int64_t>( m_bytes ) - static_cast<std::int64_t>( other->m_bytes ) );

		case column::COL_BYTESPERCENT:
			return signum( GetBytesFraction( ) - other->GetBytesFraction( ) );
		case column::COL_FILES_TYPEVIEW:
			return signum( static_cast<std::int64_t>( m_files ) - static_cast<std::int64_t>( other->m_files ) );

		case column::COL_ATTRIBUTES:
		default:
			ASSERT( false );
			return 0;
	}
	}

inline INT CListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	if ( ( subitem == column::COL_EXTENSION ) || ( subitem == column::COL_DESCRIPTION ) ) {
		default_compare( baseOther );
		}
	const auto other = static_cast< const CListItem * >( baseOther );
	return concrete_compare( other, subitem );
	}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CExtensionListControl, COwnerDrawnListCtrl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &( CExtensionListControl::OnLvnDeleteitem ) )
	ON_WM_SETFOCUS()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &( CExtensionListControl::OnLvnItemchanged ) )
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CExtensionListControl::CExtensionListControl ( CTypeView* const typeView ) : COwnerDrawnListCtrl( global_strings::type_str, 19 ), m_typeView( typeView ), m_rootSize ( 0 ), m_adjustedTiming( 0 ), m_averageExtensionNameLength( ), m_exts( nullptr ), m_exts_count( 0 ) { }


// As we will not receive WM_CREATE, we must do initialization in this extra method. The counterpart is OnDestroy().
void CExtensionListControl::Initialize( ) {

	SetSorting( column::COL_BYTES, false );

	InsertColumn(column::COL_EXTENSION,      _T( "Extension" ),   LVCFMT_LEFT,  60, column::COL_EXTENSION);
	InsertColumn(column::COL_COLOR,          _T( "Color" ),       LVCFMT_LEFT,  40, column::COL_COLOR);
	InsertColumn(column::COL_BYTES,          _T( "Bytes" ),       LVCFMT_RIGHT, 60, column::COL_BYTES);
	InsertColumn(column::COL_BYTESPERCENT,   _T( "% Bytes" ),     LVCFMT_RIGHT, 50, column::COL_BYTESPERCENT);
	InsertColumn(column::COL_FILES_TYPEVIEW, _T( "Files" ),       LVCFMT_RIGHT, 50, column::COL_FILES_TYPEVIEW);
	InsertColumn(column::COL_DESCRIPTION,    _T( "Description" ), LVCFMT_LEFT, 170, column::COL_DESCRIPTION);


	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	OnColumnsInserted( );

	// We don't use the list control's image list, but attaching an image list to the control ensures a proper line height.
	//SetImageList( NULL, LVSIL_SMALL );
	}

void CExtensionListControl::OnDestroy( ) {
	//SetImageList( NULL, LVSIL_SMALL );//Invalid parameter value!
	COwnerDrawnListCtrl::OnDestroy();
	}

_Ret_notnull_ CListItem* CExtensionListControl::GetListItem( _In_ const INT i ) const {
	const auto ret = reinterpret_cast< CListItem* > ( GetItemData( i ) );
	
	if ( ret != NULL ) {
		return ret;
		}
	ASSERT( false );
	displayWindowsMsgBoxWithMessage( L"GetListItem found NULL list item!" );
	std::terminate( );
	abort( );

	//Shut the compiler up. This code SHOULD NEVER execute, but if execution DOES get here, we'll purposely crash.
	( ( CListItem* )( 0 ) )->m_name;
	}


#ifdef new
#pragma push_macro("new")
#define WDS_TYPEVIEW_PUSHED_MACRO_NEW
#undef new
#endif

void CExtensionListControl::SetExtensionData( _In_ const std::vector<SExtensionRecord>* extData ) {
	VERIFY( DeleteAllItems( ) );
	LARGE_INTEGER frequency = help_QueryPerformanceFrequency( );
	auto startTime = help_QueryPerformanceCounter( );

	CListCtrl::SetItemCount( static_cast<int>( extData->size( ) + 1 ) );
	m_exts.reset( );
	
	const size_t ext_data_size = extData->size( );
	m_exts_count = ext_data_size;
	m_exts.reset( new CListItem[ ext_data_size ] );
	
	std::wstring::size_type total_name_length = 1u;
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		total_name_length += ( extData->at( i ).ext.length( ) + 1 );
		}

	m_name_pool.reset( total_name_length );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		const auto new_name_length = extData->at( i ).ext.length( );
		ASSERT( new_name_length < UINT16_MAX );

		PWSTR new_name_ptr = nullptr;
		//const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, extData->at( i ).ext );
		const HRESULT copy_res = m_name_pool.copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), extData->at( i ).ext );

		if ( !SUCCEEDED( copy_res ) ) {
			displayWindowsMsgBoxWithMessage( L"Failed to allocate & copy name str! (CExtensionListControl::SetExtensionData)(aborting!)" );
			displayWindowsMsgBoxWithMessage( extData->at( i ).ext.c_str( ) );
			}
		else {
			::new( m_exts.get( ) + i ) CListItem { this, ( *extData )[ i ].files, ( *extData )[ i ].bytes, ( *extData )[ i ].color, new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
			}
		}

	std::uint64_t totalSizeExtensionNameLength = 0;
	CListCtrl::SetItemCount( static_cast<int>( ext_data_size + 1 ) );
	TRACE( _T( "Built buffer of extension records, inserting....\r\n" ) );
	CWnd::SetRedraw( FALSE );
	const auto local_m_exts = m_exts.get( );

	//INT_PTR count = 0;
	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		totalSizeExtensionNameLength += static_cast<std::uint64_t>( ( local_m_exts + i )->m_name_length );
		}

	ASSERT( local_m_exts == m_exts.get( ) );
	
	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		InsertListItem( static_cast<INT_PTR>( i ), ( local_m_exts + i ) );
		}

	CWnd::SetRedraw( TRUE );
	auto doneTime = help_QueryPerformanceCounter( );
	ASSERT( frequency.QuadPart != 0 );
	const DOUBLE adjustedTimingFrequency = ( static_cast<DOUBLE>( 1.00 ) ) / static_cast<DOUBLE>( frequency.QuadPart );
	m_adjustedTiming = ( doneTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;


	//ASSERT( count == ext_data_size );
	m_averageExtensionNameLength = static_cast<DOUBLE>( totalSizeExtensionNameLength ) / static_cast<DOUBLE>( ext_data_size );
	SortItems( );
	}

#ifdef WDS_TYPEVIEW_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_TYPEVIEW_PUSHED_MACRO_NEW
#endif


void CExtensionListControl::SelectExtension( _In_z_ PCWSTR const ext ) {
	const auto countItems = CListCtrl::GetItemCount( );
	CWnd::SetRedraw( FALSE );
	for ( INT i = 0; i < countItems; i++ ) {
		if ( ( wcscmp( GetListItem( i )->m_name, ext ) == 0 ) && ( i >= 0 ) ) {
			TRACE( _T( "Selecting extension %s (item #%i)...\r\n" ), ext, i );
			CListCtrl::SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );//Unreachable code?
			CListCtrl::EnsureVisible( i, false );
			break;
			}

		}
	CWnd::SetRedraw( TRUE );
	}

_Ret_z_ PCWSTR const CExtensionListControl::GetSelectedExtension( ) const {
	auto pos = CListCtrl::GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return L"";
		}
	const auto i = CListCtrl::GetNextSelectedItem( pos );//SIX CYCLES PER INSTRUCTION!!!!
	const auto item = GetListItem( i );
	return item->m_name;
	}

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
	COwnerDrawnListCtrl::OnSetFocus( pOldWnd );
	m_frameptr->SetLogicalFocus( LOGICAL_FOCUS::LF_EXTENSIONLIST );
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
		if ( m_frameptr->GetDirstatView( ) != NULL ) {
			TRACE( _T( "TAB pressed! Focusing on directory list!\r\n" ) );
			m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_DIRECTORYLIST );
			}
		else {
			TRACE( _T( "TAB pressed! No directory list! Null focus!\r\n" ) );
			m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
			}
		}
	else if ( nChar == VK_ESCAPE ) {
		TRACE( _T( "ESCAPE pressed! Null focus!\r\n" ) );
		m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_NONE );
		}
	COwnerDrawnListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CTypeView, CView)

BEGIN_MESSAGE_MAP(CTypeView, CView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CTypeView::SetHighlightExtension( _In_ const std::wstring ext ) {
	auto Document = GetDocument( );

	if ( Document != NULL ) {
#ifndef DEBUG
		Document->SetHighlightExtension( std::move( ext ) );
#else
		Document->SetHighlightExtension( ext );
#endif
		
		if ( GetFocus( ) == &m_extensionListControl ) {
			Document->UpdateAllViews( this, UpdateAllViews_ENUM::HINT_EXTENSIONSELECTIONCHANGED );
			TRACE( _T( "Highlighted extension %s\r\n" ), ext.c_str( ) );
			}
		}
	ASSERT( Document != NULL );
	}

INT CTypeView::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( CView::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}

	RECT rect = { 0, 0, 0, 0 };
	VERIFY( m_extensionListControl.CreateEx( 0, LVS_SINGLESEL | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT, rect, this, _N_ID_EXTENSION_LIST_CONTROL ) );
	m_extensionListControl.SetExtendedStyle( m_extensionListControl.GetExtendedStyle( ) | LVS_EX_HEADERDRAGDROP );
	auto Options = GetOptions( );
	m_extensionListControl.ShowGrid( Options->m_listGrid );
	m_extensionListControl.ShowStripes( Options->m_listStripes );
	m_extensionListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
	m_extensionListControl.Initialize( );
	return 0;
	}

void CTypeView::OnUpdate0( ) {
	auto theDocument = GetDocument( );
	if ( theDocument != NULL ) {
		if ( m_showTypes && theDocument->IsRootDone( ) ) {
			m_extensionListControl.m_rootSize = theDocument->m_rootItem->size_recurse( );
			TRACE( _T( "Populating extension list...\r\n" ) );
			m_extensionListControl.SetExtensionData( theDocument->GetExtensionRecords( ) );
			TRACE( _T( "Finished populating extension list...\r\n" ) );
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
	m_extensionListControl.ShowGrid( thisOptions->m_listGrid );
	m_extensionListControl.ShowStripes( thisOptions->m_listStripes );
	m_extensionListControl.ShowFullRowSelection( thisOptions->m_listFullRowSelection );
	}

void CTypeView::OnUpdateHINT_TREEMAPSTYLECHANGED( ) {
	CWnd::InvalidateRect( NULL );
	m_extensionListControl.InvalidateRect( NULL );
	m_extensionListControl.GetHeaderCtrl( )->InvalidateRect( NULL );
	}

void CTypeView::OnUpdate( CView * /*pSender*/, LPARAM lHint, CObject * ) {
	switch ( lHint )
	{
		case UpdateAllViews_ENUM::HINT_NEWROOT:
		case 0:
			OnUpdate0( );
			// fall thru

		case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
		case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
			if ( m_showTypes ) {
				SetSelection( );
				}
			return;

		case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
			VERIFY( m_extensionListControl.RedrawWindow() );
			return;

		case UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED:
			return OnUpdateHINT_TREEMAPSTYLECHANGED( );

		case UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED:
			return OnUpdateHINT_LISTSTYLECHANGED( );

		//case UpdateAllViews_ENUM::HINT_ZOOMCHANGED:
		default:
			return;
	}
	}

void CTypeView::SetSelection( ) {
	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		const auto item = Document->m_selectedItem;
		//if ( item != NULL && item->m_type == IT_FILE ) {
		if ( ( item != NULL ) && ( item->m_child_info == nullptr ) ) {
			PCWSTR const selectedExt = m_extensionListControl.GetSelectedExtension( );
			//ASSERT( item->GetExtension( ).compare( item->CStyle_GetExtensionStrPtr( ) ) == 0 );
			if ( wcscmp( selectedExt, item->CStyle_GetExtensionStrPtr( ) ) != 0 ) {
				m_extensionListControl.SelectExtension( item->CStyle_GetExtensionStrPtr( ) );
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
	TRACE( _T( "CTypeView::Dump\r\n" ) );
	CView::Dump( dc );
	}
#endif //_DEBUG

_Must_inspect_result_ _Ret_maybenull_ CDirstatDoc* CTypeView::GetDocument( ) const {// Nicht-Debugversion ist inline
	//ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CDirstatDoc ) ) );
	//return static_cast<CDirstatDoc*>( m_pDocument );
	return STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	}

void CTypeView::SysColorChanged( ) {
	m_extensionListControl.SysColorChanged( );
	}



void CTypeView::OnSetFocus( CWnd* pOldWnd ) {
	UNREFERENCED_PARAMETER( pOldWnd );
	m_extensionListControl.SetFocus( );
	}

void CTypeView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize(nType, cx, cy);
	if ( ::IsWindow( m_extensionListControl.m_hWnd ) ) {
		const RECT rc = { 0, 0, cx, cy };
		
		ASSERT( ::IsWindow( m_hWnd ) );

		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY( ::MoveWindow( m_extensionListControl.m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );
		}
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
void trace_on_erase_bkgnd_typeview( ) {
	TRACE( _T( "CTypeView::OnEraseBkgnd!\r\n" ) );
	}

#else
#endif