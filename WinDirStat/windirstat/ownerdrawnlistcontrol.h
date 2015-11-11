// ownerdrawnlistcontrol.h	- Declaration of COwnerDrawnListCtrl and COwnerDrawnListItem
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.


#pragma once

#include "stdafx.h"

#ifndef WDS_OWNERDRAWNLISTCONTROL_H
#define WDS_OWNERDRAWNLISTCONTROL_H

WDS_FILE_INCLUDE_MESSAGE

#include "datastructures.h"

#include "mainframe.h"
#include "globalhelpers.h"
#include "options.h"
#include "windirstat.h"
#include "ScopeGuard.h"
#include "macros_that_scare_small_children.h"
#include "signum.h"
#include "stringformatting.h"

class COwnerDrawnListItem;
class COwnerDrawnListCtrl;


//Based on MFC's CDC/CClientDC
//MFC's CDC/CClientDC is in VC/atlmfc/src/mfc/wingdi.cpp
struct SimpleClientDeviceContext {
	HDC m_hDC = NULL;          // "The output DC (must be first data member)"
	HWND m_hWnd;


	/*
	BOOL CDC::Attach(HDC hDC)
	{
		ASSERT(m_hDC == NULL);      // only attach once, detach on destroy
		ASSERT(m_hAttribDC == NULL);    // only attach to an empty DC

		if (hDC == NULL)
		{
			return FALSE;
		}
		// remember early to avoid leak
		m_hDC = hDC;
		CHandleMap* pMap = afxMapHDC(TRUE); // create map if not exist
		ASSERT(pMap != NULL);
		pMap->SetPermanent(m_hDC, this);

		SetAttribDC(m_hDC);     // Default to same as output
		return TRUE;
	}
	CClientDC::CClientDC(CWnd* pWnd)
	{
		ASSERT(pWnd == NULL || ::IsWindow(pWnd->m_hWnd));

		if (!Attach(::GetDC(m_hWnd = pWnd->GetSafeHwnd())))
			AfxThrowResourceException();
	}
	*/
	SimpleClientDeviceContext( _In_opt_ const HWND hWnd ) {
		//CClientDC::CClientDC
		//CDC::Attach

		ASSERT(hWnd == NULL || ::IsWindow(hWnd));
		m_hWnd = hWnd;
		m_hDC = ::GetDC( m_hWnd );
		if ( m_hDC == NULL ) {
			std::terminate( );
			}
		}
	~SimpleClientDeviceContext( ) {
		::ReleaseDC( m_hWnd, m_hDC );
		}
	};


namespace CColorSpace {
	// Returns the brightness of color. Brightness is a value between 0 and 1.0.
	_Ret_range_( 0, 1 ) static DOUBLE GetColorBrightness( _In_ const COLORREF color ) {
		return ( GetRValue( color ) + GetGValue( color ) + GetBValue( color ) ) / 255.0 / 3.0;
		}

	}

namespace {

	// SSorting. A sorting specification. We sort by column1, and if two items equal in column1, we sort them by column2.
	struct SSorting final {
		SSorting( ) : column1 { column::COL_NAME }, column2 { column::COL_NAME }, ascending1 { false }, ascending2 { true } { }
		_Field_range_( 0, 8 ) column::ENUM_COL  column1;
		_Field_range_( 0, 8 ) column::ENUM_COL  column2;
							  bool              ascending2 : 1;
							  //C4820: 'SSorting' : '3' bytes padding added after data member 'SSorting::ascending1'
							  bool              ascending1 : 1;
		};



	inline void fixup_align_for_indent( _In_ const bool indent, _Inout_ RECT& rcRest ) {
		// Increase indentation according to tree-level
		if ( indent ) {
			//add 5
			rcRest.left += GENERAL_INDENT;
			}
		}

	const INT  TEXT_X_MARGIN    = 6;	// Horizontal distance of the text from the edge of the item rectangle
	const UINT LABEL_INFLATE_CX = 3;// How much the label is enlarged, to get the selection and focus rectangle
	const UINT LABEL_Y_MARGIN   = 2;
	}


#pragma pack(push, 1)
#pragma message( "Whoa there! I'm changing the natural data alignment for COwnerDrawnListItem. Look for a message that says I'm restoring it!" )

template<class derived_class>
class COwnerDrawnListItemImpl {
public:
	// Return value is true, if the item draws itself. width != NULL -> only determine width, do not draw. If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	bool DrawSubitem_( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const {
		return static_cast<derived_class*>( this )->DrawSubitem( subitem, pdc, rc, state, width, focusLeft, list );
		}
	};

// COwnerDrawnListItem. An item in a COwnerDrawnListCtrl. Some columns (subitems) may be owner drawn (DrawSubitem() returns true), COwnerDrawnListCtrl draws the texts (GetText()) of all others.
// DrawLabel() draws a standard label (width image, text, selection and focus rect)
class COwnerDrawnListItem {

