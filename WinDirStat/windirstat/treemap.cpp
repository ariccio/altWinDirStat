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


namespace {
	void SetPixelsShim( CDC& pdc, const int x, const int y, const COLORREF color ) {
		pdc.SetPixelV( x, y, color );
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
		ASSERT( item->m_children_vector.size( ) == 0 );
		return;
		}
	validateRectangle( item, item->TmiGetRectangle( ) );
	
	for ( size_t i = 0; i < item->m_childCount; i++ ) {
		auto child = item->TmiGetChild( i );
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
		CPen pen { PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) };
		CSelectObject sopen { pdc, pen };
		pdc.MoveTo( rc.right - 1, rc.top );
		VERIFY( pdc.LineTo( rc.right - 1, rc.bottom ) );
		pdc.MoveTo( rc.left,      rc.bottom - 1 );
		VERIFY( pdc.LineTo( rc.right,     rc.bottom - 1 ) );
		}
	rc.right--;
	rc.bottom--;

	}

void CTreemap::DrawTreemap( _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const CItemBranch* const root, _In_opt_ const Treemap_Options* const options ) {
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );
	if ( root == NULL ) {//should never happen! Ever!
		ASSERT( root != NULL );
		}

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
		ASSERT( false );
		return;
		}

	if ( options != NULL ) {
		SetOptions( *options );
		}

	compensateForGrid( rc, pdc );

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
		ASSERT( false );
		return;
		}
	if ( root->size_recurse( ) > 0 ) {//root can be null on zooming out??
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

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
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
	VERIFY( dc.DeleteDC( ) );
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

	for ( size_t i = 0; i < countOfChildren; i++ ) {
		auto child = item->TmiGetChild( i );
		if ( child->TmiGetRectangle( ).PtInRect( point ) ) {
			validateRectangle( child, rc );
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
	ASSERT( parent->m_children_vector.size( ) > 0 );
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
	for ( int row = 0; row < rows.GetSize( ); row++ ) {
		const double fBottom = top + rows[ row ] * height;
		//int( fBottom ) truncation is required here
		int bottom = int( fBottom );

		if ( row == rows.GetSize( ) - 1 ) {
			bottom = horizontalRows ? rc.bottom : rc.right;
			}
		double left = horizontalRows ? rc.left : rc.top;
		for ( INT_PTR i = 0; i < childrenPerRow[ row ]; i++, c++ ) {
			const auto size_t_c = static_cast< size_t >( c );
			const auto child = parent->TmiGetChild( size_t_c );
			ASSERT( childWidth[ c ] >= 0 );
			ASSERT( left > -2 );
			const double fRight = left + childWidth[ c ] * width;
			int right = int( fRight );

			bool lastChild = ( i == childrenPerRow[ row ] - 1 || childWidth[ c + 1 ] == 0 );

			if ( lastChild ) {
				right = horizontalRows ? rc.right : rc.bottom;
				}
			
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
					auto size_t_c_2 = static_cast< size_t >( c );
					auto childAtC = parent->TmiGetChild( size_t_c_2 );
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

	ASSERT( nextChild < parent->m_childCount );//the following loop NEEDS to iterate at least once
	for ( i = nextChild; i < parent->m_childCount; i++ ) {

		//auto childAtI = parent->TmiGetChild( i );
		//std::uint64_t childSize = 0;
		const std::uint64_t childSize = parent->TmiGetChild( i )->size_recurse( );
		parentSizes.at( i ) = childSize;
		//if ( childAtI != NULL ) {
		//	childSize = parent->TmiGetChild( i )->size_recurse( );
		//	}
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
	while ( ( i < parent->m_childCount ) && ( parent->TmiGetChild( i )->size_recurse( ) == 0 ) ) {
		i++;
		}

	childrenUsed = static_cast<INT_PTR>( i - nextChild );
	ASSERT( rowHeight != 0.00 );
	// Now as we know the rowHeight, we compute the widths of our children.
	for ( i = 0; i < static_cast<size_t>( childrenUsed ); i++ ) {
		// Rectangle(1.0 * 1.0) = mySize
		const double rowSize = mySize * rowHeight;
		double childSize = DBL_MAX;
		const auto thisChild = parent->TmiGetChild( nextChild + i );
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
	const double sizePerSquarePixel_scaleFactor = ( double ) remainingSize / remaining.Width( ) / remaining.Height( );

	// First child for next row
	size_t head = 0;

#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "head: %llu\r\n" ), head );
#endif

	while ( head < parent->m_childCount ) {
		ASSERT( remaining.Width( ) > 0 );
		ASSERT( remaining.Height( ) > 0 );

		// How we divide the remaining rectangle
		const bool horizontal = ( remaining.Width( ) >= remaining.Height( ) );
		
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "Placing rows %s...\r\n" ), ( ( horizontal ) ? L"horizontally" : L"vertically" ) );
#endif

		const int heightOfNewRow = horizontal ? remaining.Height( ) : remaining.Width( );

		// Square of height in size scale for ratio formula
		const double hh = ( heightOfNewRow * heightOfNewRow ) * sizePerSquarePixel_scaleFactor;
		ASSERT( hh > 0 );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		const auto rowBegin = head;
		auto rowEnd   = head;

		// Worst ratio so far
		double worst  = DBL_MAX;

		auto sizes = std::map<size_t, size_t>( );
		sizes[ rowBegin ] = parent->TmiGetChild( rowBegin )->size_recurse( );
		//auto maximumSizeOfChildrenInRow = parent->TmiGetChild( rowBegin )->size_recurse( );
		const auto maximumSizeOfChildrenInRow = sizes.at( rowBegin );
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "sizes[ rowBegin ]: %llu\r\n" ), sizes.at( rowBegin ) );
		TRACE( _T( "maximumSizeOfChildrenInRow: %llu\r\n" ), maximumSizeOfChildrenInRow );
#endif

		// Sum of sizes of children in row
		std::uint64_t sumOfSizesOfChildrenInRow = 0;

		//parent, rowEnd, sumOfSizesOfChildrenInRow
		// This condition will hold at least once.
		while ( rowEnd < parent->m_childCount ) {
			// We check a virtual row made up of child(rowBegin)...child(rowEnd) here.

			// Minimum size of child in virtual row
			//const auto rmin = parent->TmiGetChild( rowEnd )->size_recurse( );
			sizes[ rowEnd ] = parent->TmiGetChild( rowEnd )->size_recurse( );
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

			const double ss = ( ( double ) sumOfSizesOfChildrenInRow + rmin ) * ( ( double ) sumOfSizesOfChildrenInRow + rmin );
			const double ratio1 = hh * maximumSizeOfChildrenInRow / ss;
			const double ratio2 = ss / hh / rmin;


			//(((a) > (b)) ? (a) : (b))
			//(((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
			//const double nextWorst = (((ratio1) > (ratio2)) ? (ratio1) : (ratio2))
			//const double nextWorst = max( ratio1, ratio2 );
			const double nextWorst = ( ( ( ratio1 ) > ( ratio2 ) ) ? ( ratio1 ) : ( ratio2 ) );
			// Will the ratio get worse?
			if ( nextWorst > worst ) {
#ifdef GRAPH_LAYOUT_DEBUG
				TRACE( _T( "Breaking! Ratio would get worse! Parent item: `%s`\r\n" ), parent->m_name.c_str( ) );
#endif
				// Yes. Don't take the virtual row, but the real row (child(rowBegin)..child(rowEnd - 1)) made so far.
				break;
				}

			// Here we have decided to add child(rowEnd) to the row.
			sumOfSizesOfChildrenInRow += rmin;
			rowEnd++;
			worst = nextWorst;
			}

		// Row will be made up of child(rowBegin)...child(rowEnd - 1).
		// sumOfSizesOfChildrenInRow is the size of the row.

		// As the size of parent is greater than zero, the size of the first child must have been greater than zero, too.
		ASSERT( sumOfSizesOfChildrenInRow > 0 );

		// Width of row
		int widthOfRow = ( horizontal ? remaining.Width( ) : remaining.Height( ) );
		ASSERT( widthOfRow > 0 );

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

			//__TIMESTAMP__ = `Wed Dec  3 00:55:35 2014` 

			//__FILEW__ = `c:\users\alexander riccio\documents\visual studio 2013\projects\testparse\testparse\testparse.cpp` 

			//sumOfSizesOfChildrenInRow = `3` 
			//remainingSize = `5` 
			//widthOfRow = `7` 

			//( remainingSize * widthOfRow ) = `35` 

			//( sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) = `0` 
			//( sumOfSizesOfChildrenInRow / remainingSize ) = `0` 

			//( ( double ) sumOfSizesOfChildrenInRow / remainingSize ) = `0.6` 

			//( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) = `4.2` 

			//( ( int ) ( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) ) = `4` 

			//( static_cast<int>( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow ) ) = `4` 

			//( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize * widthOfRow ) = `4.2` 

			//( static_cast<int>( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize * widthOfRow ) ) = `4` 

			//( static_cast<int>( static_cast<double>( sumOfSizesOfChildrenInRow ) / ( remainingSize * widthOfRow ) ) ) = `0` 

			//( static_cast<int>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow ) ) = `4` 

			//widthOfRow = ( int ) ( ( double ) sumOfSizesOfChildrenInRow / remainingSize * widthOfRow );
#ifdef GRAPH_LAYOUT_DEBUG
			TRACE( _T( "sumOfSizesOfChildrenInRow: %llu, remainingSize: %llu, sumOfSizesOfChildrenInRow / remainingSize: %f\r\n" ), sumOfSizesOfChildrenInRow, remainingSize, ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) );
			TRACE( _T( "width of row before truncation: %f\r\n" ), static_cast<double>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow ) );
#endif
			widthOfRow = static_cast<int>( ( static_cast<double>( sumOfSizesOfChildrenInRow ) / remainingSize ) * widthOfRow );
			}
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "width of row: %i, sum of all children in row: %llu\r\n" ), widthOfRow, sumOfSizesOfChildrenInRow );
#endif

		// else: use up the whole width
		// width may be 0 here.

		// Build the rectangles of children.
		CRect rc;
		double fBegin;
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

		// Now put the children into their places
		for ( auto i = rowBegin; i < rowEnd; i++ ) {
			const int begin = ( int ) fBegin;
			const auto childAtI = parent->TmiGetChild( i );
			double fraction_scope_holder = DBL_MAX;
			if ( childAtI != NULL ) {
				if ( sizes.count( i ) == 0 ) {
					sizes.at( i ) = childAtI->size_recurse( );
					}
				//fraction_scope_holder = ( double ) ( childAtI->size_recurse( ) ) / sumOfSizesOfChildrenInRow;
				fraction_scope_holder = ( double ) ( sizes.at( i ) ) / sumOfSizesOfChildrenInRow;
				}
			ASSERT( fraction_scope_holder != DBL_MAX );

			const double fraction = fraction_scope_holder;

			const double fEnd = fBegin + fraction * heightOfNewRow;
			int end_scope_holder = ( int ) fEnd;

			std::uint64_t childAtIPlusOne_size = 0;

			if ( ( i + 1 ) < rowEnd ) {
				auto childAtIPlusOne = parent->TmiGetChild( i + 1 );
				if ( childAtIPlusOne != NULL ) {
					//childAtIPlusOne_size = childAtIPlusOne->size_recurse( );
					if ( sizes.count( i + 1 ) == 0 ) {
						sizes.at( i + 1) = childAtIPlusOne->size_recurse( );
						}
					childAtIPlusOne_size = sizes.at( i + 1 );
					}
				}
			bool lastChild = ( i == rowEnd - 1 || childAtIPlusOne_size == 0 );

			if ( lastChild ) {
#ifdef GRAPH_LAYOUT_DEBUG
				if ( ( i + 1 ) < rowEnd ) {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i + 1 )->m_name.c_str( ) );
					}
				else {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i )->m_name.c_str( ) );
					}
