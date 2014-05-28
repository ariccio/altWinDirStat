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
// The EqualizeColors() method creates a palette with colors
// all having the same brightness of 0.6
// Later in RenderCushion() this number is used again to
// scale the colors.

static const double PALETTE_BRIGHTNESS = 0.6;

bool CTreemap::m_IsSystem256Colors = false;

/////////////////////////////////////////////////////////////////////////////

double CColorSpace::GetColorBrightness( _In_ const COLORREF color )
{
	return ( GetRValue( color ) + GetGValue( color ) + GetBValue( color ) ) / 255.0 / 3.0;
}

COLORREF CColorSpace::MakeBrightColor( _In_ const COLORREF color, _In_ const double brightness )
{
	ASSERT( brightness >= 0.0 );
	ASSERT( brightness <= 1.0 );

	double dred   = GetRValue(color) / 255.0;
	double dgreen = GetGValue(color) / 255.0;
	double dblue  = GetBValue(color) / 255.0;

	double f = 3.0 * brightness / ( dred + dgreen + dblue );
	dred *= f;
	dgreen *= f;
	dblue *= f;

	int red   = ( int ) ( dred * 255 );
	int green = ( int ) ( dgreen * 255 );
	int blue  = ( int ) ( dblue * 255 );
	
	NormalizeColor(red, green, blue);

	return RGB(red, green, blue);
}

bool CColorSpace::Is256Colors()
{
	/*
	 Returns true, if the System has 256 Colors or less.
	 In this case options.brightness is ignored (and the slider should be disabled).
	*/
	CClientDC dc( CWnd::GetDesktopWindow( ) );
	return ( dc.GetDeviceCaps( NUMCOLORS ) != -1 );
}

