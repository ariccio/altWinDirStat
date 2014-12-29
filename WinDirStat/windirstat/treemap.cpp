// treemap.cpp	- Implementation of CColorSpace, CTreemap and CTreemapPreview
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
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
#include "treemap.h"
#include "globalhelpers.h"
#include "item.h"
#include "dirstatdoc.h"

//#include <afxwin.h>
//#include <stdio.h>

// I define the "brightness" of an rgb value as (r+b+g)/3/255.
// The EqualizeColors() method creates a palette with colors all having the same brightness of 0.6
// Later in RenderCushion() this number is used again to scale the colors.

#define DRAW_CUSHION_INDEX_ADJ ( index_of_this_row_0_in_array + ix )

namespace {
	void SetPixelsShim( CDC& pdc, const int x, const int y, const COLORREF color ) {
		pdc.SetPixelV( x, y, color );
		}

	double pixel_scale_factor( _In_ const std::uint64_t& remainingSize, _In_ const CRect& remaining ) {
		ASSERT( remaining.Width( ) != 0 );
		ASSERT( remaining.Height( ) != 0 );
		//const double sizePerSquarePixel_scaleFactor = ( double ) remainingSize / remaining.Width( ) / remaining.Height( );
		return ( ( double ) remainingSize / remaining.Width( ) / remaining.Height( ) );
		}

	const bool is_horizontal( _In_ const CRect& remaining ) {
		//const bool horizontal = ( remaining.Width( ) >= remaining.Height( ) );
		return ( remaining.Width( ) >= remaining.Height( ) );
		}

	const double gen_ss( const std::uint64_t& sumOfSizesOfChildrenInRow, const std::uint64_t& rmin ) {
		//const double ss = ( ( double ) sumOfSizesOfChildrenInRow + rmin ) * ( ( double ) sumOfSizesOfChildrenInRow + rmin );
		return ( ( ( double ) sumOfSizesOfChildrenInRow + rmin ) * ( ( double ) sumOfSizesOfChildrenInRow + rmin ) );
		}

	const double gen_nextworst( const double& ratio1, const double& ratio2 ) {
		return ( ( ( ratio1 ) > ( ratio2 ) ) ? ( ratio1 ) : ( ratio2 ) );
		}

	const double improved_gen_nextworst( const double& hh, const std::uint64_t& maximumSizeOfChildrenInRow, const std::uint64_t& rmin, const std::uint64_t sumOfSizesOfChildrenInRow ) {
		const double ss = gen_ss( sumOfSizesOfChildrenInRow, rmin );
		const double ratio1 = hh * maximumSizeOfChildrenInRow / ss;
		const double ratio2 = ss / hh / rmin;

		//const double& hh, const size_t& maximumSizeOfChildrenInRow, const double& ss, const size_t& rmin, const std::uint64_t sumOfSizesOfChildrenInRow )

		//(((a) > (b)) ? (a) : (b))
		//(((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
		//const double nextWorst = (((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
		//const double nextWorst = max( ratio1, ratio2 );
		//const double nextWorst = ( ( ( ratio1 ) > ( ratio2 ) ) ? ( ratio1 ) : ( ratio2 ) );
		return gen_nextworst( ratio1, ratio2 );
		}

	void adjust_rect_if_horizontal( const bool& horizontal, CRect& rc, const int& begin, const int& end ) {
		if ( horizontal ) {
			rc.top = begin;
			rc.bottom = end;
			}
		else {
			rc.left = begin;
			rc.right = end;
			}
		}
	
	const int gen_height_of_new_row( const bool& horizontal, const CRect& remaining ) {
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "Placing rows %s...\r\n" ), ( ( horizontal ) ? L"horizontally" : L"vertically" ) );
#endif
		return( horizontal ? remaining.Height( ) : remaining.Width( ) );
		}

	void fixup_width_of_row( _In_ const std::uint64_t& sumOfSizesOfChildrenInRow, _In_ const std::uint64_t& remainingSize, _Inout_ int& widthOfRow ) {
		if ( sumOfSizesOfChildrenInRow < remainingSize ) {
			//highest precedence is 1
			//C-Style type cast has precedence  3, right to left
			//multiplication    has precedence  5, left  to right
			//division          has precedence  5, left  to right
			//assignment        has precedence 11, left  to right
			//so,
			//   widthOfRow = ( int ) ( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow );
			//                ^        ^^                                   ^               ^             ^
			//                |        ||___________________________________|               |             |
			//                |        |____________________________________________________|             |
			//                |___________________________________________________________________________|
			//test program uses my favorite macro: { #define TRACE_OUT(x) std::endl << L"\t\t" << #x << L" = `" << x << L"` " } (braces not included), to stream to wcout
			//Output of test program:
			//_MSC_FULL_VER = `180031101` 
			//
			//__TIMESTAMP__ = `Wed Dec  3 00:55:35 2014` 
			//
			//__FILEW__ = `c:\users\alexander riccio\documents\visual studio 2013\projects\testparse\testparse\testparse.cpp` 
			//
			//sumOfSizesOfChildrenInRow = `3` 
			//remainingSize = `5` 
			//widthOfRow = `7` 
			//
			//( remainingSize * widthOfRow ) = `35` 
			//
			//( sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) = `0` 
			//( sumOfSizesOfChildrenInRow / remainingSize ) = `0` 
			//
			//( ( double ) sumOfSizesOfChildrenInRow / remainingSize ) = `0.6` 
			//
			//( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) = `4.2` 
			//
			//( ( int ) ( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) ) = `4` 
			//
			//( static_cast<int>( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) ) = `4` 
			//
			//( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize * widthOfRow ) = `4.2` 
			//
			//( static_cast<int>( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize * widthOfRow ) ) = `4` 
			//
			//( static_cast<int>( static_cast<double>( sumOfSizesOfChildrenInRow ) / ( remainingSize * widthOfRow ) ) ) = `0` 
			//
			//( static_cast<int>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow ) ) = `4` 
			//
			//widthOfRow = ( int ) ( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow );

#ifdef GRAPH_LAYOUT_DEBUG
			TRACE( _T( "sumOfSizesOfChildrenInRow: %llu, remainingSize: %llu, sumOfSizesOfChildrenInRow / remainingSize: %f\r\n" ), sumOfSizesOfChildrenInRow, remainingSize, ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) );
			TRACE( _T( "width of row before truncation: %f\r\n" ), static_cast<double>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow ) );
