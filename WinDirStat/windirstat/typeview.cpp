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

#include "mainframe.h"

//encourage inter-procedural optimization (and class-heirarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"


#include "treemap.h"
#include "dirstatdoc.h"
#include "ownerdrawnlistcontrol.h"
#include "windirstat.h"
#include "options.h"
#include "globalhelpers.h"

#pragma warning(suppress:4355)
CTypeView::CTypeView( ) : m_extensionListControl( this ), m_showTypes( true ) { }


CTypeView::~CTypeView( ) { }

bool CListItem::DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft ) const {
	//ASSERT_VALID( pdc );
	if ( subitem == column::COL_EXTENSION ) {
		DrawLabel( m_list, pdc, rc, state, width, focusLeft, true );
		return true;
		}
	else if ( subitem == column::COL_COLOR ) {
		DrawColor( pdc, rc, state, width );
		return true;
		}	
	else {
		if ( width != NULL ) {
			//Should never happen?
			*width = rc.Width( );
			}
		return false;
		}
	}

CListItem::CListItem( CListItem&& in ) {
	m_name = std::move( in.m_name );
	m_list = in.m_list;
	m_record = std::move( in.m_record );
	//m_image = std::move( in.m_image );
	}

void CListItem::DrawColor( _In_ CDC& pdc, _In_ CRect rc, _In_ const UINT state, _Out_opt_ INT* width ) const {
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

_Pre_satisfies_( subitem == column::COL_EXTENSION ) _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer_COL_EXTENSION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_EXTENSION );
	size_t chars_remaining = 0;


	const auto res = StringCchCopyExW( psz_text, strSize, m_name.get( ), NULL, &chars_remaining, 0 );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = ( m_name_length + 2u );
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		sizeBuffNeed = ( m_name_length + 2u );
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			ASSERT( m_name_length == wcslen( psz_text ) );
			chars_written = ( strSize - chars_remaining );
			sizeBuffNeed = SIZE_T_ERROR;
			}
		else {
			sizeBuffNeed = ( m_name_length + 2u );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );

	return res;
	}

_Pre_satisfies_( subitem == column::COL_COLOR ) _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer_COL_COLOR( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	ASSERT( strSize > 8 );
	ASSERT( subitem == column::COL_COLOR );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	size_t chars_remaining = 0;
	//auto res = StringCchPrintfW( psz_text, strSize, L"(color)" );
	const auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"(color)" );

	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 16;//Generic size needed, overkill;
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );

	return res;
	}

_Pre_satisfies_( subitem == column::COL_BYTES ) _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTES( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTES );
	const auto res = FormatBytes( m_record.bytes, psz_text, strSize, chars_written );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	return res;
	}

_Pre_satisfies_( subitem == column::COL_FILES_TYPEVIEW ) _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_FILES_TYPEVIEW );
	size_t chars_remaining = 0;
	//auto res = FormatBytes( m_record.files, psz_formatted_text, strSize );
	//auto res = StringCchPrintfW( psz_text, strSize, L"%I32u", m_record.files );
				
	const auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%I32u", m_record.files );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 64;//Generic size needed.
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );

	return res;

	}

_Pre_satisfies_( subitem == column::COL_DESCRIPTION ) _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer_COL_DESCRIPTION( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_DESCRIPTION );
	//auto res = StringCchPrintfW( psz_text, strSize, L"" );
	//if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
	//	chars_written = strSize;
	//	sizeBuffNeed = 2;//Generic size needed
	//	}
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
HRESULT CListItem::Text_WriteToStackBuffer_COL_BYTESPERCENT( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
#ifndef DEBUG
	UNREFERENCED_PARAMETER( subitem );
#endif
	ASSERT( subitem == column::COL_BYTESPERCENT );
	size_t chars_remaining = 0;
	const auto theDouble = GetBytesFraction( ) * 100;
	//auto res = StringCchPrintfW( psz_text, strSize, L"%.1f%%", theDouble );
	const auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"%.1f%%", theDouble );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		chars_written = strSize;
		sizeBuffNeed = 8;//Generic size needed, overkill;
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;

	}

_Success_( SUCCEEDED( return ) )
HRESULT CListItem::WriteToStackBuffer_default( WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	sizeBuffNeed = SIZE_T_ERROR;
	size_t chars_remaining = 0;
	ASSERT( strSize > 8 );
	const auto res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"BAD GetText_WriteToStackBuffer - subitem" );
	if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		if ( strSize > 8 ) {
			write_BAD_FMT( psz_text, chars_written );
			}
		else {
			chars_written = strSize;
			displayWindowsMsgBoxWithMessage( std::wstring( L"CListItem::" ) + std::wstring( global_strings::write_to_stackbuffer_err ) );
			}
		}
	else if ( ( res != STRSAFE_E_INSUFFICIENT_BUFFER ) && ( FAILED( res ) ) ) {
		chars_written = 0;
		}
	else {
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			}
		}
	ASSERT( SUCCEEDED( res ) );
	ASSERT( chars_written == wcslen( psz_text ) );
	return res;
	}