	virtual COLORREF ItemTextColor( ) const = 0;
	virtual INT      Compare( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const = 0;

	_Must_inspect_result_ _Success_( SUCCEEDED( return ) ) _Pre_satisfies_( subitem != column::COL_NAME )
	virtual HRESULT  Text_WriteToStackBuffer( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const = 0;

	// Return value is true, if the item draws itself. width != NULL -> only determine width, do not draw. If focus rectangle shall not begin leftmost, set *focusLeft to the left edge of the desired focus rectangle.
	virtual bool     DrawSubitem( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const = 0;

public:
	INT      compare_interface          ( _In_ const COwnerDrawnListItem* const other, RANGE_ENUM_COL const column::ENUM_COL subitem ) const {
		return Compare( other, subitem );
		}

	COLORREF item_text_color( ) const {
		return ItemTextColor( );
		}

	bool     DrawSubitem_               ( RANGE_ENUM_COL const column::ENUM_COL subitem, _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const COwnerDrawnListCtrl* const list ) const {
		return DrawSubitem( subitem, hDC, rc, state, width, focusLeft, list );
		}


	_Must_inspect_result_ _Success_( SUCCEEDED( return ) ) _Pre_satisfies_( subitem != column::COL_NAME )
	HRESULT  GetText_WriteToStackBuffer ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const {
		const HRESULT res = Text_WriteToStackBuffer( subitem, psz_text, strSize, sizeBuffNeed, chars_written );
#ifdef DEBUG
		if ( SUCCEEDED( res ) ) {
			const auto len_dat_str = wcslen( psz_text );
			ASSERT( chars_written == len_dat_str );
			}
#endif
		return res;
		}


private:
	//defined at the BOTTOM of this file!
	COLORREF draw_if_selected_return_text_color( _In_ const UINT state, _In_ const RECT rcLabel, _In_ const RECT rc, _In_ HDC hDC, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
		auto textColor = GetSysColor( COLOR_WINDOWTEXT );
		if ( ( state bitand ODS_SELECTED ) != 0 ) {
			if ( list_has_focus || list_is_show_selection_always ) {
				ASSERT( ( ( state bitand ODS_SELECTED ) != 0 ) && ( list_has_focus || list_is_show_selection_always ) );
			
				DrawHighlightSelectBackground( rcLabel, rc, hDC, textColor, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
				return textColor;
				}
			}
		return item_text_color( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
		}


public:

	COwnerDrawnListItem( _In_z_ _Readable_elements_( length ) PCWSTR const name, const std::uint16_t length ) : m_name( name ), m_name_length( length ) { }
	COwnerDrawnListItem( const COwnerDrawnListItem& in ) = delete;
	COwnerDrawnListItem& operator=( const COwnerDrawnListItem& in ) = delete;
	__forceinline COwnerDrawnListItem( ) = default;

	virtual ~COwnerDrawnListItem( ) {
#ifdef WDS_OWNERDRAWNLISTITEM_DESTRUCTOR_DEBUG
		TRACE( _T( "Deleting COwnerDrawnListItem (`%s` ) @ %p\r\n" ), m_name, this );
#endif
		m_name = nullptr;
		m_name_length = 0u;
		}


	INT CompareS( _In_ const COwnerDrawnListItem* const other, _In_ const SSorting& sorting ) const {
//		if ( sorting.column1 == column::COL_NAME ) {
//#pragma warning( suppress: 4711 )//C4711: function 'int __cdecl signum<int>(int)' selected for automatic inline expansion
//			const auto sort_result = signum( wcscmp( m_name, other->m_name ) );
//		
//			if ( sort_result != 0 ) {
//				return sort_result;
//				}
//			}

		auto r_1 = compare_interface( other, sorting.column1 );
		if ( abs( r_1 ) < 2 && !sorting.ascending1 ) {
			r_1 = -r_1;
			}
		auto r_2 = r_1;

		if ( r_2 == 0 && sorting.column2 != sorting.column1 ) {
			r_2 = compare_interface( other, sorting.column2 );
		
			if ( abs( r_2 ) < 2 && !sorting.ascending2 ) {
				return ( -r_2 );
				}
			}
		return r_2;
		}
	
	//defined at bottom of THIS file.
	void         DrawSelection( _In_ HDC hDC, _In_ RECT rc, _In_ const UINT state, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
		if ( !list_is_full_row_selection ) {
			return;
			}
		if ( ( !list_has_focus ) && ( !list_is_show_selection_always ) ) {
			return;
			}
		if ( ( state bitand ODS_SELECTED ) == 0 ) {
			return;
			}

		VERIFY( ::InflateRect( &rc, -( 0 ), -( static_cast<int>( LABEL_Y_MARGIN ) ) ) );
	/*
	void CDC::FillSolidRect(LPCRECT lpRect, COLORREF clr)
	{
		ENSURE_VALID(this);
		ENSURE(m_hDC != NULL);
		ENSURE(lpRect);

		::SetBkColor(m_hDC, clr);
		::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
	}
	*/
		ASSERT( hDC != NULL );


		//If [SetBkColor] fails, the return value is CLR_INVALID.
		const auto set_bk_color_res_1 = ::SetBkColor( hDC, list_highlight_color );
		ASSERT( set_bk_color_res_1 != CLR_INVALID );
	#ifndef DEBUG
		UNREFERENCED_PARAMETER( set_bk_color_res_1 );
	#endif

		//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
		VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

		//pdc.FillSolidRect( &rc, list->GetHighlightColor( ) );
		}


	COLORREF     default_item_text_color      ( ) const {
		return ::GetSysColor( COLOR_WINDOWTEXT );
		}
	

protected:
	//defined at bottom of THIS file.
	//_Pre_satisfies_( subitem == column::COL_NAME ) _Success_( SUCCEEDED( return ) )
	//HRESULT      WriteToStackBuffer_COL_NAME ( RANGE_ENUM_COL const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, _Out_ _On_failure_( _Post_valid_ ) rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written ) const;

	_Success_( SUCCEEDED( return ) )
	HRESULT WriteToStackBuffer_default( const column::ENUM_COL subitem, WDS_WRITES_TO_STACK( strSize, chars_written ) PWSTR psz_text, _In_ const rsize_t strSize, rsize_t& sizeBuffNeed, _Out_ rsize_t& chars_written, _In_z_ const PCWSTR derived_type ) const {
		sizeBuffNeed = SIZE_T_ERROR;
		size_t chars_remaining = 0;
		ASSERT( strSize > 8 );
		const HRESULT res = StringCchPrintfExW( psz_text, strSize, NULL, &chars_remaining, 0, L"BAD GetText_WriteToStackBuffer - subitem" );
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			chars_written = ( strSize - chars_remaining );
			ASSERT( chars_written == wcslen( psz_text ) );
			return res;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
		if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			if ( strSize > 8 ) {
				wds_fmt::write_BAD_FMT( psz_text, chars_written );
				}
			else {
				chars_written = strSize;
				displayWindowsMsgBoxWithMessage( std::wstring( derived_type ) + std::wstring( global_strings::write_to_stackbuffer_err ) + L", subitem:" + std::to_wstring( static_cast<int>( subitem ) ) );
				}
			}
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "StringCchPrintFExW" );

		ASSERT( SUCCEEDED( res ) );
		ASSERT( chars_written == wcslen( psz_text ) );
		return res;
		}

	INT          default_compare              ( _In_ const COwnerDrawnListItem* const baseOther ) const {
#pragma warning( suppress: 4200 )//C4711: function 'int __cdecl signum<int>(int)' selected for automatic inline expansion
		return signum( wcscmp( m_name, baseOther->m_name ) );
		}

	//defined at bottom of THIS file.
	void         DrawLabel( _In_ HDC hDC, _In_ RECT& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent, _In_ CFont* const list_font, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
		/*
		  Draws an item label (icon, text) in all parts of the WinDirStat view. The rest is drawn by DrawItem()
		*/
		RECT rcRest = rc;

		// Increase indentation according to tree-level
		fixup_align_for_indent( indent, rcRest );

		/*
	_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/

		//TODO: performance issue in the line below due to CHandleMap::FromHandle
		SelectObject_wrapper sofont( hDC, list_font->m_hObject );
	
		//subtract 6 from rcRest.right, add 6 to rcRest.left
		VERIFY( ::InflateRect( &rcRest, -( TEXT_X_MARGIN ), -( 0 ) ) );

		RECT rcLabel = rcRest;
		VERIFY( ::DrawTextW( hDC, m_name, static_cast<int>( m_name_length ), &rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP ) );//DT_CALCRECT modifies rcLabel!!!

		AdjustLabelForMargin( rcRest, rcLabel );

		CSetBkMode bk( hDC, TRANSPARENT );
		//auto textColor = GetSysColor( COLOR_WINDOWTEXT );


		const auto textColor = draw_if_selected_return_text_color( state, rcLabel, rc, hDC, list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );

		//COLORREF draw_if_selected_return_text_color( width, state, list, rcLabel, rc, pdc )
		//if ( width == NULL && ( state bitand ODS_SELECTED ) != 0 && ( list->HasFocus( ) || list->IsShowSelectionAlways( ) ) ) {
		//	DrawHighlightSelectBackground( rcLabel, rc, list, pdc, textColor );
		//	}
		//else {
		//	textColor = item_text_color( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
		//	}

		// Set text color for device context
		CSetTextColor stc( hDC, textColor );

		if ( width == NULL ) {
			VERIFY( ::DrawTextW( hDC, m_name, static_cast<int>( m_name_length ), &rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP ) );
			}

		//subtract one from left, add one to right
		VERIFY( ::InflateRect( &rcLabel, 1, 1 ) );

		*focusLeft = rcLabel.left;

		if ( ( ( state bitand ODS_FOCUS ) != 0 ) && list_has_focus && ( width == NULL ) && ( !( list_is_full_row_selection ) ) ) {
			VERIFY( ::DrawFocusRect( hDC, &rcLabel ) );
			rcLabel.left = rc.left;
			rc = rcLabel;
			return;
			}


		rcLabel.left = rc.left;
		rc = rcLabel;
		if ( width != NULL ) {
			*width = ( rcLabel.right - rcLabel.left ) + 5; // +5 because GENERAL_INDENT?
			return;
			}
		}
	
	//defined at bottom of THIS file.
	void         DrawHighlightSelectBackground( _In_ const RECT& rcLabel, _In_ const RECT& rc, _In_ HDC hDC, _Inout_ COLORREF& textColor, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
		// Color for the text in a highlighted item (usually white)
		textColor = list_highlight_text_color;

		RECT selection = rcLabel;
		// Depending on "FullRowSelection" style
		if ( list_is_full_row_selection ) {
			selection.right = rc.right;
			}

		// Fill the selection rectangle background (usually dark blue)
		//pdc.FillSolidRect( &selection, list_highlight_color );

		//SetBkColor function: https://msdn.microsoft.com/en-us/library/dd162964.aspx
		//If the function succeeds, the return value specifies the previous background color as a COLORREF value.
		//If the function fails, the return value is CLR_INVALID.
		const COLORREF result = ::SetBkColor( hDC, list_highlight_color );
		if ( result == CLR_INVALID ) {
			std::terminate( );
			}
		VERIFY( ::ExtTextOut( hDC, 0, 0, ETO_OPAQUE, &selection, NULL, 0, NULL ) );

		}


	void         AdjustLabelForMargin         ( _In_ const RECT& rcRest, _Inout_ RECT& rcLabel ) const {
		//subtract three from left, add three to right
		VERIFY( ::InflateRect( &rcLabel, ( static_cast< int >( LABEL_INFLATE_CX ) ), 0 ) );
		//rcLabel.InflateRect( LABEL_INFLATE_CX, 0 );

		rcLabel.top    = rcRest.top + static_cast<LONG>( LABEL_Y_MARGIN );
		rcLabel.bottom = rcRest.bottom - static_cast<LONG>( LABEL_Y_MARGIN );
		}


	public:
	_Field_z_ _Field_size_( m_name_length ) PCWSTR         m_name;
	                                        //C4820: 'COwnerDrawnListItem' : '6' bytes padding added after data member 'COwnerDrawnListItem::m_name_length'
	                                        std::uint16_t  m_name_length;
	};

#pragma message( "Restoring data alignment.... " )
#pragma pack(pop)


namespace {
	static INT CALLBACK _CompareFunc( _In_ const LPARAM lParam1, _In_ const LPARAM lParam2, _In_ const LPARAM lParamSort ) {
		const auto sorting = reinterpret_cast<const SSorting*>( lParamSort );
		return reinterpret_cast<const COwnerDrawnListItem*>( lParam1 )->CompareS( reinterpret_cast<const COwnerDrawnListItem*>( lParam2 ), *sorting );
		}

	//defined at the BOTTOM of this file!
	void repopulate_right_aligned_cache( _Out_ std::vector<bool>& right_aligned_cache, _In_ _In_range_( 1, SIZE_T_MAX ) const size_t thisLoopSize, _In_ const CHeaderCtrl* const thisHeaderCtrl, _In_ const COwnerDrawnListCtrl* const owner_drawn_list_ctrl );

	_Pre_satisfies_( countArray <= itemCount )
	void handle_countArray_too_small( _In_ const rsize_t countArray, _In_ const size_t itemCount ) {
		TRACE( _T( "%i <= %i !!!! Something is REALLY wrong!!!\r\n" ), static_cast< int >( countArray ), static_cast< int >( itemCount ) );
		displayWindowsMsgBoxWithMessage( L"countArray <= itemCount !!!! Something is REALLY wrong!!!" );
		std::wstring item_count_str;
		item_count_str = std::to_wstring( itemCount );

		std::wstring count_array_str;
		count_array_str = std::to_wstring( countArray );

		std::wstring err_str( L"Count array: " + count_array_str + L" itemCount: " + item_count_str + L"......aborting!" );

		displayWindowsMsgBoxWithMessage( std::move( err_str ) );
		std::terminate( );
		}

	_Pre_satisfies_( !SUCCEEDED( fmt_res ) )
	void handle_formatting_error_COwnerDrawnListCtrl_SortItems( _In_ const HRESULT fmt_res ) {
		displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SortItems - StringCchPrintfW failed!(aborting)" );
		//TODO:use
		//WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
		//WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfW" );

		if ( fmt_res == STRSAFE_E_END_OF_FILE ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SortItems - StringCchPrintfW failed, STRSAFE_E_END_OF_FILE! This is an error that makes no sense.(aborting)" );
			std::terminate( );
			}
		if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SortItems - StringCchPrintfW failed, STRSAFE_E_INSUFFICIENT_BUFFER!(aborting)" );
			std::terminate( );
			}

		if ( fmt_res == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SortItems - StringCchPrintfW failed, STRSAFE_E_INVALID_PARAMETER!(aborting)" );
			std::terminate( );
			}
		else {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SortItems - StringCchPrintfW failed, unknown error!!(aborting)" );
			std::terminate( );
			}
		}
	inline RECT adjust_rect_for_grid_by_value( _In_ const bool show_grid, _In_ RECT rc_temp ) {
		if ( show_grid ) {
			rc_temp.bottom--;
			rc_temp.right--;
			return rc_temp;
			}
		return rc_temp;
		}

	inline void adjust_rect_for_grid( _In_ const bool show_grid, _Inout_ RECT& rcItem_temp ) {
		if ( show_grid ) {
			rcItem_temp.bottom--;
			rcItem_temp.right--;
			}
		}

	void check_validity_of_resize_size( _In_ const int resize_size, _In_ const rsize_t stack_array_size ) {
		if ( resize_size == -1 ) {
			displayWindowsMsgBoxWithMessage( L"thisHeaderCtrl->GetItemCount( ) returned -1! (aborting!)" );
			std::terminate( );
			}
		if ( static_cast< size_t >( resize_size ) > stack_array_size ) {
			displayWindowsMsgBoxWithMessage( L"COwnerDrawnListCtrl::DrawItem array too small!(aborting!)" );
			std::terminate( );
			//abort( );
			}
		}


	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	template<size_t count>
	inline void iterate_over_columns_and_populate_column_fields_( _In_ _In_range_( 1, count ) const size_t thisLoopSize, _In_ _In_reads_( thisLoopSize ) const INT( &order )[ count ], _Out_ _Out_writes_( thisLoopSize ) column::ENUM_COL( &subitems_temp )[ count ] ) {
		static_assert( std::is_convertible< INT, std::underlying_type<column::ENUM_COL>::type>::value, "" );
		for ( size_t i = 0; i < thisLoopSize; ++i ) {
			//iterate over columns, properly populate fields.
			subitems_temp[ i ] = static_cast<column::ENUM_COL>( order[ i ] );
			}
		}

	//defined at the BOTTOM of this file!
	template<size_t count>
	void build_array_of_rects_from_subitem_rects( _In_ _In_range_( 1, count ) const size_t thisLoopSize, _In_ _In_reads_( thisLoopSize ) const column::ENUM_COL( &subitems_temp )[ count ], _Out_ _Out_writes_( thisLoopSize ) RECT( &rects_temp )[ count ], _In_ PDRAWITEMSTRUCT pdis, _In_ const COwnerDrawnListCtrl* const owner_drawn_list_ctrl, _In_ CHeaderCtrl* const thisHeaderCtrl );


	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	template<size_t count>
	inline void build_array_of_drawable_rects_by_offsetting_( _In_ _In_range_( 1, count ) const size_t thisLoopSize, _Inout_updates_( thisLoopSize ) RECT( &rects )[ count ], _In_ const LONG rcItem_left, _In_ const LONG rcItem_top ) {
		//Not vectorized: Loop contains loop-carried data dependences that prevent vectorization. Different iterations of the loop interfere with each other such that vectorizing the loop would produce wrong answers, and the auto-vectorizer cannot prove to itself that there are no such data dependences.
		for ( size_t i = 0; i < thisLoopSize; ++i ) {
			VERIFY( ::OffsetRect( &( rects[ i ] ), -( rcItem_left ), -( rcItem_top ) ) );
			}
		}

	template<size_t count>
	void build_focusLefts_from_drawable_rects( _In_ _In_range_( 1, count ) const size_t thisLoopSize, _In_ _In_reads_( thisLoopSize ) const RECT( &rects_draw )[ count ], _Out_ _Out_writes_( thisLoopSize ) int( &focusLefts_temp )[ count ] ) {
		for ( size_t i = 0; i < thisLoopSize; i++ ) {
			//draw the proper text in each column?
			focusLefts_temp[ i ] = rects_draw[ i ].left;
			}
		}

	template<size_t count>
	void draw_proper_text_for_each_column( _In_ COwnerDrawnListItem* const item, _In_ const rsize_t thisLoopSize, _In_ _In_reads_( thisLoopSize ) const column::ENUM_COL( &subitems )[ count ], _In_ HDC hDC, _In_ _In_reads_( thisLoopSize ) const RECT( &rects_draw )[ count ], _In_ const PDRAWITEMSTRUCT pdis, _In_ _In_reads_( thisLoopSize ) int( &focusLefts_temp )[ count ], _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, _In_ const std::vector<bool>& is_right_aligned_cache, _In_ const COwnerDrawnListCtrl* const owner_drawn_list_ctrl ) {
		for ( size_t i = 0; i < thisLoopSize; i++ ) {
			//draw the proper text in each column?
			
			//focusLefts_temp[ i ] = rects_draw[ i ].left;
			//if DrawSubItem returns true, item draws self. Therefore `!item->DrawSubitem` is true when item DOES NOT draw self
			//CTreeListItem draws self FOR NAME column ONLY!
			//CDriveItem NEVER draws self.
			//CListItem (typeview) draws self ONLY for: NAME, and COLOR. 
			if ( !item->DrawSubitem_( subitems[ i ], hDC, rects_draw[ i ], pdis->itemState, NULL, &focusLefts_temp[ i ], owner_drawn_list_ctrl ) ) {
				owner_drawn_list_ctrl->DoDrawSubItemBecauseItCannotDrawItself( item, subitems[ i ], hDC, rects_draw[ i ], pdis, showSelectionAlways, bIsFullRowSelection, is_right_aligned_cache );
				}
			}
		}

	//thisLoopSize has essentially the range of RANGE_ENUM_COL, but it's never zero.
	template<size_t count>
	void draw_focus_rects_draw_focus( _In_ _In_range_( 1, 8 ) const rsize_t thisLoopSize, _In_ HDC hMemoryDeviceContext, _In_ _In_reads_( thisLoopSize ) const RECT( &rects_draw )[ count ], _In_ _In_reads_( thisLoopSize ) const int( &focusLefts )[ count ], _In_ HDC hDC, _Inout_ RECT& rcFocus, _In_ const RECT& rcItem ) {
		//first iteration is a special case, so we handle it outside the loop, and reduce the number of comparisons in the loop
		ASSERT( thisLoopSize > 0 );
		size_t i = 0;
		if ( focusLefts[ i ] > rects_draw[ i ].left ) {
			rcFocus.left = focusLefts[ i ];
			}
		rcFocus.right = rects_draw[ i ].right;
		VERIFY( ::BitBlt( hDC, ( rcItem.left + rects_draw[ i ].left ), ( rcItem.top + rects_draw[ i ].top ), ( rects_draw[ i ].right - rects_draw[ i ].left ), ( rects_draw[ i ].bottom - rects_draw[ i ].top ), hMemoryDeviceContext, rects_draw[ i ].left, rects_draw[ i ].top, SRCCOPY ) );

		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( ; i < thisLoopSize; i++ ) {
			if ( focusLefts[ i ] > rects_draw[ i ].left ) {
				
				VERIFY( ::DrawFocusRect( hDC, &rcFocus ) );
				//pdc->DrawFocusRect( &rcFocus );
				rcFocus.left = focusLefts[ i ];
				}
			rcFocus.right = rects_draw[ i ].right;
			VERIFY( ::BitBlt( hDC, ( rcItem.left + rects_draw[ i ].left ), ( rcItem.top + rects_draw[ i ].top ), ( rects_draw[ i ].right - rects_draw[ i ].left ), ( rects_draw[ i ].bottom - rects_draw[ i ].top ), hMemoryDeviceContext, rects_draw[ i ].left, rects_draw[ i ].top, SRCCOPY ) );
			}
		}

	//thisLoopSize has essentially the range of RANGE_ENUM_COL, but it's never zero.
	template<size_t count>
	void draw_focus_rects( _In_ _In_range_( 1, 8 ) const rsize_t thisLoopSize, _In_ HDC hDC, _In_ _In_reads_( thisLoopSize ) const RECT( &rects_draw )[ count ], _In_ _In_reads_( thisLoopSize ) const int( &focusLefts )[ count ], _In_ CDC* pdc, _Inout_ RECT& rcFocus, _In_ const RECT& rcItem, _In_ const bool drawFocus ) {
		if ( drawFocus ) {
			return draw_focus_rects_draw_focus( thisLoopSize, hDC, rects_draw, focusLefts, pdc->m_hDC, rcFocus, rcItem );
			}
		for ( size_t i = 0; i < thisLoopSize; i++ ) {
			if ( focusLefts[ i ] > rects_draw[ i ].left ) {
				rcFocus.left = focusLefts[ i ];
				}
			rcFocus.right = rects_draw[ i ].right;
			VERIFY( ::BitBlt( pdc->m_hDC, ( rcItem.left + rects_draw[ i ].left ), ( rcItem.top + rects_draw[ i ].top ), ( rects_draw[ i ].right - rects_draw[ i ].left ), ( rects_draw[ i ].bottom - rects_draw[ i ].top ), hDC, rects_draw[ i ].left, rects_draw[ i ].top, SRCCOPY ) );
			}
		}


	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	template<size_t count>
	inline void map_column_number_to_ENUM_and_build_drawable_rect( const rsize_t thisLoopSize, _In_ const INT( &order )[ count ], _Out_ _Out_writes_( thisLoopSize ) column::ENUM_COL( &subitems_temp )[ count ], _Out_ _Out_writes_( thisLoopSize ) RECT( &rects_temp )[ count ], _In_ const PDRAWITEMSTRUCT pdis, _Inout_ const COwnerDrawnListCtrl* const this_ctrl, _Inout_ CHeaderCtrl* thisHeaderCtrl, _In_ const RECT rcItem ) {
		iterate_over_columns_and_populate_column_fields_( thisLoopSize, order, subitems_temp );
		build_array_of_rects_from_subitem_rects( thisLoopSize, subitems_temp, rects_temp, pdis, this_ctrl, thisHeaderCtrl );
		build_array_of_drawable_rects_by_offsetting_( thisLoopSize, rects_temp, rcItem.left, rcItem.top );
		}

	void second_try_failed( _In_ const column::ENUM_COL subitem, _In_ const rsize_t sizeNeeded, _In_ const rsize_t new_size_needed ) {
		displayWindowsMsgBoxWithMessage( L"COwnerDrawnListCtrl::GetSubItemWidth, second try of `item->GetText_WriteToStackBuffer` failed!!(aborting)" );
		std::wstring err_str;
		err_str += L"DEBUGGING INFO: subitem: ";
		err_str += std::to_wstring( subitem );
		err_str += L", size of buffer in characters: ";
		err_str += std::to_wstring( sizeNeeded );
		err_str += L", returned size needed: ";
		err_str += std::to_wstring( new_size_needed );
		displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
		}

	void check_column_buf_size( _In_ const int header_ctrl_item_count, _In_ const rsize_t column_buf_size ) {
		ASSERT( header_ctrl_item_count < static_cast<int>( column_buf_size ) );
		if ( header_ctrl_item_count > static_cast<int>( column_buf_size ) ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::handle_EraseBkgnd, header_ctrl_item_count greater than capacity of column buffer! (aborting)" );
			std::wstring err_str( L"DEBUGGING INFO: header_ctrl_item_count: " );
			err_str += std::to_wstring( header_ctrl_item_count );
			err_str += L", column_buf_size: ";
			err_str += std::to_wstring( column_buf_size );
			displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
			//too many columns!
			std::terminate( );
			}
		}

	}


// COwnerDrawnListCtrl. Must be report view. Deals with COwnerDrawnListItems.
// Can have a grid or not (own implementation, don't set LVS_EX_GRIDLINES). Flicker-free.
class COwnerDrawnListCtrl : public CListCtrl {
	/*
	
	#define DECLARE_DYNAMIC(class_name) \
	public: \
		static const CRuntimeClass class##class_name; \
		virtual CRuntimeClass* GetRuntimeClass() const; \


	#define DECLARE_DYNAMIC(COwnerDrawnListCtrl) \
	public: \
		static const CRuntimeClass classCOwnerDrawnListCtrl; \
		virtual CRuntimeClass* GetRuntimeClass() const; \

	*/

protected:
	virtual void DrawItem( _In_ PDRAWITEMSTRUCT pdis ) override final {
		const auto item = reinterpret_cast< COwnerDrawnListItem *> ( pdis->itemData );
		const auto pdc = CDC::FromHandle( pdis->hDC );
		const auto bIsFullRowSelection = m_showFullRowSelection;
		ASSERT_VALID( pdc );
		//RECT rcItem_temp( pdis->rcItem );

		const RECT rcItem = adjust_rect_for_grid_by_value( m_showGrid, pdis->rcItem );

		//adjust_rect_for_grid( m_showGrid, rcItem_temp );


		CDC dcmem; //compiler seems to vectorize this!

		VERIFY( dcmem.CreateCompatibleDC( pdc ) );
		CBitmap bm;
		VERIFY( bm.CreateCompatibleBitmap( pdc, ( rcItem.right - rcItem.left ), ( rcItem.bottom - rcItem.top ) ) );
		SelectObject_wrapper sobm( dcmem.m_hDC, bm.m_hObject );
		RECT rect_to_fill_solidly = rcItem;
		const tagPOINT point_to_offset_by = { rcItem.left, rcItem.top };
		VERIFY( ::OffsetRect( &rect_to_fill_solidly, -( point_to_offset_by.x ), -( point_to_offset_by.y ) ) );
		
		//ASSERT( ( rcItem - rcItem.TopLeft( ) ) == rect_to_fill_solidly );
		//dcmem.FillSolidRect( &rect_to_fill_solidly, GetItemBackgroundColor( pdis->itemID ) ); //NOT vectorized!

		//SetBkColor function: https://msdn.microsoft.com/en-us/library/dd162964.aspx
		//If the function succeeds, the return value specifies the previous background color as a COLORREF value.
		//If the function fails, the return value is CLR_INVALID.
		const COLORREF set_bk_result = ::SetBkColor( dcmem.m_hDC, GetItemBackgroundColor( pdis->itemID ) );
		if ( set_bk_result == CLR_INVALID ) {
			std::terminate( );
			}
		VERIFY( ::ExtTextOut( dcmem.m_hDC, 0, 0, ETO_OPAQUE, &rect_to_fill_solidly, NULL, 0, NULL ) );

		const bool drawFocus = ( pdis->itemState bitand ODS_FOCUS ) != 0 && HasFocus( ) && bIsFullRowSelection; //partially vectorized

		const rsize_t stack_array_size = 12;
		static_assert( stack_array_size > column::COL_ATTRIBUTES, "we're gonna need a bigger array!" );

		//std::vector<INT> order_temp;
		INT order_temp[ stack_array_size ] = { 0 };

		const bool showSelectionAlways = IsShowSelectionAlways( );
		const auto thisHeaderCtrl = CListCtrl::GetHeaderCtrl( );//HORRENDOUSLY slow. Pessimisation of memory access, iterates (with a for loop!) over a map. MAXIMUM branch prediction failures! Maximum Bad Speculation stalls!

		const auto resize_size = thisHeaderCtrl->GetItemCount( );

		check_validity_of_resize_size( resize_size, stack_array_size );

		//order_temp.resize( static_cast<size_t>( resize_size ) );
		ASSERT( static_cast<size_t>( resize_size ) < stack_array_size );

		VERIFY( thisHeaderCtrl->GetOrderArray( order_temp, resize_size )) ;

		const rsize_t thisLoopSize = static_cast<rsize_t>( resize_size );
		if ( is_right_aligned_cache.empty( ) ) {
			repopulate_right_aligned_cache( is_right_aligned_cache, thisLoopSize, thisHeaderCtrl, this );
			}
		ASSERT( thisLoopSize < 8 );

		RECT rcFocus = rcItem;

		//rcFocus.DeflateRect( 0, LABEL_Y_MARGIN - 1 );
		VERIFY( ::InflateRect( &rcFocus, -( 0 ), -( static_cast<int>( LABEL_Y_MARGIN - 1 ) ) ) );
		
		const INT (&order)[ stack_array_size ] = order_temp;

		column::ENUM_COL subitems_temp[ stack_array_size ];
		int focusLefts_temp[ stack_array_size ] = { 0 };
		RECT rects_temp[ stack_array_size ] = { 0 };

		//RECT rects_draw_temp[ stack_array_size ] = { 0 };

		//build map of column# -> ENUM_COL
		//build drawable rect for each column
		map_column_number_to_ENUM_and_build_drawable_rect( thisLoopSize, order, subitems_temp, rects_temp, pdis, this, thisHeaderCtrl, rcItem );

		const column::ENUM_COL (&subitems)[ stack_array_size ] = subitems_temp;
		const RECT (&rects_draw)[ stack_array_size ] = rects_temp;

		build_focusLefts_from_drawable_rects( thisLoopSize, rects_draw, focusLefts_temp );


		draw_proper_text_for_each_column( item, thisLoopSize, subitems, dcmem.m_hDC, rects_draw, pdis, focusLefts_temp, showSelectionAlways, bIsFullRowSelection, is_right_aligned_cache, this );

		const int (&focusLefts)[ stack_array_size ] = focusLefts_temp;

		draw_focus_rects( thisLoopSize, dcmem.m_hDC, rects_draw, focusLefts, pdc, rcFocus, rcItem, drawFocus );

		if ( drawFocus ) {
			pdc->DrawFocusRect( &rcFocus );
			}

		}

	virtual const AFX_MSGMAP* GetMessageMap( ) const override {
		return GetThisMessageMap( );
		}
public:
	static const CRuntimeClass classCOwnerDrawnListCtrl;
	virtual CRuntimeClass* GetRuntimeClass( ) const {
		return ((CRuntimeClass*)(&COwnerDrawnListCtrl::classCOwnerDrawnListCtrl) );
		}

	//DECLARE_DYNAMIC(COwnerDrawnListCtrl)
	COwnerDrawnListCtrl ( _In_z_ PCWSTR name, _In_range_( 0, UINT_MAX ) const UINT rowHeight ) : m_persistent_name( name ), m_indicatedColumn( -1 ), m_rowHeight( rowHeight ), m_showGrid( false ), m_showStripes( false ), m_showFullRowSelection( false ), m_frameptr( GetMainFrame( ) ) {
		ASSERT( rowHeight > 0 );
		InitializeColors( );
		}

	virtual ~COwnerDrawnListCtrl( ) = default;

	COwnerDrawnListCtrl& operator=( const COwnerDrawnListCtrl& in ) = delete;
	COwnerDrawnListCtrl( const COwnerDrawnListCtrl& in ) = delete;

	void LoadPersistentAttributes( ) {
		//TRACE statements in headers output the FULL source file path. Ick.
		//TRACE( _T( "Loading persistent attributes....\r\n" ) );

		const auto itemCount_default_type = CListCtrl::GetHeaderCtrl( )->GetItemCount( );
		const auto itemCount = static_cast<size_t>( itemCount_default_type );
		const rsize_t countArray = 10;

		//void handle_countArray_too_small( _In_ const rsize_t, _In_ const size_t itemCount )


		if ( itemCount < 2 ) {
			//CPersistence expects more than one item arrays
			//Since that's a nonsensical condition, no point in continuing.
			std::terminate( );
			abort( );//Maybe VS2015 will understand that std::terminate( ) doesn't return.
			}

		if ( countArray <= itemCount ) {
			handle_countArray_too_small( countArray, itemCount );
			}

		ASSERT( countArray > itemCount );
	
		INT col_order_array[ countArray ] = { 0 };

		const auto res_2 = CListCtrl::GetColumnOrderArray( col_order_array, itemCount_default_type );
		if ( res_2 == 0 ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::LoadPersistenAttributes - GetColumnOrderArray failed!(aborting)" );
			std::terminate( );
			}

		std::wstring column_order_default;

		for ( size_t i = 0; i < itemCount; i++ ) {
			column_order_default += std::to_wstring( i );
			column_order_default += L",";
			}

		ASSERT( column_order_default.back( ) == L',' );
		column_order_default.pop_back( );
		ASSERT( column_order_default.back( ) != L',' );
		
		CPersistence::GetColumnOrder( m_persistent_name, col_order_array, itemCount, column_order_default.c_str( ) );

		const auto res2 = CListCtrl::SetColumnOrderArray( static_cast<int>( itemCount ), col_order_array );
		if ( res2 == 0 ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::LoadPersistenAttributes - SetColumnOrderArray failed!(aborting)" );
			std::terminate( );
			}

		for ( size_t i = 0; i < itemCount; i++ ) {
			col_order_array[ i ] = CListCtrl::GetColumnWidth( static_cast<int>( i ) );
			}
		std::wstring column_widths_default;

		for ( size_t i = 0; i < itemCount; i++ ) {
			column_widths_default += std::to_wstring( col_order_array[ i ] );
			column_widths_default += L",";
			}

		ASSERT( column_widths_default.back( ) == L',' );
		column_widths_default.pop_back( );
		ASSERT( column_widths_default.back( ) != L',' );

		CPersistence::GetColumnWidths( m_persistent_name, col_order_array, itemCount, column_widths_default.c_str( ) );

		for ( size_t i = 0; i < itemCount; i++ ) {
			// To avoid "insane" settings we set the column width to maximal twice the default width.
			const auto maxWidth = CListCtrl::GetColumnWidth( static_cast<int>( i ) ) * 2;
		
	#pragma push_macro("min")
	#undef min
			const auto w =	( col_order_array[ i ], maxWidth );
	#pragma pop_macro("min")

			VERIFY( CListCtrl::SetColumnWidth( static_cast<int>( i ), w ) );
			}
		// We refrain from saving the sorting because it is too likely, that users start up with insane settings and don't get it.
		}

	void SavePersistentAttributes( ) {
		const rsize_t col_array_size = 128;
		int col_array[ col_array_size ] = { 0 };

		const auto itemCount = CListCtrl::GetHeaderCtrl( )->GetItemCount( );

		if ( itemCount < 2 ) {
			//CPersistence expects more than one item arrays
			//Since that's a nonsensical condition, no point in continuing.
			std::terminate( );
			abort( );//Maybe VS2015 will understand that std::terminate( ) doesn't return.
			}

		if ( itemCount >= col_array_size ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SavePersistentAttributes - GetItemCount returned an itemCount of a size bigger than the array allocated!(aborting)" );
			std::terminate( );
			}

		const auto get_res = CListCtrl::GetColumnOrderArray( col_array, itemCount );

		if ( get_res == 0 ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::SavePersistentAttributes - GetColumnOrderArray failed!(aborting)" );
			std::terminate( );
			}


		CPersistence::SetColumnOrder( m_persistent_name, col_array, static_cast<rsize_t>( itemCount ) );

		for ( INT_PTR i = 0; i < itemCount; i++ ) {
			col_array[ i ] = CListCtrl::GetColumnWidth( static_cast<int>( i ) );
			}
		CPersistence::SetColumnWidths( m_persistent_name, col_array, static_cast<rsize_t>( itemCount ) );
		}

	void SortItems( ) {
		VERIFY( CListCtrl::SortItems( &_CompareFunc, reinterpret_cast<DWORD_PTR>( &m_sorting ) ) );
		auto hditem = zero_init_struct<HDITEM>( );
		auto thisHeaderCtrl = CListCtrl::GetHeaderCtrl( );

		//http://msdn.microsoft.com/en-us/library/windows/desktop/bb775247(v=vs.85).aspx specifies 260
		const rsize_t text_char_count = 260u;


		_Null_terminated_ wchar_t text_buffer_1[ text_char_count ] = { 0 };
		hditem.mask       = HDI_TEXT;
		//hditem.pszText    = text.GetBuffer( text_char_count );
		hditem.pszText    = text_buffer_1;
		hditem.cchTextMax = text_char_count;

		if ( m_indicatedColumn != -1 ) {
			VERIFY( thisHeaderCtrl->GetItem( m_indicatedColumn, &hditem ) );
			//text.ReleaseBuffer( );
			//text           = text.Mid( 2 );
			//PWSTR text_str = &( text_buffer_1[ 2 ] );

			//hditem.pszText = text.GetBuffer( text_char_count );
			hditem.pszText = &( text_buffer_1[ 2 ] );
			hditem.cchTextMax = ( text_char_count - 3 );
			VERIFY( thisHeaderCtrl->SetItem( m_indicatedColumn, &hditem ) );
			//text.ReleaseBuffer( );
			}

		//hditem.pszText = text.GetBuffer( text_char_count );
		hditem.pszText = text_buffer_1;
		hditem.cchTextMax = text_char_count;
		VERIFY( thisHeaderCtrl->GetItem( m_sorting.column1, &hditem ) );
		//text.ReleaseBuffer( );

		_Null_terminated_ wchar_t text_buffer_2[ text_char_count ] = { 0 };
		const HRESULT fmt_res = StringCchPrintfW( text_buffer_2, text_char_count, L"%s%s", ( ( m_sorting.ascending1 ) ? _T( "< " ) : _T( "> " ) ), text_buffer_1 );
		//text = ( ( m_sorting.ascending1 ) ? _T( "< " ) : _T( "> " ) ) + text;
		ASSERT( SUCCEEDED( fmt_res ) );
		if ( !SUCCEEDED( fmt_res ) ) {
			handle_formatting_error_COwnerDrawnListCtrl_SortItems( fmt_res );
			//	}

			}
		//hditem.pszText = text.GetBuffer( text_char_count );
		hditem.pszText = text_buffer_2;
		VERIFY( thisHeaderCtrl->SetItem( m_sorting.column1, &hditem ) );

		//goddamnit, static_assert is AWESOME when combined with template metaprogramming!
		static_assert( std::is_convertible<std::underlying_type<column::ENUM_COL>::type, decltype( m_indicatedColumn )>::value, "m_sorting.column1 MUST be convertible to an ENUM_COL!" );
		//static_assert( std::is_convertible<std::underlying_type<column::ENUM_COL>::type, std::int8_t>::value, "m_sorting.column1 MUST be convertible to an ENUM_COL!" );
	
		m_indicatedColumn = static_cast<std::int8_t>( m_sorting.column1 );
		//text.ReleaseBuffer( );

		}

	_Success_( return != -1 ) _Ret_range_( 0, INT_MAX )
	INT FindListItem( _In_ const COwnerDrawnListItem* const item ) const {
		auto fi = zero_init_struct<LVFINDINFO>( );

		fi.flags  = LVFI_PARAM;
		fi.lParam = reinterpret_cast<LPARAM>( item );

		const auto i = static_cast<INT>( CListCtrl::FindItem( &fi ) );

		return i;
		}

	void OnColumnsInserted( ) {
		/*
		  This method MUST be called BEFORE the Control is shown.
		*/
		// The pacmen shall not draw over our header control.
		VERIFY( CWnd::ModifyStyle( 0, WS_CLIPCHILDREN ) );

		// Where does the 1st Item begin vertically?
		if ( CListCtrl::GetItemCount( ) > 0 ) {
			RECT rc;
			VERIFY( CListCtrl::GetItemRect( 0, &rc, LVIR_BOUNDS ) );
			m_yFirstItem = rc.top;
			}
		else {
			InsertItem( 0, _T( "_tmp" ), 0 );
			RECT rc;
			VERIFY( CListCtrl::GetItemRect( 0, &rc, LVIR_BOUNDS ) );
			VERIFY( CListCtrl::DeleteItem( 0 ) );
			m_yFirstItem = rc.top;
			}

		LoadPersistentAttributes( );
		}

	void AdjustColumnWidth( RANGE_ENUM_COL const column::ENUM_COL col ) {
		WTL::CWaitCursor wc;

		INT width = 10;
		const auto itemCount = CListCtrl::GetItemCount( );
		for ( INT i = 0; i < itemCount; i++ ) {
			ASSERT( itemCount == CListCtrl::GetItemCount( ) );
			const auto item = GetItem( i );
			if ( item == NULL ) {
				displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::AdjustColumnWidth - item == NULL (aborting)" );
				std::terminate( );
				//`/analyze` is confused.
				return;
				}
			const auto w = GetSubItemWidth( item, col );
			ASSERT( w == ( CListCtrl::GetStringWidth( item->m_name ) + 20 ) );
			//ASSERT( w == ( GetStringWidth( item->m_name ) + 10 ) );
			if ( w > width ) {
				width = w;
				}
			}
		VERIFY( CListCtrl::SetColumnWidth( col, width + 5 ) );
		}
	
	void InsertListItem( _In_ _In_range_( 0, INT32_MAX ) const INT_PTR i, _In_ const COwnerDrawnListItem* const item ) {
		auto lvitem = zero_init_struct<LVITEM>( );

		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.iItem   = static_cast<int>( i );
		lvitem.pszText = LPSTR_TEXTCALLBACKW;
		lvitem.iImage  = I_IMAGECALLBACK;
		lvitem.lParam  = reinterpret_cast< LPARAM >( item );

		VERIFY( i == CListCtrl::InsertItem( &lvitem ) );
		}
	
	void AddExtendedStyle( _In_ const DWORD exStyle ) {
		CListCtrl::SetExtendedStyle( CListCtrl::GetExtendedStyle( ) bitor exStyle );
		}
	
	//COLORREF GetItemSelectionBackgroundColor ( _In_ _In_range_( 0, INT_MAX )   const INT i  ) const;

	COLORREF GetItemSelectionTextColor( _In_ _In_range_( 0, INT_MAX )   const INT i ) const {
		auto selected = ( CListCtrl::GetItemState( i, LVIS_SELECTED ) & LVIS_SELECTED ) != 0;
		if ( selected && m_showFullRowSelection && ( HasFocus( ) || IsShowSelectionAlways( ) ) ) {
			return GetHighlightTextColor( );
			}
		return ::GetSysColor( COLOR_WINDOWTEXT );
		}
	
	RECT GetWholeSubitemRect( _In_ const INT item, _In_ const INT subitem, CHeaderCtrl* const thisHeader ) const {
		CRect rc;
		if ( subitem == 0 ) {
			// Special case column 0:
			// If we did GetSubItemRect(item 0, LVIR_LABEL, rc) and we have an image list, then we would get the rectangle excluding the image.
			auto hditem = zero_init_struct<HDITEM>( );
			hditem.mask = HDI_WIDTH;
			VERIFY( thisHeader->GetItem( 0, &hditem ) );

			VERIFY( CListCtrl::GetItemRect( item, rc, LVIR_LABEL ) );
			rc.left = rc.right - hditem.cxy;
			}
		else {

			/*
#define ListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
        (BOOL)SNDMSG((hwnd), LVM_GETSUBITEMRECT, (WPARAM)(int)(iItem), \
                ((prc) ? ((((LPRECT)(prc))->top = (iSubItem)), (((LPRECT)(prc))->left = (code)), (LPARAM)(prc)) : (LPARAM)(LPRECT)NULL))
			*/
			//a.k.a.
			/*
        (BOOL)SendMessageW(	hwnd,  \
							LVM_GETSUBITEMRECT,   \
							(WPARAM)iItem, \
							((prc) ? (
										(((LPRECT)(prc))->top = (iSubItem)),
										(((LPRECT)(prc))->left = (code)),
										(LPARAM)(prc)
									)
									: (LPARAM)reinterpret_cast<LPRECT>( NULL)
							)
							)
			*/
			//a.k.a.
			/*
        (BOOL)SendMessageW(	hwnd,  \
							LVM_GETSUBITEMRECT,   \
							reinterpret_cast<WPARAM>( iItem ), \
								(
									(prc->top = iSubItem),
									(prc->left = code),
									reinterpret_cast<LPARAM>(prc)
								)
							)
			*/


			/*
BOOL CListCtrl::GetSubItemRect(int iItem, int iSubItem, int nArea, CRect& ref) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(nArea == LVIR_BOUNDS || nArea == LVIR_ICON || nArea == LVIR_LABEL || nArea == LVIR_SELECTBOUNDS);

	RECT rect;
	rect.top = iSubItem;
	rect.left = nArea;
	BOOL bRet = (BOOL) ::SendMessage(m_hWnd, LVM_GETSUBITEMRECT,
		iItem, (LPARAM) &rect);

	if (bRet)
		ref = rect;
	return bRet;
}
			*/

			VERIFY( CListCtrl::GetSubItemRect( item, subitem, LVIR_LABEL, rc ) );
			}

		if ( m_showGrid ) {
			rc.right--;
			rc.bottom--;
			}
		return rc;
		}

	_Must_inspect_result_ _Success_( return != NULL ) _Ret_maybenull_
	COwnerDrawnListItem* GetItem( _In_ _In_range_( 0, INT_MAX )   const int i ) const {
		ASSERT( i < CListCtrl::GetItemCount( ) );
		const auto itemCount = CListCtrl::GetItemCount( );
		if ( i < itemCount ) {
			return reinterpret_cast< COwnerDrawnListItem* >( CListCtrl::GetItemData( static_cast<int>( i ) ) );
			}
		return NULL;
		}

	void SetSorting( RANGE_ENUM_COL const column::ENUM_COL sortColumn, _In_ const bool ascending ) {
		m_sorting.ascending2 = m_sorting.ascending1;
		m_sorting.column1    = sortColumn;
		m_sorting.column2    = m_sorting.column1;
		m_sorting.ascending1 = ascending;
		}

	void ShowFullRowSelection( _In_ const bool show ) {
		m_showFullRowSelection = show;
		if ( ::IsWindow( m_hWnd ) ) {
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
			//InvalidateRect( NULL );
			}
		}

	void ShowGrid( _In_ const bool show ) {
		m_showGrid = show;
		if ( ::IsWindow( m_hWnd ) ) {
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
			//InvalidateRect( NULL );
			}
		}

	void ShowStripes( _In_ const bool show ) {
		m_showStripes = show;
		if ( ::IsWindow( m_hWnd ) ) {
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
			//InvalidateRect( NULL );
			}
		}

	COLORREF GetHighlightColor( ) const {
		if ( HasFocus( ) ) {
			return ::GetSysColor( COLOR_HIGHLIGHT );
			}
		return RGB( 190, 190, 190 );
		}
	
	_Success_( return != COLORREF( 0 ) )
	COLORREF GetHighlightTextColor( ) const {
		if ( HasFocus( ) ) {
			return ::GetSysColor( COLOR_HIGHLIGHTTEXT );
			}
		return RGB( 0, 0, 0 );
		}


	_Success_( return != COLORREF( 0 ) )
	COLORREF GetItemBackgroundColor( _In_ _In_range_( 0, UINT_MAX ) const UINT i ) const {
		return ( IsItemStripeColor( i ) ? m_stripeColor : m_windowColor );
		}

	static_assert( INT_MAX < UINT_MAX, "" );
	bool IsItemStripeColor( _In_ _In_range_( 0, UINT_MAX ) const UINT i ) const {
		return ( m_showStripes && ( i % 2 != 0 ) );
		}

	bool IsItemStripeColor( _In_ const COwnerDrawnListItem* const item ) const {
		const auto itemPos = FindListItem( item );
		if ( itemPos >= 0 ) {
			return IsItemStripeColor( static_cast<UINT>( itemPos ) );
			}
		return COLORREF( 0 );
		}
	bool HasFocus( ) const {
		return ::GetFocus( ) == m_hWnd;
		}
	bool IsShowSelectionAlways( ) const {
		return ( CWnd::GetStyle( ) bitand LVS_SHOWSELALWAYS ) != 0;
		}

	bool AscendingDefault( ) const {
		//return GetAscendingDefault( );
		return true;
		}


public:
	void DoDrawSubItemBecauseItCannotDrawItself( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ HDC hDC, _In_ const RECT& rcDraw, _In_ const PDRAWITEMSTRUCT& pdis, _In_ const bool showSelectionAlways, _In_ const bool bIsFullRowSelection, const std::vector<bool>& is_right_aligned_cache ) const {
		item->DrawSelection( hDC, rcDraw, pdis->itemState, HasFocus( ), IsShowSelectionAlways( ), GetHighlightColor( ), m_showFullRowSelection );

		RECT rcText = rcDraw;
		VERIFY( ::InflateRect( &rcText, -( TEXT_X_MARGIN ), -( 0 ) ) );

		CSetBkMode bk( hDC, TRANSPARENT );

		/*
		_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/
		SelectObject_wrapper sofont( hDC, CWnd::GetFont( )->m_hObject );
	
		//const auto align = IsColumnRightAligned( subitem ) ? DT_RIGHT : DT_LEFT;
		const auto align = is_right_aligned_cache[ static_cast<size_t>( subitem ) ] ? DT_RIGHT : DT_LEFT;

		// Get the correct color in case of compressed or encrypted items
		auto textColor = item->item_text_color( );

		if ( ( pdis->itemState bitand ODS_SELECTED ) && ( showSelectionAlways || HasFocus( ) ) && ( bIsFullRowSelection ) ) {
			textColor = GetItemSelectionTextColor( static_cast<INT>( pdis->itemID ) );
			}

		CSetTextColor tc( hDC, textColor );

		if ( subitem == column::COL_NAME ) {
			//fastpath. No work to be done!
			VERIFY( ::DrawTextW( hDC, item->m_name, static_cast< int >( item->m_name_length ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) ) );
			return;
			}

		ASSERT( subitem != column::COL_NAME );
		rsize_t size_needed = 0;
		const HRESULT stackbuffer_draw_res = drawSubItem_stackbuffer( item, rcText, align, subitem, hDC, size_needed );
		if ( !SUCCEEDED( stackbuffer_draw_res ) ) {
			DrawText_dynamic( item, rcText, align, subitem, hDC, size_needed );
			}

		}

protected:
	_Success_( SUCCEEDED( return ) ) _Pre_satisfies_( subitem != column::COL_NAME )
	HRESULT drawSubItem_stackbuffer( _In_ const COwnerDrawnListItem* const item, _In_ RECT& rcText, const int& align, _In_ _In_range_( 1, 6 ) const column::ENUM_COL subitem, _In_ HDC hDC, _On_failure_( _Post_valid_ ) rsize_t& sizeNeeded ) const {
		const rsize_t subitem_text_size = 128;
		_Null_terminated_ wchar_t psz_subitem_formatted_text[ subitem_text_size ] = { 0 };
		//rsize_t sizeNeeded = 0;
		rsize_t chars_written = 0;

		ASSERT( subitem != column::COL_NAME );
		const HRESULT res = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text, subitem_text_size, sizeNeeded, chars_written );
		if ( SUCCEEDED( res ) ) {
			VERIFY( ::DrawTextW( hDC, psz_subitem_formatted_text, static_cast<int>( chars_written ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) ) );
			return res;
			}
		if ( ( MAX_PATH * 2 ) > sizeNeeded ) {
			const rsize_t subitem_text_size_2 = ( MAX_PATH * 2 );
			_Null_terminated_ wchar_t psz_subitem_formatted_text_2[ subitem_text_size_2 ] = { 0 };
			rsize_t chars_written_2 = 0;
			ASSERT( subitem != column::COL_NAME );
			const HRESULT res_2 = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text_2, subitem_text_size_2, sizeNeeded, chars_written_2 );
			if ( SUCCEEDED( res_2 ) ) {
				VERIFY( ::DrawTextW( hDC, psz_subitem_formatted_text_2, static_cast<int>( chars_written_2 ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) ) );
				
				//shut analyze up!
				sizeNeeded = 0;
				
				return res;
				}
			return res;
			}
		ASSERT( !SUCCEEDED( res ) );
		return res;

		}

private:
	_Pre_satisfies_( subitem != column::COL_NAME ) _Pre_satisfies_( subitem != column::COL_NAME )
	void DrawText_dynamic( _In_ const COwnerDrawnListItem* const item, _In_ RECT& rcText, const int& align, _In_ _In_range_( 1, 6 ) const column::ENUM_COL subitem, _In_ HDC hDC, _In_ const rsize_t size_needed ) const {
		ASSERT( size_needed < 33000 );
		std::unique_ptr<_Null_terminated_ wchar_t[ ]> buffer ( std::make_unique<wchar_t[ ]>( size_needed + 2 ) );
		SecureZeroMemory( buffer.get( ), ( ( size_needed + 2 ) * sizeof( wchar_t ) ) );

		rsize_t new_size_needed = 0;
		rsize_t chars_written = 0;
		ASSERT( subitem != column::COL_NAME );
		const HRESULT res = item->GetText_WriteToStackBuffer( subitem, buffer.get( ), size_needed, new_size_needed, chars_written );
		if ( !SUCCEEDED( res ) ) {
			
			displayWindowsMsgBoxWithMessage( L"COwnerDrawnListCtrl::DrawText_dynamic failed!!(aborting)" );
			std::wstring err_str;
			err_str += L"DEBUGGING INFO: subitem: ";
			err_str += std::to_wstring( subitem );
			err_str += L", size of buffer in characters: ";
			err_str += std::to_wstring( size_needed );
			err_str += L", returned size needed: ";
			err_str += std::to_wstring( new_size_needed );
			displayWindowsMsgBoxWithMessage( err_str.c_str( ) );
			std::terminate( );

			//shut `/analyze` up.
			return;
			//abort( );
			}
		VERIFY( ::DrawTextW( hDC, buffer.get( ), static_cast<int>( chars_written ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | static_cast< UINT >( align ) ) );
		}

protected:
	void InitializeColors( ) {
		// I try to find a good contrast to COLOR_WINDOW (usually white or light grey).
		// This is a result of experiments. 

		const DOUBLE diff      = 0.07;		// Try to alter the brightness by diff.
		const DOUBLE threshold = 1.04;	// If result would be brighter, make color darker.
		m_windowColor          = ::GetSysColor( COLOR_WINDOW );

		auto b = CColorSpace::GetColorBrightness( m_windowColor );

		if ( b + diff > threshold ) {
			b -= diff;
			}
		else {
			b += diff;
			if ( b > 1.0 ) {
				b = 1.0;
				}
			}

#ifdef COLOR_DEBUGGING
#ifdef DEBUG
		trace_m_stripe_color_make_bright_color( m_windowColor, b );
#endif
#endif
		m_stripeColor = CColorSpace::MakeBrightColor( m_windowColor, b );
#ifdef COLOR_DEBUGGING
#ifdef DEBUG
		trace_m_stripeColor( m_stripeColor );
#endif
#endif

		}

public:
	bool IsColumnRightAligned( _In_ const INT col, const CHeaderCtrl* const thisHeaderControl ) const {
		auto hditem = zero_init_struct<HDITEM>( );
		hditem.mask = HDI_FORMAT;
		VERIFY( thisHeaderControl->GetItem( col, &hditem ) );
		return ( hditem.fmt bitand HDF_RIGHT ) != 0;
		}
	
private:

	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX )
	INT first_try_failed( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CHeaderCtrl* const thisHeaderCtrl, _In_ RECT& rc, _In_ const HDC hDC, _In_ const rsize_t sizeNeeded ) const {
		ASSERT( sizeNeeded < 33000 );
		std::unique_ptr<_Null_terminated_ wchar_t[ ]> buffer( std::make_unique<_Null_terminated_ wchar_t[ ]>( sizeNeeded + 2 ) );
		SecureZeroMemory( buffer.get( ), ( ( sizeNeeded + 2 ) * sizeof( wchar_t ) ) );

		rsize_t new_size_needed = 0;
		rsize_t chars_written_2 = 0;
		ASSERT( subitem != column::COL_NAME );
		const HRESULT res_2 = item->GetText_WriteToStackBuffer( subitem, buffer.get( ), sizeNeeded, new_size_needed, chars_written_2 );
		if ( !SUCCEEDED( res_2 ) ) {
			second_try_failed( subitem, sizeNeeded, new_size_needed );
			abort( );
			}
		if ( chars_written_2 == 0 ) {
			return 0;
			}

		/*
		_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/
		SelectObject_wrapper sofont( hDC, CWnd::GetFont( )->m_hObject );
		const auto align = IsColumnRightAligned( subitem, thisHeaderCtrl ) ? DT_RIGHT : DT_LEFT;
		VERIFY( ::DrawTextW( hDC, buffer.get( ), static_cast<int>( chars_written_2 ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) ) );

		VERIFY( ::InflateRect( &rc, TEXT_X_MARGIN, 0 ) );
		//rc.InflateRect( TEXT_X_MARGIN, 0 );
			
		return ( rc.right - rc.left );

		}

	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX )
	INT GetWidthFastPath( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ CHeaderCtrl* const thisHeaderCtrl, _In_ RECT& rc, _In_ const HDC hDC ) const {
		//column::COL_NAME requires very little work!
		if ( item->m_name_length == 0 ) {
			ASSERT( 0 == CListCtrl::GetStringWidth( item->m_name ) );
			return 0;
			}

		/*
		_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/
		SelectObject_wrapper sofont( hDC, CWnd::GetFont( )->m_hObject );
		const auto align = IsColumnRightAligned( subitem, thisHeaderCtrl ) ? DT_RIGHT : DT_LEFT;
		VERIFY( ::DrawTextW( hDC, item->m_name, static_cast<int>( item->m_name_length ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) ) );
			
		VERIFY( ::InflateRect( &rc, TEXT_X_MARGIN, 0 ) );
		//rc.InflateRect( TEXT_X_MARGIN, 0 );
		ASSERT( ( rc.right - rc.left ) == CListCtrl::GetStringWidth( item->m_name ) );
		return ( rc.right - rc.left );
		}

	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX )
	INT GetWidth_not_ownerdrawn( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem, _In_ RECT& rc, _In_ const HDC hDC ) const {
		const auto thisHeaderCtrl = CListCtrl::GetHeaderCtrl( );
		if ( subitem == column::COL_NAME ) {
			return GetWidthFastPath( item, subitem, thisHeaderCtrl, rc, hDC );
			}

		const rsize_t subitem_text_size = 128;
		_Null_terminated_ wchar_t psz_subitem_formatted_text[ subitem_text_size ] = { 0 };
		rsize_t sizeNeeded = 0;
		rsize_t chars_written = 0;

		ASSERT( subitem != column::COL_NAME );
		const HRESULT res_1 = item->GetText_WriteToStackBuffer( subitem, psz_subitem_formatted_text, subitem_text_size, sizeNeeded, chars_written );
		if ( !SUCCEEDED( res_1 ) ) {
			return first_try_failed( item, subitem, thisHeaderCtrl, rc, hDC, sizeNeeded );
			}

		if ( chars_written == 0 ) {
			return 0;
			}

		/*
		_AFXWIN_INLINE CFont* CWnd::GetFont() const
		{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
			(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
		*/
		SelectObject_wrapper sofont( hDC, CWnd::GetFont( )->m_hObject );
		const auto align = IsColumnRightAligned( subitem, thisHeaderCtrl ) ? DT_RIGHT : DT_LEFT;
		VERIFY( ::DrawTextW( hDC, psz_subitem_formatted_text, static_cast<int>( chars_written ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP | static_cast<UINT>( align ) ) );

		VERIFY( ::InflateRect( &rc, TEXT_X_MARGIN, 0 ) );
		//rc.InflateRect( TEXT_X_MARGIN, 0 );
		return ( rc.right - rc.left );
		}

protected:
	_Success_( return >= 0 ) _Ret_range_( 0, INT_MAX ) _On_failure_( _Ret_range_( -1, -1 ) )
	INT GetSubItemWidth( _In_ const COwnerDrawnListItem* const item, _In_ _In_range_( 0, INT_MAX ) const column::ENUM_COL subitem ) const {
		if ( item == NULL ) {
			return -1;
			}
		
		SimpleClientDeviceContext dc( m_hWnd );
		RECT rc { 0, 0, 1000, NODE_HEIGHT };

		INT width = 0;	
		INT dummy = rc.left;
		//it appears that if the item draws itself, then we must ask it to do so in order to find out how wide it is.
		//TODO: find a better way to do this!
		//store item width in some sort of cache?
		//BUGBUG: this is an extremely slow way of doing this!
		if ( item->DrawSubitem_( subitem, dc.m_hDC, rc, 0, &width, &dummy, this ) ) {
			return width;
			}

		return GetWidth_not_ownerdrawn( item, subitem, rc, dc.m_hDC );
		}

public:


	                      CMainFrame* const m_frameptr;
	                      bool              m_showGrid             : 1; // Whether to draw a grid
	                      bool              m_showStripes          : 1; // Whether to show stripes
						  //C4820: 'COwnerDrawnListCtrl' : '3' bytes padding added after data member 'COwnerDrawnListCtrl::m_showFullRowSelection'
	                      bool              m_showFullRowSelection : 1; // Whether to draw full row selection

	_Field_range_( 0, UINT_MAX )
	                      UINT              m_rowHeight;                // Height of an item
	                      LONG              m_yFirstItem;               // Top of a first list item
	                      COLORREF          m_windowColor;              // The default background color if !m_showStripes
						  //Warning	59	warning C4820: 'COwnerDrawnListCtrl' : '4' bytes padding added after data member 'COwnerDrawnListCtrl::m_stripeColor'
	                      COLORREF          m_stripeColor;              // The stripe color, used for every other item if m_showStripes
	            _Field_z_ PCWSTR            m_persistent_name;          // for persistence
						  SSorting          m_sorting;
						  //C4820: 'COwnerDrawnListCtrl' : '3' bytes padding added after data member 'COwnerDrawnListCtrl::m_indicatedColumn'
	_Field_range_( 0, 8 ) std::int8_t       m_indicatedColumn;
						  std::vector<bool> is_right_aligned_cache;


private:
	
	_Post_satisfies_( readable <= capacity )
	void buildArrayFromItemsInHeaderControl( _In_ _Pre_readable_size_( capacity ) const int* const columnOrder, _Out_ _Pre_writable_size_( capacity ) _Post_readable_size_( readable ) int* vertical, _In_ const rsize_t capacity, _Out_ rsize_t& readable, _In_ const CHeaderCtrl* header_ctrl ) const {
		ASSERT( static_cast<int>( capacity ) >= header_ctrl->GetItemCount( ) );
		readable = 0;


		auto x   = -( CWnd::GetScrollPos( SB_HORZ ) );
		auto hdi = zero_init_struct<HDITEM>( );
		hdi.mask = HDI_WIDTH;
		const auto header_ctrl_item_count = header_ctrl->GetItemCount( );
		if ( header_ctrl_item_count <= 0 ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::buildArrayFromItemsInHeaderControl, header_ctrl_item_count <= 0 (aborting)" );
			std::terminate( );
			}
		if ( static_cast<rsize_t>( header_ctrl_item_count ) > capacity ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::buildArrayFromItemsInHeaderControl, header_ctrl_item_count greater than capacity of array! (aborting)" );
			std::terminate( );
			}
	
		//Probably NOT vectorizable anyway.
		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( INT i = 0; i < header_ctrl_item_count; i++ ) {
			VERIFY( header_ctrl->GetItem( columnOrder[ i ], &hdi ) );
			x += hdi.cxy;
			vertical[ i ] = x;
			++readable;
			}
		}


protected:
	//manually expanded DECLARE_MESSAGE_MAP
	static const AFX_MSGMAP* PASCAL GetThisMessageMap( ) {
		static const AFX_MSGMAP_ENTRY _messageEntries[ ] = {
				{ WM_NOTIFY,                 static_cast<WORD>( static_cast<int>( HDN_DIVIDERDBLCLICKW ) ), 0u, 0u, AfxSigNotify_v,  reinterpret_cast<AFX_PMSG>(static_cast<void(AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*)>(&COwnerDrawnListCtrl::OnHdnDividerdblclick))                 },
				{ WM_NOTIFY,                 static_cast<WORD>( static_cast<int>( HDN_ITEMCLICKW ) ),       0u, 0u, AfxSigNotify_v,  reinterpret_cast<AFX_PMSG>(static_cast<void(AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*)>(&COwnerDrawnListCtrl::OnHdnItemclick))                       },
				{ WM_NOTIFY,                 static_cast<WORD>( static_cast<int>( HDN_ITEMDBLCLICKW ) ),    0u, 0u, AfxSigNotify_v,  reinterpret_cast<AFX_PMSG>(static_cast<void(AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*)>(&COwnerDrawnListCtrl::OnHdnItemdblclick))                    },
				{ WM_NOTIFY,                 static_cast<WORD>( static_cast<int>( HDN_ITEMCHANGINGW ) ),    0u, 0u, AfxSigNotify_v,  reinterpret_cast<AFX_PMSG>(static_cast<void(AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*)>(&COwnerDrawnListCtrl::OnHdnItemchanging))                    },
				{ WM_NOTIFY+WM_REFLECT_BASE, static_cast<WORD>( static_cast<int>( LVN_GETDISPINFO ) ),      0u, 0u, AfxSigNotify_v,  reinterpret_cast<AFX_PMSG>(static_cast<void(AFX_MSG_CALL CCmdTarget::*)(NMHDR*, LRESULT*)>(&COwnerDrawnListCtrl::OnLvnGetdispinfo))                     },
				{ WM_ERASEBKGND,             0u,                                                            0u, 0u, AfxSig_bD,       static_cast<AFX_PMSG>( reinterpret_cast<AFX_PMSGW>( static_cast<BOOL(AFX_MSG_CALL CWnd::*)(CDC*)>(&COwnerDrawnListCtrl::OnEraseBkgnd)))                   },
				{ WM_VSCROLL,                0u,                                                            0u, 0u, AfxSig_vwwW,     static_cast<AFX_PMSG>( reinterpret_cast<AFX_PMSGW>( static_cast<void(AFX_MSG_CALL CWnd::*)(UINT, UINT, CScrollBar*)>(&COwnerDrawnListCtrl::OnVScroll)))   },
				{ WM_SHOWWINDOW,             0u,                                                            0u, 0u, AfxSig_vbw,      static_cast<AFX_PMSG>( reinterpret_cast<AFX_PMSGW>( static_cast<void(AFX_MSG_CALL CWnd::*)(BOOL, UINT)>(&COwnerDrawnListCtrl::OnShowWindow)))             },
				{ WM_DESTROY,                0u,                                                            0u, 0u, AfxSig_vv,       static_cast<AFX_PMSG>( reinterpret_cast<AFX_PMSGW>( static_cast<void(AFX_MSG_CALL CWnd::*)(void)>(&COwnerDrawnListCtrl::OnDestroy)))                      },
				{ 0u, 0u, 0u, 0u, AfxSig_end, (AFX_PMSG)( 0 )                             }
				};
		static const AFX_MSGMAP messageMap = { &CListCtrl::GetThisMessageMap, &_messageEntries[0] };
		return &messageMap;
		}


	//DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		ASSERT_VALID( pDC );
		ASSERT( CListCtrl::GetHeaderCtrl( )->GetItemCount( ) > 0 );
		//TRACE( _T( "COwnerDrawnListCtrl::OnEraseBkgnd!\r\n" ) );
		handle_EraseBkgnd( pDC );
		return true;
		}
	afx_msg void OnHdnDividerdblclick( NMHDR *pNMHDR, LRESULT *pResult ) {
		WTL::CWaitCursor wc;
		ASSERT( pNMHDR != NULL );
		if ( pNMHDR != NULL ) {
			auto phdr = reinterpret_cast< LPNMHEADER >( pNMHDR );
			const INT subitem = phdr->iItem;
			AdjustColumnWidth( static_cast<column::ENUM_COL>( subitem ) );
			}
		ASSERT( pResult != NULL );
		if ( pResult != NULL ) {
			*pResult = 0;
			}
		}
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		// Owner drawn list controls with LVS_EX_GRIDLINES don't repaint correctly when scrolled (under Windows XP). So we force a complete repaint here.
		CWnd::InvalidateRect( NULL );
		}
	afx_msg void OnHdnItemclick( NMHDR* pNMHDR, LRESULT* pResult ) {
		const auto phdr = reinterpret_cast<LPNMHEADERW>(pNMHDR);
		*pResult = 0;
		const auto col = static_cast<column::ENUM_COL>( phdr->iItem );
		if ( col == m_sorting.column1 ) {
			m_sorting.ascending1 =  ! m_sorting.ascending1;
			}
		else {
			//SetSorting( col, true ); //GetAscendingDefault( col ) == true, unconditionally
			SetSorting( col, AscendingDefault( ) ); //GetAscendingDefault( col ) == true, unconditionally
			}
		SortItems( );
		}
	afx_msg void OnHdnItemdblclick( NMHDR* pNMHDR, LRESULT* pResult ) {
		OnHdnItemclick( pNMHDR, pResult );
		}
	afx_msg void OnHdnItemchanging( NMHDR *pNMHDR, LRESULT *pResult ) {
		UNREFERENCED_PARAMETER( pNMHDR );
		CWnd::Default( );
		CWnd::InvalidateRect( NULL );
		ASSERT( pResult != NULL );
		if ( pResult != NULL ) {
			*pResult = 0;
			}
		}
	afx_msg void OnDestroy( ) {
#ifdef DEBUG
		trace_on_destroy( m_persistent_name );
#endif
		SavePersistentAttributes( );
		CListCtrl::OnDestroy( );
		}
	afx_msg void OnLvnGetdispinfo( NMHDR* pNMHDR, LRESULT* pResult ) {
		static_assert( sizeof( NMHDR* ) == sizeof( NMLVDISPINFOW* ), "some size issues. Good luck with that cast!" );
		ASSERT( ( pNMHDR != NULL ) && ( pResult != NULL ) );
		handle_LvnGetdispinfo( pNMHDR, pResult );
		}


private:
	void draw_grid_for_EraseBkgnd( _In_ const COLORREF gridColor, _In_ CDC* pDC, _In_ const RECT& rcClient, _In_ const rsize_t vertical_readable, _In_ _In_reads_( vertical_readable ) const int* const vertical_buf ) const {
		CPen pen( PS_SOLID, 1, gridColor );
		const SelectObject_wrapper sopen( pDC->m_hDC, pen.m_hObject );

		const auto rowHeight = m_rowHeight;
		for ( auto y = ( m_yFirstItem + static_cast<LONG>( rowHeight ) - 1 ); y < rcClient.bottom; y += static_cast<LONG>( rowHeight ) ) {
			ASSERT( rowHeight == m_rowHeight );
			pDC->MoveTo( rcClient.left, static_cast<INT>( y ) );
			VERIFY( pDC->LineTo( rcClient.right, static_cast<INT>( y ) ) );
			}
		const auto verticalSize = vertical_readable;
		for ( size_t i = 0; i < verticalSize; i++ ) {
			pDC->MoveTo( ( vertical_buf[ i ] - 1 ), rcClient.top );
			VERIFY( pDC->LineTo( ( vertical_buf[ i ] - 1 ), rcClient.bottom ) );
			}
		}

	void handle_EraseBkgnd( _In_ CDC* pDC ) {
		// We should recalculate m_yFirstItem here (could have changed e.g. when the XP-Theme changed).
		if ( CListCtrl::GetItemCount( ) > 0 ) {
			RECT rc;
			VERIFY( CListCtrl::GetItemRect( CListCtrl::GetTopIndex( ), &rc, LVIR_BOUNDS ) );
			m_yFirstItem = rc.top;
			}
		// else: if we did the same thing as in OnColumnsCreated(), we get repaint problems.

		const COLORREF gridColor = RGB( 212, 208, 200 );

		RECT rcClient;
		CWnd::GetClientRect( &rcClient );

		RECT rcHeader;
		const auto header_ctrl = CListCtrl::GetHeaderCtrl( );
		header_ctrl->GetWindowRect( &rcHeader );
		CWnd::ScreenToClient( &rcHeader );

		RECT rcBetween   = rcClient;// between header and first item
		rcBetween.top    = rcHeader.bottom;
		rcBetween.bottom = m_yFirstItem;
		pDC->FillSolidRect( &rcBetween, gridColor );
		const rsize_t column_buf_size = 10;
	
		const auto header_ctrl_item_count = header_ctrl->GetItemCount( );
		
		check_column_buf_size( header_ctrl_item_count, column_buf_size );


		int column_order[ column_buf_size ] = { 0 };

		VERIFY( CListCtrl::GetColumnOrderArray( column_order, header_ctrl_item_count ) );

		int vertical_buf_temp[ column_buf_size ] = { 0 };
		rsize_t vertical_readable = 0;
		buildArrayFromItemsInHeaderControl( column_order, vertical_buf_temp, column_buf_size, vertical_readable, header_ctrl );
		ASSERT( vertical_readable < column_buf_size );

		if ( m_showGrid ) {
			draw_grid_for_EraseBkgnd( gridColor, pDC, rcClient, vertical_readable, vertical_buf_temp );
			}

		const auto bgcolor    = ::GetSysColor( COLOR_WINDOW );
		const int  gridWidth  = ( m_showGrid ? 1 : 0 );
		const auto lineCount  = CListCtrl::GetCountPerPage( ) + 1;
		const auto firstItem  = CListCtrl::GetTopIndex( );
		//((( a ) < ( b )) ? ( a ) : ( b ))
		//((( firstItem + lineCount ) < ( CListCtrl::GetItemCount( ) )) ? ( firstItem + lineCount ) : ( CListCtrl::GetItemCount( ) ))
		//const auto lastItem   = ( min( firstItem + lineCount, CListCtrl::GetItemCount( ) ) - 1 );
		const auto lastItem = ( ( ( firstItem + lineCount ) < ( CListCtrl::GetItemCount( ) ) ) ? ( firstItem + lineCount ) : ( CListCtrl::GetItemCount( ) ) );

		ASSERT( CListCtrl::GetItemCount( ) == 0 || firstItem < CListCtrl::GetItemCount( ) );
		ASSERT( CListCtrl::GetItemCount( ) == 0 || lastItem <= CListCtrl::GetItemCount( ) );
		ASSERT( CListCtrl::GetItemCount( ) == 0 || lastItem >= firstItem );

		const auto itemCount = ( lastItem - firstItem + 1 );

		const int( &vertical_buf )[ column_buf_size ] = vertical_buf_temp;

		RECT fill;
		fill.left   = vertical_buf[ vertical_readable - 1 ];
		fill.right  = rcClient.right;
		fill.top    = m_yFirstItem;
		fill.bottom = fill.top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
		
		for ( INT i = 0; i < itemCount; i++ ) {
			pDC->FillSolidRect( &fill, bgcolor );
			VERIFY( ::OffsetRect( &fill, 0, static_cast<int>( m_rowHeight ) ) );
			}

		const auto rowHeight = m_rowHeight;
		auto top = fill.top;
		while ( top < rcClient.bottom ) {
			fill.top    = top;
			fill.bottom = top + static_cast<LONG>( m_rowHeight ) - static_cast<LONG>( gridWidth );
		
			INT left = 0;
			const auto verticalSize = vertical_readable;
			for ( size_t i = 0; i < verticalSize; i++ ) {
				fill.left = left;
				fill.right = vertical_buf[ i ] - gridWidth;
				pDC->FillSolidRect( &fill, bgcolor );
				left = vertical_buf[ i ];
				}
			fill.left  = left;
			fill.right = rcClient.right;
			pDC->FillSolidRect( &fill, bgcolor );

			ASSERT( rowHeight == m_rowHeight );
			top += rowHeight;
			}

		}

	void handle_LvnGetdispinfo( _In_ NMHDR* pNMHDR, _In_ LRESULT* pResult ) {
		auto di = reinterpret_cast< NMLVDISPINFOW* >( pNMHDR );
		*pResult = 0;
		ASSERT( di->item.iItem <= CListCtrl::GetItemCount( ) );
		auto item = reinterpret_cast<COwnerDrawnListItem*>( di->item.lParam );
		ASSERT( item != NULL );
		if ( item == NULL ) {
			return;
			}
		if ( ( di->item.mask bitand LVIF_TEXT ) == 0 ) {
			return;
			}
		if ( static_cast< column::ENUM_COL >( di->item.iSubItem ) == column::COL_NAME ) {
			//Easy fastpath! No name -> no text
			if ( item->m_name == nullptr ) {
				return;
				}
			//Just copy name into buffer. No formatting required!
			size_t chars_remaining = 0;
			const HRESULT res_1 = StringCchCopyExW( di->item.pszText, static_cast< rsize_t >( di->item.cchTextMax ), item->m_name, NULL, &chars_remaining, 0 );
			ASSERT( SUCCEEDED( res_1 ) );
			if ( SUCCEEDED( res_1 ) ) {
				return;
				}
			WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res_1 );
			WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res_1, "StringCchCopyExW" );
			if ( !SUCCEEDED( res_1 ) ) {
				displayWindowsMsgBoxWithMessage( global_strings::COwnerDrawnListCtrl_handle_LvnGetdispinfo_err );
				std::terminate( );
				}
			return;
			}

		rsize_t chars_needed = 0;
		rsize_t chars_written = 0;
		ASSERT( di->item.iSubItem != column::COL_NAME );
		const HRESULT res = item->GetText_WriteToStackBuffer( static_cast< column::ENUM_COL >( di->item.iSubItem ), di->item.pszText, static_cast< rsize_t >( di->item.cchTextMax ), chars_needed, chars_written );
		ASSERT( SUCCEEDED( res ) );
		if ( SUCCEEDED( res ) ) {
			return;
			}
		WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( res );
		WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( res, "(COwnerDrawnListCtrl::handle_LvnGetdispinfo): item->GetText_WriteToStackBuffer" );
		//this first case is *maybe* tolerable.
		if ( res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( L"Error in COwnerDrawnListCtrl::handle_LvnGetdispinfo - STRSAFE_E_INSUFFICIENT_BUFFER" );
			std::terminate( );
			}
		WDS_ASSERT_NEVER_REACHED( );
		if ( res == STRSAFE_E_END_OF_FILE ) {
			displayWindowsMsgBoxWithMessage( L"Unexpected error in COwnerDrawnListCtrl::handle_LvnGetdispinfo - STRSAFE_E_END_OF_FILE" );
			std::terminate( );
			}
		displayWindowsMsgBoxWithMessage( L"Unknown GetText_WriteToStackBuffer error in COwnerDrawnListCtrl::handle_LvnGetdispinfo" );
		std::terminate( );
		}
	};

////need to explicitly ask for inlining else compiler bitches about ODR
//inline void COwnerDrawnListItem::DrawHighlightSelectBackground( _In_ const RECT& rcLabel, _In_ const RECT& rc, _In_ CDC& pdc, _Inout_ COLORREF& textColor, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
//	// Color for the text in a highlighted item (usually white)
//	textColor = list_highlight_text_color;
//
//	RECT selection = rcLabel;
//	// Depending on "FullRowSelection" style
//	if ( list_is_full_row_selection ) {
//		selection.right = rc.right;
//		}
//	// Fill the selection rectangle background (usually dark blue)
//	pdc.FillSolidRect( &selection, list_highlight_color );
//	}

//inline COLORREF COwnerDrawnListItem::draw_if_selected_return_text_color( _In_ const UINT state, _In_ const RECT rcLabel, _In_ const RECT rc, _In_ CDC& pdc, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
//	auto textColor = GetSysColor( COLOR_WINDOWTEXT );
//	if ( ( state bitand ODS_SELECTED ) != 0 ) {
//		if ( list_has_focus || list_is_show_selection_always ) {
//			ASSERT( ( ( state bitand ODS_SELECTED ) != 0 ) && ( list_has_focus || list_is_show_selection_always ) );
//			
//			DrawHighlightSelectBackground( rcLabel, rc, pdc, textColor, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
//			return textColor;
//			}
//		}
//	return item_text_color( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
//	}


////need to explicitly ask for inlining else compiler bitches about ODR
//inline void COwnerDrawnListItem::DrawLabel( _In_ CDC& pdc, _In_ RECT& rc, _In_ const UINT state, _Out_opt_ INT* const width, _Inout_ INT* const focusLeft, _In_ const bool indent,  _In_ CFont* const list_font, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_text_color, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
//	/*
//	  Draws an item label (icon, text) in all parts of the WinDirStat view. The rest is drawn by DrawItem()
//	*/
//	RECT rcRest = rc;
//
//	// Increase indentation according to tree-level
//	fixup_align_for_indent( indent, rcRest );
//
//	/*
//_AFXWIN_INLINE CFont* CWnd::GetFont() const
//	{ ASSERT(::IsWindow(m_hWnd)); return CFont::FromHandle(
//		(HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0)); }
//	*/
//
//	//TODO: performance issue in the line below due to CHandleMap::FromHandle
//	CSelectObject sofont( pdc, *( list_font ) );
//	
//	//subtract 6 from rcRest.right, add 6 to rcRest.left
//	VERIFY( ::InflateRect( &rcRest, -( TEXT_X_MARGIN ), -( 0 ) ) );
//
//	RECT rcLabel = rcRest;
//	pdc.DrawTextW( m_name, static_cast<int>( m_name_length ), &rcLabel, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX | DT_NOCLIP );//DT_CALCRECT modifies rcLabel!!!
//
//	AdjustLabelForMargin( rcRest, rcLabel );
//
//	CSetBkMode bk( pdc, TRANSPARENT );
//	//auto textColor = GetSysColor( COLOR_WINDOWTEXT );
//
//
//	const auto textColor = draw_if_selected_return_text_color( state, rcLabel, rc, pdc, list_has_focus, list_is_show_selection_always, list_highlight_text_color, list_highlight_color, list_is_full_row_selection );
//
//	//COLORREF draw_if_selected_return_text_color( width, state, list, rcLabel, rc, pdc )
//	//if ( width == NULL && ( state bitand ODS_SELECTED ) != 0 && ( list->HasFocus( ) || list->IsShowSelectionAlways( ) ) ) {
//	//	DrawHighlightSelectBackground( rcLabel, rc, list, pdc, textColor );
//	//	}
//	//else {
//	//	textColor = item_text_color( ); // Use the color designated for this item. This is currently only for encrypted and compressed items
//	//	}
//
//	// Set text color for device context
//	CSetTextColor stc( pdc, textColor );
//
//	if ( width == NULL ) {
//		pdc.DrawTextW( m_name, static_cast<int>( m_name_length ), &rcRest, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP );
//		}
//
//	//subtract one from left, add one to right
//	VERIFY( ::InflateRect( &rcLabel, 1, 1 ) );
//
//	*focusLeft = rcLabel.left;
//
//	if ( ( ( state bitand ODS_FOCUS ) != 0 ) && list_has_focus && ( width == NULL ) && ( !( list_is_full_row_selection ) ) ) {
//		pdc.DrawFocusRect( &rcLabel );
//		rcLabel.left = rc.left;
//		rc = rcLabel;
//		return;
//		}
//
//
//	rcLabel.left = rc.left;
//	rc = rcLabel;
//	if ( width != NULL ) {
//		*width = ( rcLabel.right - rcLabel.left ) + 5; // +5 because GENERAL_INDENT?
//		return;
//		}
//	}

////need to explicitly ask for inlining else compiler bitches about ODR
//inline void COwnerDrawnListItem::DrawSelection( _In_ CDC& pdc, _In_ RECT rc, _In_ const UINT state, _In_ const bool list_has_focus, _In_ const bool list_is_show_selection_always, _In_ const COLORREF list_highlight_color, _In_ const bool list_is_full_row_selection ) const {
//	if ( !list_is_full_row_selection ) {
//		return;
//		}
//	if ( ( !list_has_focus ) && ( !list_is_show_selection_always ) ) {
//		return;
//		}
//	if ( ( state bitand ODS_SELECTED ) == 0 ) {
//		return;
//		}
//
//	VERIFY( ::InflateRect( &rc, -( 0 ), -( static_cast<int>( LABEL_Y_MARGIN ) ) ) );
///*
//void CDC::FillSolidRect(LPCRECT lpRect, COLORREF clr)
//{
//	ENSURE_VALID(this);
//	ENSURE(m_hDC != NULL);
//	ENSURE(lpRect);
//
//	::SetBkColor(m_hDC, clr);
//	::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
//}
//*/
//	ASSERT( pdc.m_hDC != NULL );
//
//
//	//If [SetBkColor] fails, the return value is CLR_INVALID.
//	const auto set_bk_color_res_1 = ::SetBkColor( pdc.m_hDC, list_highlight_color );
//	ASSERT( set_bk_color_res_1 != CLR_INVALID );
//#ifndef DEBUG
//	UNREFERENCED_PARAMETER( set_bk_color_res_1 );
//#endif
//
//	//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
//	VERIFY( ::ExtTextOutW( pdc.m_hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );
//
//	//pdc.FillSolidRect( &rc, list->GetHighlightColor( ) );
//
//	}