#endif
			widthOfRow = static_cast<int>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow );
			}

		}

	const double gen_fEnd( const double& fBegin, const double& fraction, const int& heightOfNewRow ) {
		return( fBegin + fraction * heightOfNewRow );
		}

	const double fixup_frac_scope_holder( const std::uint64_t& sizes_at_i, const std::uint64_t& sumOfSizesOfChildrenInRow ) {
		return( ( double ) ( sizes_at_i ) / sumOfSizesOfChildrenInRow );
		}

	const bool gen_last_child( const size_t& i, const size_t& rowEnd, const std::uint64_t& childAtIPlusOne_size ) {
		return ( i == rowEnd - 1 || childAtIPlusOne_size == 0 );
		}

	void Put_next_row_into_the_rest_of_rectangle( _In_ const bool& horizontal, _Inout_ CRect& remaining, const int& widthOfRow ) {
		if ( horizontal ) {
			remaining.left += widthOfRow;
			}
		else {
			remaining.top += widthOfRow;
			}
		}

	const double build_children_rectangle( _In_ const CRect& remaining, _Out_ CRect& rc, _In_ const bool& horizontal, const int& widthOfRow ) {
		double fBegin = DBL_MAX;
		if ( horizontal ) {
			rc.left = remaining.left;
			rc.right = remaining.left + widthOfRow;
			fBegin = remaining.top;
			}
		else {
			rc.top = remaining.top;
			rc.bottom = remaining.top + widthOfRow;
			fBegin = remaining.left;
			}
		return fBegin;
		}

	const int if_last_child_end_scope_holder( _In_ const size_t& i, _In_ const bool& horizontal, _In_ const CRect& remaining, _In_ const int& heightOfNewRow, _Inout_ int& end_scope_holder, _In_ const bool& lastChild, _In_ const std::vector<CTreeListItem*>& parent_vector_of_children ) {
		if ( lastChild ) {
#ifdef GRAPH_LAYOUT_DEBUG
			if ( ( i + 1 ) < rowEnd ) {
				//ASSERT( parent->GetChildGuaranteedValid( i + 1 ) == static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) ) );
				TRACE( _T( "Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) )->m_name.c_str( ) );
				}
			else {
				//ASSERT( parent->GetChildGuaranteedValid( i ) == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );
				TRACE( _T( "Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i ) )->m_name.c_str( ) );
				}
#else
			UNREFERENCED_PARAMETER( i );
			UNREFERENCED_PARAMETER( parent_vector_of_children );
#endif
			// Use up the whole height
			end_scope_holder = ( horizontal ? remaining.top + heightOfNewRow : remaining.left + heightOfNewRow );
			}
		return end_scope_holder;
		}

	_Success_( return < UINT64_MAX )
	const double child_at_i_fraction( _Inout_ std::map<std::uint64_t, std::uint64_t>& sizes, _In_ const size_t& i, _In_ const std::uint64_t& sumOfSizesOfChildrenInRow, _In_ const CItemBranch* child_at_I ) {
		double fraction_scope_holder = DBL_MAX;
		if ( sizes.count( i ) == 0 ) {
			sizes.at( i ) = child_at_I->size_recurse( );
			}
		fraction_scope_holder = fixup_frac_scope_holder( sizes.at( i ), sumOfSizesOfChildrenInRow );
		ASSERT( fraction_scope_holder != DBL_MAX );
		return fraction_scope_holder;
		}

	const std::uint64_t if_i_plus_one_less_than_rowEnd( _In_ const size_t& rowEnd, _In_ const size_t& i, _Inout_ std::map<std::uint64_t, std::uint64_t>& sizes, _In_ const std::vector<CTreeListItem*>& parent_vector_of_children ) {
		std::uint64_t childAtIPlusOne_size = 0;
		if ( ( i + 1 ) < rowEnd ) {
			//const auto childAtIPlusOne = parent->GetChildGuaranteedValid( i + 1 );
			//ASSERT( childAtIPlusOne == static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) ) );
			const auto childAtIPlusOne = static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) );
			if ( childAtIPlusOne != NULL ) {
				//childAtIPlusOne_size = childAtIPlusOne->size_recurse( );
				if ( sizes.count( i + 1 ) == 0 ) {
					sizes.at( i + 1 ) = childAtIPlusOne->size_recurse( );
					}
				childAtIPlusOne_size = sizes.at( i + 1 );
				}
			}
		return childAtIPlusOne_size;
		}

#ifdef DEBUG
	void assert_children_rect_smaller_than_parent_rect( const CRect& rc, const CRect& remaining ) {
		ASSERT( rc.left <= rc.right );
		ASSERT( rc.top <= rc.bottom );

		ASSERT( rc.left >= remaining.left );
		ASSERT( rc.right <= remaining.right );
		ASSERT( rc.top >= remaining.top );
		ASSERT( rc.bottom <= remaining.bottom );
		}
#endif

	const double gen_hh_size_pixel_scalefactor( _In_ const int& heightOfNewRow, _In_ const double& sizePerSquarePixel_scaleFactor ) {
		return ( ( heightOfNewRow * heightOfNewRow ) * sizePerSquarePixel_scaleFactor );
		}

	void add_child_rowEnd_to_row( _Inout_ std::uint64_t& sumOfSizesOfChildrenInRow, _In_ const std::uint64_t& rmin, _Inout_ size_t& rowEnd, _Inout_ double& worst, _In_ const double& nextWorst ) {
		sumOfSizesOfChildrenInRow += rmin;
		rowEnd++;
		worst = nextWorst;
		}

	const int gen_width_of_row( _In_ const bool& horizontal, _In_ const CRect& remaining, const std::uint64_t& sumOfSizesOfChildrenInRow, const std::uint64_t& remainingSize ) {
		// Width of row
		int widthOfRow = ( horizontal ? remaining.Width( ) : remaining.Height( ) );
		ASSERT( widthOfRow > 0 );
		fixup_width_of_row( sumOfSizesOfChildrenInRow, remainingSize, widthOfRow );
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "width of row: %i, sum of all children in row: %llu\r\n" ), widthOfRow, sumOfSizesOfChildrenInRow );
#endif
		return widthOfRow;
		}

	const std::uint64_t max_size_of_children_in_row( _In_ const std::map<std::uint64_t, std::uint64_t>& sizes, _In_ const size_t& rowBegin ) {
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "sizes[ rowBegin ]: %llu\r\n" ), sizes.at( rowBegin ) );
		TRACE( _T( "maximumSizeOfChildrenInRow: %llu\r\n" ), maximumSizeOfChildrenInRow );
#endif
		return sizes.at( rowBegin );
		}

	void shrink_for_grid( _In_ CDC& pdc, _Inout_ CRect& rc ) {
		CPen pen { PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) };
		CSelectObject sopen { pdc, pen };
		        pdc.MoveTo( rc.right - 1, rc.top );
		VERIFY( pdc.LineTo( rc.right - 1, rc.bottom ) );
		        pdc.MoveTo( rc.left,      rc.bottom - 1 );
		VERIFY( pdc.LineTo( rc.right,     rc.bottom - 1 ) );
		}

	const bool zero_size_rect( _In_ const CRect& rc ) {
		if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
			return true;
			}
		return false;
		}

	const int gen_bottom( _In_ const double& fBottom, _In_ const CArray<double, double>& rows, _In_ const bool& horizontalRows, _In_ const CRect& rc, _In_ const int& row ) {
		//int( fBottom ) truncation is required here
		int bottom = int( fBottom );

		if ( row == rows.GetSize( ) - 1 ) {
			bottom = horizontalRows ? rc.bottom : rc.right;
			}
		return bottom;
		}

	const int gen_right( _In_ const bool& lastChild, _In_ const double& fRight, _In_ const CRect& rc, _In_ const bool& horizontalRows ) {
		int right = int( fRight );

		if ( lastChild ) {
			right = horizontalRows ? rc.right : rc.bottom;
			}
		return right;
		}

	const CRect build_rc_child( _In_ const double& left, _In_ const int& right, _In_ const bool& horizontalRows, _In_ const int& bottom, _In_ const double& top ) {
		CRect rcChild;
		if ( horizontalRows ) {
			//int( left ) truncation is required here
			rcChild.left = int( left );
			rcChild.right = right;
			//int( top ) truncation is required here
			rcChild.top =  int( top );

			rcChild.bottom = bottom;
			}
		else {
			//int( left ) truncation is required here
			rcChild.left = int( top );
			rcChild.right = bottom;
			//int( top ) truncation is required here
			rcChild.top = int( left );

			rcChild.bottom = right;
			}
		rcChild.NormalizeRect( );
		return rcChild;
		}

	}

CTreemap::CTreemap( ) {
	//m_callback = callback;
	SetOptions( _defaultOptions );
	IsCushionShading_current = IsCushionShading( );
#ifdef GRAPH_LAYOUT_DEBUG
	bitSetMask = std::make_unique<std::vector<std::vector<bool>>>( 3000, std::vector<bool>( 3000, false ) );//what a mouthful
	numCalls = 0;
#endif
	}

void CTreemap::UpdateCushionShading( _In_ const bool newVal ) { 
	IsCushionShading_current = newVal;
	}

void CTreemap::SetOptions( _In_ const Treemap_Options& options ) {
	m_options = options;

	// Derive normalized vector here for performance
	const DOUBLE lx = m_options.lightSourceX;// negative = left
	const DOUBLE ly = m_options.lightSourceY;// negative = top

	const DOUBLE len = sqrt( lx*lx + ly*ly + 10*10 );
	m_Lx = lx / len;
	m_Ly = ly / len;
	m_Lz = 10 / len;

	}

#ifdef _DEBUG
void CTreemap::RecurseCheckTree( _In_ const CItemBranch* const item ) const {
 	if ( item == NULL ) {
		return;
		}

	if ( item->m_type == IT_FILE ) {
		//item doesn't have children, nothing to check
		ASSERT( item->m_childCount == 0 );
		//ASSERT( item->m_children_vector.size( ) == 0 );
		return;
		}
	validateRectangle( item, item->TmiGetRectangle( ) );
	const auto item_vector_of_children = item->size_sorted_vector_of_children( );

	for ( size_t i = 0; i < item->m_childCount; i++ ) {
		//const auto child = item->GetChildGuaranteedValid( i );
		const auto child = static_cast< CItemBranch* >( item_vector_of_children.at( i ) );
		//ASSERT( child == static_cast< CItemBranch* >( item_vector_of_children.at( i ) ) );
		validateRectangle( child, item->TmiGetRectangle( ) );
		//if ( i > 0 ) {
		//	auto child_2 = item->TmiGetChild( i - 1 );
		//	}
		RecurseCheckTree( child );
		}
}