_Must_inspect_result_ _Success_( SUCCEEDED( return ) )
HRESULT CListItem::Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
	switch ( subitem )
	{
			case column::COL_NAME:
				return Text_WriteToStackBuffer_COL_EXTENSION( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_COLOR:
				return Text_WriteToStackBuffer_COL_COLOR( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTES:
				return Text_WriteToStackBuffer_COL_BYTES( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_FILES_TYPEVIEW:
				return Text_WriteToStackBuffer_COL_FILES_TYPEVIEW( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_DESCRIPTION:
				return Text_WriteToStackBuffer_COL_DESCRIPTION( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			case column::COL_BYTESPERCENT:
				return Text_WriteToStackBuffer_COL_BYTESPERCENT( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
			default:
				return WriteToStackBuffer_default( psz_text, strSize, sizeBuffNeed, chars_written );
//COL_ATTRIBUTES not handled: of course not! we don't have one of those!
#pragma warning(suppress:4061)
	}
	}

COLORREF CListItem::ItemTextColor( ) const {
	return default_item_text_color( );
	}

//std::wstring CListItem::Text( _In_ _In_range_( 0, INT32_MAX ) const column::ENUM_COL subitem ) const {
//	switch (subitem)
//	{
//		case column::COL_NAME:
//			return m_name;
//
//		case column::COL_COLOR:
//			return L"(color)";
//
//		case column::COL_BYTES:
//			return FormatBytes( m_record.bytes, GetOptions( )->m_humanFormat );
//
//		case column::COL_FILES_TYPEVIEW:
//			return FormatCount( m_record.files );
//
//		case column::COL_DESCRIPTION:
//			return L"";//DRAW_ICONS
//
//		case column::COL_BYTESPERCENT:
//			return GetBytesPercent( );
//
//		default:
//			ASSERT( false );
//			return L"";
////COL_ATTRIBUTES not handled: of course not! we don't have one of those!
//#pragma warning(suppress:4061)
//	}
//	}

//std::wstring CListItem::GetBytesPercent( ) const {//TODO, C-style string!
//	auto theDouble =  GetBytesFraction( ) * 100;
//	const size_t bufSize = 12;
//	wchar_t buffer[ bufSize ] = { 0 };
//	auto res = CStyle_FormatDouble( theDouble, buffer, bufSize );
//	if ( !SUCCEEDED( res ) ) {
//		rsize_t chars_written = 0;
//		write_BAD_FMT( buffer, chars_written );
//		}
//	else {
//		wchar_t percentage[ 2 ] = { '%', 0 };
//		auto res2 = StringCchCatW( buffer, bufSize, percentage );
//		if ( !SUCCEEDED( res2 ) ) {
//			return std::wstring( buffer ) + L'%';
//			}
//		}
//	return buffer;
//	}

DOUBLE CListItem::GetBytesFraction( ) const {
	if ( m_list->m_rootSize == 0 ) {
		return 0;
		}
	return static_cast<DOUBLE>( m_record.bytes ) / static_cast<DOUBLE>( m_list->m_rootSize );
	}

INT CListItem::Compare( _In_ const COwnerDrawnListItem* const baseOther, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
	auto other = static_cast< const CListItem * >( baseOther );

	switch ( subitem )
	{
		case column::COL_EXTENSION:
			return signum( wcscmp( m_name.get( ), other->m_name.get( ) ) );

		case column::COL_COLOR:
		case column::COL_BYTES:
			return signum( static_cast<std::int64_t>( m_record.bytes ) - static_cast<std::int64_t>( other->m_record.bytes ) );

		case column::COL_FILES_TYPEVIEW:
			return signum( static_cast<std::int64_t>( m_record.files ) - static_cast<std::int64_t>( other->m_record.files ) );

		case column::COL_DESCRIPTION:
			return 0;//DRAW_ICONS
		case column::COL_BYTESPERCENT:
			return signum( GetBytesFraction( ) - other->GetBytesFraction( ) );
			
		default:
			ASSERT( false );
			return 0;
//COL_ATTRIBUTES not handled: of course not! we don't have one of those!
#pragma warning(suppress:4061)
	}
	}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CExtensionListControl, COwnerDrawnListCtrl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_WM_SETFOCUS()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CExtensionListControl::CExtensionListControl ( CTypeView* const typeView ) : COwnerDrawnListCtrl( global_strings::type_str, 19 ), m_typeView( typeView ), m_rootSize ( 0 ), m_adjustedTiming( 0 ), m_averageExtensionNameLength( ), m_exts( nullptr ), m_exts_count( 0 ) { }

CExtensionListControl::~CExtensionListControl( ) {
	//delete[ ] m_exts;
	//m_exts = NULL;
	}



bool CExtensionListControl::GetAscendingDefault( _In_ const column::ENUM_COL column ) const {
	switch ( column )
	{
		case column::COL_EXTENSION:
		case column::COL_DESCRIPTION:
			return true;
		case column::COL_COLOR:
		case column::COL_BYTES:
		case column::COL_FILES_TYPEVIEW:
		case column::COL_BYTESPERCENT:
			return false;

		case column::COL_ATTRIBUTES:
		default:
			ASSERT(false);
			return true;
	}
	}

// As we will not receive WM_CREATE, we must do initialization in this extra method. The counterpart is OnDestroy().
void CExtensionListControl::Initialize( ) {

	SetSorting( column::COL_BYTES, false );

	InsertColumn(column::COL_EXTENSION,      _T( "Extension" ),   LVCFMT_LEFT,  60, column::COL_EXTENSION);
	InsertColumn(column::COL_COLOR,          _T( "Color" ),       LVCFMT_LEFT,  40, column::COL_COLOR);
	InsertColumn(column::COL_BYTES,          _T( "Bytes" ),       LVCFMT_RIGHT, 60, column::COL_BYTES);
	InsertColumn(column::COL_BYTESPERCENT,   _T( "% Bytes" ),     LVCFMT_RIGHT, 50, column::COL_BYTESPERCENT);
	InsertColumn(column::COL_FILES_TYPEVIEW, _T( "Files" ),       LVCFMT_RIGHT, 50, column::COL_FILES_TYPEVIEW);
	InsertColumn(column::COL_DESCRIPTION,    _T( "Description" ), LVCFMT_LEFT, 170, column::COL_DESCRIPTION);

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
	displayWindowsMsgBoxWithMessage( std::wstring( L"GetListItem found NULL list item!" ) );
	std::terminate( );
	abort( );

	//Shut the compiler up. This code SHOULD NEVER execute, but if execution DOES get here, we'll purposely crash.
	( ( CListItem* )( 0 ) )->m_name;
	}

void CExtensionListControl::SetExtensionData( _In_ const std::vector<SExtensionRecord>* extData ) {
	VERIFY( DeleteAllItems( ) );
	LARGE_INTEGER frequency = help_QueryPerformanceFrequency( );
	auto startTime = help_QueryPerformanceCounter( );

	SetItemCount( static_cast<int>( extData->size( ) + 1 ) );
	//delete[ ] m_exts;
	//m_exts = NULL;
	m_exts.reset( );
	//m_extensionItems.clear( );
	
	const size_t ext_data_size = extData->size( );
	m_exts_count = ext_data_size;

	m_exts.reset( new CListItem[ ext_data_size ] );

	for ( size_t i = 0; i < ext_data_size; ++i ) {
		const auto new_name_length = extData->at( i ).ext.length( );
		ASSERT( new_name_length < UINT16_MAX );
		//_Null_terminated_ _Field_size_( new_name_length + 1u ) PWSTR new_name_ptr = new wchar_t[ new_name_length + 1u ];
		//const auto cpy_res = wcscpy_s( new_name_ptr, ( new_name_length + 1u ), extData->at( i ).ext.c_str( ) );
		//if ( cpy_res != 0 ) {
		//	std::terminate( );
		//	}
		//ASSERT( wcslen( new_name_ptr ) == new_name_length );
		//ASSERT( wcscmp( new_name_ptr, extData->at( i ).ext.c_str( ) ) == 0 );

		PWSTR new_name_ptr = nullptr;
		const HRESULT copy_res = allocate_and_copy_name_str( new_name_ptr, new_name_length, extData->at( i ).ext );
		if ( !SUCCEEDED( copy_res ) ) {
			_CrtDbgBreak( );
			}
		else {
			::new( m_exts.get( ) + i ) CListItem { this, ( *extData )[ i ], new_name_ptr, static_cast< std::uint16_t >( new_name_length ) };
			}
		}

	//m_extensionItems.reserve( ext_data_size + 1 );
	//for ( size_t i = 0; i < ext_data_size; ++i ) {
	//	m_extensionItems.emplace_back( m_exts + i );
	//	}

	std::uint64_t totalSizeExtensionNameLength = 0;
	SetItemCount( static_cast<int>( ext_data_size + 1 ) );
	TRACE( _T( "Built buffer of extension records, inserting....\r\n" ) );

#ifdef PERF_DEBUG_SLEEP
	Sleep( 1000 );
#endif

	SetRedraw( FALSE );

	//INT_PTR count = 0;
	for ( size_t i = 0; i < ext_data_size; ++i ) {
		//ASSERT( m_extensionItems.at( i ) == ( m_exts + i ) );
		totalSizeExtensionNameLength += static_cast<std::uint64_t>( ( m_exts.get( ) + i )->m_name_length );
		//count++;
		}

	for ( size_t i = 0; i < ext_data_size; ++i ) {
		InsertListItem( static_cast<INT_PTR>( i ), ( m_exts.get( ) + i ) );
		}


	//ASSERT( count == ext_data_size );

	SetRedraw( TRUE );
	auto doneTime = help_QueryPerformanceCounter( );
	ASSERT( frequency.QuadPart != 0 );
	const DOUBLE adjustedTimingFrequency = ( static_cast<DOUBLE>( 1.00 ) ) / static_cast<DOUBLE>( frequency.QuadPart );
	m_adjustedTiming = ( doneTime.QuadPart - startTime.QuadPart ) * adjustedTimingFrequency;


	//ASSERT( count == ext_data_size );
	m_averageExtensionNameLength = static_cast<DOUBLE>( totalSizeExtensionNameLength ) / static_cast<DOUBLE>( ext_data_size );
	SortItems( );
	}

void CExtensionListControl::SelectExtension( _In_ const std::wstring ext ) {
	const auto countItems = this->GetItemCount( );
	SetRedraw( FALSE );
	for ( INT i = 0; i < countItems; i++ ) {
		if ( ( wcscmp( GetListItem( i )->m_name.get( ), ext.c_str( ) ) == 0 ) && ( i >= 0 ) ) {
			TRACE( _T( "Selecting extension %s (item #%i)...\r\n" ), ext.c_str( ), i );
			SetItemState( i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );//Unreachable code?
			EnsureVisible( i, false );
			break;
			}

		}
	SetRedraw( TRUE );
	}

const std::wstring CExtensionListControl::GetSelectedExtension( ) const {
	auto pos = GetFirstSelectedItemPosition( );
	if ( pos == NULL ) {
		return _T( "" );
		}
	const auto i = GetNextSelectedItem( pos );//SIX CYCLES PER INSTRUCTION!!!!
	const auto item = GetListItem( i );
	return std::wstring( item->m_name.get( ) );
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
	ASSERT( GetMainFrame( ) == m_frameptr );
	m_frameptr->SetLogicalFocus( LOGICAL_FOCUS::LF_EXTENSIONLIST );
	}

void CExtensionListControl::OnLvnItemchanged( NMHDR *pNMHDR, LRESULT *pResult ) {
	LPNMLISTVIEW pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	if ( ( pNMLV->uNewState bitand LVIS_SELECTED ) != 0 ) {
		m_typeView->SetHighlightExtension( std::move( GetSelectedExtension( ) ) );
		}
	*pResult = 0;
	}

void CExtensionListControl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	if ( nChar == VK_TAB ) {
		ASSERT( GetMainFrame( ) == m_frameptr );
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
		ASSERT( GetMainFrame( ) == m_frameptr );
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
	m_extensionListControl.ShowGrid( thisOptions->m_listGrid );
	m_extensionListControl.ShowStripes( thisOptions->m_listStripes );
	m_extensionListControl.ShowFullRowSelection( thisOptions->m_listFullRowSelection );
	}

void CTypeView::OnUpdateHINT_TREEMAPSTYLECHANGED( ) {
	InvalidateRect( NULL );
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

		case UpdateAllViews_ENUM::HINT_ZOOMCHANGED:
		default:
			break;
	}
	}

void CTypeView::SetSelection( ) {
	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		const auto item = Document->m_selectedItem;
		//if ( item != NULL && item->m_type == IT_FILE ) {
		if ( ( item != NULL ) && ( item->m_children == nullptr ) ) {
			const auto selectedExt = m_extensionListControl.GetSelectedExtension( );
			ASSERT( item->GetExtension( ).compare( item->CStyle_GetExtensionStrPtr( ) ) == 0 );
			if ( selectedExt.compare( item->CStyle_GetExtensionStrPtr( ) ) != 0 ) {
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
	if ( IsWindow( m_extensionListControl.m_hWnd ) ) {
		CRect rc( 0, 0, cx, cy );
		m_extensionListControl.MoveWindow( rc );
		}
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