	AFX_COMDAT const CRuntimeClass COwnerDrawnListCtrl::classCOwnerDrawnListCtrl =
	{
		"COwnerDrawnListCtrl",
		sizeof(class COwnerDrawnListCtrl),
		0xFFFF,
		NULL,
		((CRuntimeClass*)(&CListCtrl::classCListCtrl)),
		NULL,
		NULL
	};


namespace{

	void repopulate_right_aligned_cache( _Out_ std::vector<bool>& right_aligned_cache, _In_ _In_range_( 1, SIZE_T_MAX ) const size_t thisLoopSize, _In_ const CHeaderCtrl* const thisHeaderCtrl, _In_ const COwnerDrawnListCtrl* const owner_drawn_list_ctrl ) {
		right_aligned_cache.reserve( thisLoopSize );
		for ( size_t i = 0; i < thisLoopSize; ++i ) {
			right_aligned_cache.push_back( owner_drawn_list_ctrl->IsColumnRightAligned( static_cast<int>( i ), thisHeaderCtrl ) );
			}
		}

	template<size_t count>
	void build_array_of_rects_from_subitem_rects( _In_ _In_range_( 1, count ) const size_t thisLoopSize, _In_ _In_reads_( thisLoopSize ) const column::ENUM_COL( &subitems_temp )[ count ], _Out_ _Out_writes_( thisLoopSize ) RECT( &rects_temp )[ count ], _In_ const PDRAWITEMSTRUCT pdis, _In_ const COwnerDrawnListCtrl* const owner_drawn_list_ctrl, _In_ CHeaderCtrl* const thisHeaderCtrl ) {
		for ( size_t i = 0; i < thisLoopSize; ++i ) {
			rects_temp[ i ] = owner_drawn_list_ctrl->GetWholeSubitemRect( static_cast<INT>( pdis->itemID ), subitems_temp[ i ], thisHeaderCtrl );
			}
		}


}

#else
#error ass
#endif