#else

void CTreemap::RecurseCheckTree( _In_ const CItemBranch* const item ) const {
	UNREFERENCED_PARAMETER( item );
	CString msg = _T( "RecurseCheckTree was called in the release build! This shouldn't happen!" );
	AfxMessageBox( msg );
	}

#endif

void CTreemap::compensateForGrid( _Inout_ CRect& rc, _In_ CDC& pdc ) const {
	if ( m_options.grid ) {
		rc.NormalizeRect( );
		pdc.FillSolidRect( rc, m_options.gridColor );
		}
	else {
		// We shrink the rectangle here, too. If we didn't do this, the layout of the treemap would change, when grid is switched on and off.
		shrink_for_grid( pdc, rc );
		}
	rc.right--;
	rc.bottom--;

	if ( zero_size_rect( rc ) ) {
		ASSERT( false );
		return;
		}

	}

void CTreemap::DrawTreemap( _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const CItemBranch* const root, _In_opt_ const Treemap_Options* const options ) {
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );
	if ( root == NULL ) {//should never happen! Ever!
		ASSERT( root != NULL );
		}

	if ( zero_size_rect( rc ) ) {
		ASSERT( false );
		return;
		}

	if ( options != NULL ) {
		SetOptions( *options );
		}

	compensateForGrid( rc, pdc );

	if ( zero_size_rect( rc ) ) {
		ASSERT( false );
		return;
		}

	if ( root->size_recurse( ) > 0 ) {
		DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };
		rc.NormalizeRect( );

		root->TmiSetRectangle( rc );
		RecurseDrawGraph( pdc, root, rc, true, surface, m_options.height );
		}
	else {
		rc.NormalizeRect( );
		pdc.FillSolidRect( rc, RGB( 0, 0, 0 ) );
		}
	validateRectangle( root, root->TmiGetRectangle( ) );
	}

void CTreemap::DrawTreemapDoubleBuffered( _In_ CDC& pdc, _In_ const CRect& rc, _In_ CItemBranch* const root, _In_opt_ const Treemap_Options* const options ) {
	// Same as above but double buffered
	//ASSERT_VALID( pdc );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );
	if ( options != NULL ) {
		SetOptions( *options );
		}

	if ( zero_size_rect( rc ) ) {
		return;
		}

	CDC dc;
	VERIFY( dc.CreateCompatibleDC( &pdc ) );

	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( &pdc, ( rc.Width( ) ), ( rc.Height( ) ) ) );

	CSelectObject sobmp { dc, bm };

	CRect rect{ CPoint( 0, 0 ), rc.Size( ) };

	DrawTreemap( dc, rect, root, NULL );

	VERIFY( pdc.BitBlt( rc.left, rc.top, ( rc.Width( ) ), ( rc.Height( ) ), &dc, 0, 0, SRCCOPY ) );
	//VERIFY( dc.DeleteDC( ) );
	}

void CTreemap::validateRectangle( _In_ const CItemBranch* const child, _In_ const CRect& rc ) const {
#ifdef _DEBUG
	auto rcChild = child->TmiGetRectangle( );

	ASSERT(   rc.bottom < 32767 );
	ASSERT(   rc.left   < 32767 );
	ASSERT(   rc.right  < 32767 );
	ASSERT(   rc.top    < 32767 );
	ASSERT( ( ( 0-32768 ) < rc.left   ) );
	ASSERT( ( ( 0-32768 ) < rc.top    ) );
	ASSERT( ( ( 0-32768 ) < rc.right  ) );
	ASSERT( ( ( 0-32768 ) < rc.bottom ) );
	ASSERT(   rcChild.right      >=   rcChild.left );
	ASSERT(   rcChild.bottom     >=   rcChild.top );
	ASSERT(   rc.bottom          >=   rc.top );
	//ASSERT(   rcChild.right      <= ( rc.right + 1 ) );
	//ASSERT(   rcChild.bottom     <=   rc.bottom );
	rcChild.NormalizeRect( );
	ASSERT(   rcChild.Width( )   < 32767 );
	ASSERT(   rcChild.Height( )  < 32767 );
#else
	UNREFERENCED_PARAMETER( child );
	UNREFERENCED_PARAMETER( rc );
#endif
	}

_Success_( return != NULL ) _Ret_maybenull_ _Must_inspect_result_ CItemBranch* CTreemap::FindItemByPoint( _In_ const CItemBranch* const item, _In_ const CPoint point ) const {
	/*
	  In the resulting treemap, find the item below a given coordinate. Return value can be NULL - the only case that this function returns NULL is that point is not inside the rectangle of item.

	  `item` (First parameter) MUST NOT BE NULL! I'm serious.

	  Take notice of
	     (a) the very right an bottom lines, which can be "grid" and are not covered by the root rectangle,
	     (b) the fact, that WM_MOUSEMOVEs can occur after WM_SIZE but before WM_PAINT.
	
	*/
	auto rc = item->TmiGetRectangle( );
	rc.NormalizeRect( );

	if ( !rc.PtInRect( point ) ) {
		return NULL;
		}

	ASSERT( rc.PtInRect( point ) );

	auto gridWidth = m_options.grid ? 1 : 0;
	
	if ( ( ( rc.Width( ) ) <= gridWidth ) || ( ( rc.Height( ) ) <= gridWidth ) || ( item->m_type == IT_FILE ) ) {
		return const_cast<CItemBranch*>( item );
		}
	ASSERT( item->size_recurse( ) > 0 );

	ASSERT( item->m_childCount > 0 );
	auto countOfChildren = item->m_childCount;

	const auto item_vector_of_children = item->size_sorted_vector_of_children( );
	
	for ( size_t i = 0; i < countOfChildren; i++ ) {
		//auto child = item->GetChildGuaranteedValid( i );
		const auto child = static_cast< CItemBranch* >( item_vector_of_children.at( i ) );
		ASSERT( item->m_children != nullptr );
		//ASSERT( child == static_cast< CItemBranch* >( item_vector_of_children.at( i ) ) );
		ASSERT( child != NULL );
		if ( child->TmiGetRectangle( ).PtInRect( point ) ) {
			validateRectangle(	 child, rc );
			auto ret = FindItemByPoint( child, point );
			if ( ret != NULL ) {
				validateRectangle( ret, rc );
				return ret;
				}
			}
		}
	return const_cast<CItemBranch*>( item );
	}

void CTreemap::DrawColorPreview( _In_ CDC& pdc, _In_ const CRect& rc, _In_ const COLORREF color, _In_ const Treemap_Options* const options ) {
	// Draws a sample rectangle in the given style (for color legend)
	//ASSERT_VALID( pdc );
	if ( options != NULL ) {
		SetOptions( *options );
		}

	DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	AddRidge( rc, surface, m_options.height * m_options.scaleFactor );

	RenderRectangle( pdc, rc, surface, color );
	if ( m_options.grid ) {
		CPen pen { PS_SOLID, 1, m_options.gridColor };
		CSelectObject sopen{ pdc, pen };
		CSelectStockObject sobrush { pdc, NULL_BRUSH };
		VERIFY( pdc.Rectangle( rc ) );
		}
	}

void CTreemap::RecurseDrawGraph( _In_ CDC& pdc, _In_ const CItemBranch* const item, _In_ const CRect& rc, _In_ const bool asroot, _In_ const DOUBLE ( &psurface )[ 4 ], _In_ const DOUBLE height ) const {
	//ASSERT_VALID( pdc );
	ASSERT( item != NULL );
	if ( item->m_type == IT_FILE ) {
		if ( !( item->size_recurse( ) > 0 ) ) {
			return;
			}
		}
#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( " RecurseDrawGraph working on rect l: %li, r: %li, t: %li, b: %li, name: `%s`, isroot: %s\r\n" ), rc.left, rc.right, rc.top, rc.bottom, item->m_name.c_str( ), ( asroot ? L"TRUE" : L"FALSE" ) );
#endif

	//item->TmiSetRectangle( rc );
	validateRectangle( item, rc );
	const auto gridWidth = m_options.grid ? 1 : 0;

	//empty directory is a valid possibility!
	if ( ( rc.Width( ) < gridWidth ) || ( rc.Height( ) < gridWidth ) ) {
		return;
		}
	DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	if ( IsCushionShading_current ) {
		surface[ 0 ] = psurface[ 0 ];
		surface[ 1 ] = psurface[ 1 ];
		surface[ 2 ] = psurface[ 2 ];
		surface[ 3 ] = psurface[ 3 ];
		if ( !asroot ) {
			AddRidge( rc, surface, height );
			validateRectangle( item, rc );
			}
		}
	if ( item->m_type == IT_FILE ) {
		RenderLeaf( pdc, item, surface );
		}
	else {
		if ( !( item->m_childCount > 0 ) ) {
			return;
			}
		DrawChildren( pdc, item, surface, height );
		}
	validateRectangle( item, rc );
	}