#endif
				// Use up the whole height
				end_scope_holder = ( horizontal ? remaining.top + heightOfNewRow : remaining.left + heightOfNewRow );
				}
			const int end = end_scope_holder;

			if ( horizontal ) {
				rc.top = begin;
				rc.bottom = end;
				}
			else {
				rc.left = begin;
				rc.right = end;
				}

			ASSERT( rc.left <= rc.right );
			ASSERT( rc.top <= rc.bottom );

			ASSERT( rc.left >= remaining.left );
			ASSERT( rc.right <= remaining.right );
			ASSERT( rc.top >= remaining.top );
			ASSERT( rc.bottom <= remaining.bottom );

			const auto child_parent_i = parent->TmiGetChild( i );
			child_parent_i->TmiSetRectangle( rc );
			RecurseDrawGraph( pdc, child_parent_i, rc, false, surface, h * m_options.scaleFactor );

			if ( lastChild ) {
#ifdef GRAPH_LAYOUT_DEBUG
				if ( ( i + 1 ) < rowEnd ) {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i + 1 )->m_name.c_str( ) );
					}
				else {
					TRACE( _T( "Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i )->m_name.c_str( ) );
					}
#endif
				break;
				}
			else {
#ifdef GRAPH_LAYOUT_DEBUG
				if ( ( i + 1 ) < rowEnd ) {
					TRACE( _T( "NOT Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i + 1 )->m_name.c_str( ) );
					}
				else {
					TRACE( _T( "NOT Last child! Parent item: `%s`\r\n" ), parent->TmiGetChild( i )->m_name.c_str( ) );
					}
#endif
				}

			ASSERT( !lastChild );
			fBegin = fEnd;
			}

		// Put the next row into the rest of the rectangle
		if ( horizontal ) {
			remaining.left += widthOfRow;
			}
		else {
			remaining.top += widthOfRow;
			}

		remainingSize -= sumOfSizesOfChildrenInRow;

		ASSERT( remaining.left <= remaining.right );
		ASSERT( remaining.top <= remaining.bottom );

		head += ( rowEnd - rowBegin );

		if ( remaining.Width( ) <= 0 || remaining.Height( ) <= 0 ) {
			if ( head < parent->m_childCount ) {
				parent->TmiGetChild( head )->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
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

#define EXPERIMENTAL_BITBLT


//void CTreemap::SetPixels( _In_ CDC& pdc, _In_ const std::vector<COLORREF>& pixles, _In_ const int& yStart, _In_ const int& xStart, _In_ const int& yEnd, _In_ const int& xEnd, _In_ const int rcWidth, _In_ const size_t offset, const size_t maxIndex ) const {
void CTreemap::SetPixels ( _In_ CDC& pdc, _In_reads_( maxIndex ) _Pre_readable_size_( pixlesSize ) const COLORREF* pixles, _In_ const int&   yStart, _In_ const int& xStart, _In_ const int& yEnd, _In_ const int& xEnd,   _In_ const int rcWidth, _In_ const size_t offset, const size_t maxIndex, const size_t pixlesSize ) const {
//row = iy * rc.Width( );
	//stride = ix;
	//index = row + stride;

#ifdef EXPERIMENTAL_BITBLT
	CDC tempDCmem;
	VERIFY( tempDCmem.CreateCompatibleDC( &pdc ) );
	CBitmap bmp;
	

	const auto index = ( yStart * rcWidth ) + xStart - offset;
	//auto index = ( yStart * ( xEnd - xStart ) ) + xStart;
	ASSERT( rcWidth == ( xEnd - xStart ) );
	const auto res = bmp.CreateBitmap( rcWidth, ( yEnd - yStart ), 1, 32, &pixles[ index ] );
	//auto success = pdc.BitBlt( xStart, yStart, rcWidth, ( yEnd - yStart ), &tempDCmem, 0, 0, SRCCOPY );
	CBitmap* oldBMP = tempDCmem.SelectObject( &bmp );
	if ( ( rcWidth != 0 ) && ( ( yEnd - yStart ) != 0 ) ) {
		auto success = pdc.TransparentBlt( xStart, yStart, rcWidth, ( yEnd - yStart ), &tempDCmem, 0, 0, rcWidth, ( yEnd - yStart ), RGB( 255, 255, 255 ) );
		ASSERT( success != FALSE );
		}

	VERIFY( tempDCmem.DeleteDC( ) );

#else

	size_t largestIndexReadFrom = 0;
	for ( auto iy = yStart; iy < yEnd; ++iy ) {
		for ( auto ix = xStart; ix < xEnd; ++ix ) {
			const auto index = ( iy * rcWidth ) + ix;
			ASSERT( rcWidth == ( xEnd - xStart ) );
			const auto adjustedIndex = index - offset;
			if ( adjustedIndex > largestIndexReadFrom ) {
				largestIndexReadFrom = adjustedIndex;
				}
			if ( adjustedIndex <= maxIndex ) {
#ifdef GRAPH_LAYOUT_DEBUG
				debugSetPixel( pdc, ix, iy, pixles[ adjustedIndex ] );
#else
				SetPixelsShim( pdc, ix, iy, pixles[ adjustedIndex ] );
#endif
				}
			else {
				ASSERT( adjustedIndex <= maxIndex );
				displayWindowsMsgBoxWithMessage( std::wstring( L"Attempt to read outside bounds of array!" ) );
				std::terminate( );
				}
			}
		}
	//TRACE( _T( "largestIndexReadFrom: %I64u\r\n" ), std::uint64_t( largestIndexReadFrom ) );
#endif
	}


//EXPERIMENTAL_BITBLT works, but colors are fucked. not sure why.
void CTreemap::DrawCushion( _In_ CDC& pdc, const _In_ CRect& rc, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) const {
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

	//( ( rc.bottom * ( rc.right - rc.left ) ) + rc.right ) + 1;
	//const auto vecSize = static_cast< size_t >( static_cast< size_t >( rc.bottom * static_cast< size_t >( rc.right - rc.left ) ) + rc.right ) + 1;
	const auto vecSize = static_cast< size_t >( static_cast< size_t >( rc.bottom * static_cast< size_t >( rc.right - rc.left ) ) ) + 1;


	//std::vector<COLORREF> pixles( vecSize );
	std::unique_ptr<COLORREF[ ]> pixles( new COLORREF[ vecSize ] );
	const auto offset = static_cast<size_t>( ( rc.top * rc.Width( ) ) + rc.left );
	size_t largestIndexWritten = 0;
	size_t smallestIndexWritten = SIZE_T_MAX;
	for ( INT iy = rc.top; iy < rc.bottom; iy++ ) {
		for ( INT ix = rc.left; ix < rc.right; ix++ ) {
			const auto nx = -( 2.00 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
			const auto ny = -( 2.00 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );
			const auto cosa = ( nx*m_Lx + ny*m_Ly + m_Lz ) / sqrt( nx*nx + ny*ny + 1.0 );

			ASSERT( cosa <= 1.0 );

			auto pixel = Is * cosa;
			//causing lots of branch mispredictions!
			if ( pixel < 0 ) {
				pixel = 0;
				}

			pixel += Ia;
			ASSERT( pixel <= 1.0 );

			// Now, pixel is the brightness of the pixel, 0...1.0.
			// Apply contrast.
			// Not implemented.
			// Costs performance and nearly the same effect can be made width the m_options->ambientLight parameter.
			// pixel= pow(pixel, m_options->contrast);
			// Apply "brightness"
			pixel *= brightness / PALETTE_BRIGHTNESS;

			// Make color value
			auto red = INT( colR * pixel );
			auto green = INT( colG * pixel );
			auto blue = INT( colB * pixel );
			if ( red >= 256 ) {
				red = 255;
				}
			if ( green >= 256 ) {
				green = 255;
				}
			if ( blue >= 256 ) {
				blue = 255;
				}
			//TRACE( _T( "red: %i, green: %i, blue: %i\r\n" ), red, green, blue );
			NormalizeColor( red, green, blue );
			if ( red == 0 ) {
				red++;
				}
			if ( green == 0 ) {
				green++;
				}
			if ( blue == 0 ) {
				blue++;
				}
			// ... and set!
			ASSERT( RGB( red, green, blue ) != 0 );

			//row = iy * rc.Width( );
			//stride = ix;
			//index = row + stride;
			const auto index = ( iy * rc.Width( ) ) + ix;
			const size_t indexAdjusted = index - offset;
			if ( indexAdjusted > largestIndexWritten ) {
				largestIndexWritten = indexAdjusted;
				}
			if ( smallestIndexWritten > indexAdjusted ) {
				smallestIndexWritten = indexAdjusted;
				}
			//pixles.at( indexAdjusted ) = RGB( red, green, blue );
			pixles[ indexAdjusted ] = RGB( red, green, blue );
			}
		}
	//if ( !pixles.empty( ) ) {
	if ( vecSize != 0 ) {
		//TRACE( _T( "Largest index written: %I64u, size of pixles: %I64u\r\n" ), std::uint64_t( largestIndexWritten ), std::uint64_t( pixles.size( ) ) );
		//TRACE( _T( "Largest index written: %I64u, size of pixles: %I64u\r\n" ), std::uint64_t( largestIndexWritten ), std::uint64_t( vecSize ) );
		//SetPixels( pdc, pixles, rc.top, rc.left, rc.bottom, rc.right, rc.Width( ), offset, largestIndexWritten );
		SetPixels( pdc, pixles.get( ), rc.top, rc.left, rc.bottom, rc.right, rc.Width( ), offset, largestIndexWritten, vecSize );
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
