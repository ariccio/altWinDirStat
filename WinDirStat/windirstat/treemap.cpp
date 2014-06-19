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
#include <afxwin.h>
#include <stdio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// I define the "brightness" of an rgb value as (r+b+g)/3/255.
// The EqualizeColors() method creates a palette with colors all having the same brightness of 0.6
// Later in RenderCushion() this number is used again to scale the colors.

static const double PALETTE_BRIGHTNESS = 0.6;

bool CTreemap::m_IsSystem256Colors = false;

/////////////////////////////////////////////////////////////////////////////

_Ret_range_( 0, 1 ) double CColorSpace::GetColorBrightness( _In_ const COLORREF color ) {
	return ( GetRValue( color ) + GetGValue( color ) + GetBValue( color ) ) / 255.0 / 3.0;
	}

COLORREF CColorSpace::MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) {
	ASSERT( brightness >= 0.0 );
	ASSERT( brightness <= 1.0 );

	DOUBLE dred   = GetRValue( color ) / 255.0;
	DOUBLE dgreen = GetGValue( color ) / 255.0;
	DOUBLE dblue  = GetBValue( color ) / 255.0;

	DOUBLE f = 3.0 * brightness / ( dred + dgreen + dblue );
	dred   *= f;
	dgreen *= f;
	dblue  *= f;

	INT red   = ( INT ) ( dred * 255 );
	INT green = ( INT ) ( dgreen * 255 );
	INT blue  = ( INT ) ( dblue * 255 );
	
	NormalizeColor(red, green, blue);

	return RGB( red, green, blue );
	}

bool CColorSpace::Is256Colors( ) {
	/*
	 Returns true, if the System has 256 Colors or less.
	 In this case options.brightness is ignored (and the slider should be disabled).
	*/
	CClientDC dc( CWnd::GetDesktopWindow( ) );
	return ( dc.GetDeviceCaps( NUMCOLORS ) != -1 );
	}

void CColorSpace::NormalizeColor( _Inout_ _Out_range_(0, 255) INT& red, _Inout_ _Out_range_(0, 255) INT& green, _Inout_ _Out_range_(0, 255) INT& blue ) {
	ASSERT( red + green + blue <= 3 * 255 );

	if ( red > 255 ) {
		DistributeFirst( red, green, blue );
		}
	else if ( green > 255 ) {
		DistributeFirst( green, red, blue );
		}
	else if ( blue > 255 ) {
		DistributeFirst( blue, red, green );
		}
	}

void CColorSpace::DistributeFirst( _Inout_ _Out_range_(0, 255) INT& first, _Inout_ _Out_range_(0, 255) INT& second, _Inout_ _Out_range_(0, 255) INT& third ) {
	INT h = ( first - 255 ) / 2;
	first = 255;
	second += h;
	third += h;

	if ( second > 255 ) {
		INT h2 = second - 255;
		second = 255;
		third += h2;
		}
	else if ( third > 255 ) {
		INT h3 = third - 255;
		third = 255;
		second += h3;
		}
	ASSERT( second <= 255 );
	ASSERT( third <= 255 );
	}

/////////////////////////////////////////////////////////////////////////////

const CTreemap::Options CTreemap::_defaultOptions =    { KDirStatStyle, false, RGB( 0, 0, 0 ), 0.88, 0.38, 0.91, 0.13, -1.0, -1.0 };

const CTreemap::Options CTreemap::_defaultOptionsOld = { KDirStatStyle, false, RGB( 0, 0, 0 ), 0.85, 0.40, 0.90, 0.15, -1.0, -1.0 };

const COLORREF CTreemap::_defaultCushionColors[ ] = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 150, 150, 255 ), RGB( 255, 150, 150 ), RGB( 150, 255, 150 ), RGB( 150, 255, 255 ), RGB( 255, 150, 255 ), RGB( 255, 255, 150 ), RGB( 255, 255, 255 ) };

const COLORREF CTreemap::_defaultCushionColors256[ ] = { RGB( 0, 0, 255 ), RGB( 255, 0, 0 ), RGB( 0, 255, 0 ), RGB( 0, 255, 255 ), RGB( 255, 0, 255 ), RGB( 255, 255, 0 ), RGB( 100, 100, 100 ) };



void CTreemap::GetDefaultPalette(_Inout_ CArray<COLORREF, COLORREF&>& palette)
{
	if ( m_IsSystem256Colors) {
		palette.SetSize( countof( _defaultCushionColors256 ) );
		for ( INT i = 0; i < countof( _defaultCushionColors256 ); i++ ) {
			palette[ i ] = _defaultCushionColors256[ i ];
			}

		// We don't do `EqualizeColors(_defaultCushionColors256, countof(_defaultCushionColors256), palette);` because on 256 color screens, the resulting colors are not distinguishable.
		}
	else {
		EqualizeColors( _defaultCushionColors, countof( _defaultCushionColors ), palette );
		}
}

void CTreemap::EqualizeColors( _In_ const COLORREF *colors, _In_ INT count, _Inout_ CArray<COLORREF, COLORREF&>& out ) {
	out.SetSize( count );
	for ( INT i = 0; i < count; i++ ) {
		out[ i ] = CColorSpace::MakeBrightColor( colors[ i ], PALETTE_BRIGHTNESS );
		}
	}

CTreemap::Options CTreemap::GetDefaultOptions()
{
	return _defaultOptions;
}

CTreemap::Options CTreemap::GetOldDefaultOptions()
{
	return _defaultOptionsOld;
}

CTreemap::CTreemap( Callback *callback ) {
	AfxCheckMemory( );
	m_callback = callback;
	m_IsSystem256Colors = CColorSpace::Is256Colors( );
	SetOptions( &_defaultOptions );
	SetBrightnessFor256( );
	IsCushionShading_current = IsCushionShading( );
	}

void CTreemap::UpdateCushionShading( bool newVal ) { 
	IsCushionShading_current = newVal;
	}