void CTreemap::DrawChildren( _In_ CDC& pdc, _In_ const CItemBranch* const parent, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE height ) const {
	/*
	  My first approach was to make this member pure virtual and have three classes derived from CTreemap. The disadvantage is then, that we cannot simply have a member variable of type CTreemap but have to deal with pointers, factory methods and explicit destruction. It's not worth.
	*/
	//ASSERT_VALID( pdc );
	if ( m_options.style == KDirStatStyle ) {
		KDS_DrawChildren( pdc, parent, surface, height );
		}
	else {
		ASSERT( m_options.style == SequoiaViewStyle );
		SQV_DrawChildren( pdc, parent, surface, height );
		}
	}


bool CTreemap::KDS_PlaceChildren( _In_ const CItemBranch* const parent, _Inout_ CArray<double, double>& childWidth, _Inout_ CArray<double, double>& rows, _Inout_ CArray<INT_PTR, INT_PTR>& childrenPerRow ) const {
	/*
	  return: whether the rows are horizontal.
	*/
	ASSERT( !( parent->m_type == IT_FILE ) );
	
	ASSERT( parent->m_childCount > 0 );
	//ASSERT( parent->m_children_vector.size( ) > 0 );
	const auto parentSize = parent->size_recurse( );
	if ( parentSize == 0 ) {
		rows.Add( 1.0 );
		childrenPerRow.Add( static_cast<INT_PTR>( parent->m_childCount ) );
		for ( int i = 0; static_cast<size_t>( i ) < parent->m_childCount; i++ ) {
			childWidth[ i ] = 1.0 / parent->m_childCount;
			}
		return true;
		}

	bool horizontalRows = ( parent->TmiGetRectangle( ).Width( ) >= parent->TmiGetRectangle( ).Height( ) );
#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "Placing rows %s...\r\n" ), ( ( horizontalRows ) ? L"horizontally" : L"vertically" ) );
#endif
	DOUBLE width = 1.0;
	if ( horizontalRows ) {
		if ( parent->TmiGetRectangle( ).Height( ) > 0 ) {
			width = static_cast<DOUBLE>( parent->TmiGetRectangle( ).Width( ) ) / static_cast<DOUBLE>( parent->TmiGetRectangle( ).Height( ) );
			}
		}
	else {
		if ( parent->TmiGetRectangle( ).Width( ) > 0 ) {
			width = static_cast<DOUBLE>( parent->TmiGetRectangle( ).Height( ) ) / static_cast<DOUBLE>( parent->TmiGetRectangle( ).Width( ) );
			}
		}

	size_t nextChild = 0;
	
	while ( nextChild < parent->m_childCount ) {
		INT_PTR childrenUsed;
		rows.Add( KDS_CalcNextRow( parent, nextChild, width, childrenUsed, childWidth, parentSize ) );
		childrenPerRow.Add( childrenUsed );
		nextChild += childrenUsed;
		}
	return horizontalRows;
	}

