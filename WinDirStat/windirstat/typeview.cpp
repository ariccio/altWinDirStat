// typeview.cpp		- Implementation of CExtensionListControl and CTypeView
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#include "stdafx.h"

#pragma once



#ifndef WDS_TYPEVIEW_CPP
#define WDS_TYPEVIEW_CPP


WDS_FILE_INCLUDE_MESSAGE

#include "mainframe.h"
#include "datastructures.h"

//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
//#include "item.h"
#include "typeview.h"

#include "ChildrenHeapManager.h"

#include "treemap.h"
#include "dirstatdoc.h"
//#include "ownerdrawnlistcontrol.h"
#include "windirstat.h"
#include "options.h"
#include "globalhelpers.h"
#include "signum.h"
#include "LOGICAL_FOCUS_enum.h"

#include "stringformatting.h"


namespace {
	constexpr const UINT _N_ID_EXTENSION_LIST_CONTROL = 4711u;
	}

#pragma warning(suppress:4355)
CTypeView::CTypeView( ) : m_showTypes(true), m_extensionListControl( this ) {
	m_showTypes = CPersistence::GetShowFileTypes( );
	}


//CTypeView::~CTypeView( ) { }

bool CListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const noexcept {
	//ASSERT_VALID( pdc );
	//Why are we bothering to draw this ourselves?
	if ( subitem == column::COL_EXTENSION ) {
		ASSERT( list == m_list );
		/*
		_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/
		CFont* const list_font = list->GetFont( );
		const bool list_has_focus = list->HasFocus( );
		const bool list_is_show_selection_always = list->IsShowSelectionAlways( );
		const COLORREF list_highlight_text_color = list->GetHighlightTextColor( );
		const COLORREF list_highlight_color = list->GetHighlightColor( );
		const bool list_is_full_row_selection = list->m_showFullRowSelection;
		//list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection

		COwnerDrawnListItem::DrawLabel( hDC, rc, state, width, focusLeft, true, list_font->m_hObject, list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
		return true;
		}
	else if ( subitem == column::COL_COLOR ) {
		CListItem::DrawColor( hDC, rc, state, width );
		return true;
		}	
	if ( width == nullptr ) {
		return false;
		}
	//Should never happen?
	*width = ( rc.right - rc.left );
	return false;

	}

void CListItem::DrawColor( _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* width ) const noexcept {
	if ( width != nullptr ) {
		*width = 40;
		return;
		}

	COwnerDrawnListItem::DrawSelection( hDC, rc, state, m_list->HasFocus( ), m_list->IsShowSelectionAlways( ), m_list->GetHighlightColor( ), m_list->m_showFullRowSelection );
	VERIFY( ::InflateRect( &rc, -( 2 ), -( 3 ) ) );

	if ( ( rc.right <= rc.left ) || ( rc.bottom <= rc.top ) ) {
		return;
		}

	CTreemap treemap;
#ifdef DEBUG
	treemap.m_is_typeview = true;
#endif
	treemap.DrawColorPreview( hDC, rc, color, &( GetOptions( )->m_treemapOptions ) );
	}

_Pre_satisfies_( subitem == column::COL_BYTES ) _Success_( SUCCEEDED( return ) )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTES );
	const HRESULT res = wds_fmt::FormatBytes( m_bytes, psz_text, strSize, chars_written, sizeBuffNeed );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		ASSERT( chars_written == ::wcslen( psz_text ) );
		return res;
		}
	//handled in FormatBytes.
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	return res;
	}

_Pre_satisfies_( subitem == column::COL_FILES_TYPEVIEW ) _Success_( SUCCEEDED( return ) )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_FILES_TYPEVIEW );
	size_t chars_remaining = 0;

	const HRESULT res = ::StringCchPrintfExW( psz_text, strSize, nullptr, &chars_remaining, 0, L"%I32u", m_files );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == ::wcslen( psz_text ) );
		return res;
		}
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 64u, sizeBuffNeed, chars_written );
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintfExW" );
	chars_written = 0;
	return res;
	}

_Pre_satisfies_( subitem == column::COL_DESCRIPTION ) _Success_( SUCCEEDED( return ) ) _Pre_satisfies_( strSize > 0 )
const HRESULT CListItem::Text_WriteToStackBuffer_COL_DESCRIPTION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_DESCRIPTION );
	ASSERT( strSize > 0 );
	//This routine is NOPed out.
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
const HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTESPERCENT( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTESPERCENT );
	size_t chars_remaining = 0;
	const auto theDouble = CListItem::GetBytesFraction( ) * 100;
	const HRESULT res = ::StringCchPrintfExW( psz_text, strSize, nullptr, &chars_remaining, 0, L"%.1f%%", theDouble );
	ASSERT( SUCCEEDED( res ) );
	if ( SUCCEEDED( res ) ) {
		chars_written = ( strSize - chars_remaining );
		ASSERT( chars_written == ::wcslen( psz_text ) );
		return res;
		}
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		handle_stack_insufficient_buffer( strSize, 8u, sizeBuffNeed, chars_written );
		return res;
		}
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintfExW" );
	chars_written = 0;
	return res;
	}

_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const noexcept {
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
				return COwnerDrawnListItem::WriteToStackBuffer_default( subitem, psz_text, strSize, sizeBuffNeed, chars_written, L"CListItem::" );
			case column::COL_DESCRIPTION:
				return CListItem::Text_WriteToStackBuffer_COL_DESCRIPTION( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTES:
				return CListItem::Text_WriteToStackBuffer_COL_BYTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTESPERCENT:
				return CListItem::Text_WriteToStackBuffer_COL_BYTESPERCENT( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_FILES_TYPEVIEW:
				return CListItem::Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
	}
	}

DOUBLE CListItem::GetBytesFraction( ) const noexcept {
	if ( m_list->m_rootSize == 0 ) {
		return 0;
		}
	return static_cast<DOUBLE>( m_bytes ) / static_cast<DOUBLE>( m_list->m_rootSize );
	}

INT CListItem::concrete_compare( _In_ const CListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const noexcept {
	switch ( subitem )
	{
		case column::COL_DESCRIPTION:
		case column::COL_EXTENSION:
			return COwnerDrawnListItem::default_compare( other );

		case column::COL_COLOR:
		case column::COL_BYTES:
			return signum( static_cast<std::int64_t>( m_bytes ) - static_cast<std::int64_t>( other->m_bytes ) );

		case column::COL_BYTESPERCENT:
			return signum( CListItem::GetBytesFraction( ) - other->GetBytesFraction( ) );
		case column::COL_FILES_TYPEVIEW:
			return signum( static_cast<std::int64_t>( m_files ) - static_cast<std::int64_t>( other->m_files ) );

		case column::COL_ATTRIBUTES:
		default:
			ASSERT( false );
			return 0;
	}
	}

inline INT CListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const noexcept {
	if ( ( subitem == column::COL_EXTENSION ) || ( subitem == column::COL_DESCRIPTION ) ) {
		COwnerDrawnListItem::default_compare( baseOther );
		}
	const CListItem* const other = static_cast< const CListItem * >( baseOther );
	return CListItem::concrete_compare( other, subitem );
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

CExtensionListControl::CExtensionListControl ( CTypeView* const typeView ) : COwnerDrawnListCtrl( global_strings::type_str, 19 ), m_averageExtensionNameLength( ), m_rootSize (0), m_adjustedTiming(0), m_exts_count( 0 ), m_exts(nullptr), m_typeView(typeView) { }


// As we will not receive WM_CREATE, we must do initialization in this extra method. The counterpart is OnDestroy().
void CExtensionListControl::Initialize( ) noexcept {

	COwnerDrawnListCtrl::SetSorting( column::COL_BYTES, false );

	CListCtrl::InsertColumn(column::COL_EXTENSION,      _T( "Extension" ),   LVCFMT_LEFT,  60, column::COL_EXTENSION);
	CListCtrl::InsertColumn(column::COL_COLOR,          _T( "Color" ),       LVCFMT_LEFT,  40, column::COL_COLOR);
	CListCtrl::InsertColumn(column::COL_BYTES,          _T( "Bytes" ),       LVCFMT_RIGHT, 60, column::COL_BYTES);
	CListCtrl::InsertColumn(column::COL_BYTESPERCENT,   _T( "% Bytes" ),     LVCFMT_RIGHT, 50, column::COL_BYTESPERCENT);
	CListCtrl::InsertColumn(column::COL_FILES_TYPEVIEW, _T( "Files" ),       LVCFMT_RIGHT, 50, column::COL_FILES_TYPEVIEW);
	CListCtrl::InsertColumn(column::COL_DESCRIPTION,    _T( "Description" ), LVCFMT_LEFT, 170, column::COL_DESCRIPTION);


	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	COwnerDrawnListCtrl::OnColumnsInserted( );

	// We don't use the list control's image list, but attaching an image list to the control ensures a proper line height.
	//SetImageList( NULL, LVSIL_SMALL );
	}

void CExtensionListControl::OnDestroy( ) {
	//SetImageList( NULL, LVSIL_SMALL );//Invalid parameter value!
	COwnerDrawnListCtrl::OnDestroy();
	}

_Ret_notnull_ CListItem* CExtensionListControl::GetListItem( _In_ _In_range_( >=, 0 ) const INT i ) const {
	ASSERT( i >= 0 );
	const auto ret = reinterpret_cast< CListItem* > ( CListCtrl::GetItemData( i ) );
	
	if ( ret != nullptr) {
		return ret;
		}
	ASSERT( false );
	displayWindowsMsgBoxWithMessage( L"GetListItem found NULL list item!" );
	std::terminate( );

	//Shut the compiler up. This code SHOULD NEVER execute, but if execution DOES get here, we'll purposely crash.
	( ( CListItem* )( 0 ) )->m_name;
	}


#ifdef new
#pragma push_macro("new")
#define WDS_TYPEVIEW_PUSHED_MACRO_NEW
#undef new
#endif

void CExtensionListControl::SetExtensionData( _In_ const std::vector<SExtensionRecord>* extData ) {
	VERIFY( CListCtrl::DeleteAllItems( ) );
	const LARGE_INTEGER frequency = help_QueryPerformanceFrequency( );
	const LARGE_INTEGER startTime = help_QueryPerformanceCounter( );

	CListCtrl::SetItemCount( static_cast<int>( extData->size( ) + 1 ) );
	m_exts.reset( );
	
	const size_t ext_data_size = extData->size( );
	m_exts_count = ext_data_size;
	m_exts.reset( new CListItem[ ext_data_size ] );
	
	std::wstring::size_type total_name_length = 1u;
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		total_name_length += ( extData->at( i ).ext.length( ) + 1 );
		}

	m_name_pool.reset( new Children_String_Heap_Manager( total_name_length ) );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		const auto new_name_length = extData->at( i ).ext.length( );
		ASSERT( new_name_length < UINT16_MAX );

		PWSTR new_name_ptr = nullptr;
		const HRESULT copy_res = m_name_pool->m_buffer_impl->copy_name_str_into_buffer( new_name_ptr, ( new_name_length + 1u ), extData->at( i ).ext );

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

	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		totalSizeExtensionNameLength += static_cast<std::uint64_t>( ( local_m_exts + i )->m_name_length );
		}

	ASSERT( local_m_exts == m_exts.get( ) );
	
	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		COwnerDrawnListCtrl::InsertListItem( static_cast<INT_PTR>( i ), ( local_m_exts + i ) );
		}

	CWnd::SetRedraw( TRUE );
	const LARGE_INTEGER doneTime = help_QueryPerformanceCounter( );
	ASSERT( frequency.QuadPart != 0 );
	const DOUBLE adjustedTimingFrequency = ( static_cast<DOUBLE>( 1.00 ) ) / static_cast<DOUBLE>( frequency.QuadPart );
	m_adjustedTiming = ( doneTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;


	//ASSERT( count == ext_data_size );
	m_averageExtensionNameLength = static_cast<DOUBLE>( totalSizeExtensionNameLength ) / static_cast<DOUBLE>( ext_data_size );
	COwnerDrawnListCtrl::SortItems( );
	}

#ifdef WDS_TYPEVIEW_PUSHED_MACRO_NEW
#pragma pop_macro("new")
#undef WDS_TYPEVIEW_PUSHED_MACRO_NEW
#endif


void CExtensionListControl::SelectExtension( _In_z_ PCWSTR const ext ) noexcept {
	const auto countItems = CListCtrl::GetItemCount( );
	CWnd::SetRedraw( FALSE );
	for ( INT i = 0; i < countItems; i++ ) {
		if ( ( ::wcscmp( CExtensionListControl::GetListItem( i )->m_name, ext ) == 0 ) && ( i >= 0 ) ) {
			TRACE( _T( "Selecting extension %s (item #%i)...\r\n" ), ext, i );
			CListCtrl::SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );//Unreachable code?
			CListCtrl::EnsureVisible( i, false );
			break;
			}

		}
	CWnd::SetRedraw( TRUE );
	}

_Ret_z_ PCWSTR const CExtensionListControl::GetSelectedExtension( ) const noexcept {
	auto pos = CListCtrl::GetFirstSelectedItemPosition( );
	if ( pos == nullptr) {
		return L"";
		}
	const auto i = CListCtrl::GetNextSelectedItem( pos );//SIX CYCLES PER INSTRUCTION!!!!
	const CListItem* const item = CExtensionListControl::GetListItem( i );
	return item->m_name;
	}

void CExtensionListControl::OnLvnDeleteitem( NMHDR *pNMHDR, LRESULT *pResult ) {
	ASSERT( pNMHDR != nullptr );
	ASSERT( pResult != nullptr );
	if ( pNMHDR != nullptr ) {
		auto lv = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
		lv->lParam = { NULL };
		}

	if ( pResult != nullptr ) {
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
	const NMLISTVIEW* const pNMLV = reinterpret_cast< const LPNMLISTVIEW >( pNMHDR );
	if ( ( pNMLV->uNewState bitand LVIS_SELECTED ) != 0 ) {
		m_typeView->SetHighlightExtension( CExtensionListControl::GetSelectedExtension( ) );
		}
	*pResult = 0;
	}

void CExtensionListControl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	if ( nChar == VK_TAB ) {
		if ( m_frameptr->GetDirstatView( ) != nullptr) {
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

/*
From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:598:
#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:594:
#define _RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))

C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:707:
#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject, NULL)

C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:688:
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL, class_init }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); }


Sooo...
	IMPLEMENT_DYNCREATE(CTypeView, CView)
		--becomes--
	CObject* PASCAL CTypeView::CreateObject() \
		{ return new CTypeView; } \
	IMPLEMENT_RUNTIMECLASS(CTypeView, CView, 0xFFFF, CTypeView::CreateObject, NULL)
		--becomes--
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass CTypeView::classCTypeView = { \
		"CTypeView", sizeof(class CTypeView), 0xFFFF, CTypeView::CreateObject, \
			RUNTIME_CLASS(CView), NULL, NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(CTypeView); }

And...
	RUNTIME_CLASS(CView)
		--becomes--
	_RUNTIME_CLASS(CView)
		--becomes--
	((CRuntimeClass*)(&CView::classCView))

And...
	RUNTIME_CLASS(CTypeView)
		--becomes--
	_RUNTIME_CLASS(CTypeView)
		--becomes--
	((CRuntimeClass*)(&CTypeView::classCTypeView))


*/

//IMPLEMENT_DYNCREATE(CTypeView, CView)
CObject* PASCAL CTypeView::CreateObject() {
	return new CTypeView;
	}

AFX_COMDAT const CRuntimeClass CTypeView::classCTypeView = {
	"CTypeView" /*m_lpszClassName*/,
	sizeof(class CTypeView) /*m_nObjectSize*/,
	0xFFFF /*wSchema*/,
	CTypeView::CreateObject /*pfnNew*/,
	(const_cast<CRuntimeClass*>(&CView::classCView)) /*RUNTIME_CLASS(CView)*/ /*m_pBaseClass*/,
	nullptr  /*m_pNextClass*/,
	nullptr  /*class_init*/
	};

CRuntimeClass* CTypeView::GetRuntimeClass() const {
	return 	(const_cast<CRuntimeClass*>(&CTypeView::classCTypeView)) /*RUNTIME_CLASS(CTypeView)*/;
	}


BEGIN_MESSAGE_MAP(CTypeView, CView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CTypeView::SetHighlightExtension( _In_ const std::wstring ext ) noexcept {
	auto Document = CTypeView::GetDocument( );

	if ( Document != nullptr ) {
		Document->SetHighlightExtension( ext );
		
		if ( GetFocus( ) == &m_extensionListControl ) {
			Document->UpdateAllViews( this, UpdateAllViews_ENUM::HINT_EXTENSIONSELECTIONCHANGED );
			TRACE( _T( "Highlighted extension %s\r\n" ), ext.c_str( ) );
			}
		}
	ASSERT( Document != nullptr );
	}

INT CTypeView::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( CView::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}

	RECT rect = { 0, 0, 0, 0 };
	VERIFY( m_extensionListControl.CreateEx( 0, LVS_SINGLESEL | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT, rect, this, _N_ID_EXTENSION_LIST_CONTROL ) );
	m_extensionListControl.SetExtendedStyle( m_extensionListControl.GetExtendedStyle( ) | LVS_EX_HEADERDRAGDROP );
	const COptions* const Options = GetOptions( );
	m_extensionListControl.ShowGrid( Options->m_listGrid );
	m_extensionListControl.ShowStripes( Options->m_listStripes );
	m_extensionListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
	m_extensionListControl.Initialize( );
	return 0;
	}

void CTypeView::OnUpdate0( ) noexcept {
	auto theDocument = CTypeView::GetDocument( );
	if ( theDocument != nullptr ) {
		if ( m_showTypes && theDocument->IsRootDone( ) ) {
			m_extensionListControl.m_rootSize = theDocument->m_rootItem->size_recurse( );
			TRACE( _T( "Populating extension list...\r\n" ) );
			m_extensionListControl.SetExtensionData( theDocument->GetExtensionRecords( ) );
			TRACE( _T( "Finished populating extension list...\r\n" ) );
			// If there is no vertical scroll bar, the header control doesn't repaint correctly. Don't know why. But this helps:
			hwnd::InvalidateErase(m_extensionListControl.GetHeaderCtrl()->m_hWnd);
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		}
	else {
		if ( m_showTypes ) {
			hwnd::InvalidateErase(m_extensionListControl.GetHeaderCtrl()->m_hWnd);
			}
		else {
			m_extensionListControl.DeleteAllItems( );
			}
		AfxCheckMemory( );
		ASSERT( false );
		}

	}

void CTypeView::OnUpdateHINT_LISTSTYLECHANGED( ) noexcept {
	const COptions* const thisOptions = GetOptions( );
	m_extensionListControl.ShowGrid( thisOptions->m_listGrid );
	m_extensionListControl.ShowStripes( thisOptions->m_listStripes );
	m_extensionListControl.ShowFullRowSelection( thisOptions->m_listFullRowSelection );
	}

void CTypeView::OnUpdateHINT_TREEMAPSTYLECHANGED( ) noexcept {
	hwnd::InvalidateErase(m_hWnd);	
	hwnd::InvalidateErase(m_extensionListControl.m_hWnd);
	hwnd::InvalidateErase(m_extensionListControl.GetHeaderCtrl()->m_hWnd);
	}

void CTypeView::OnUpdate( CView * /*pSender*/, LPARAM lHint, CObject * ) {
	switch ( lHint )
	{
		case UpdateAllViews_ENUM::HINT_NEWROOT:
		case 0:
			CTypeView::OnUpdate0( );
			// fall thru

		case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
		case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
			if ( m_showTypes ) {
				CTypeView::SetSelection( );
				}
			return;

		case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
			VERIFY( m_extensionListControl.RedrawWindow() );
			return;

		case UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED:
			return CTypeView::OnUpdateHINT_TREEMAPSTYLECHANGED( );

		case UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED:
			return CTypeView::OnUpdateHINT_LISTSTYLECHANGED( );
		default:
			return;
	}
	}

void CTypeView::SetSelection( ) noexcept {
	const CDirstatDoc* const Document = CTypeView::GetDocument( );
	ASSERT( Document != nullptr );
	if ( Document == nullptr ) {
		return;
		}
	const auto item = Document->m_selectedItem;
	if ( item == nullptr ) {
		return;
		}
	if ( item->m_child_info.m_child_info_ptr == nullptr ) {
		PCWSTR const selectedExt = m_extensionListControl.GetSelectedExtension( );
		if ( ::wcscmp( selectedExt, item->CStyle_GetExtensionStrPtr( ) ) != 0 ) {
			m_extensionListControl.SelectExtension( item->CStyle_GetExtensionStrPtr( ) );
			}
		}
	}

_Must_inspect_result_ _Ret_maybenull_ CDirstatDoc* CTypeView::GetDocument( ) const noexcept {// Nicht-Debugversion ist inline
	return static_cast<CDirstatDoc*>( m_pDocument );
	}

void CTypeView::SysColorChanged( ) noexcept {
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
void trace_on_erase_bkgnd_typeview( ) noexcept {
	TRACE( _T( "CTypeView::OnEraseBkgnd!\r\n" ) );
	}

#else
#endif