void CTreemap::SetOptions( _In_ const Options *options ) {
	ASSERT( options != NULL );
	m_options = *options;

	// Derive normalized vector here for performance
	const DOUBLE  lx = m_options.lightSourceX;			// negative = left
	const DOUBLE ly = m_options.lightSourceY;			// negative = top
	static const DOUBLE lz = 10;

	const DOUBLE len = sqrt( lx*lx + ly*ly + lz*lz );
	m_Lx = lx / len;
	m_Ly = ly / len;
	m_Lz = lz / len;

	SetBrightnessFor256( );
	}

CTreemap::Options CTreemap::GetOptions( ) {
	return m_options;
	}

void CTreemap::SetBrightnessFor256( ) {
	if ( m_IsSystem256Colors ) {
		m_options.brightness = PALETTE_BRIGHTNESS;
		}
	}

#ifdef _DEBUG
void CTreemap::RecurseCheckTree( _In_ Item *item ) {
 
	item;//do we need???
	if ( item == NULL ) {
		return;
		}

	//TRACE(_T("RecurseCheckTree!\r\n") );
	if ( item->TmiIsLeaf( ) ) {
		ASSERT( item->TmiGetChildrenCount( ) == 0 );
		}
	else {
// ###Todo: check that children are sorted by size.
		//LONGLONG sum = 0;
		//for (INT i=0; i < item->TmiGetChildrenCount(); i++)
		//{
		//	Item *child= item->TmiGetChild(i);
		//	sum+= child->TmiGetSize();
		//	RecurseCheckTree(child);
		//}
		for ( auto i = 0; i < item->TmiGetChildrenCount( ); i++ ) {
			//translate into ranged for?
			auto child = item->TmiGetChild( i );
			
			if ( i > 0 ) {
				auto child_2 = item->TmiGetChild( i - 1 );
				if ( ( child_2 != NULL ) && ( child != NULL ) ) {
					ASSERT( child_2->TmiGetSize( ) >= child->TmiGetSize( ) );
					}
				else {
					ASSERT( false );
					}
				}
			
			//sum += child->TmiGetSize( );
			//ASSERT( sum <= child->TmiGetSize( ) );
			RecurseCheckTree( child );
			}
		//if ( !( sum == item->TmiGetSize( ) ) ) {
			//TRACE( _T( "\tsum: %lld, item->TmiGetSize: %lld\r\n" ), sum, item->TmiGetSize( ) );
			//}
		/*ASSERT(sum == item->TmiGetSize());*/
		//ASSERT( sum <= item->TmiGetSize( ) );
		}

	return;

}

#else

void CTreemap::RecurseCheckTree( _In_ Item *item ) {
	( void ) item;
	CString msg = _T( "RecurseCheckTree was called in the release build! This shouldn't happen!" );
	AfxMessageBox( msg );
	//abort( );
	}

#endif

void CTreemap::DrawTreemap( _In_ CDC *pdc, _In_ CRect& rc, _In_ Item *root, _In_opt_ const Options *options ) {
	//ASSERT_VALID( pdc );//callers have verified.
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
#ifdef _DEBUG
	RecurseCheckTree( root );
#endif
	if ( root == NULL ) {
		//should never happen!
		ASSERT( false );
		}

	if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
		ASSERT( false );
		return;
		}

	if ( options != NULL ) {
		SetOptions( options );
		}

	if ( m_options.grid ) {
		pdc->FillSolidRect( rc, m_options.gridColor );
		}
	else {
		// We shrink the rectangle here, too.
		// If we didn't do this, the layout of the treemap would change, when grid is switched on and off.
		CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
		CSelectObject sopen( pdc, &pen );
		pdc->MoveTo( rc.right - 1, rc.top );
		pdc->LineTo( rc.right - 1, rc.bottom );
		pdc->MoveTo( rc.left,      rc.bottom - 1 );
		pdc->LineTo( rc.right,     rc.bottom - 1 );
		}
	rc.right--;
	rc.bottom--;
	if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
		return;
		}
	if ( root->TmiGetSize( ) > 0 ) {//root can be null on zooming out??
		DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };
		RecurseDrawGraph( pdc, root, rc, true, surface, m_options.height, 0 );
#ifdef STRONGDEBUG	// slow, but finds bugs!
#ifdef _DEBUG
		for (INT x=rc.left; x < rc.right - m_options.grid; x++)
		for (INT y=rc.top; y < rc.bottom - m_options.grid; y++)
			ASSERT(FindItemByPoint(root, CPoint(x, y)) != NULL);
#endif
#endif
		}
	else {
		pdc->FillSolidRect( rc, RGB( 0, 0, 0 ) );
		}
	}

void CTreemap::DrawTreemapDoubleBuffered( _In_ CDC *pdc, _In_ const CRect& rc, _In_ Item *root, _In_opt_ const Options *options ) {
	ASSERT_VALID( pdc );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
	if ( options != NULL ) {
		SetOptions( options );
		}

	if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
		return;
		}

	CDC dc;
	VERIFY( dc.CreateCompatibleDC( pdc ) );

	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( pdc, ( rc.right - rc.left ), ( rc.bottom - rc.top ) ) );

	CSelectObject sobmp( &dc, &bm );

	CRect rect( CPoint( 0, 0 ), rc.Size( ) );

	DrawTreemap( &dc, rect, root, NULL );

	VERIFY( pdc->BitBlt( rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), &dc, 0, 0, SRCCOPY ) );
	}