void CTreemap::KDS_DrawChildren( _In_ CDC& pdc, _In_ const CItemBranch* const parent, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const {
	/*
	  I learned this squarification style from the KDirStat executable. It's the most complex one here but also the clearest, imho.
	*/

	ASSERT( parent->m_childCount > 0 );


	//TODO: why is this a CRect& and not a CRect?
	const CRect& rc = parent->TmiGetRectangle( );

	CArray<double, double> rows;               // Our rectangle is divided into rows, each of which gets this height (fraction of total height).
	CArray<INT_PTR, INT_PTR> childrenPerRow;   // childrenPerRow[i] = # of children in rows[i]
	CArray<double, double> childWidth;         // Widths of the children (fraction of row width).

	childWidth.SetSize( static_cast<INT_PTR>( parent->m_childCount ) );
	const bool horizontalRows = KDS_PlaceChildren( parent, childWidth, rows, childrenPerRow );

	const int width = horizontalRows ? rc.Width( ) : rc.Height( );
	const int height = horizontalRows ? rc.Height( ) : rc.Width( );
	ASSERT( width >= 0 );
	ASSERT( height >= 0 );

	INT_PTR c = 0;
	double top = horizontalRows ? rc.top : rc.left;
	const auto parent_vector_of_children = parent->size_sorted_vector_of_children( );

	for ( int row = 0; row < rows.GetSize( ); row++ ) {

		const double fBottom = top + rows[ row ] * height;
		const int bottom = gen_bottom( fBottom, rows, horizontalRows, rc, row );

		double left = horizontalRows ? rc.left : rc.top;
		for ( INT_PTR i = 0; i < childrenPerRow[ row ]; i++, c++ ) {
			//const auto child = parent->GetChildGuaranteedValid( static_cast< size_t >( c ) );

			const auto child = static_cast< CItemBranch* >( parent_vector_of_children.at( static_cast< size_t >( c ) ) );
			//ASSERT( child == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );


			ASSERT( childWidth[ c ] >= 0 );
			ASSERT( left > -2 );
			const double fRight = left + childWidth[ c ] * width;
			
			const bool lastChild = ( i == childrenPerRow[ row ] - 1 || childWidth[ c + 1 ] == 0 );
			const int right = gen_right( lastChild, fRight, rc, horizontalRows );

			const CRect rcChild = build_rc_child( left, right, horizontalRows, bottom, top );

#ifdef _DEBUG
			if ( rcChild.Width( ) > 0 && rcChild.Height( ) > 0 ) {
				CRect test;
				VERIFY( test.IntersectRect( parent->TmiGetRectangle( ), rcChild ) );
				}
#endif

			child->TmiSetRectangle( rcChild );
			RecurseDrawGraph( pdc, child, rcChild, false, surface, h * m_options.scaleFactor );

			if ( lastChild ) {
				i++, c++;

				if ( i < childrenPerRow[ row ] ) {
					//const auto childAtC = parent->GetChildGuaranteedValid( static_cast< size_t >( c ) );
					const auto childAtC = static_cast< CItemBranch* >( parent_vector_of_children.at( static_cast< size_t >( c ) ) );
					//ASSERT( childAtC == static_cast< CItemBranch* >( parent_vector_of_children.at( c ) ) );
					if ( childAtC != NULL ) {
						childAtC->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
						}
					}

				c += childrenPerRow[ row ] - i;
				break;
				}

			left = fRight;
			}
		// This asserts due to rounding error: ASSERT(left == (horizontalRows ? rc.right : rc.bottom));
		top = fBottom;
		}
	// This asserts due to rounding error: ASSERT(top == (horizontalRows ? rc.bottom : rc.right));
	}

DOUBLE CTreemap::KDS_CalcNextRow( _In_ const CItemBranch* const parent, _In_ _In_range_( 0, INT_MAX ) const size_t nextChild, _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Out_ INT_PTR& childrenUsed, _Inout_ CArray<DOUBLE, DOUBLE>& childWidth, const std::uint64_t parentSize ) const {
	size_t i = 0;
	static const double _minProportion = 0.4;
	ASSERT( _minProportion < 1 );
	ASSERT( nextChild < parent->m_childCount );
	ASSERT( width >= 1.0 );

#ifdef DEBUG
	auto parentSizeRecurse = parent->size_recurse( );
	ASSERT( parentSizeRecurse == parentSize );
#endif

	const double mySize = static_cast<double>( parentSize );
	ASSERT( mySize > 0 );
	ULONGLONG sizeUsed = 0;
	double rowHeight = 0;

	std::vector<std::uint64_t> parentSizes( parent->m_childCount, UINT64_MAX );

	const auto parent_vector_of_children = parent->size_sorted_vector_of_children( );


	ASSERT( nextChild < parent->m_childCount );//the following loop NEEDS to iterate at least once
	for ( i = nextChild; i < parent->m_childCount; i++ ) {

		//auto childAtI = parent->TmiGetChild( i );
		//std::uint64_t childSize = 0;
		//ASSERT( parent->GetChildGuaranteedValid( i ) == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );
		//const std::uint64_t childSize = parent->GetChildGuaranteedValid( i )->size_recurse( );
		const std::uint64_t childSize = static_cast< CItemBranch* >( parent_vector_of_children.at( i ) )->size_recurse( );
		parentSizes.at( i ) = childSize;
		if ( childSize == 0 ) {
			ASSERT( i > nextChild );  // first child has size > 0
			break;
			}

		sizeUsed += childSize;
		const double virtualRowHeight = sizeUsed / mySize;
		ASSERT( virtualRowHeight > 0 );
		ASSERT( virtualRowHeight <= 1 );

		// Rectangle(mySize)    = width * 1.0
		// Rectangle(childSize) = childWidth * virtualRowHeight
		// Rectangle(childSize) = childSize / mySize * width
		const double childWidth_loc = ( childSize / mySize * width / virtualRowHeight );
		if ( childWidth_loc / virtualRowHeight < _minProportion ) {
			ASSERT( i > nextChild ); // because width >= 1 and _minProportion < 1.
			// For the first child we have:
			// childWidth / rowHeight
			// = childSize / mySize * width / rowHeight / rowHeight
			// = childSize * width / sizeUsed / sizeUsed * mySize
			// > childSize * mySize / sizeUsed / sizeUsed
			// > childSize * childSize / childSize / childSize
			// = 1 > _minProportion.
			break;
			}
		rowHeight = virtualRowHeight;
		ASSERT( rowHeight != 0.00 );
	}
	ASSERT( i > nextChild );

	// Now i-1 is the last child used and rowHeight is the height of the row.

	// We add the rest of the children, if their size is 0.
#pragma warning(suppress: 6011)//not null here!
	//while ( ( i < parent->m_childCount ) && ( parent->GetChildGuaranteedValid( i )->size_recurse( ) == 0 ) ) {
	//	ASSERT( parent->GetChildGuaranteedValid( i ) == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );
	//	i++;
	//	}

	while ( ( i < parent->m_childCount ) && ( static_cast< CItemBranch* >( parent_vector_of_children.at( i ) )->size_recurse( ) == 0 ) ) {
		//ASSERT( parent->GetChildGuaranteedValid( i ) == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );
		i++;
		}


	childrenUsed = static_cast<INT_PTR>( i - nextChild );
	ASSERT( rowHeight != 0.00 );
	// Now as we know the rowHeight, we compute the widths of our children.
	for ( i = 0; i < static_cast<size_t>( childrenUsed ); i++ ) {
		// Rectangle(1.0 * 1.0) = mySize
		const double rowSize = mySize * rowHeight;
		double childSize = DBL_MAX;
		//const auto thisChild = parent->GetChildGuaranteedValid( nextChild + i );
		const auto thisChild = static_cast< CItemBranch* >( parent_vector_of_children.at( nextChild + i ) );
		//ASSERT( thisChild == static_cast< CItemBranch* >( parent_vector_of_children.at( nextChild + i ) ) );
		if ( parentSizes.at( nextChild + i ) != UINT64_MAX ) {
			childSize = ( double ) parentSizes.at( nextChild + i );
			}
		else {
			childSize = ( double ) thisChild->size_recurse( );
			}

		//childSize = ( double ) thisChild->size_recurse( );
		ASSERT( rowSize != 0.00 );
		ASSERT( childSize != DBL_MAX );
		const double cw = childSize / rowSize;
		ASSERT( cw >= 0 );

#ifdef DEBUG
		const auto val = nextChild + i;
		ASSERT( val < static_cast<size_t>( childWidth.GetSize( ) ) );
#endif

		childWidth[ static_cast<INT_PTR>( nextChild + i ) ] = cw;
		}
	return rowHeight;
	}


// The classical squarification method.
void CTreemap::SQV_DrawChildren( _In_ CDC& pdc, _In_ const CItemBranch* const parent, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const {
	// Rest rectangle to fill
	CRect remaining( parent->TmiGetRectangle( ) );

	if ( ( remaining.Width( ) == 0 ) || ( remaining.Height( ) == 0 ) ) {
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "SQV_DrawChildren encountered an invalid `remaining` rectangle. Width & Height must be greater than 0! Width: %i, Height: %i\r\n" ), remaining.Width( ), remaining.Height( ) );
#endif
		return;
		}

	ASSERT( remaining.Width( ) > 0 );
	ASSERT( remaining.Height( ) > 0 );

	// Size of rest rectangle
	auto remainingSize = parent->size_recurse( );
	ASSERT( remainingSize > 0 );

	// Scale factor
	const double sizePerSquarePixel_scaleFactor = pixel_scale_factor( remainingSize, remaining );
	// First child for next row
	size_t head = 0;

	const auto parent_vector_of_children = parent->size_sorted_vector_of_children( );

#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "head: %llu\r\n" ), head );
#endif

	while ( head < parent->m_childCount ) {
		ASSERT( remaining.Width( ) > 0 );
		ASSERT( remaining.Height( ) > 0 );

		// How we divide the remaining rectangle
		const bool horizontal = is_horizontal( remaining );

		const int heightOfNewRow = gen_height_of_new_row( horizontal, remaining );

		// Square of height in size scale for ratio formula
		const double hh = gen_hh_size_pixel_scalefactor( heightOfNewRow, sizePerSquarePixel_scaleFactor );
		ASSERT( hh > 0 );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		const auto rowBegin = head;
		auto rowEnd   = head;

		// Worst ratio so far
		double worst  = DBL_MAX;

		auto sizes = std::map<std::uint64_t, std::uint64_t>( );
		///ASSERT( parent_vector_of_children.at( rowBegin )->size_recurse_( ) == parent->GetChildGuaranteedValid( rowBegin )->size_recurse( ) );
		//sizes[ rowBegin ] = parent->GetChildGuaranteedValid( rowBegin )->size_recurse( );
		sizes[ rowBegin ] = parent_vector_of_children.at( rowBegin )->size_recurse_( );

		const auto maximumSizeOfChildrenInRow = max_size_of_children_in_row( sizes, rowBegin );

		// Sum of sizes of children in row
		std::uint64_t sumOfSizesOfChildrenInRow = 0;

		// This condition will hold at least once.
		while ( rowEnd < parent->m_childCount ) {
			// We check a virtual row made up of child(rowBegin)...child(rowEnd) here.

			// Minimum size of child in virtual row
			//sizes[ rowEnd ] = parent->GetChildGuaranteedValid( rowEnd )->size_recurse( );
			sizes[ rowEnd ] = parent_vector_of_children.at( rowEnd )->size_recurse_( );
			//ASSERT( parent_vector_of_children.at( rowEnd )->size_recurse_( ) == parent->GetChildGuaranteedValid( rowEnd )->size_recurse( ) );

#ifdef GRAPH_LAYOUT_DEBUG
			TRACE( _T( "sizes[ rowEnd ]: %llu\r\n" ), sizes[ rowEnd ] );
#endif
			const auto rmin = sizes.at( rowEnd );
			if ( rmin == 0 ) {
				rowEnd = parent->m_childCount;
#ifdef GRAPH_LAYOUT_DEBUG
				TRACE( _T( "Hit row end! Parent item: `%s`\r\n" ), parent->m_name.c_str( ) );
#endif
				break;
				}
			ASSERT( rmin != 0 );
			// Calculate the worst ratio in virtual row.
			// Formula taken from the "Squarified Treemaps" paper. ('stm.pdf')
			// (http://http://www.win.tue.nl/~vanwijk/)

			//const double ss = ( ( double ) sumOfSizesOfChildrenInRow + rmin ) * ( ( double ) sumOfSizesOfChildrenInRow + rmin );
			//
			//const double ss = gen_ss( sumOfSizesOfChildrenInRow, rmin );
			//const double ratio1 = hh * maximumSizeOfChildrenInRow / ss;
			//const double ratio2 = ss / hh / rmin;
			//
			////const double& hh, const size_t& maximumSizeOfChildrenInRow, const double& ss, const size_t& rmin, const std::uint64_t sumOfSizesOfChildrenInRow )
			//
			////(((a) > (b)) ? (a) : (b))
			////(((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
			////const double nextWorst = (((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
			////const double nextWorst = max( ratio1, ratio2 );
			////const double nextWorst = ( ( ( ratio1 ) > ( ratio2 ) ) ? ( ratio1 ) : ( ratio2 ) );
			//
			//const double nextWorst = gen_nextworst( ratio1, ratio2 );

			const double nextWorst = improved_gen_nextworst( hh, maximumSizeOfChildrenInRow, rmin, sumOfSizesOfChildrenInRow );

			// Will the ratio get worse?
			if ( nextWorst > worst ) {
#ifdef GRAPH_LAYOUT_DEBUG
				TRACE( _T( "Breaking! Ratio would get worse! Parent item: `%s`\r\n" ), parent->m_name.c_str( ) );
#endif
				// Yes. Don't take the virtual row, but the real row (child(rowBegin)..child(rowEnd - 1)) made so far.
				break;
				}

			// Here we have decided to add child(rowEnd) to the row.
			add_child_rowEnd_to_row( sumOfSizesOfChildrenInRow, rmin, rowEnd, worst, nextWorst );
			}

		// Row will be made up of child(rowBegin)...child(rowEnd - 1).
		// sumOfSizesOfChildrenInRow is the size of the row.

		// As the size of parent is greater than zero, the size of the first child must have been greater than zero, too.
		ASSERT( sumOfSizesOfChildrenInRow > 0 );


		const int widthOfRow = gen_width_of_row( horizontal, remaining, sumOfSizesOfChildrenInRow, remainingSize );


		// else: use up the whole width
		// width may be 0 here.

		// Build the rectangles of children.
		CRect rc;
		double fBegin = build_children_rectangle( remaining, rc, horizontal, widthOfRow );

		// Now put the children into their places
		for ( auto i = rowBegin; i < rowEnd; i++ ) {
			const int begin = ( int ) fBegin;
			const auto child_at_I = static_cast< CItemBranch* >( parent_vector_of_children.at( i ) );

			//ASSERT( child_at_I == static_cast< CItemBranch* >( parent_vector_of_children.at( i ) ) );

			const double fraction = child_at_i_fraction( sizes, i, sumOfSizesOfChildrenInRow, child_at_I );

			const double fEnd = gen_fEnd( fBegin, fraction, heightOfNewRow );
			int end_scope_holder = ( int ) fEnd;

			const std::uint64_t childAtIPlusOne_size = if_i_plus_one_less_than_rowEnd( rowEnd, i, sizes, parent_vector_of_children );

			const bool lastChild = gen_last_child( i, rowEnd, childAtIPlusOne_size );

			const int end = if_last_child_end_scope_holder( i, horizontal, remaining, heightOfNewRow, end_scope_holder, lastChild, parent_vector_of_children );

			adjust_rect_if_horizontal( horizontal, rc, begin, end );

#ifdef DEBUG
			assert_children_rect_smaller_than_parent_rect( rc, remaining );
#endif

			child_at_I->TmiSetRectangle( rc );
			RecurseDrawGraph( pdc, child_at_I, rc, false, surface, h * m_options.scaleFactor );

			if ( lastChild ) {
#ifdef GRAPH_LAYOUT_DEBUG
				if ( ( i + 1 ) < rowEnd ) {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) )->m_name.c_str( ) );
					}
				else {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i ) )->m_name.c_str( ) );
					}