void CColorSpace::NormalizeColor(_Inout_ INT& red, _Inout_ INT& green, _Inout_ INT& blue)
{
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

void CColorSpace::DistributeFirst(_Inout_ int& first, _Inout_ int& second, _Inout_ int& third)
{
	int h = ( first - 255 ) / 2;
	first = 255;
	second += h;
	third += h;

	if ( second > 255 ) {
		int h2 = second - 255;
		second = 255;
		third += h2;
		}
	else if ( third > 255 ) {
		int h3 = third - 255;
		third = 255;
		second += h3;
		}
	ASSERT( second <= 255 );
	ASSERT( third <= 255 );
}



/////////////////////////////////////////////////////////////////////////////

const CTreemap::Options CTreemap::_defaultOptions = {
	KDirStatStyle,
	false,
	RGB(0,0,0),
	0.88,
	0.38,
	0.91,
	0.13,
	-1.0,
	-1.0
};

const CTreemap::Options CTreemap::_defaultOptionsOld = {
	KDirStatStyle,
	false,
	RGB(0,0,0),
	0.85,
	0.4,
	0.9,
	0.15,
	-1.0,
	-1.0
};

const COLORREF CTreemap::_defaultCushionColors[] = {
	RGB(0, 0, 255),
	RGB(255, 0, 0),
	RGB(0, 255, 0),
	RGB(0, 255, 255),
	RGB(255, 0, 255),
	RGB(255, 255, 0),
	RGB(150, 150, 255),
	RGB(255, 150, 150),
	RGB(150, 255, 150),
	RGB(150, 255, 255),
	RGB(255, 150, 255),
	RGB(255, 255, 150),
	RGB(255, 255, 255)
};

const COLORREF CTreemap::_defaultCushionColors256[] = {
	RGB(0, 0, 255),
	RGB(255, 0, 0),
	RGB(0, 255, 0),
	RGB(0, 255, 255),
	RGB(255, 0, 255),
	RGB(255, 255, 0),
	RGB(100, 100, 100)
};



void CTreemap::GetDefaultPalette(_Inout_ CArray<COLORREF, COLORREF&>& palette)
{
	if ( m_IsSystem256Colors) {
		palette.SetSize( countof( _defaultCushionColors256 ) );
		for ( int i = 0; i < countof( _defaultCushionColors256 ); i++ ) {
			palette[ i ] = _defaultCushionColors256[ i ];
			}

		// We don't do
		//EqualizeColors(_defaultCushionColors256, countof(_defaultCushionColors256), palette);
		// because on 256 color screens, the resulting colors are not distinguishable.
		}
	else {
		EqualizeColors( _defaultCushionColors, countof( _defaultCushionColors ), palette );
		}
}

void CTreemap::EqualizeColors(_In_ const COLORREF *colors, _In_ INT count, _Inout_ CArray<COLORREF, COLORREF&>& out)
{
	out.SetSize(count);

	for (int i=0; i < count; i++) {
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

CTreemap::CTreemap(Callback *callback)
{
	m_callback = callback;
	m_IsSystem256Colors = CColorSpace::Is256Colors( );
	SetOptions( &_defaultOptions );
	SetBrightnessFor256( );
	IsCushionShading_current = IsCushionShading( );

}

void CTreemap::UpdateCushionShading( bool newVal ) { 
	IsCushionShading_current = newVal;
	}

void CTreemap::SetOptions(_In_ const Options *options)
{
	ASSERT( options != NULL );
	m_options = *options;

	// Derive normalized vector here for performance
	const double lx = m_options.lightSourceX;			// negative = left
	const double ly = m_options.lightSourceY;			// negative = top
	static const double lz = 10;

	const double len = sqrt( lx*lx + ly*ly + lz*lz );
	m_Lx = lx / len;
	m_Ly = ly / len;
	m_Lz = lz / len;

	SetBrightnessFor256( );
}

CTreemap::Options CTreemap::GetOptions()
{
	return m_options;
}

void CTreemap::SetBrightnessFor256()
{
	if ( m_IsSystem256Colors ) {
		m_options.brightness = PALETTE_BRIGHTNESS;
		}
}

void CTreemap::RecurseCheckTree(_In_ Item *item)
{
 #ifdef _DEBUG
	item;//do we need???
	if ( item == NULL ) {
		return;
		}

	//TRACE(_T("RecurseCheckTree!\r\n") );
	if (item->TmiIsLeaf())
	{
		ASSERT(item->TmiGetChildrenCount() == 0);
	}
	else
	{
// ###Todo: check that children are sorted by size.
		//LONGLONG sum = 0;
		//for (int i=0; i < item->TmiGetChildrenCount(); i++)
		//{
		//	Item *child= item->TmiGetChild(i);
		//	sum+= child->TmiGetSize();
		//	RecurseCheckTree(child);
		//}
		for ( auto i = 0; i < item->TmiGetChildrenCount( ); i++ ) {
			//translate into ranged for?
			Item *child = item->TmiGetChild( i );
			
			if ( i>0 ) {
				Item *child_2 = item->TmiGetChild( i - 1 );
				ASSERT( child_2->TmiGetSize() >= child->TmiGetSize( ) );
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
#else
	return;
#endif
}

void CTreemap::DrawTreemap(_In_ CDC *pdc, _In_ CRect& rc, _In_ Item *root, _In_opt_ const Options *options)
{
	ASSERT_VALID( pdc );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
#ifdef _DEBUG
	RecurseCheckTree(root);
#endif
	if ( options != NULL ) {
		SetOptions( options );
		}
	if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
#ifdef _DEBUG
		ASSERT( false );
#endif
		return;
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
	if ( root == NULL ) {
	//return;
		}
	if ( root->TmiGetSize( ) > 0 ) {//root can be null on zooming out??
		double surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };
		RecurseDrawGraph( pdc, root, rc, true, surface, m_options.height, 0 );
#ifdef STRONGDEBUG	// slow, but finds bugs!
#ifdef _DEBUG
		for (int x=rc.left; x < rc.right - m_options.grid; x++)
		for (int y=rc.top; y < rc.bottom - m_options.grid; y++)
			ASSERT(FindItemByPoint(root, CPoint(x, y)) != NULL);
#endif
#endif
		}
	else {
		pdc->FillSolidRect( rc, RGB( 0, 0, 0 ) );
		}
}

void CTreemap::DrawTreemapDoubleBuffered(_In_ CDC *pdc, _In_ const CRect& rc, _In_ Item *root, _In_opt_ const Options *options)
{
	ASSERT_VALID( pdc );
	ASSERT( ( rc.right - rc.left ) == rc.Width());
	ASSERT( ( rc.bottom - rc.top ) == rc.Height());
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

CTreemap::Item *CTreemap::FindItemByPoint(_In_ Item *item, _In_ CPoint point)
{
	ASSERT( item != NULL );
	const CRect& rc = item->TmiGetRectangle( );

	if ( !rc.PtInRect( point ) ) {
		// The only case that this function returns NULL is that point is not inside the rectangle of item.
		//
		// Take notice of
		//   (a) the very right an bottom lines, which can be "grid" and are not covered by the root rectangle,
		//   (b) the fact, that WM_MOUSEMOVEs can occur after WM_SIZE but before WM_PAINT.
		return NULL;
		}

	ASSERT( rc.PtInRect( point ) );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );

	Item *ret = NULL;
	int gridWidth = m_options.grid ? 1 : 0;
	if ( ( rc.right - rc.left ) <= gridWidth || ( rc.bottom - rc.top ) <= gridWidth || item->TmiIsLeaf( ) ) {
		ret = item;
		}
	else {
		ASSERT( item->TmiGetSize( ) > 0 );
		ASSERT( item->TmiGetChildrenCount( ) > 0 );
		Item *child = NULL;
		auto countOfChildren = item->TmiGetChildrenCount( );
		for ( int i = 0; i < countOfChildren; i++ ) {
			child = item->TmiGetChild( i );
			ASSERT( child->TmiGetSize( ) > 0 );
#ifdef _DEBUG
			CRect rcChild = child->TmiGetRectangle( );
			ASSERT( rcChild.right >= rcChild.left );
			ASSERT( rcChild.bottom >= rcChild.top );
			ASSERT( rcChild.left >= rc.left );
			ASSERT( rcChild.right <= rc.right );
			ASSERT(rcChild.top >= rc.top);
			ASSERT(rcChild.bottom <= rc.bottom);
#endif
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
	}
	ASSERT( ret != NULL );
	if (ret == NULL) {
		ret = item;
		}
	return ret;
}

void CTreemap::DrawColorPreview(_In_ CDC *pdc, _In_ const CRect& rc, _In_ COLORREF color, _In_ const Options *options)
{
	ASSERT_VALID( pdc );
	if ( options != NULL ) {
		SetOptions( options );
		}

	double surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	AddRidge( rc, surface, m_options.height * m_options.scaleFactor );

	RenderRectangle( pdc, rc, surface, color );
	if ( m_options.grid ) {
		CPen pen( PS_SOLID, 1, m_options.gridColor );
		CSelectObject sopen( pdc, &pen );
		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		pdc->Rectangle( rc );
		}
}

void CTreemap::RecurseDrawGraph( _In_ CDC *pdc, _In_ Item *item, _In_ const CRect& rc, _In_ const bool asroot, _In_ const double *psurface, _In_ const double h, _In_ const DWORD flags )
{
	ASSERT_VALID( pdc );
	/*
	//short-circuit for rects of size zero?
	//if ( ( rc.Width( ) != 0 ) && ( rc.Height( ) != 0 ) ) {
	//	return;
	//	}
	//testing if NOT EQUAL to zero avoids need to normalize
	//ASSERT(rc.Width() != 0);
	//ASSERT(rc.Height() != 0);
	//ASSERT( ( rc.Width( ) != 0 ) || ( rc.Height( ) != 0) );
	*/
	ASSERT( item->TmiGetSize( ) > 0 );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
	if ( m_callback != NULL ) {
		m_callback->TreemapDrawingCallback( );
		}
	item->TmiSetRectangle( rc );
	int gridWidth = m_options.grid ? 1 : 0;
	if ( ( rc.right - rc.left )  <= gridWidth || ( rc.bottom - rc.top ) <= gridWidth ) {
		return;
		}
	double surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	if ( IsCushionShading_current ) {
		for ( int i = 0; i < 4; i++ ) {
			surface[ i ] = psurface[ i ];
			}
		if ( !asroot ) {
			AddRidge( rc, surface, h );
			}
		}
	if ( item->TmiIsLeaf( ) ) {
		RenderLeaf( pdc, item, surface );
		}
	else {
		ASSERT( item->TmiGetChildrenCount( ) > 0 );
		ASSERT( item->TmiGetSize( ) > 0 );
		DrawChildren( pdc, item, surface, h, flags );
		}
}


void CTreemap::DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const double *surface, _In_ const double h, _In_ const DWORD flags )
{
	/*
	  My first approach was to make this member pure virtual and have three classes derived from CTreemap. The disadvantage is then, that we cannot simply have a member variable of type CTreemap but have to deal with pointers, factory methods and explicit destruction. It's not worth.
	*/
	ASSERT_VALID( pdc );
	switch ( m_options.style )
	{
		case KDirStatStyle:
			KDirStat_DrawChildren( pdc, parent, surface, h, flags );
			break;

		case SequoiaViewStyle:
			SequoiaView_DrawChildren( pdc, parent, surface, h, flags );
			break;

		case SimpleStyle:
			Simple_DrawChildren( pdc, parent, surface, h, flags );
			break;
	}
}


void CTreemap::KDirStat_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const double *surface, _In_ const double h, _In_ const DWORD /*flags*/ )
{
	/*
	  I learned this squarification style from the KDirStat executable. It's the most complex one here but also the clearest, imho.
	*/
	ASSERT_VALID( pdc );
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );

	const CRect& rc = parent->TmiGetRectangle( );

	CArray<double, double> rows;	// Our rectangle is divided into rows, each of which gets this height (fraction of total height).
	CArray<int, int> childrenPerRow;// childrenPerRow[i] = # of children in rows[i]

	CArray<double, double> childWidth; // Widths of the children (fraction of row width).
	childWidth.SetSize( parent->TmiGetChildrenCount( ) );

	bool horizontalRows = KDirStat_ArrangeChildren( parent, childWidth, rows, childrenPerRow );

	const int width = horizontalRows ? rc.Width( ) : rc.Height( );
	const int height = horizontalRows ? rc.Height( ) : rc.Width( );
	ASSERT( width >= 0 );
	ASSERT( height >= 0 );

	int c = 0;
	double top = horizontalRows ? rc.top : rc.left;
	for ( int row = 0; row < rows.GetSize( ); row++ ) {
		double fBottom = top + rows[ row ] * height;
		int bottom = ( int ) fBottom;
		if ( row == rows.GetSize( ) - 1 ) {
			bottom = horizontalRows ? rc.bottom : rc.right;
			}
		double left = horizontalRows ? rc.left : rc.top;
		for ( int i = 0; i < childrenPerRow[ row ]; i++, c++ ) {
			Item *child = parent->TmiGetChild( c );
			ASSERT( childWidth[ c ] >= 0 );
			double fRight = left + childWidth[ c ] * width;
			int right = ( int ) fRight;
			bool lastChild = ( i == childrenPerRow[ row ] - 1 || childWidth[ c + 1 ] == 0 );
			if ( lastChild ) {
				right = horizontalRows ? rc.right : rc.bottom;
				}
			CRect rcChild;
			if ( horizontalRows ) {
				rcChild.left = ( int ) left;
				rcChild.right = right;
				rcChild.top = ( int ) top;
				rcChild.bottom = bottom;
				}
			else {
				rcChild.left = ( int ) top;
				rcChild.right = bottom;
				rcChild.top = ( int ) left;
				rcChild.bottom = right;
				}
			#ifdef _DEBUG
			if ( rcChild.Width( ) > 0 && rcChild.Height( ) > 0 ) {
				}
			#endif			
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



bool CTreemap::KDirStat_ArrangeChildren( _In_ Item *parent, _Inout_ CArray<double, double>& childWidth, _Inout_ CArray<double, double>& rows, _Inout_ CArray<int, int>& childrenPerRow )
{
	/*
	  return: whether the rows are horizontal.
	*/
	ASSERT( !parent->TmiIsLeaf( ) );
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );

	if ( parent->TmiGetSize( ) == 0 ) {
		rows.Add( 1.0 );
		childrenPerRow.Add( parent->TmiGetChildrenCount( ) );
		for ( int i = 0; i < parent->TmiGetChildrenCount( ); i++ ) {
			childWidth[ i ] = 1.0 / parent->TmiGetChildrenCount( );
			}
		return true;
		}

	bool horizontalRows = ( parent->TmiGetRectangle( ).Width( ) >= parent->TmiGetRectangle( ).Height( ) );

	double width = 1.0;
	if ( horizontalRows ) {
		if ( parent->TmiGetRectangle( ).Height( ) > 0 ) {
			width = ( double ) parent->TmiGetRectangle( ).Width( ) / parent->TmiGetRectangle( ).Height( );
			}
		}
	else {
		if ( parent->TmiGetRectangle( ).Width( ) > 0 ) {
			width = ( double ) parent->TmiGetRectangle( ).Height( ) / parent->TmiGetRectangle( ).Width( );
			}
		}

	int nextChild = 0;
	while ( nextChild < parent->TmiGetChildrenCount( ) ) {
		int childrenUsed = 0;//WAS UNINITIALIZED
		rows.Add( KDirStat_CalcutateNextRow( parent, nextChild, width, childrenUsed, childWidth ) );
		childrenPerRow.Add( childrenUsed );
		nextChild += childrenUsed;
	}
	return horizontalRows;
}

double CTreemap::KDirStat_CalcutateNextRow( _In_ Item *parent, _In_ const INT nextChild, _In_ double width, _Inout_ INT& childrenUsed, _Inout_ CArray<double, double>& childWidth ) {
	ASSERT( nextChild >= 0 );
	static const double _minProportion = 0.4;
	ASSERT( _minProportion < 1 );

	ASSERT( nextChild < parent->TmiGetChildrenCount( ) );
	ASSERT( width >= 1.0 );

	const double mySize = ( double ) parent->TmiGetSize( );
	ASSERT( mySize > 0 );
	LONGLONG sizeUsed = 0;
	double rowHeight = 0;
	int i = 0;
	auto parent_tmiGetChildCount = parent->TmiGetChildrenCount( );
	for ( i = nextChild; i < parent_tmiGetChildCount ; ++i ) {
		LONGLONG childSize = parent->TmiGetChild( i )->TmiGetSize( );
		if ( childSize == 0 ) {
			ASSERT( i > nextChild );	// first child has size > 0
			break;
			}

		sizeUsed += childSize;
		double virtualRowHeight = sizeUsed / mySize;
		if ( ( sizeUsed / mySize ) > 1.00000000 ) {
			TRACE( _T( "sizeUsed(%lld) / mySize(%f) = %f\r\n\tTHAT'S BIGGER THAN 1!\r\n" ), sizeUsed, mySize, virtualRowHeight );
			}
		ASSERT( virtualRowHeight > 0 );
		double aChildWidth = childSize / mySize * width / virtualRowHeight;//WTF
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
	//int i = 0;
	while ( i < parent->TmiGetChildrenCount( ) && parent->TmiGetChild( i )->TmiGetSize( ) == 0 ) {
		i++;
		}
	childrenUsed = i - nextChild;
		
	// Now as we know the rowHeight, we compute the widths of our children.
	for (int j = 0; j < childrenUsed; j++ ) {
		// Rectangle(1.0 * 1.0) = mySize
		double rowSize = mySize * rowHeight;
		double childSize = ( double ) parent->TmiGetChild( nextChild + j )->TmiGetSize( );
		double cw = childSize / rowSize;
		ASSERT( cw >= 0 );
		childWidth[ nextChild + j ] = cw;
		}
	
	return rowHeight;
}


// The classical squarification method.
//
void CTreemap::SequoiaView_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const double *surface, _In_ const double h, _In_ const DWORD /*flags*/ )
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
	const double sizePerSquarePixel = ( double ) parent->TmiGetSize( ) / ( remaining.right - remaining.left ) / ( remaining.bottom - remaining.top );

	// First child for next row
	int head = 0;

	// At least one child left
	while ( head < parent->TmiGetChildrenCount( ) ) {
		ASSERT( remaining.Width( ) > 0 );
		ASSERT( remaining.Height( ) > 0 );
		ASSERT( ( remaining.right - remaining.left ) == remaining.Width( ) );
		ASSERT( ( remaining.bottom - remaining.top ) == remaining.Height( ) );
		// How we divide the remaining rectangle 
		bool horizontal = ( ( remaining.right - remaining.left ) >= ( remaining.bottom - remaining.top ) );

		// Height of the new row
		const int height = horizontal ? ( remaining.bottom - remaining.top )  : ( remaining.right - remaining.left );

		// Square of height in size scale for ratio formula
		const double hh = ( height * height ) * sizePerSquarePixel;
		ASSERT( hh > 0 );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		int rowBegin = head;
		int rowEnd   = head;

		// Worst ratio so far
		double worst = DBL_MAX;

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

			const double ss = ( ( double ) sum + rmin ) * ( ( double ) sum + rmin );
			const double ratio1 = hh * rmax / ss;
			const double ratio2 = ss / hh / rmin;

			const double nextWorst = max(ratio1, ratio2);

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
		int width = (horizontal ? ( remaining.right - remaining.left ) : ( remaining.bottom - remaining.top ) );
		ASSERT( width > 0 );

		if ( sum < remainingSize ) {
			width = ( int ) ( ( double ) sum / remainingSize * width );
			}
		// else: use up the whole width
		// width may be 0 here.

		// Build the rectangles of children.
		CRect rc;
		double fBegin;
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
		for (int i = rowBegin; i < rowEnd; i++) {
			int begin = (int)fBegin;
			double fraction = ( double ) ( parent->TmiGetChild( i )->TmiGetSize( ) ) / sum;
			double fEnd = fBegin + fraction * height;
			int end = ( int ) fEnd;

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
				parent->TmiGetChild( head )->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );
				}
			break;
			}
	}
	ASSERT( remainingSize == 0 );
	ASSERT( remaining.left == remaining.right || remaining.top == remaining.bottom );
}


// No squarification. Children are arranged alternately horizontally and vertically.
//
void CTreemap::Simple_DrawChildren( _In_ CDC *pdc, _In_ Item *parent, _In_ const double *surface, _In_ const double h, _In_ const DWORD flags )
{
#if 1
	ASSERT_VALID( pdc );
	ASSERT( false ); // Not used in Windirstat.
	pdc;
	parent;
	surface;
	h;
	flags;

#else
	ASSERT(parent->TmiGetChildrenCount() > 0);
	ASSERT(parent->TmiGetSize() > 0);

	const CRect& rc= parent->TmiGetRectangle();

	bool horizontal = (flags == 0);

	int width = horizontal ? rc.Width() : rc.Height();
	ASSERT(width >= 0);

	double fBegin = horizontal ? rc.left : rc.top;
	int veryEnd = horizontal ? rc.right : rc.bottom;

	for (auto i=0; i < parent->TmiGetChildrenCount(); i++)
	{
		double fraction = (double)(parent->TmiGetChild(i)->TmiGetSize()) / parent->TmiGetSize();

		double fEnd = fBegin + fraction * width;

		bool lastChild = (i == parent->TmiGetChildrenCount() - 1 || parent->TmiGetChild(i + 1)->TmiGetSize() == 0);

		if (lastChild)
			fEnd = veryEnd;

		int begin= (int)fBegin;
		int end= (int)fEnd;

		ASSERT(begin <= end);
		ASSERT(end <= veryEnd);

		CRect rcChild;
		if (horizontal)
		{
			rcChild.left= begin;
			rcChild.right= end;
			rcChild.top= rc.top;
			rcChild.bottom= rc.bottom;
		}
		else
		{
			rcChild.top= begin;
			rcChild.bottom= end;
			rcChild.left= rc.left;
			rcChild.right= rc.right;
		}

		RecurseDrawGraph(
			pdc, 
			parent->TmiGetChild(i), 
			rcChild,
			false,
			surface,
			h * m_options.scaleFactor,
			flags == 0 ? 1 : 0
		);

		if (lastChild)
		{
			i++;
			break;
		}

		fBegin= fEnd;
	}
	if (i < parent->TmiGetChildrenCount())
		parent->TmiGetChild(i)->TmiSetRectangle(CRect(-1, -1, -1, -1));
#endif
}

bool CTreemap::IsCushionShading( ) const
{
	return m_options.ambientLight < 1.0 && m_options.height > 0.0 && m_options.scaleFactor > 0.0;
}

void CTreemap::RenderLeaf(_In_ CDC *pdc, _In_ Item *item, _In_ const double *surface)
{
	ASSERT_VALID( pdc );
	CRect rc = item->TmiGetRectangle( );

	if ( m_options.grid ) {
		rc.top++;
		rc.left++;
		if ( rc.Width( ) <= 0 || rc.Height( ) <= 0 ) {
			return;
			}
		}

	RenderRectangle( pdc, rc, surface, item->TmiGetGraphColor( ) );
}

void CTreemap::RenderRectangle(_In_ CDC *pdc, _In_ const CRect& rc, _In_ const double *surface, _In_ DWORD color)
{
	ASSERT_VALID( pdc );
	double brightness = m_options.brightness;

	if ( ( color & COLORFLAG_MASK ) != 0 ) {
		DWORD flags = ( color & COLORFLAG_MASK );
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

void CTreemap::DrawSolidRect( _In_ CDC *pdc, _In_ const CRect& rc, _In_ const COLORREF col, _In_ const double brightness )
{
	ASSERT_VALID( pdc );
	int red   = GetRValue( col );
	int green = GetGValue( col );
	int blue  = GetBValue( col );
	
	const double factor = brightness / PALETTE_BRIGHTNESS;

	red   = ( int ) ( red * factor );
	green = ( int ) ( green * factor );
	blue  = ( int ) ( blue * factor );

	CColorSpace::NormalizeColor( red, green, blue );

	pdc->FillSolidRect(rc, RGB(red, green, blue));
}

void CTreemap::DrawCushion(_In_ CDC *pdc, const _In_ CRect& rc, _In_ const double *surface, _In_ COLORREF col, _In_ double brightness)
{
	ASSERT_VALID( pdc );
	// Cushion parameters
	const double Ia = m_options.ambientLight;

	// Derived parameters
	const double Is = 1 - Ia;			// shading

	const double colR = GetRValue( col );//THIS does NOT get vectorized!
	const double colG = GetGValue( col );//THIS gets vectorized
	const double colB = GetBValue( col );//THIS does NOT get vectorized!

	for ( int iy = rc.top; iy < rc.bottom; iy++ )
	for ( int ix = rc.left; ix < rc.right; ix++ )
	{
		/*
		  BOTH for initializations get vectorized
		  EVERYTHING until (NOT including) NormalizeColor gets vectorized :)
		  THAT SAID, there are still two branches (iy < rc.botton, ix < rc.right)
		*/
		double nx = -( 2 * surface[ 0 ] * ( ix + 0.5 ) + surface[ 2 ] );
		double ny = -( 2 * surface[ 1 ] * ( iy + 0.5 ) + surface[ 3 ] );
		double cosa = ( nx*m_Lx + ny*m_Ly + m_Lz ) / sqrt( nx*nx + ny*ny + 1.0 );
		if ( cosa > 1.0 ) {
			cosa = 1.0;
			}
		
		double pixel= Is * cosa;
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
		int red   = ( int ) ( colR * pixel );
		int green = ( int ) ( colG * pixel );
		int blue  = ( int ) ( colB * pixel );
		if ( red >= 256 ) {
			red = 255;
			}
		if ( green >= 256 ) {
			green = 255;
			}
		if ( blue >= 256 ) {
			blue = 255;
			}

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
		pdc->SetPixel( ix, iy, RGB( red, green, blue ) );
	}
}

void CTreemap::AddRidge(_In_ const CRect& rc, _Inout_ double *surface, _In_ double h)
{

	int width  = rc.Width( );
	int height = rc.Height( );

	ASSERT( width > 0 && height > 0 );

	double h4 = 4 * h;

	double wf   = h4 / width;
	surface[ 2 ] += wf * ( rc.right + rc.left );
	surface[ 0 ] -= wf;

	double hf   = h4 / height;
	surface[ 3 ] += hf * ( rc.bottom + rc.top );
	surface[ 1 ] -= hf;
}


/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTreemapPreview, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CTreemapPreview::CTreemapPreview()
{
	m_root= NULL;
	BuildDemoData();
}

CTreemapPreview::~CTreemapPreview()
{
	delete m_root;
}

void CTreemapPreview::SetOptions(_In_ const CTreemap::Options *options)
{
	m_treemap.SetOptions(options);
	Invalidate();
}

void CTreemapPreview::BuildDemoData()
{
	CTreemap::GetDefaultPalette(m_colors);
	int col = -1;
	COLORREF color;
	int i = 0;

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
	color= GetNextColor(col);
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

void CTreemapPreview::OnPaint()
{
	CPaintDC dc( this );
	CRect rc;
	GetClientRect( rc );
	m_treemap.DrawTreemapDoubleBuffered( &dc, rc, m_root );
}


// $Log$
// Revision 1.6  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