_Success_(return != NULL) _Must_inspect_result_ CTreemap::Item *CTreemap::FindItemByPoint(_In_ Item *item, _In_ CPoint point) {
	/*
	  In the resulting treemap, find the item below a given coordinate. Return value can be NULL - the only case that this function returns NULL is that point is not inside the rectangle of item.

	  `item` (First parameter) MUST NOT BE NULL! I'm serious.

	  Take notice of
	     (a) the very right an bottom lines, which can be "grid" and are not covered by the root rectangle,
	     (b) the fact, that WM_MOUSEMOVEs can occur after WM_SIZE but before WM_PAINT.
	
	*/

	ASSERT( item != NULL );
	if ( item == NULL ) {
		ASSERT( false );
		return NULL;
		}
	const CRect& rc = item->TmiGetRectangle( );

	if ( !rc.PtInRect( point ) ) {
		return NULL;
		}

	ASSERT( rc.PtInRect( point ) );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );

	Item *ret = NULL;
	auto gridWidth = m_options.grid ? 1 : 0;
	if ( ( rc.right - rc.left ) <= gridWidth || ( rc.bottom - rc.top ) <= gridWidth || item->TmiIsLeaf( ) ) {
		ret = item;
		}
	else {
		ASSERT( item->TmiGetSize( ) > 0 );
		ASSERT( item->TmiGetChildrenCount( ) > 0 );
		Item *child = NULL;
		auto countOfChildren = item->TmiGetChildrenCount( );
		for ( INT i = 0; i < countOfChildren; i++ ) {
			child = item->TmiGetChild( i );
			ASSERT( child->TmiGetSize( ) > 0 );
#ifdef _DEBUG
			CRect rcChild = child->TmiGetRectangle( );
			ASSERT( rcChild.right >= rcChild.left );
			ASSERT( rcChild.bottom >= rcChild.top );
			ASSERT( rcChild.left >= rc.left );
			ASSERT( rcChild.right <= rc.right );
			ASSERT( rcChild.top >= rc.top );
			ASSERT( rcChild.bottom <= rc.bottom );
#endif
			if ( child != NULL ) {
				if ( child->TmiGetRectangle( ).PtInRect( point ) ) {
					ret = FindItemByPoint( child, point );
					ASSERT( ret != NULL );
#ifdef STRONGDEBUG
#ifdef _DEBUG
					for ( i++; i < item->TmiGetChildrenCount( ); i++ ) {
						child = item->TmiGetChild( i );
						if ( child->TmiGetSize( ) == 0 ) {
							break;
							}
						rcChild= child->TmiGetRectangle( );
						if (rcChild.left == -1) {
							ASSERT(rcChild.top == -1);
							ASSERT(rcChild.right == -1);
							ASSERT(rcChild.bottom == -1);
							break;
							}

						ASSERT(rcChild.right >= rcChild.left);
						ASSERT(rcChild.bottom >= rcChild.top);
						ASSERT(rcChild.left >= rc.left);
						ASSERT(rcChild.right <= rc.right);
						ASSERT(rcChild.top >= rc.top);
						ASSERT(rcChild.bottom <= rc.bottom);
						}
#endif
#endif
					break;
					}
				}
			else {
				ASSERT( false );
				}
			}
	}
	ASSERT( ret != NULL );
	if (ret == NULL) {
		ret = item;
		}
	AfxCheckMemory( );
	return ret;
	}

void CTreemap::DrawColorPreview( _In_ CDC *pdc, _In_ const CRect& rc, _In_ COLORREF color, _In_ const Options *options ) {
	ASSERT_VALID( pdc );
	if ( options != NULL ) {
		SetOptions( options );
		}

	DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	AddRidge( rc, surface, m_options.height * m_options.scaleFactor );

	RenderRectangle( pdc, rc, surface, color );
	if ( m_options.grid ) {
		CPen pen( PS_SOLID, 1, m_options.gridColor );
		CSelectObject sopen( pdc, &pen );
		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		pdc->Rectangle( rc );
		}
	}

void CTreemap::RecurseDrawGraph( _In_ CDC *pdc, _In_ Item *item, _In_ const CRect& rc, _In_ const bool asroot, _In_ const DOUBLE* psurface, _In_ const DOUBLE height, _In_ const DWORD flags ) {
	ASSERT_VALID( pdc );
	ASSERT( item->TmiGetSize( ) > 0 );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
	if ( m_callback != NULL ) {
		m_callback->TreemapDrawingCallback( );
		}
	item->TmiSetRectangle( rc );
	auto gridWidth = m_options.grid ? 1 : 0;
	if ( ( rc.right - rc.left )  <= gridWidth || ( rc.bottom - rc.top ) <= gridWidth ) {
		return;
		}
	DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	if ( IsCushionShading_current ) {
		//for ( INT i = 0; i < 4; i++ ) {
		//	surface[ i ] = psurface[ i ];
		//	}
		surface[ 0 ] = psurface[ 0 ];
		surface[ 1 ] = psurface[ 1 ];
		surface[ 2 ] = psurface[ 2 ];
		surface[ 3 ] = psurface[ 3 ];
		if ( !asroot ) {
			AddRidge( rc, surface, height );
			}
		}
	if ( item->TmiIsLeaf( ) ) {
		RenderLeaf( pdc, item, surface );
		}
	else {
		ASSERT( item->TmiGetChildrenCount( ) > 0 );
		ASSERT( item->TmiGetSize( ) > 0 );
		DrawChildren( pdc, item, surface, height, flags );
		}
	}

void CTreemap::DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const DOUBLE* surface, _In_ const DOUBLE height, _In_ const DWORD flags ) {
	/*
	  My first approach was to make this member pure virtual and have three classes derived from CTreemap. The disadvantage is then, that we cannot simply have a member variable of type CTreemap but have to deal with pointers, factory methods and explicit destruction. It's not worth.
	*/
	ASSERT_VALID( pdc );
	switch ( m_options.style )
	{
		case KDirStatStyle:
			KDirStat_DrawChildren( pdc, parent, surface, height, flags );
			break;

		case SequoiaViewStyle:
			SequoiaView_DrawChildren( pdc, parent, surface, height, flags );
			break;

		case SimpleStyle:
			Simple_DrawChildren( pdc, parent, surface, height, flags );
			break;
	}
	}