#endif
				break;
				}
			else {
#ifdef GRAPH_LAYOUT_DEBUG
				if ( ( i + 1 ) < rowEnd ) {
					TRACE( _T( "NOT Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i + 1 ) )->m_name.c_str( ) );
					}
				else {
					TRACE( _T( "NOT Last child! Parent item: `%s`\r\n" ), static_cast< CItemBranch* >( parent_vector_of_children.at( i ) )->m_name.c_str( ) );
					}
#endif
				}

			ASSERT( !lastChild );
			fBegin = fEnd;
			}

		// Put the next row into the rest of the rectangle
		Put_next_row_into_the_rest_of_rectangle( horizontal, remaining, widthOfRow );

		remainingSize -= sumOfSizesOfChildrenInRow;

		ASSERT( remaining.left <= remaining.right );
		ASSERT( remaining.top <= remaining.bottom );

		head += ( rowEnd - rowBegin );

		if ( remaining.Width( ) <= 0 || remaining.Height( ) <= 0 ) {
			if ( head < parent->m_childCount ) {
				//parent->GetChildGuaranteedValid( head )->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
				static_cast< CItemBranch* >( parent_vector_of_children.at( head ) )->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
				//const auto temp_parent_vector_of_children_at = static_cast< const CItemBranch*>( parent_vector_of_children.at( rowEnd ) );
				//ASSERT( temp_parent_vector_of_children_at == ( parent->GetChildGuaranteedValid( rowEnd ) ) );
				}
			break;
			}
		}

	ASSERT( remainingSize == 0 );
	ASSERT( remaining.left == remaining.right || remaining.top == remaining.bottom );

	}

bool CTreemap::IsCushionShading( ) const {
	return m_options.ambientLight < 1.0 && m_options.height > 0.0 && m_options.scaleFactor > 0.0;
	}

void CTreemap::RenderLeaf( _In_ CDC& pdc, _In_ const CItemBranch* const item, _In_ const DOUBLE ( &surface )[ 4 ] ) const {
	// Leaves space for grid and then calls RenderRectangle()
	//const auto ass = surface[ 4 ];
	auto rc = item->TmiGetRectangle( );
	if ( m_options.grid ) {
		rc.top++;
		rc.left++;
		if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
			return;
			}
		}
	rc.NormalizeRect( );
	//auto colorOfItem = item->GetGraphColor( );
	COLORREF colorOfItem;
	if ( item->m_type == IT_FILE ) {
		colorOfItem = GetDocument( )->GetCushionColor( item->CStyle_GetExtensionStrPtr( ) );
		}
	else {
		ASSERT( item->m_type == IT_FILE );
		colorOfItem = RGB( 254, 254, 254 );
		}
	RenderRectangle( pdc, rc, surface, colorOfItem );
	}

void CTreemap::RenderRectangle( _In_ CDC& pdc, _In_ const CRect& rc, _In_ const DOUBLE ( &surface )[ 4 ], _In_ DWORD color ) const {
	auto brightness = m_options.brightness;
	//const auto ass = surface[ 4 ];
	if ( ( color bitand COLORFLAG_MASK ) != 0 ) {
		auto flags = ( color bitand COLORFLAG_MASK );
		color = CColorSpace::MakeBrightColor( color, PALETTE_BRIGHTNESS );
		if ( ( flags bitand COLORFLAG_DARKER ) != 0 ) {
			brightness *= 0.66;
			}
		else {
			brightness *= 1.2;
			if ( brightness > 1.0 ) {
				brightness = 1.0;
				}
			}
		}
	ASSERT( color != 0 );
	//ASSERT( ( brightness / PALETTE_BRIGHTNESS ) <= 1.0 );
	if ( IsCushionShading_current ) {
		DrawCushion( pdc, rc, surface, color, brightness );
		}
	else {
		DrawSolidRect( pdc, rc, color, brightness );
		}
	}

void CTreemap::DrawSolidRect( _In_ CDC& pdc, _In_ const CRect& rc, _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) const {
	INT red   = GetRValue( col );
	INT green = GetGValue( col );
	INT blue  = GetBValue( col );
	
	const DOUBLE factor = brightness / PALETTE_BRIGHTNESS;

	red = INT( std::lround( red * factor ) );
	green = INT( std::lround( green * factor ));
	blue  = INT( std::lround( blue * factor ) );

	NormalizeColor( red, green, blue );

	pdc.FillSolidRect( rc, RGB( red, green, blue ) );
	}

static_assert( sizeof( INT ) == sizeof( std::int_fast32_t ), "setPixStruct bad point type!!" );
static_assert( sizeof( std::int_fast32_t ) == sizeof( COLORREF ), "setPixStruct bad color type!!" );

void CTreemap::SetPixels ( _In_ CDC& pdc, _In_reads_( maxIndex ) _Pre_readable_size_( maxIndex ) const COLORREF* pixles, _In_ const int&   yStart, _In_ const int& xStart, _In_ const int& yEnd, _In_ const int& xEnd,   _In_ const int rcWidth, _In_ const size_t offset, const size_t maxIndex ) const {
	//row = iy * rc.Width( );
	//stride = ix;
	//index = row + stride;

	CDC tempDCmem;
	VERIFY( tempDCmem.CreateCompatibleDC( &pdc ) );
	CBitmap bmp;
	

	const auto index = ( yStart * rcWidth ) + xStart - offset;
	//auto index = ( yStart * ( xEnd - xStart ) ) + xStart;
	ASSERT( rcWidth == ( xEnd - xStart ) );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( xEnd );
#endif

	const auto res = bmp.CreateBitmap( rcWidth, ( yEnd - yStart ), 1, 32, &pixles[ index ] );
	//auto success = pdc.BitBlt( xStart, yStart, rcWidth, ( yEnd - yStart ), &tempDCmem, 0, 0, SRCCOPY );
	CBitmap* oldBMP = tempDCmem.SelectObject( &bmp );
	if ( ( rcWidth != 0 ) && ( ( yEnd - yStart ) != 0 ) ) {
		auto success = pdc.TransparentBlt( xStart, yStart, rcWidth, ( yEnd - yStart ), &tempDCmem, 0, 0, rcWidth, ( yEnd - yStart ), RGB( 255, 255, 255 ) );
		ASSERT( success != FALSE );
		}

	//VERIFY( tempDCmem.DeleteDC( ) );
	}



void CTreemap::DrawCushion( _In_ CDC& pdc, const _In_ CRect& rc, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) const {
	ASSERT( rc.bottom >= 0 );
	ASSERT( rc.top >= 0 );
	ASSERT( rc.right >= 0 );
	ASSERT( rc.left >= 0 );
	// Cushion parameters
	const DOUBLE Ia = m_options.ambientLight;
	// Derived parameters
	const DOUBLE Is = 1 - Ia;			// shading

	const DOUBLE colR = GetRValue( col );
	const DOUBLE colG = GetGValue( col );
	const DOUBLE colB = GetBValue( col );


#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "DrawCushion drawing rectangle    l: %li, r: %li, t: %li, b: %li\r\n" ), rc.left, rc.right, rc.top, rc.bottom );
#endif

	//const auto ass = surface[ 4 ];

	
	
	//size_t smallestIndexWritten = SIZE_T_MAX;
	ASSERT( rc.bottom >= 0 );
	ASSERT( rc.right >= 0 );
	ASSERT( rc.left >= 0 );
	ASSERT( rc.top >= 0 );
	const auto loop_rect__end__outer = static_cast<size_t>( rc.bottom );
	const auto loop_rect__end__inner = static_cast<size_t>( rc.right  );
	const auto loop_rect_start_inner = static_cast<size_t>( rc.left   );
	const auto loop_rect_start_outer = static_cast<size_t>( rc.top    );
	//const auto rc_width = ( loop_rect__end__inner - loop_rect_start_inner );
	const auto inner_stride = ( loop_rect__end__inner - loop_rect_start_inner );

	const auto offset = static_cast<size_t>( ( loop_rect_start_outer * inner_stride ) + loop_rect_start_inner );
	const size_t largestIndexWritten = ( ( loop_rect__end__outer * inner_stride ) - offset ) + loop_rect__end__inner;
	//( ( rc.bottom * ( rc.right - rc.left ) ) + rc.right ) + 1;
	//const auto vecSize = static_cast< size_t >( static_cast< size_t >( rc.bottom * static_cast< size_t >( rc.right - rc.left ) ) + rc.right ) + 1;
	//const auto vecSize = static_cast< size_t >( static_cast< size_t >( loop_rect__end__outer * static_cast< size_t >( loop_rect__end__inner - loop_rect_start_inner ) ) ) + 1;
	const auto vecSize = largestIndexWritten;

	//in windef.h: `typedef DWORD COLORREF`;
	std::unique_ptr<COLORREF[ ]> pixles( new COLORREF[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> cosa_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> nx_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> ny_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> sqrt_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> pixel_double_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> pixel_R_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> pixel_G_array( new DOUBLE[ vecSize ] );
	std::unique_ptr<DOUBLE[ ]> pixel_B_array( new DOUBLE[ vecSize ] );

	const auto surface_0 = ( 2.00 * surface[ 0 ] );
	const auto surface_1 = ( 2.00 * surface[ 1 ] );

	const auto surface_2 = surface[ 2 ];
	const auto surface_3 = surface[ 3 ];

	/*

                _MSC_FULL_VER = `180031101`

                __TIMESTAMP__ = `Mon Dec 22 22:55:33 2014`

                __FILEW__ = `c:\users\alexander riccio\documents\visual studio 2013\projects\testparse\testparse\testparse.cpp`

                s1 = `3`
                s2 = `7`
                s3 = `6`

                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-10`
                -( ( 2.00 * s1 * ( i + 0.5 ) ) + s2 ) = `-10`
                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-10`
                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-10`
                -( s3 * ( i + 0.5 ) + s2 ) = `-10`
                -( ( s3 + 0.5 ) * i + s2 ) = `-7`
                -( ( ( s3 + 0.5 ) * i ) + s2 ) = `-7`


                -( 2.00 * s1 * ( i + 0.5 ) + s2 ) = `-10`



                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-16`
                -( ( 2.00 * s1 * ( i + 0.5 ) ) + s2 ) = `-16`
                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-16`
                -( 2.00 * ( s1 * ( i + 0.5 ) ) + s2 ) = `-16`
                -( s3 * ( i + 0.5 ) + s2 ) = `-16`
                -( ( s3 + 0.5 ) * i + s2 ) = `-13.5`
                -( ( ( s3 + 0.5 ) * i ) + s2 ) = `-13.5`


                -( 2.00 * s1 * ( i + 0.5 ) + s2 ) = `-16`


	*/

	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
#ifdef ACCESS_PATTERN_DEBUGGING
			const size_t indexAdjusted_dbg = ( ( ( iy * inner_stride ) + loop_rect_start_inner ) - offset );
			TRACE( _T( "( iy * inner_stride ): %ld\r\n" ), ( iy * inner_stride ) );
			TRACE( _T( "ix range: %ld\r\n" ), ( loop_rect__end__inner - loop_rect_start_inner ) );
			TRACE( _T( "offset: %llu\r\n" ), offset );
			TRACE( _T( "indx: %I64u\r\n" ), indexAdjusted_dbg );
#endif
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1200, data dependence
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
			
			const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
			ASSERT( indexAdjusted < largestIndexWritten );
#ifdef DEBUG
			if ( ix > loop_rect_start_inner ) {
				const auto idx_minus_one = ( ( ( iy * inner_stride ) + ( ix - 1 ) ) - offset );
				ASSERT( indexAdjusted == ( idx_minus_one + 1 ) );
				}
#endif
			nx_array[ indexAdjusted ] = -( ( surface_0 * ( ix + 0.5 ) ) + surface_2 );
			}
		}



	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1200, data dependence
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
			const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
			ny_array[ indexAdjusted ] = -( ( surface_1 * ( iy + 0.5 ) ) + surface_3 );
			}
		}


	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1200, data dependence
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
			const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
			//const auto nx = -( 2.00 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
			//const auto ny = -( 2.00 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );

			sqrt_array[ indexAdjusted ] = 
				sqrt( 
					nx_array[ indexAdjusted ] * nx_array[ indexAdjusted ] + 
					ny_array[ indexAdjusted ] * ny_array[ indexAdjusted ] +
					1.0 
					);
			//cosa_array[ ( indexAdjusted ) ] = ( nx*m_Lx + ny*m_Ly + m_Lz ) / sqrt_val;
			}
		}

	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1200, data dependence
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
			const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
			//const auto nx = -( 2.00 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
			//const auto ny = -( 2.00 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );
			//sqrt_array[ indexAdjusted ] = sqrt( nx_array[ ( indexAdjusted ) ] * nx_array[ ( indexAdjusted ) ] + ny_array[ ( indexAdjusted ) ] * ny_array[ ( indexAdjusted ) ] +1.0 );

			cosa_array[ indexAdjusted ] = 
				( 
				nx_array[ indexAdjusted ] * m_Lx + 
				ny_array[ indexAdjusted ] * m_Ly + 
				m_Lz 
				)
				/
				sqrt_array[ indexAdjusted ];
			}
		}

	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1200, data dependence
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
			const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
			ASSERT( cosa_array[ indexAdjusted ] <= 1.0 );

			pixel_double_array[ indexAdjusted ] = Is * cosa_array[ indexAdjusted ];

			//ASSERT( pixel >= 0 );
			//causing lots of branch mis-predictions!
			//if ( pixel < 0 ) {
			//	//pixel = 0;
			//	_CrtDbgBreak( );
			//	}
			pixel_double_array[ indexAdjusted ] -= ( ( pixel_double_array[ indexAdjusted ] < 0 ) ? pixel_double_array[ indexAdjusted ] : 0 );


			pixel_double_array[ indexAdjusted ] += Ia;
			ASSERT( pixel_double_array[ indexAdjusted ] <= 1.0 );

			// Now, pixel is the brightness of the pixel, 0...1.0.
			// Apply contrast.
			// Not implemented.
			// Costs performance and nearly the same effect can be made width the m_options->ambientLight parameter.
			// pixel= pow(pixel, m_options->contrast);
			// Apply "brightness"
			pixel_double_array[ indexAdjusted ] *= brightness / PALETTE_BRIGHTNESS;

			}
		}

	
	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1304, assignments of different sizes
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {

			//row = iy * rc.Width( );
			//stride = ix;
			//index = row + stride;
			//const auto index = ( iy * ( loop_rect__end__inner - loop_rect_start_inner ) ) + ix;
			//const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );

			//ASSERT( cosa_array[ indexAdjusted ] <= 1.0 );
			//auto pixel = Is * cosa_array[ indexAdjusted ];
			////ASSERT( pixel >= 0 );
			////causing lots of branch mis-predictions!
			////if ( pixel < 0 ) {
			////	//pixel = 0;
			////	_CrtDbgBreak( );
			////	}
			//pixel -= ( ( pixel < 0 ) ? pixel : 0 );
			//pixel += Ia;
			//ASSERT( pixel <= 1.0 );
			//// Now, pixel is the brightness of the pixel, 0...1.0.
			//// Apply contrast.
			//// Not implemented.
			//// Costs performance and nearly the same effect can be made width the m_options->ambientLight parameter.
			//// pixel= pow(pixel, m_options->contrast);
			//// Apply "brightness"
			//pixel *= brightness / PALETTE_BRIGHTNESS;

			// Make color value
			auto red   = colR * pixel_double_array[ DRAW_CUSHION_INDEX_ADJ ];
			auto green = colG * pixel_double_array[ DRAW_CUSHION_INDEX_ADJ ];
			auto blue  = colB * pixel_double_array[ DRAW_CUSHION_INDEX_ADJ ];


			//if ( red >= 256 ) {
			//	red = 255;
			//	}
			//if ( red >= 256 ) {
			//	_CrtDbgBreak( );
			//	}
			red -= ( ( red >= 256.00 ) ? ( red - 255.00 ) : 0.00 );
			//if ( red == 0 ) {
			//	red++;
			//	}
			red += ( ( red == 0.00 ) ? 1.00 : 0.00 );

			//if ( green >= 256 ) {
			//	green = 255;
			//	}
			//if ( green >= 256 ) {
			//	_CrtDbgBreak( );
			//	}

			green -= ( ( green >= 256.00 ) ? ( green - 255.00 ) : 0.00 );
			//if ( green == 0 ) {
			//	green++;
			//	}
			green += ( ( green == 0.00 ) ? 1.00 : 0.00 );

			//if ( blue >= 256 ) {
			//	blue = 255;
			//	}
			//if ( blue >= 256 ) {
			//	_CrtDbgBreak( );
			//	}

			blue -= ( ( blue >= 256.00 ) ? ( blue - 255.00 ) : 0.00 );
			//if ( blue == 0 ) {
			//	blue++;
			//	}
			blue += ( ( blue == 0.00 ) ? 1.00 : 0.00 );

			//TRACE( _T( "red: %i, green: %i, blue: %i\r\n" ), red, green, blue );

			ASSERT( red < 256.00 );
			ASSERT( green < 256.00 );
			ASSERT( blue < 256.00 );


			pixel_R_array[ DRAW_CUSHION_INDEX_ADJ ] = red;
			pixel_G_array[ DRAW_CUSHION_INDEX_ADJ ] = green;
			pixel_B_array[ DRAW_CUSHION_INDEX_ADJ ] = blue;


			//BECAUSE none of the values are greater than 255, we NEVER need to call NormalizeColor!!
			//NormalizeColor( red, green, blue );
			// ... and set!
			ASSERT( RGB( red, green, blue ) != 0 );

			}
		}
			
	//Not vectorized: 1106, outer loop
	for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
		const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
		//Not vectorized: 1300
		for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {

			//row = iy * rc.Width( );
			//stride = ix;
			//index = row + stride;
			//const auto index = ( iy * ( loop_rect__end__inner - loop_rect_start_inner ) ) + ix;
			//const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );

			//pixles.at( indexAdjusted ) = RGB( red, green, blue );
			pixles[ DRAW_CUSHION_INDEX_ADJ ] = RGB( 
													static_cast<INT>( pixel_R_array[ DRAW_CUSHION_INDEX_ADJ ] ), 
													static_cast<INT>( pixel_G_array[ DRAW_CUSHION_INDEX_ADJ ] ), 
													static_cast<INT>( pixel_B_array[ DRAW_CUSHION_INDEX_ADJ ] )
													);

			}
		}

	////Not vectorized: 1106, outer loop
	//for ( auto iy = loop_rect_start_outer; iy < loop_rect__end__outer; iy++ ) {
	//	const auto index_of_this_row_0_in_array = ( ( iy * inner_stride ) - offset );
	//	//Not vectorized: 1305, not enough type information
	//	for ( auto ix = loop_rect_start_inner; ix < loop_rect__end__inner; ix++ ) {
	//		const size_t indexAdjusted = ( index_of_this_row_0_in_array + ix );
	//		//if ( indexAdjusted > largestIndexWritten ) {
	//		//	largestIndexWritten = indexAdjusted;
	//		//	}
	//		
	//		largestIndexWritten += ( ( indexAdjusted > largestIndexWritten ) ? ( indexAdjusted - largestIndexWritten ) : 0 );
	//		//if ( smallestIndexWritten > indexAdjusted ) {
	//		//	smallestIndexWritten = indexAdjusted;
	//		//	}
	//		
	//		//smallestIndexWritten -= ( ( smallestIndexWritten > indexAdjusted ) ? ( smallestIndexWritten - indexAdjusted ) : 0 );
	//		}
	//	}
	