void CTreemap::KDirStat_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD /*flags*/ ) {
	/*
	  I learned this squarification style from the KDirStat executable. It's the most complex one here but also the clearest, imho.
	*/
	ASSERT_VALID( pdc );
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );

	const CRect& rc = parent->TmiGetRectangle( );

	CArray<DOUBLE, DOUBLE> rows;	// Our rectangle is divided into rows, each of which gets this height (fraction of total height).
	CArray<INT, INT> childrenPerRow;// childrenPerRow[i] = # of children in rows[i]

	CArray<DOUBLE, DOUBLE> childWidth; // Widths of the children (fraction of row width).
	childWidth.SetSize( parent->TmiGetChildrenCount( ) );

	bool horizontalRows = KDirStat_ArrangeChildren( parent, childWidth, rows, childrenPerRow );

	const INT width = horizontalRows ? rc.Width( ) : rc.Height( );
	const INT height = horizontalRows ? rc.Height( ) : rc.Width( );
	ASSERT( width >= 0 );
	ASSERT( height >= 0 );

	INT c = 0;
	auto top = horizontalRows ? rc.top : rc.left;
	auto rowsSize = rows.GetSize( );
	for ( INT row = 0; row < rowsSize; row++ ) {
		ASSERT( rowsSize == rows.GetSize( ) );
		auto fBottom = top + rows[ row ] * height;
		INT bottom = fBottom;
		if ( row == ( rowsSize - 1 ) ) {
			bottom = horizontalRows ? rc.bottom : rc.right;
			}
		auto left = horizontalRows ? rc.left : rc.top;
		for ( INT i = 0; i < childrenPerRow[ row ]; i++, c++ ) {
			Item *child = parent->TmiGetChild( c );
			ASSERT( childWidth[ c ] >= 0 );
			auto fRight = left + childWidth[ c ] * width;
			auto right = ( LONG ) fRight;
			bool lastChild = ( i == childrenPerRow[ row ] - 1 || childWidth[ c + 1 ] == 0 );
			if ( lastChild ) {
				right = horizontalRows ? rc.right : rc.bottom;
				}
			CRect rcChild;
			if ( horizontalRows ) {
				rcChild.left = left;
				rcChild.right = right;
				rcChild.top = top;
				rcChild.bottom = bottom;
				}
			else {
				rcChild.left = top;
				rcChild.right = bottom;
				rcChild.top = left;
				rcChild.bottom = right;
				}
			RecurseDrawGraph(pdc, child, rcChild, false, surface, h * m_options.scaleFactor, 0);
			if ( lastChild ) {
				i++, c++;
				if ( i < childrenPerRow[ row ] ) {
					parent->TmiGetChild( c )->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
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

bool CTreemap::KDirStat_ArrangeChildren( _In_ Item *parent, _Inout_ CArray<DOUBLE, DOUBLE>& childWidth, _Inout_ CArray<DOUBLE, DOUBLE>& rows, _Inout_ CArray<INT, INT>& childrenPerRow ) {
	/*
	  return: whether the rows are horizontal.

	  TODO: so many function calls!
	*/
	ASSERT( !parent->TmiIsLeaf( ) );
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );

	if ( parent->TmiGetSize( ) == 0 ) {
		rows.Add( 1.0 );
		childrenPerRow.Add( parent->TmiGetChildrenCount( ) );
		for ( INT i = 0; i < parent->TmiGetChildrenCount( ); i++ ) {
			childWidth[ i ] = 1.0 / parent->TmiGetChildrenCount( );
			}
		return true;
		}

	bool horizontalRows = ( ( parent->TmiGetRectangle( ).Width( ) ) >= parent->TmiGetRectangle( ).Height( ) );

	DOUBLE width = 1.00;
	if ( horizontalRows ) {
		if ( parent->TmiGetRectangle( ).Height( ) > 0 ) {
			width = ( DOUBLE ) parent->TmiGetRectangle( ).Width( ) / parent->TmiGetRectangle( ).Height( );
			}
		}
	else {
		if ( parent->TmiGetRectangle( ).Width( ) > 0 ) {
			width = ( DOUBLE ) parent->TmiGetRectangle( ).Height( ) / parent->TmiGetRectangle( ).Width( );
			}
		}

	INT nextChild = 0;
	while ( nextChild < parent->TmiGetChildrenCount( ) ) {
		INT childrenUsed = 0;
		rows.Add( KDirStat_CalcutateNextRow( parent, nextChild, width, childrenUsed, childWidth ) );
		childrenPerRow.Add( childrenUsed );
		nextChild += childrenUsed;
		}
	return horizontalRows;
	}

DOUBLE CTreemap::KDirStat_CalcutateNextRow( _In_ Item *parent, _In_ const INT nextChild, _In_ _In_range_(0, 32767) DOUBLE width, _Inout_ INT& childrenUsed, _Inout_ CArray<DOUBLE, DOUBLE>& childWidth ) {
	ASSERT( nextChild >= 0 );
	static const DOUBLE _minProportion = 0.4;
	ASSERT( _minProportion < 1 );

	ASSERT( nextChild < parent->TmiGetChildrenCount( ) );
	ASSERT( width >= 1.0 );

	const DOUBLE mySize = ( DOUBLE ) parent->TmiGetSize( );
	ASSERT( mySize > 0 );
	LONGLONG sizeUsed = 0;
	DOUBLE rowHeight = 0;
	INT i = 0;
	auto parent_tmiGetChildCount = parent->TmiGetChildrenCount( );
	for ( i = nextChild; i < parent_tmiGetChildCount ; ++i ) {
		auto childOfParent = parent->TmiGetChild( i );
		LONGLONG childSize = 0;
		if ( childOfParent != NULL ) {
			childSize = childOfParent->TmiGetSize( );
			}
		else { 
			ASSERT( false );
			}
		if ( childSize == 0 ) {
			ASSERT( i > nextChild );	// first child has size > 0
			break;
			}

		sizeUsed += childSize;
		DOUBLE virtualRowHeight = sizeUsed / mySize;
		if ( ( sizeUsed / mySize ) > 1.00000000 ) {
			TRACE( _T( "sizeUsed(%lld) / mySize(%f) = %f\r\n\tTHAT'S BIGGER THAN 1!\r\n" ), sizeUsed, mySize, virtualRowHeight );
			}
		ASSERT( virtualRowHeight > 0 );
		DOUBLE aChildWidth = childSize / mySize * width / virtualRowHeight;//WTF
		if ( aChildWidth / virtualRowHeight < _minProportion ) {
			ASSERT( i > nextChild ); // because width >= 1 and _minProportion < 1.
			break;
			}
		rowHeight = virtualRowHeight;
		//TRACE( _T( "i: %i, nextChild: %i, childSize: %lld\r\n" ), i, nextChild, childSize );
		}
	ASSERT( i > nextChild );
	// Now i-1 is the last child used and rowHeight is the height of the row.
	// We add the rest of the children, if their size is 0.
	//INT i = 0;
	while ( i < parent->TmiGetChildrenCount( ) && parent->TmiGetChild( i )->TmiGetSize( ) == 0 ) {
		i++;
		}
	childrenUsed = i - nextChild;
		
	// Now as we know the rowHeight, we compute the widths of our children.
	for ( INT j = 0; j < childrenUsed; j++ ) {
		// Rectangle(1.0 * 1.0) = mySize
		DOUBLE rowSize = mySize * rowHeight;
		auto childOfParent = parent->TmiGetChild( nextChild + j );
		if ( childOfParent != NULL ) {
			DOUBLE childSize = ( DOUBLE ) childOfParent->TmiGetSize( );
			DOUBLE cw = childSize / rowSize;
			ASSERT( cw >= 0 );
			childWidth[ nextChild + j ] = cw;
			}
		else {
			ASSERT( false );
			}
		}
	
	return rowHeight;
}

// The classical squarification method.
void CTreemap::SequoiaView_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD /*flags*/ )
{
	// Rest rectangle to fill
	CRect remaining = parent->TmiGetRectangle( );
	ASSERT_VALID( pdc );
	ASSERT( remaining.Width( ) > 0 );
	ASSERT( remaining.Height( ) > 0 );
	ASSERT( ( remaining.right - remaining.left ) == remaining.Width( ) );
	ASSERT( ( remaining.bottom - remaining.top ) == remaining.Height( ) );

	// Size of rest rectangle
	LONGLONG remainingSize = parent->TmiGetSize( );
	ASSERT( remainingSize > 0 );

	// Scale factor
	const DOUBLE sizePerSquarePixel = ( DOUBLE ) parent->TmiGetSize( ) / ( remaining.right - remaining.left ) / ( remaining.bottom - remaining.top );

	// First child for next row
	INT head = 0;

	// At least one child left
	while ( head < parent->TmiGetChildrenCount( ) ) {
		ASSERT( remaining.Width( ) > 0 );
		ASSERT( remaining.Height( ) > 0 );
		ASSERT( ( remaining.right - remaining.left ) == remaining.Width( ) );
		ASSERT( ( remaining.bottom - remaining.top ) == remaining.Height( ) );
		// How we divide the remaining rectangle 
		bool horizontal = ( ( remaining.right - remaining.left ) >= ( remaining.bottom - remaining.top ) );

		// Height of the new row
		const INT height = horizontal ? ( remaining.bottom - remaining.top )  : ( remaining.right - remaining.left );

		// Square of height in size scale for ratio formula
		const DOUBLE hh = ( height * height ) * sizePerSquarePixel;
		ASSERT( hh > 0 );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		INT rowBegin = head;
		INT rowEnd   = head;

		// Worst ratio so far
		DOUBLE worst = DBL_MAX;

		// Maximum size of children in row
		LONGLONG rmax = parent->TmiGetChild( rowBegin )->TmiGetSize( );

		// Sum of sizes of children in row
		LONGLONG sum = 0;

		// This condition will hold at least once.
		while ( rowEnd < parent->TmiGetChildrenCount( ) ) {
			// We check a virtual row made up of child(rowBegin)...child(rowEnd) here.

			// Minimum size of child in virtual row
			LONGLONG rmin = parent->TmiGetChild( rowEnd )->TmiGetSize( );

			// If sizes of the rest of the children is zero, we add all of them
			if ( rmin == 0 ) {
				rowEnd = parent->TmiGetChildrenCount( );
				break;
				}

			// Calculate the worst ratio in virtual row.
			// Formula taken from the "Squarified Treemaps" paper.
			// (http://http://www.win.tue.nl/~vanwijk/)

			const DOUBLE ss = ( ( DOUBLE ) sum + rmin ) * ( ( DOUBLE ) sum + rmin );
			const DOUBLE ratio1 = hh * rmax / ss;
			const DOUBLE ratio2 = ss / hh / rmin;

			const DOUBLE nextWorst = max( ratio1, ratio2 );

			// Will the ratio get worse?
			if ( nextWorst > worst ) {
				// Yes. Don't take the virtual row, but the real row (child(rowBegin)..child(rowEnd - 1)) made so far.
				break;
				}

			// Here we have decided to add child(rowEnd) to the row.
			sum += rmin;
			rowEnd++;
			worst = nextWorst;
			}
		// Row will be made up of child(rowBegin)...child(rowEnd - 1).
		// sum is the size of the row.
		// As the size of parent is greater than zero, the size of the first child must have been greater than zero, too.
		ASSERT( sum > 0 );

		// Width of row
		INT width = (horizontal ? ( remaining.right - remaining.left ) : ( remaining.bottom - remaining.top ) );
		ASSERT( width > 0 );

		if ( sum < remainingSize ) {
			width = ( INT ) ( ( DOUBLE ) sum / remainingSize * width );
			}
		// else: use up the whole width
		// width may be 0 here.

		// Build the rectangles of children.
		CRect rc;
		DOUBLE fBegin;
		if (horizontal) {
			rc.left = remaining.left;
			rc.right = remaining.left + width;
			fBegin = remaining.top;
			}
		else {
			rc.top = remaining.top;
			rc.bottom = remaining.top + width;
			fBegin = remaining.left;
			}

		// Now put the children into their places
		for (INT i = rowBegin; i < rowEnd; i++) {
			INT begin = (INT)fBegin;
			DOUBLE fraction = ( DOUBLE ) ( parent->TmiGetChild( i )->TmiGetSize( ) ) / sum;
			DOUBLE fEnd = fBegin + fraction * height;
			INT end = ( INT ) fEnd;

			bool lastChild = ( i == rowEnd - 1 || parent->TmiGetChild( i + 1 )->TmiGetSize( ) == 0 );

			if (lastChild) {
				// Use up the whole height
				end = ( horizontal ? remaining.top + height : remaining.left + height );
				}
		
			if (horizontal) {
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

			RecurseDrawGraph( pdc, parent->TmiGetChild( i ), rc, false, surface, h * m_options.scaleFactor, 0 );

			if ( lastChild ) {
				break;
				}

			fBegin = fEnd;
		}

		// Put the next row into the rest of the rectangle
		if ( horizontal ) {
			remaining.left += width;
			}
		else {
			remaining.top += width;
			}
		remainingSize -= sum;
		
		ASSERT( remaining.left <= remaining.right );
		ASSERT( remaining.top <= remaining.bottom );
		ASSERT( remainingSize >= 0 );

		head += ( rowEnd - rowBegin );

		if ( ( remaining.right - remaining.left ) <= 0 || ( remaining.bottom - remaining.top ) <= 0) {
			if ( head < parent->TmiGetChildrenCount( ) ) {
				auto childOfParent = parent->TmiGetChild( head );
				if ( childOfParent != NULL ) {
					childOfParent->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
					}
				else {
					ASSERT( false );
					}
				}
			break;
			}
	}
	ASSERT( remainingSize == 0 );
	ASSERT( remaining.left == remaining.right || remaining.top == remaining.bottom );
}

// No squarification. Children are arranged alternately horizontally and vertically.
void CTreemap::Simple_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD flags ) {
	ASSERT_VALID( pdc );
	ASSERT( false ); // Not used in Windirstat.
	pdc;
	parent;
	surface;
	h;
	flags;
	}

bool CTreemap::IsCushionShading( ) const
{
	return m_options.ambientLight < 1.0 && m_options.height > 0.0 && m_options.scaleFactor > 0.0;
}

void CTreemap::RenderLeaf( _In_ CDC *pdc, _In_ Item *item, _In_ const DOUBLE* surface ) {
	CRect rc = item->TmiGetRectangle( );
	if ( m_options.grid ) {
		rc.top++;
		rc.left++;
		if ( ( rc.right = rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
			return;
			}
		}
	RenderRectangle( pdc, rc, surface, item->TmiGetGraphColor( ) );
	}

void CTreemap::RenderRectangle( _In_ CDC *pdc, _In_ const CRect& rc, _In_ const DOUBLE* surface, _In_ DWORD color ) {
	auto brightness = m_options.brightness;
	if ( ( color & COLORFLAG_MASK ) != 0 ) {
		auto flags = ( color & COLORFLAG_MASK );
		color = CColorSpace::MakeBrightColor( color, PALETTE_BRIGHTNESS );
		if ( ( flags & COLORFLAG_DARKER ) != 0 ) {
			brightness *= 0.66;
			}
		else {
			brightness *= 1.2;
			if ( brightness > 1.0 ) {
				brightness = 1.0;
				}
			}
		}
	if ( IsCushionShading_current ) {
		DrawCushion( pdc, rc, surface, color, brightness );
		}
	else {
		DrawSolidRect( pdc, rc, color, brightness );
		}
	}

void CTreemap::DrawSolidRect( _In_ CDC *pdc, _In_ const CRect& rc, _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) {
	INT red   = GetRValue( col );
	INT green = GetGValue( col );
	INT blue  = GetBValue( col );
	
	const DOUBLE factor = brightness / PALETTE_BRIGHTNESS;

	red   = ( INT ) ( red * factor );
	green = ( INT ) ( green * factor );
	blue  = ( INT ) ( blue * factor );

	CColorSpace::NormalizeColor( red, green, blue );

	pdc->FillSolidRect( rc, RGB( red, green, blue ) );
	}

void setPix( CDC* pdc, std::mutex* pixlesMutex, std::mutex* pdcMutex, std::queue<setPixStruct>& pixles, std::condition_variable* isDataReady, std::atomic_bool* isDone) {
	while ( !std::atomic_load( isDone ) ) {
		//if ( std::atomic_load( isDone ) ) {
		//	return;
		//	}
		std::unique_lock<std::mutex> lck( *pixlesMutex );
		if ( std::atomic_load( isDone ) ) {
			return;
			}

		isDataReady->wait( lck );
		//pixlesMutex.lock( );
		//isDataReady.wait( pixlesMutex );
		//TRACE( _T( "Got lock!\r\n" ) );
		//if ( std::atomic_load( isDone ) ) {
		//	return;
		//	}
		//if ( ( pixles.empty( ) || ( pixles.size( ) == 0) ) && std::atomic_load(isDone)) {
		//	lck.unlock( );
		//	lck.release( );
		//	//pixlesMutex.unlock( );
		//	return;
		//	}
		//else if ( ( pixles.empty( ) || ( pixles.size( ) == 0) ) && ( !std::atomic_load(isDone) ) ) {
		//	//goto tryagain;
		//	return;
		//	}
					{
					if ( pixles.size( ) > 0 ) {
						auto aPixle = std::move( pixles.front( ) );
						pixles.pop( );
						//pixlesMutex.unlock( );
						lck.unlock( );
						//std::unique_lock<std::mutex> pdcLck( *pdcMutex );
						pdcMutex->lock( );

						TRACE( _T( "Setting color %i\r\n" ), aPixle.color );
						pdc->SetPixel( aPixle.ix, aPixle.iy, aPixle.color );
						pdcMutex->unlock( );
						//pdcLck.unlock( );
						}
					else {
						return;
						}
					}
	
		}
doneWithLoop:
	//TRACE( _T( "Done with loop!\r\n" ) );
	return;
	}


void CTreemap::DrawCushion( _In_ CDC *pdc, const _In_ CRect& rc, _In_ const DOUBLE* surface, _In_ COLORREF col, _In_ _In_range_(0, 1) DOUBLE brightness ) {
	// Cushion parameters
	const DOUBLE Ia = m_options.ambientLight;

	// Derived parameters
	const DOUBLE Is = 1 - Ia;			// shading

	const DOUBLE colR = GetRValue( col );//THIS does NOT get vectorized!
	const DOUBLE colG = GetGValue( col );//THIS gets vectorized
	const DOUBLE colB = GetBValue( col );//THIS does NOT get vectorized!
	
	//isDone = false;
	//std::vector<std::thread> someThreads;
	//for ( auto i = 0; i < 1; ++i ) {
	//	someThreads.emplace_back( std::thread( setPix, pdc, &pixlesMutex, &pdcMutex, pixles, &isDataReady, &isDone ) );
	//	}

	std::vector<setPixStruct> xPixles;
	xPixles.reserve( ( rc.right - rc.left ) + 1 );
	
	for ( INT iy = rc.top; iy < rc.bottom; iy++ ) {
		xPixles.reserve( ( rc.right - rc.left ) + 1 );
		for ( INT ix = rc.left; ix < rc.right; ix++ ) {
			/*
			  BOTH for initializations get vectorized
			  EVERYTHING until (NOT including) NormalizeColor gets vectorized :)
			  THAT SAID, there are still two branches (iy < rc.botton, ix < rc.right)
			  */
			auto nx = -( 2.00 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
			auto ny = -( 2.00 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );
			auto cosa = ( nx*m_Lx + ny*m_Ly + m_Lz ) / sqrt( nx*nx + ny*ny + 1.0 );

			ASSERT( cosa <= 1.0 );

			auto pixel = Is * cosa;
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

			/*
			  for some reason, issues a "Convert two packed signed doubleword integers from xmm2/mem to two packed double-precision floating-point values in xmm1" (vcvtdq2pd) instruction, an vmulsd ("Multiply the low double-precision floating-point value in xmm3/mem64 by low double precision floating-point value in xmm2") instruction, then a "Convert one double-precision floating-point value from xmm1/m64 to one signed doubleword integer in r32 using truncation" (vcvttsd2si) instruction. Idiotic.

			  INT red   = ( INT ) ( colR * pixel );
			  INT green = ( INT ) ( colG * pixel );
			  INT blue  = ( INT ) ( colB * pixel );
			  */


			// Make color value
			INT red = ( colR * pixel );
			INT green = ( colG * pixel );
			INT blue = ( colB * pixel );
			if ( red >= 256 ) {
				red = 255;
				}
			if ( green >= 256 ) {
				green = 255;
				}
			if ( blue >= 256 ) {
				blue = 255;
				}

			//#pragma omp critical
			CColorSpace::NormalizeColor( red, green, blue );
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

			xPixles.emplace_back( setPixStruct( ix, iy, RGB( red, green, blue ) ) );
			//pdc->SetPixel( ix, iy, RGB( red, green, blue ) );
			}
		for ( INT ix = rc.left; ix < rc.right; ix++ ) {
			setPixStruct& setP = xPixles.at( ix - rc.left );
			pdc->SetPixel( setP.ix, setP.iy, setP.color );//this is the sole remaining bottleneck here. The drawing time is a direct function of the drawing area - i.e. how many times we have to setPixel!
			}
		xPixles.clear( );
		}
	//std::atomic_store_explicit(&isDone, true, std::memory_order::memory_order_release);
	//std::atomic_thread_fence( std::memory_order::memory_order_acq_rel );
	//isDataReady.notify_all( );

	//for ( auto i = 0; i < 1; ++i ) {
	//	//isDataReady.notify_one( );
	//	(someThreads.at( i )).join( );
	//	}
	//isDataReady.notify_one( );
	}

//void CTreemap::stdDrawCushion( _In_ CDC *pdc, const _In_ CRect& rc, _In_ const double *surface, _In_ COLORREF col, _In_ double brightness ) {
//	ASSERT_VALID( pdc );
//	// Cushion parameters
//	const double Ia = m_options.ambientLight;
//
//	// Derived parameters
//	const double Is = 1 - Ia;			// shading
//
//	const double colR = GetRValue( col );//THIS does NOT get vectorized!
//	const double colG = GetGValue( col );//THIS gets vectorized
//	const double colB = GetBValue( col );//THIS does NOT get vectorized!
//	colorMatrix colorMap( rc.Height(), rc.Width());
//
//	//#pragma omp parallel for
//	for ( size_t iy = rc.top; iy < rc.bottom; iy++ ) {
//		for ( size_t ix = rc.left; ix < rc.right; ix++ ) {
//			/*
//			  BOTH for initializations get vectorized
//			  EVERYTHING until (NOT including) NormalizeColor gets vectorized :)
//			  THAT SAID, there are still two branches (iy < rc.botton, ix < rc.right)
//			  */
//			double nx = -( 2 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
//			double ny = -( 2 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );
//			double cosa = ( nx*m_Lx + ny*m_Ly + m_Lz ) / sqrt( nx*nx + ny*ny + 1.0 );
//			if ( cosa > 1.0 ) {
//				cosa = 1.0;
//				}
//
//			double pixel = Is * cosa;
//			if ( pixel < 0 ) {
//				pixel = 0;
//				}
//
//			pixel += Ia;
//			ASSERT( pixel <= 1.0 );
//
//			// Now, pixel is the brightness of the pixel, 0...1.0.
//			// Apply contrast.
//			// Not implemented.
//			// Costs performance and nearly the same effect can be made width the m_options->ambientLight parameter.
//			// pixel= pow(pixel, m_options->contrast);
//			// Apply "brightness"
//			pixel *= brightness / PALETTE_BRIGHTNESS;
//
//			// Make color value
//			INT red = ( INT ) ( colR * pixel );
//			INT green = ( INT ) ( colG * pixel );
//			INT blue = ( INT ) ( colB * pixel );
//			if ( red >= 256 ) {
//				red = 255;
//				}
//			if ( green >= 256 ) {
//				green = 255;
//				}
//			if ( blue >= 256 ) {
//				blue = 255;
//				}
//
//			//#pragma omp critical
//			CColorSpace::NormalizeColor( red, green, blue );
//			if ( red == 0 ) {
//				red++;
//				}
//			if ( green == 0 ) {
//				green++;
//				}
//			if ( blue == 0 ) {
//				blue++;
//				}
//			// ... and set!
//			//#pragma omp critical
//
//			if ( !( iy < colorMap.pixles_y.size( ) ) ) {
//				DebugBreak( );
//				}
//			CString n;
//			n.Format( _T( "%lu, %lu" ), iy, ix );
//			OutputDebugString( (LPCTSTR)n );
//			colorMap.pixles_y[ iy ].pixles_x[ ix ].red = red;
//			colorMap.pixles_y[ iy ].pixles_x[ ix ].green = green;
//			colorMap.pixles_y[ iy ].pixles_x[ ix ].blue = blue;
//
//			//pdc->SetPixel( ix, iy, RGB( red, green, blue ) );
//			}
//		}
//
//	for ( size_t iy = rc.top; iy < rc.bottom; iy++ ) {
//		for ( size_t ix = rc.left; ix < rc.right; ix++ ) {
//			pdc->SetPixel( ix, iy, RGB( ( ( INT ) colorMap.pixles_y[ iy ].pixles_x[ ix ].red ), ( ( INT ) colorMap.pixles_y[ iy ].pixles_x[ ix ].green ), ( ( INT ) colorMap.pixles_y[ iy ].pixles_x[ ix ].blue ) ) );
//			}
//		}
//	}


void CTreemap::AddRidge( _In_ const CRect& rc, _Inout_ DOUBLE* surface, _In_ const DOUBLE h ) {
	auto width  = ( rc.right - rc.left );
	auto height = ( rc.bottom - rc.top );

	ASSERT( width > 0 && height > 0 );

	DOUBLE h4 = 4 * h;

	DOUBLE wf   = h4 / width;
	surface[ 2 ] += wf * ( rc.right + rc.left );
	surface[ 0 ] -= wf;

	DOUBLE hf   = h4 / height;
	surface[ 3 ] += hf * ( rc.bottom + rc.top );
	surface[ 1 ] -= hf;
	}


/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTreemapPreview, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CTreemapPreview::CTreemapPreview( ) {
	AfxCheckMemory( );
	m_root = NULL;
	BuildDemoData();
	}

CTreemapPreview::~CTreemapPreview( ) {
	AfxCheckMemory( );
	if ( m_root != NULL ) {
		delete m_root;
		m_root = NULL;
		}
	AfxCheckMemory( );
	}

void CTreemapPreview::SetOptions(_In_ const CTreemap::Options *options)
{
	m_treemap.SetOptions(options);
	Invalidate();
}

void CTreemapPreview::BuildDemoData()
{
	CTreemap::GetDefaultPalette(m_colors);
	INT col = -1;
	COLORREF color;
	INT i = 0;

	CArray<CItem *, CItem *> c4;
	color = GetNextColor( col );
	for ( i = 0; i < 30; i++ ) {
		c4.Add( new CItem( 1 + 100 * i, color ) );
		}
	CArray<CItem *, CItem *> c0;
	for ( i = 0; i < 8; i++ ) {
		c0.Add( new CItem( 500 + 600 * i, GetNextColor( col ) ) );
		}
	CArray<CItem *, CItem *> c1;
	color= GetNextColor(col);
	for ( i = 0; i < 10; i++ ) {
		c1.Add( new CItem( 1 + 200 * i, color ) );
		}
	c0.Add(new CItem(c1));

	CArray<CItem *, CItem *> c2;
	color = GetNextColor( col );
	for ( i = 0; i < 160; i++ ) {
		c2.Add( new CItem( 1 + i, color ) );
		}

	CArray<CItem *, CItem *> c3;
	c3.Add( new CItem( 10000, GetNextColor( col ) ) );
	c3.Add( new CItem( c4 ) );
	c3.Add( new CItem( c2 ) );
	c3.Add( new CItem( 6000, GetNextColor( col ) ) );
	c3.Add( new CItem( 1500, GetNextColor( col ) ) );

	CArray<CItem *, CItem *> c10;
	c10.Add( new CItem( c0 ) );
	c10.Add( new CItem( c3 ) );;
	
	m_root = new CItem( c10 );
}

COLORREF CTreemapPreview::GetNextColor(_Inout_ INT& i)
{
	i++;
	i %= m_colors.GetSize( );
	return m_colors[ i ];
}

void CTreemapPreview::OnPaint( ) {
	CPaintDC dc( this );
	CRect rc;
	GetClientRect( rc );
	m_treemap.DrawTreemapDoubleBuffered( &dc, rc, m_root );
	}


// $Log$
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