#ifdef SIMD_ACCESS_DEBUGGING
	//ASSERT( ( largestIndexWritten % 2 ) == 0 );
	for ( size_t i = 2; i < 16; i += 2 ) {
		if ( ( ( largestIndexWritten % i ) % 2 ) == 0 ) {
			TRACE( _T( "%u %% %u: %u\r\n" ), unsigned( largestIndexWritten ), unsigned( i ), unsigned( largestIndexWritten % i ) );
			}
		}
#endif
	if ( vecSize != 0 ) {
		//TRACE( _T( "Largest index written: %I64u, size of pixels: %I64u\r\n" ), std::uint64_t( largestIndexWritten ), std::uint64_t( vecSize ) );
		SetPixels( pdc, pixles.get( ), rc.top, rc.left, rc.bottom, rc.right, rc.Width( ), offset, largestIndexWritten );
		}
	}

#ifdef GRAPH_LAYOUT_DEBUG
void CTreemap::debugSetPixel( CDC& pdc, int x, int y, COLORREF c ) const {
	++numCalls;
	//This function detects drawing collisions!
	if ( !( bitSetMask->at( x ).at( y ) ) ) {
		( *bitSetMask )[ x ][ y ] = true;//we already know that we're in bounds.
		pdc.SetPixel( x, y, c );

		SetPixelsShim( pdc, x, y, c );
		
		}
	else {
		ASSERT( false );
		AfxDebugBreak( );
		}
	}
#endif

void CTreemap::AddRidge( _In_ const CRect& rc, _Inout_ DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const {
	auto width = ( rc.Width( ) );
	auto height = ( rc.Height( ) );

	if ( ( width == 0 ) || ( height == 0 ) ) {
		//TRACE( _T( "AddRidge passed a bad rectangle! Width & Height must be greater than 0! Width: %i, Height: %i\r\n" ), width, height );//Too noisy!
		return;
		}
	ASSERT( width > 0 && height > 0 );

	DOUBLE h4 = 4 * h;

	DOUBLE wf   = h4 / width;
	surface[ 2 ] += wf * ( rc.right + rc.left );
	surface[ 0 ] -= wf;

	DOUBLE hf   = h4 / height;
	surface[ 3 ] += hf * ( rc.bottom + rc.top );
	surface[ 1 ] -= hf;
	}
// $Log$
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
