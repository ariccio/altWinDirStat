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
//#include <afxwin.h>
//#include <stdio.h>
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
	ASSERT( RGB( red, green, blue ) != 0 );
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

std::vector<COLORREF> CTreemap::GetDefaultPaletteAsVector( ) {
	std::vector<COLORREF> colorVector;
	colorVector.reserve( defaultColorVec.size() + 1 );
	for ( std::vector<COLORREF>::size_type i = 0; i < defaultColorVec.size(); ++i ) {
		colorVector.emplace_back( CColorSpace::MakeBrightColor( defaultColorVec.at( i ), PALETTE_BRIGHTNESS ) );
		}
	return std::move( colorVector );
	}

//void CTreemap::EqualizeColors( _In_ _In_reads_( count ) const COLORREF* colors, _In_ const INT count, _Inout_ CArray<COLORREF, COLORREF&>& out ) {
//	out.SetSize( count );
//	for ( INT i = 0; i < count; i++ ) {
//		out[ i ] = CColorSpace::MakeBrightColor( colors[ i ], PALETTE_BRIGHTNESS );
//		}
//	}

CTreemap::Options CTreemap::GetDefaultOptions( ) {
	return _defaultOptions;
	}

CTreemap::Options CTreemap::GetOldDefaultOptions( ) {
	return _defaultOptionsOld;
	}

CTreemap::CTreemap( Callback *callback ) {
	m_callback = callback;
	m_IsSystem256Colors = CColorSpace::Is256Colors( );
	SetOptions( &_defaultOptions );
	SetBrightnessFor256( );
	IsCushionShading_current = IsCushionShading( );
#ifdef GRAPH_LAYOUT_DEBUG
	//std::vector<std::vector<bool>> x(3000, std::vector<bool>(3000, false))
	bitSetMask = std::make_unique<std::vector<std::vector<bool>>>( 3000, std::vector<bool>( 3000, false ) );//what a mouthful
	numCalls = 0;
#endif
	}

void CTreemap::UpdateCushionShading( _In_ const bool newVal ) { 
	IsCushionShading_current = newVal;
	}

void CTreemap::SetOptions( _In_ const Options* options ) {
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
		//item doesn't have children, nothing to check
		ASSERT( item->TmiGetChildrenCount( ) == 0 );
		}
	else {
		validateRectangle( item, item->TmiGetRectangle( ) );
		for ( auto i = 0; i < item->TmiGetChildrenCount( ); i++ ) {
			//translate into ranged for?
			auto child = item->TmiGetChild( i );
			//validateRectangle( child, item->TmiGetRectangle( ) );
			if ( i > 0 ) {
				auto child_2 = item->TmiGetChild( i - 1 );
				if ( ( child_2 != NULL ) && ( child != NULL ) ) {
					//TRACE( _T( "child 2: %lld, child 1: %lld\r\n" ), child_2->TmiGetSize( ), child->TmiGetSize( ) );
					//ASSERT( ( child_2->TmiGetSize( ) >= child->TmiGetSize( ) ) );
					}
				else {
					AfxCheckMemory( );
					ASSERT( false );
					}
				}
			
			RecurseCheckTree( child );
			}
		}
	return;
}

#else

void CTreemap::RecurseCheckTree( _In_ Item* item ) {
	( void ) item;
	CString msg = _T( "RecurseCheckTree was called in the release build! This shouldn't happen!" );
	AfxMessageBox( msg );
	//abort( );
	}

#endif

void CTreemap::compensateForGrid( _Inout_ CRect& rc, _In_ CDC* pdc ) {
	if ( m_options.grid ) {
		rc.NormalizeRect( );
		pdc->FillSolidRect( rc, m_options.gridColor );
		}
	else {
		// We shrink the rectangle here, too. If we didn't do this, the layout of the treemap would change, when grid is switched on and off.
		CPen pen { PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) };
		CSelectObject sopen { pdc, &pen };
		pdc->MoveTo( rc.right - 1, rc.top );
		pdc->LineTo( rc.right - 1, rc.bottom );
		pdc->MoveTo( rc.left,      rc.bottom - 1 );
		pdc->LineTo( rc.right,     rc.bottom - 1 );
		}
	rc.right--;
	rc.bottom--;

	}

void CTreemap::DrawTreemap( _In_ CDC* pdc, _In_ CRect& rc, _In_ Item* root, _In_opt_ const Options* options ) {
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );
	if ( root == NULL ) {//should never happen! Ever!
		ASSERT( root != NULL );
		}

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
		ASSERT( false );
		return;
		}

	if ( options != NULL ) {
		SetOptions( options );
		}

	compensateForGrid( rc, pdc );

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
		ASSERT( false );
		return;
		}
	if ( root->TmiGetSize( ) > 0 ) {//root can be null on zooming out??
		DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };
		rc.NormalizeRect( );
		RecurseDrawGraph( pdc, root, rc, true, surface, m_options.height, 0 );
		}
	else {
		rc.NormalizeRect( );
		pdc->FillSolidRect( rc, RGB( 0, 0, 0 ) );
		}
	validateRectangle( root, root->TmiGetRectangle( ) );
	}

void CTreemap::DrawTreemapDoubleBuffered( _In_ CDC *pdc, _In_ const CRect& rc, _In_ Item *root, _In_opt_ const Options *options ) {
	ASSERT_VALID( pdc );
	ASSERT( ( rc.right - rc.left ) == rc.Width( ) );
	ASSERT( ( rc.bottom - rc.top ) == rc.Height( ) );
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );
	if ( options != NULL ) {
		SetOptions( options );
		}

	if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
		return;
		}

	CDC dc;
	VERIFY( dc.CreateCompatibleDC( pdc ) );

	CBitmap bm;
	VERIFY( bm.CreateCompatibleBitmap( pdc, ( rc.Width( ) ), ( rc.Height( ) ) ) );

	CSelectObject sobmp { &dc, &bm };

	CRect rect{ CPoint( 0, 0 ), rc.Size( ) };

	DrawTreemap( &dc, rect, root, NULL );

	VERIFY( pdc->BitBlt( rc.left, rc.top, ( rc.Width( ) ), ( rc.Height( ) ), &dc, 0, 0, SRCCOPY ) );
	}

void CTreemap::validateRectangle( _In_ const Item* child, _In_ const CRect& rc ) const {
#ifdef _DEBUG
	ASSERT( child->TmiGetSize( ) >= 0 );//really should be greater than ( not greater than or equal ) as we shouldn't be drawing zero-size rectangles!
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
	ASSERT( ( rcChild.left + 1 ) >=   rc.left );
	ASSERT( ( rc.right + 1 )     >=   rc.left );
	ASSERT(   rc.bottom          >=   rc.top );
	ASSERT(   rcChild.top        >=   rc.top );

	ASSERT(   rcChild.right      <= ( rc.right + 1 ) );
	ASSERT(   rcChild.bottom     <=   rc.bottom );
	
	rcChild.NormalizeRect( );
	ASSERT(   rcChild.Width( )   < 32767 );
	ASSERT(   rcChild.Height( )  < 32767 );
#else
	UNREFERENCED_PARAMETER( child );
	UNREFERENCED_PARAMETER( rc );
#endif
	}

_Success_( return != NULL ) _Must_inspect_result_ CTreemap::Item *CTreemap::FindItemByPoint( _In_ Item* item, _In_ const CPoint point ) {
	/*
	  In the resulting treemap, find the item below a given coordinate. Return value can be NULL - the only case that this function returns NULL is that point is not inside the rectangle of item.

	  `item` (First parameter) MUST NOT BE NULL! I'm serious.

	  Take notice of
	     (a) the very right an bottom lines, which can be "grid" and are not covered by the root rectangle,
	     (b) the fact, that WM_MOUSEMOVEs can occur after WM_SIZE but before WM_PAINT.
	
	*/
	//CRect& rc_a = item->TmiGetRectangle( );
	auto rc = item->TmiGetRectangle( );
	rc.NormalizeRect( );

	if ( !rc.PtInRect( point ) ) {
		return NULL;
		}

	ASSERT( rc.PtInRect( point ) );

	auto gridWidth = m_options.grid ? 1 : 0;
	
	if ( ( ( rc.Width( ) ) <= gridWidth ) || ( ( rc.Height( ) ) <= gridWidth ) || item->TmiIsLeaf( ) ) {
		return item;
		}
	ASSERT( item->TmiGetSize( ) > 0 );
	ASSERT( item->TmiGetChildrenCount( ) > 0 );

	auto countOfChildren = item->TmiGetChildrenCount( );
	for ( INT i = 0; i < countOfChildren; i++ ) {
		auto child = item->TmiGetChild( i );
		if ( child != NULL ) {
			if ( child->TmiGetRectangle( ).PtInRect( point ) ) {
				validateRectangle( child, rc );
				auto ret = FindItemByPoint( child, point );
				if ( ret != NULL ) {
					validateRectangle( ret, rc );
					return ret;
					}
				}
			}
		}
	return item;
	}

void CTreemap::DrawColorPreview( _In_ CDC* pdc, _In_ const CRect& rc, _In_ const COLORREF color, _In_ const Options* options ) {
	ASSERT_VALID( pdc );
	if ( options != NULL ) {
		SetOptions( options );
		}

	DOUBLE surface[ 4 ] = { 0.00, 0.00, 0.00, 0.00 };

	AddRidge( rc, surface, m_options.height * m_options.scaleFactor );

	RenderRectangle( pdc, rc, surface, color );
	if ( m_options.grid ) {
		CPen pen { PS_SOLID, 1, m_options.gridColor };
		CSelectObject sopen{ pdc, &pen };
		CSelectStockObject sobrush { pdc, NULL_BRUSH };
		pdc->Rectangle( rc );
		}
	}

void CTreemap::RecurseDrawGraph( _In_ CDC* pdc, _In_ Item* item, _In_ const CRect& rc, _In_ const bool asroot, _In_ _In_reads_( 4 ) const DOUBLE* psurface, _In_ const DOUBLE height, _In_ const DWORD flags ) {
	ASSERT_VALID( pdc );
	ASSERT( item != NULL );
	if ( item->TmiIsLeaf( ) ) {
		ASSERT( item->TmiGetSize( ) > 0 );
		}
#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( " RecurseDrawGraph working on rect left: %li, right: %li, top: %li, bottom: %li, isroot: %i\r\n" ), rc.left, rc.right, rc.top, rc.bottom, ( asroot ? 1 : 0 ) );
#endif

	if ( m_callback != NULL ) {
		m_callback->TreemapDrawingCallback( );
		}
	item->TmiSetRectangle( rc );
	validateRectangle( item, rc );
	auto gridWidth = m_options.grid ? 1 : 0;
	auto childCount = item->TmiGetChildrenCount( );
	if ( ( ( ( rc.Width( ) ) <= gridWidth ) || ( ( rc.Height( ) ) <= gridWidth ) ) && ( childCount == 0 ) ) {
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
	if ( item->TmiIsLeaf( ) ) {
		RenderLeaf( pdc, item, surface );
		}
	else {
		ASSERT( item->TmiGetChildrenCount( ) > 0 );
		ASSERT( item->TmiGetSize( ) > 0 );
		DrawChildren( pdc, item, surface, height, flags );
		}
	validateRectangle( item, rc );
	}

void CTreemap::DrawChildren( _In_ CDC* pdc, _In_ Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE height, _In_ const DWORD flags ) {
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

CRect CTreemap::KDirStat_buildrcChildVerticalOrHorizontalRow( _In_ const bool horizontalRows, _In_ _In_range_( 0, 32767 ) const LONG left, _In_ _In_range_( 0, 32767 ) const LONG right, _In_ _In_range_( 0, 32767 ) const LONG top, _In_ _In_range_( 0, 32767 ) const LONG bottom ) {
	ASSERT( ( right + 1 ) >= left );
	ASSERT( bottom >= top );
	CRect rcChild;
	if ( horizontalRows ) {
		rcChild.left   = left;
		rcChild.right  = right;
		rcChild.top    = top;
		rcChild.bottom = bottom;
		}
	else {
		rcChild.left   = top;
		rcChild.top    = left;
		rcChild.right  = bottom;
		rcChild.bottom = right;
		}
	ASSERT( ( rcChild.right + 1 ) >= rcChild.left );
	ASSERT( rcChild.bottom >= rcChild.top );
	return std::move( rcChild );
	}

void CTreemap::KDirStat_DrawChildrenInThisRow( _In_ const std::vector<INT_PTR>& childrenPerRow, _Inout_ INT_PTR& c, _In_ const Item* parent, _Inout_ LONG& left, _In_ const INT& width, _In_ const std::vector<DOUBLE>& childWidth, _In_ const bool horizontalRows, _In_ const LONG top, _In_ const LONG bottom, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ CDC* pdc, _In_ const INT row ) {
	for ( INT i = 0; i < childrenPerRow.at( row ); i++, c++ ) {
		auto child = parent->TmiGetChild( c );
		ASSERT( ( c < childWidth.size( ) ) && ( childWidth.at( c ) >= 0 ) );
		auto cChildWidth = childWidth.at( c );
		auto fRight      = left + cChildWidth * width;
		auto right       = std::lround( fRight );
		ASSERT( right >= left );
		//ASSERT( (c+1) < childWidth.GetSize( ) );
		bool lastChild = false;

		if ( ( c + 1 ) < childWidth.size( ) ) {
			lastChild = ( ( i == childrenPerRow.at( row ) - 1 ) || ( childWidth.at( c + 1 ) == 0 ) );
			}
		if ( lastChild ) {
			//right = horizontalRows ? rc.right : rc.bottom;
			right = horizontalRows ? right : bottom;
			ASSERT( ( right + 1 ) >= left );
			if ( !( right >= left ) ) {
				right = left;
				}
			}
		auto rcChild = KDirStat_buildrcChildVerticalOrHorizontalRow( horizontalRows, left, right, top, bottom );
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "left: %ld, right: %ld, top: %ld, bottom: %ld\r\n" ), rcChild.left, rcChild.right, rcChild.top, rcChild.bottom );
#endif
		RecurseDrawGraph( pdc, child, rcChild, false, surface, h * m_options.scaleFactor, 0 );
		if ( lastChild ) {
			i++, c++;
			if ( i < childrenPerRow.at( row ) ) {
				auto childToSet = parent->TmiGetChild( c );
				if ( childToSet != NULL ) {
					childToSet->TmiSetRectangle( CRect { -1, -1, -1, -1 } );//WTF
					//RenderRectangle( pdc, childToSet->TmiGetRectangle( ), surface, childToSet->TmiGetGraphColor( ) );
					}
				}
			c += childrenPerRow.at( row ) - i;
			break;
			}
		left = std::lround( fRight );
		}

	}

bool CTreemap::KDirStat_ArrangeChildren( _In_ const Item* parent, _Inout_ std::vector<DOUBLE>& childWidth, _Inout_ std::vector<DOUBLE>& rows, _Inout_ std::vector<INT_PTR>& childrenPerRow ) {
	/*
	  return: whether the rows are horizontal.
	*/
	ASSERT( !parent->TmiIsLeaf( ) );//We don't have children if we're a leaf.....
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );//will we assign to childWidth? (we should)

	if ( parent->TmiGetSize( ) == 0 ) {
		//Parent has size zero
		rows.push_back( 1.0 );
		childrenPerRow.push_back( parent->TmiGetChildrenCount( ) );
		for ( INT i = 0; i < parent->TmiGetChildrenCount( ); i++ ) {
			ASSERT( ( parent->TmiGetChildrenCount( ) != 0 ) && ( i < childWidth.size( ) ) );
			childWidth[ i ] = 1.0 / parent->TmiGetChildrenCount( );
			}
		return true;
		}
	ASSERT( parent->TmiGetSize( ) > 0 );

	bool horizontalRows = ( ( parent->TmiGetRectangle( ).Width( ) ) >= parent->TmiGetRectangle( ).Height( ) );

	auto width = KDirStat_GetWidth( parent, horizontalRows );
	INT nextChild = 0;
	
	if ( !( nextChild < parent->TmiGetChildrenCount( ) ) ) {
		ASSERT( false );
		//Shouldn't happen, but just in case, make sure we do SOMETHING that initializes the data.
		childWidth.push_back( 1.0 );
		rows.push_back( 1.0 );
		childrenPerRow.push_back( 1 );
		return horizontalRows;
		}

	ASSERT( nextChild < parent->TmiGetChildrenCount( ) );

	while ( nextChild < parent->TmiGetChildrenCount( ) ) {
		INT childrenUsed = 0;
		rows.push_back( KDirStat_CalcutateNextRow( parent, nextChild, width, childrenUsed, childWidth ) );
		childrenPerRow.push_back( childrenUsed );
		nextChild += childrenUsed;
		}
	if ( parent->TmiGetChildrenCount( ) < 1 ) {
		ASSERT( false );
		ASSERT( childWidth.size( ) == 0);
		ASSERT( rows.size( ) == 0 );
		ASSERT( childrenPerRow.size( ) == 0 );
		//childWidth[ 0 ] = 0.00;
		//rows[ 0 ] = 0.00;
		//childrenPerRow[ 0 ] = 0;
		}

	return horizontalRows;
	}

void CTreemap::KDirStat_DrawChildren( _In_ CDC* pdc, _In_ const Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD /*flags*/ ) {
	/*
	  I learned this squarification style from the KDirStat executable. It's the most complex one here but also the clearest, imho.
	*/
	ASSERT_VALID( pdc );
	ASSERT( parent->TmiGetChildrenCount( ) > 0 );

	const auto& rc = parent->TmiGetRectangle( );
	ASSERT( ( rc.Height( ) + rc.Width( ) ) > 0 );

	std::vector<DOUBLE> rows;       // Our rectangle is divided into rows, each of which gets this height (fraction of total height).
	std::vector<INT_PTR> childrenPerRow;   // childrenPerRow[i] = # of children in rows[i]
	std::vector<DOUBLE> childWidth; // Widths of the children (fraction of row width).
	
	//childrenPerRow.SetSize( parent->TmiGetChildrenCount( ) + 1 );
	//childWidth.SetSize    ( parent->TmiGetChildrenCount( ) + 1 );
	//rows.SetSize          ( parent->TmiGetChildrenCount( ) + 1 );

	const bool horizontalRows = KDirStat_ArrangeChildren( parent, childWidth, rows, childrenPerRow );

	const INT width  = horizontalRows ? rc.Width( )  : rc.Height( );
	const INT height = horizontalRows ? rc.Height( ) : rc.Width( );

	ASSERT( ( width >= 0 ) && ( height >= 0 ) );
	
	INT_PTR c = 0;
	auto top = horizontalRows ? rc.top : rc.left;
	const auto rowsSize = rows.size( );
	for ( INT row = 0; row < rowsSize; row++ ) {
		
		ASSERT( ( rowsSize == rows.size( ) ) && ( row < rows.size( ) ) );
		auto fBottom = top + rows.at( row ) * height;
		auto bottom  = std::lround( fBottom );
		if ( row == ( rowsSize - 1 ) ) {
			bottom = horizontalRows ? rc.bottom : rc.right;
			}
		LONG left = horizontalRows ? rc.left : rc.top;
		//ASSERT( ( childWidth.GetUpperBound( ) > -1 ) && ( row < childrenPerRow.GetSize( ) ) );
		
		KDirStat_DrawChildrenInThisRow( childrenPerRow, c, parent, left, width, childWidth, horizontalRows, top, bottom, surface, h, pdc, row );
		//
//		for ( INT i = 0; i < childrenPerRow[ row ]; i++, c++ ) {
//			auto child = parent->TmiGetChild( c );
//			ASSERT( ( c < childWidth.GetSize( ) ) && ( childWidth[ c ] >= 0 ) );
//			auto cChildWidth = childWidth[ c ];
//			auto fRight      = left + cChildWidth * width;
//			auto right       = std::lround( fRight );
//			ASSERT( right >= left );
//			//ASSERT( (c+1) < childWidth.GetSize( ) );
//			bool lastChild = false;
//
//			if ( ( c + 1 ) < childWidth.GetSize( ) ) {
//				lastChild = ( ( i == childrenPerRow[ row ] - 1 ) || ( childWidth[ c + 1 ] == 0 ) );
//				}
//			if ( lastChild ) {
//				//right = horizontalRows ? rc.right : rc.bottom;
//				right = horizontalRows ? right : bottom;
//				ASSERT( ( right + 1 ) >= left );
//				if ( !( right >= left ) ) {
//					right = left;
//					}
//				}
//			auto rcChild = KDirStat_buildrcChildVerticalOrHorizontalRow( horizontalRows, left, right, top, bottom );
//#ifdef GRAPH_LAYOUT_DEBUG
//			TRACE( _T( "left: %ld, right: %ld, top: %ld, bottom: %ld\r\n" ), rcChild.left, rcChild.right, rcChild.top, rcChild.bottom );
//#endif
//			RecurseDrawGraph( pdc, child, rcChild, false, surface, h * m_options.scaleFactor, 0 );
//			if ( lastChild ) {
//				i++, c++;
//				if ( i < childrenPerRow[ row ] ) {
//					auto childToSet = parent->TmiGetChild( c );
//					if ( childToSet != NULL ) {
//						childToSet->TmiSetRectangle( CRect { -1, -1, -1, -1 } );//WTF
//						//RenderRectangle( pdc, childToSet->TmiGetRectangle( ), surface, childToSet->TmiGetGraphColor( ) );
//						}
//					}
//				c += childrenPerRow[ row ] - i;
//				break;
//				}
//			left = std::lround( fRight );
//			}

		// This asserts due to rounding error: ASSERT(left == (horizontalRows ? rc.right : rc.bottom));
		top = std::lround( fBottom );
		}
	// This asserts due to rounding error: ASSERT(top == (horizontalRows ? rc.bottom : rc.right));
	}

DOUBLE CTreemap::KDirStat_GetWidth( _In_ const Item* parent, _In_ const bool horizontalRows ) {
	/*
	  Returns the Width/Height OR Height/Width ratio of parent, depending on horizontalRows
	*/
	if ( horizontalRows ) {
		if ( parent->TmiGetRectangle( ).Height( ) > 0 ) {
			return DOUBLE( parent->TmiGetRectangle( ).Width( ) ) / DOUBLE( parent->TmiGetRectangle( ).Height( ) );
			}
		}
	else {
		if ( parent->TmiGetRectangle( ).Width( ) > 0 ) {
			return DOUBLE( parent->TmiGetRectangle( ).Height( ) ) / DOUBLE( parent->TmiGetRectangle( ).Width( ) );
			}
		}
	return 0.00;
	}

void CTreemap::KDirStat_IterateOverAllChilrenInParent( _In_ const Item* parent, _In_ _In_range_( 0, INT_MAX ) const INT nextChild, _Inout_ DOUBLE& sizeUsed, _In_ _In_range_( 0, 32767 ) const DOUBLE width, const _In_ DOUBLE mySize, _In_ const DOUBLE _minProportion, _Inout_ DOUBLE& rowHeight, _Inout_ INT& i  ) {
	const auto parent_tmiGetChildCount = parent->TmiGetChildrenCount( );
	for ( i = nextChild; i < parent_tmiGetChildCount; ++i ) { //iterate over all children of parent, starting at nextChild,
		const auto childOfParent = parent->TmiGetChild( i );
		DOUBLE childSize = 0.00;
		if ( childOfParent != NULL ) {
			childSize += DOUBLE( childOfParent->TmiGetSize( ) );
			}
		ASSERT( childOfParent != NULL );
		if ( std::lround( childSize ) == 0 ) {
//			ASSERT( i > nextChild );	// first child has size > 0
			break;
			}
		ASSERT( ( std::lround( childSize ) != 0 ) && ( mySize != 0 ) );
		sizeUsed += childSize;
		auto virtualRowHeight = sizeUsed / mySize;

		if ( virtualRowHeight > 1.00 ) {
			TRACE( _T( "sizeUsed(%f) / mySize(%f) = %f\r\n\tTHAT'S BIGGER THAN 1!\r\n" ), sizeUsed, mySize, virtualRowHeight );
			}
		ASSERT( ( virtualRowHeight > 0 ) && ( virtualRowHeight <= 1.00 ) && ( childSize <= mySize ) && ( mySize != 0 ) );
		const auto aChildWidth = ( ( childSize * width ) / mySize ) / virtualRowHeight;//WTF
		
		if ( aChildWidth / virtualRowHeight < _minProportion ) {
			ASSERT( ( ( aChildWidth / virtualRowHeight ) < _minProportion ) && ( width >= 1.0 ) && ( i >= nextChild ) );// i >= nextChild because width >= 1 and _minProportion < 1.
			++i;
			break;
			}

#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "i: %i, nextChild: %i, childSize: %f, virtualRowHeight: %f\r\n" ), i, nextChild, childSize, virtualRowHeight );
#endif
		rowHeight = virtualRowHeight;
		}

	}

void CTreemap::KDirStat_OperateOnSingleChild( _In_ const Item* parent, _In_ _In_range_( 0, INT_MAX ) const INT nextChild, _In_ const DOUBLE mySize, _Inout_ DOUBLE& rowHeight, _Inout_ std::vector<DOUBLE>& childWidth, _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Inout_ DOUBLE& cwTotal, _Inout_ DOUBLE& sizeSoFar, _In_ const INT j ) {
	// Rectangle(1.0 * 1.0) = mySize
	DOUBLE rowSize = { mySize * rowHeight };
	//rowSize += 1;//Fuck you floating point!
	sizeSoFar += mySize;
	auto childOfParent = parent->TmiGetChild( nextChild + j );
	if ( childOfParent != NULL ) {
#ifdef GRAPH_LAYOUT_DEBUG
		TRACE( _T( "Iter J:%i, Calculating rectangle for %i\r\n" ), j, ( nextChild + j ) );
#endif
		const auto childSize = DOUBLE( childOfParent->TmiGetSize( ) );
		ASSERT( std::fmod( childSize, 1 ) == 0 );
		//childSize -= 1;//Fuck you floating point!
		//ASSERT( ( ( childSize / rowSize ) + cwTotal ) <= width );
		//ASSERT( childSize == childOfParent->TmiGetSize( ) );
		ASSERT( childSize <= ( rowSize + 0.01 ) );//Fuck you, floating point!
		DOUBLE cw = childSize / rowSize;
		if ( ( cw + cwTotal ) > width ) {
			cw += ( width - ( cw + cwTotal ) );//ugly hack to deal with floating point madness!
			}
		ASSERT( ( cwTotal <= width ) && ( cw <= width ) && ( ( cw + cwTotal ) <= width ) );
		//cw = cw * ( sizeSoFar/rowSize );//
		//ASSERT( cwTotal >= shit );
		//ASSERT( cw <= ( ( width - cwTotal ) + 0.01 ) );//Fuck you, floating point!
			
		ASSERT( cw >= 0 );
			
		if ( childWidth.size( ) == ( nextChild + j ) ) {
			ASSERT( ( cw < 32767 ) && ( ( cw + cwTotal ) <= width ) && ( cw <= 1.00001 ) );//Fuck you, floating point!
			childWidth.push_back( cw );
			cwTotal += cw;
			ASSERT( cwTotal <= ( parent->TmiGetRectangle( ).right - parent->TmiGetRectangle( ).left ) );
			}
		else {
			ASSERT( ( childWidth.size( ) >= ( nextChild + j ) ) && ( cw <= 1 ) && ( ( nextChild + j ) < childWidth.size( ) ) && ( ( cw + cwTotal ) <= width ) );
			childWidth[ nextChild + j ] = cw;
			cwTotal += cw;
			ASSERT( cwTotal <= ( parent->TmiGetRectangle( ).right - parent->TmiGetRectangle( ).left ) );
			}
		}
	ASSERT( childOfParent != NULL );
	}

DOUBLE CTreemap::KDirStat_CalcutateNextRow( _In_ const Item* parent, _In_ _In_range_( 0, INT_MAX ) const INT nextChild, _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Inout_ INT& childrenUsed, _Inout_ std::vector<DOUBLE>& childWidth ) {
	static const DOUBLE _minProportion = 0.4;
	ASSERT( ( nextChild < parent->TmiGetChildrenCount( ) ) && ( nextChild >= 0 ) && ( width >= 1.0 ) );
	const auto mySize = DOUBLE( parent->TmiGetSize( ) );
	ASSERT( ( std::fmod( mySize, 1 ) == 0 ) && ( mySize > 0 ) );
	DOUBLE sizeUsed  = 0.00;
	DOUBLE rowHeight = 0.00;
	INT i = 0;
	
	//( _In_ const Item* parent, _In_ const INT_PTR parent_tmiGetChildCount, _Inout_ DOUBLE& sizeUsed, _Inout_ DOUBLE& rowHeight )

	KDirStat_IterateOverAllChilrenInParent( parent, nextChild, sizeUsed, width, mySize, _minProportion, rowHeight, i );


#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "rowSize on exiting inner loop: %f\r\n" ), rowHeight );
#endif

	ASSERT( i >= nextChild );
	// Now i-1 is the last child used and rowHeight is the height of the row. We add the rest of the children, if their size is 0.

	while ( i < parent->TmiGetChildrenCount( ) && parent->TmiGetChild( i )->TmiGetSize( ) == 0 ) {
		++i;
		}

	//(mySize * rowHeight)
	childrenUsed = i - nextChild;
	//ASSERT( childrenUsed > 0 );
	DOUBLE cwTotal = 0.00;
	ASSERT( cwTotal <= width );
	DOUBLE sizeSoFar = 0.00;
	// Now as we know the rowHeight, we compute the widths of our children.
	//ASSERT( 0 < childrenUsed );
	if ( !( 0 < childrenUsed ) ) {
		//We need to make sure that childWidth is `initialized` before returning. The for loop SHOULD iterate at least once, but in the event that it won't, let's do SOMETHING to initialize childWidth. /analyze will complain because it can't figure out that if execution skips this branch, we WON'T skip the following loop - as the conditional test in this loop is the converse of that therein the for loop. //changed childWidth back to `_Inout_` from `_Out_`, no longer complains
		childWidth.push_back( 1.00 );
		}

	for ( INT j = 0; j < childrenUsed; j++ ) {
		KDirStat_OperateOnSingleChild( parent, nextChild, mySize, rowHeight, childWidth, width, cwTotal, sizeSoFar, j );
		}

#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "Exiting second inner loop, cwTotal: %f, sizeSoFar: %f, sizeUsed: %f, rowHeight: %f, childrenUsed: %f\r\n" ), cwTotal, sizeSoFar, sizeUsed, rowHeight, childrenUsed );
#endif
	ASSERT( cwTotal <= ( width + 0.01 ) );//Fuck you, floating point!
	return rowHeight;
	}

//layoutrow() == PlaceChildren?
void CTreemap::SequoiaView_PlaceChildren( _In_ CDC* pdc, _In_ const Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const INT_PTR rowBegin, _In_ const INT_PTR rowEnd, _In_ DOUBLE fBegin, _In_ const std::uint64_t sum, _In_ const bool horizontal, _In_ const CRect& remaining, _Inout_ CRect& rc, _In_ const INT height ) {

	// Now put the children into their places
	for ( auto i = rowBegin; i < rowEnd; i++ ) {
		const INT    begin    = INT( std::lround( fBegin ) );
		const auto   childAtI = parent->TmiGetChild( i );
		DOUBLE       fraction = DBL_MAX;

		if ( childAtI != NULL ) {
			fraction = ( DOUBLE( childAtI->TmiGetSize( ) ) ) / sum;
			}
		ASSERT( childAtI != NULL );
		const auto fEnd = fBegin + fraction * DOUBLE( height );
		auto end        = INT( std::lround( fEnd ) );

		const auto lastChild = ( i == rowEnd - 1 || parent->TmiGetChild( i + 1 )->TmiGetSize( ) == 0 );

		if ( lastChild ) { // Use up the whole height
			end = ( horizontal ? ( remaining.top + height ) : ( remaining.left + height ) );
			}
		
		if ( horizontal ) {
			rc.top    = begin;
			rc.bottom = end;
			}
		else {
			rc.left  = begin;
			rc.right = end;
			}

		rc.NormalizeRect( );

		ASSERT( rc.left   <= rc.right );
		ASSERT( rc.top    <= rc.bottom );
		ASSERT( rc.left   >= remaining.left );
		ASSERT( rc.right  <= remaining.right );
		ASSERT( rc.top    >= remaining.top );
		ASSERT( rc.bottom <= remaining.bottom );

		ASSERT( childAtI == parent->TmiGetChild( i ) );
		if ( childAtI != NULL ) {
			RecurseDrawGraph( pdc, childAtI, rc, false, surface, h * m_options.scaleFactor, 0 );
			}
		ASSERT( childAtI != NULL );
		if ( lastChild ) {
			break;
			}

		fBegin = fEnd;
		}
	
	}

bool WillGetWorse( _In_ const std::uint64_t sumOfSizeOfChildrenInThisRow, _In_ const LONGLONG minSizeOfChildrenInThisRow, _In_ const LONGLONG maxSizeOfChildrenInThisRow, _In_ const DOUBLE worstRatioSoFar, _In_ const DOUBLE hh, _Inout_ DOUBLE& nextWorst ) {

	const auto ss = ( DOUBLE( sumOfSizeOfChildrenInThisRow + minSizeOfChildrenInThisRow ) ) * ( DOUBLE( sumOfSizeOfChildrenInThisRow + minSizeOfChildrenInThisRow ) );
	ASSERT( ss != 0 );
	ASSERT( hh != 0 );
	ASSERT( minSizeOfChildrenInThisRow != 0 );

	const DOUBLE ratio1 = hh * maxSizeOfChildrenInThisRow / ss;
	const DOUBLE ratio2 = ss / hh / minSizeOfChildrenInThisRow;
	nextWorst = max( ratio1, ratio2 );
	return nextWorst > worstRatioSoFar;
	}

void assign_rc_and_fBegin_horizontalOrVertical( _In_ const CRect& remainingRectangleToFill, _Inout_ CRect& rc, _Inout_ DOUBLE& fBegin, _In_ const bool divideHorizontally, _In_ const int widthOfThisRow ) {
	if ( divideHorizontally ) {
		rc.left   = remainingRectangleToFill.left;
		rc.right  = remainingRectangleToFill.left + widthOfThisRow;
		fBegin    = remainingRectangleToFill.top;
		}
	else {
		rc.top    = remainingRectangleToFill.top;
		rc.bottom = remainingRectangleToFill.top + widthOfThisRow;
		fBegin    = remainingRectangleToFill.left;
		}

	}

void addChild_rowEnd_toRow( _Inout_ std::uint64_t& sumOfSizeOfChildrenInThisRow, _In_ const LONGLONG minSizeOfChildrenInThisRow, _Inout_ INT_PTR& rowEnd, _In_ const DOUBLE& nextWorst, _Inout_ DOUBLE& worstRatioSoFar ) {
	sumOfSizeOfChildrenInThisRow += minSizeOfChildrenInThisRow;
	ASSERT( sumOfSizeOfChildrenInThisRow >= 0 );
	worstRatioSoFar = nextWorst;
	++rowEnd;
	}

void CTreemap::checkVirtualRowOf_rowBegin_to_rowEnd__thenAdd( _In_ Item* parent, _Inout_ INT_PTR& rowEnd, _Inout_ std::uint64_t& sumOfSizeOfChildrenInThisRow, _In_ const LONGLONG maxSizeOfChildrenInThisRow, _Inout_ DOUBLE& worstRatioSoFar, _In_ const DOUBLE hh ) {
	// This condition will hold at least once.
	while ( rowEnd < parent->TmiGetChildrenCount( ) ) { // We check a virtual row made up of child(rowBegin)...child(rowEnd) here.
		//

		// Minimum size of child in virtual row
		LONGLONG minSizeOfChildrenInThisRow = 0;
		auto childRowEnd = parent->TmiGetChild( rowEnd );
		if ( childRowEnd != NULL ) {
			minSizeOfChildrenInThisRow = childRowEnd->TmiGetSize( );
			}
		// If sizes of the rest of the children is zero, we add all of them
		if ( minSizeOfChildrenInThisRow == 0 ) {
			rowEnd = parent->TmiGetChildrenCount( );
			break;
			}

		// Calculate the worst ratio in virtual row. Formula taken from the "Squarified Treemaps" paper: (http://http://www.win.tue.nl/~vanwijk/)

		DOUBLE nextWorst = DBL_MAX;

		if ( WillGetWorse( sumOfSizeOfChildrenInThisRow, minSizeOfChildrenInThisRow, maxSizeOfChildrenInThisRow, worstRatioSoFar, hh, nextWorst ) ) {
			break;// Yes. Don't take the virtual row, but the real row (child(rowBegin)..child(rowEnd - 1)) made so far.
			}

		// Here we have decided to add child( rowEnd ) to the row.
		addChild_rowEnd_toRow( sumOfSizeOfChildrenInThisRow, minSizeOfChildrenInThisRow, rowEnd, nextWorst, worstRatioSoFar );
		}

	}

// The classical squarification method.
void CTreemap::SequoiaView_DrawChildren( _In_ CDC* pdc, _In_ Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD /*flags*/ ) {
	ASSERT_VALID( pdc );
	INT_PTR head = 0;                                      // First child for next row
	auto remainingSize            = parent->TmiGetSize( ); // Size of rest rectangle
	auto remainingRectangleToFill = parent->TmiGetRectangle( );
	const auto OrigRemainingSize  = remainingSize;

	ASSERT( remainingRectangleToFill.Width( )  > 0 );
	ASSERT( remainingRectangleToFill.Height( ) > 0 );
	const auto scaleFactor_sizePerSquarePixel = DOUBLE( parent->TmiGetSize( ) ) / DOUBLE( remainingRectangleToFill.Width( ) ) / DOUBLE( remainingRectangleToFill.Height( ) );
	ASSERT( scaleFactor_sizePerSquarePixel > 0 );

	ASSERT( remainingSize > 0 );
#define SIZE_OF_PARENT_CHECK __LINE__-1

	// At least one child left
	while ( head < parent->TmiGetChildrenCount( ) ) {
		ASSERT( ( remainingRectangleToFill.Width( ) > 0 ) && ( remainingRectangleToFill.Height( ) > 0 ) );

		//The first step of our algorithm is to split the initial rectangle. [if] We choose for a horizontal subdivision, [we did so] because the original rectangle is wider than high.
		const bool  divideHorizontally = ( remainingRectangleToFill.Width( ) >= remainingRectangleToFill.Height( ) );
		const auto  heightOfNewRow     = divideHorizontally ? remainingRectangleToFill.Height( ) : remainingRectangleToFill.Width( );

		// Square of height in size scale for ratio formula
		const auto hh = ( heightOfNewRow * heightOfNewRow ) * scaleFactor_sizePerSquarePixel;
		ASSERT( hh > 0 );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		const auto rowBegin = head;
		      auto rowEnd   = head;

		// initialized to DBL_MAX so as not to later divide by zero accidentally.
		DOUBLE worstRatioSoFar = DBL_MAX;

		const auto childRowBegin = parent->TmiGetChild( rowBegin );
		LONGLONG maxSizeOfChildrenInThisRow = 0;
		if ( childRowBegin != NULL ) {
			maxSizeOfChildrenInThisRow = childRowBegin->TmiGetSize( ); // Maximum size of children in row
			}

		std::uint64_t sumOfSizeOfChildrenInThisRow = 0;

		checkVirtualRowOf_rowBegin_to_rowEnd__thenAdd( parent, rowEnd, sumOfSizeOfChildrenInThisRow, maxSizeOfChildrenInThisRow, worstRatioSoFar, hh );

		// Row will be made up of child(rowBegin)...child(rowEnd - 1); sumOfSizeOfChildrenInThisRow is the size of the row. As the size of parent is greater than zero ( see line SIZE_OF_PARENT_CHECK ), the size of the first child must have been greater than zero, too.
		if ( remainingSize > 0 ) {
			ASSERT( sumOfSizeOfChildrenInThisRow > 0 );
			}
		auto widthOfThisRow = ( divideHorizontally ? ( remainingRectangleToFill.Width( ) ) : ( remainingRectangleToFill.Height( ) ) );
		ASSERT( widthOfThisRow > 0 );

		if ( sumOfSizeOfChildrenInThisRow < remainingSize ) {
			widthOfThisRow = INT( round( DOUBLE( sumOfSizeOfChildrenInThisRow ) / remainingSize * widthOfThisRow ) );// else: use up the whole width; widthOfThisRow may be 0 here.
			}

		// Build the rectangles of children.
		CRect rc;
		DOUBLE fBegin = DBL_MAX;

		assign_rc_and_fBegin_horizontalOrVertical( remainingRectangleToFill, rc, fBegin, divideHorizontally, widthOfThisRow );
		SequoiaView_PlaceChildren( pdc, parent, surface, h, rowBegin, rowEnd, fBegin, sumOfSizeOfChildrenInThisRow, divideHorizontally, remainingRectangleToFill, rc, heightOfNewRow );

		// Put the next row into the rest of the rectangle
		if ( divideHorizontally ) {
			remainingRectangleToFill.left += widthOfThisRow;
			}
		else {
			remainingRectangleToFill.top += widthOfThisRow;
			}
		ASSERT( remainingSize >= 0 );//may assert?
		ASSERT( sumOfSizeOfChildrenInThisRow >= 0 );//may assert?

#pragma push_macro("max")
#undef max
		ASSERT( ( ULONGLONG( sumOfSizeOfChildrenInThisRow ) + ULONGLONG( remainingSize ) ) < std::numeric_limits<LONGLONG>::max( ) );//god, I hate name collisions
#pragma pop_macro("max")

		remainingSize -= sumOfSizeOfChildrenInThisRow;
		
		ASSERT( ( remainingRectangleToFill.left <= remainingRectangleToFill.right ) && ( remainingRectangleToFill.top <= remainingRectangleToFill.bottom ) && ( remainingSize >= 0 ) );

		head += ( rowEnd - rowBegin );

		if ( ( remainingRectangleToFill.Width( ) ) <= 0 || ( remainingRectangleToFill.Height( ) ) <= 0) {
			if ( head < parent->TmiGetChildrenCount( ) ) {
				auto childOfParent = parent->TmiGetChild( head );
				if ( childOfParent != NULL ) {
					//childOfParent->TmiSetRectangle( CRect( -1, -1, -1, -1 ) );//??????????
					RenderRectangle( pdc, childOfParent->TmiGetRectangle( ), surface, childOfParent->TmiGetGraphColor( ) );
					}
				ASSERT( childOfParent != NULL );
				}
			break;
			}
		}
	ASSERT( remainingSize < OrigRemainingSize );
	//ASSERT( ( remainingSize == 0 ) || ( remainingSize == 1 ) || ( (remainingSize % 2) == 1 ));//rounding error
	//ASSERT( remainingRectangleToFill.left == remainingRectangleToFill.right || remainingRectangleToFill.top == remainingRectangleToFill.bottom );
	}

// No squarification. Children are arranged alternately horizontally and vertically.
void CTreemap::Simple_DrawChildren( _In_ const CDC* pdc, _In_ const Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD flags ) {
	ASSERT_VALID( pdc );
	ASSERT( false ); // Not used in Windirstat.
	UNREFERENCED_PARAMETER( pdc );
	UNREFERENCED_PARAMETER( parent );
	UNREFERENCED_PARAMETER( surface );
	UNREFERENCED_PARAMETER( h );
	UNREFERENCED_PARAMETER( flags );
	}

bool CTreemap::IsCushionShading( ) const {
	return m_options.ambientLight < 1.0 && m_options.height > 0.0 && m_options.scaleFactor > 0.0;
	}

void CTreemap::RenderLeaf( _In_ CDC* pdc, _In_ Item* item, _In_ _In_reads_( 4 ) const DOUBLE* surface ) {
	auto rc = item->TmiGetRectangle( );
	if ( m_options.grid ) {
		rc.top++;
		rc.left++;
		if ( ( rc.Width( ) ) <= 0 || ( rc.Height( ) ) <= 0 ) {
			return;
			}
		}
	rc.NormalizeRect( );
	auto colorOfItem = item->TmiGetGraphColor( );
	ASSERT( colorOfItem != 0 );
	RenderRectangle( pdc, rc, surface, colorOfItem );
	}

void CTreemap::RenderRectangle( _In_ CDC* pdc, _In_ const CRect& rc, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ DWORD color ) {
	//ASSERT( ( rc.Width( ) > 0 ) || ( rc.Height( ) > 0 ) );
	if ( ( ( rc.Width( ) ) == 0 ) || ( ( rc.Height( ) ) ) ) {
		//TRACE( _T( "Huh?\r\n" ) );
		//return;
		}
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
	ASSERT( color != 0 );
	if ( IsCushionShading_current ) {
		DrawCushion( pdc, rc, surface, color, brightness );
		}
	else {
		DrawSolidRect( pdc, rc, color, brightness );
		}
	}

void CTreemap::DrawSolidRect( _In_ CDC* pdc, _In_ const CRect& rc, _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness ) {
	INT red   = GetRValue( col );
	INT green = GetGValue( col );
	INT blue  = GetBValue( col );
	
	const DOUBLE factor = brightness / PALETTE_BRIGHTNESS;

	red = INT( std::lround( red * factor ) );
	green = INT( std::lround( green * factor ));
	blue  = INT( std::lround( blue * factor ) );

	CColorSpace::NormalizeColor( red, green, blue );

	pdc->FillSolidRect( rc, RGB( red, green, blue ) );
	}

//void setPix( CDC* pdc, std::mutex* pixlesMutex, std::mutex* pdcMutex, std::queue<setPixStruct>& pixles, std::condition_variable* isDataReady, std::atomic_bool* isDone) {
//	while ( !std::atomic_load( isDone ) ) {
//		std::unique_lock<std::mutex> lck( *pixlesMutex );
//		if ( std::atomic_load( isDone ) ) {
//			return;
//			}
//
//		isDataReady->wait( lck );
//		{
//		if ( pixles.size( ) > 0 ) {
//			auto aPixle = std::move( pixles.front( ) );
//			pixles.pop( );
//			lck.unlock( );
//			pdcMutex->lock( );
//
//			TRACE( _T( "Setting color %i\r\n" ), aPixle.color );
//			pdc->SetPixel( aPixle.ix, aPixle.iy, aPixle.color );
//			pdcMutex->unlock( );
//			}
//		else {
//			return;
//			}
//		}
//		}
//	return;
//	}


void CTreemap::DrawCushion( _In_ CDC *pdc, const _In_ CRect& rc, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const COLORREF col, _In_ _In_range_(0, 1) const DOUBLE brightness ) {
	//ASSERT( ( rc.Width()  > 0 ) || ( rc.Height() > 0 ) );
	// Cushion parameters
	const DOUBLE Ia = m_options.ambientLight;

	// Derived parameters
	const DOUBLE Is = 1 - Ia;			// shading

	const DOUBLE colR = GetRValue( col );//THIS does NOT get vectorized!
	const DOUBLE colG = GetGValue( col );//THIS gets vectorized
	const DOUBLE colB = GetBValue( col );//THIS does NOT get vectorized!
	
	std::vector<setPixStruct> xPixles;
	xPixles.reserve( size_t( ( rc.Width( ) ) + 1 ) );
#ifdef GRAPH_LAYOUT_DEBUG
	TRACE( _T( "DrawCushion drawing rectangle    left: %li, right: %li, top: %li, bottom: %li\r\n" ), rc.left, rc.right, rc.top, rc.bottom );
#endif
	for ( INT iy = rc.top; iy < rc.bottom; iy++ ) {
		xPixles.reserve( size_t( ( rc.Width( ) ) + 1 ) );
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
			auto red   = INT( colR * pixel );
			auto green = INT( colG * pixel );
			auto blue  = INT( colB * pixel );
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
			ASSERT( RGB( red, green, blue ) != 0 );
			static_assert( sizeof( INT ) == sizeof( std::int_fast32_t ), "setPixStruct bad point type!!" );
			static_assert( sizeof( std::int_fast32_t ) == sizeof( COLORREF ), "setPixStruct bad color type!!" );
			xPixles.emplace_back( setPixStruct ( ix, iy, RGB( red, green, blue ) ) );//TODO fix implicit conversion!
			}
		for ( INT ix = rc.left; ix < rc.right; ix++ ) {
			setPixStruct& setP = xPixles.at( ix - rc.left );
#ifdef GRAPH_LAYOUT_DEBUG
			debugSetPixel( pdc, setP.ix, setP.iy, setP.color );//debug version that detects drawing collisions
#else
			pdc->SetPixel( setP.ix, setP.iy, setP.color );//this is the sole remaining bottleneck here. The drawing time is a direct function of the drawing area - i.e. how many times we have to setPixel!
#endif
			}
		xPixles.clear( );
		}
	}

#ifdef GRAPH_LAYOUT_DEBUG
void CTreemap::debugSetPixel( CDC* pdc, int x, int y, COLORREF c ) {
	++numCalls;
	//This function detects drawing collisions!
	if ( !( bitSetMask->at( x ).at( y ) ) ) {
		( *bitSetMask )[ x ][ y ] = true;//we already know that we're in bounds.
		pdc->SetPixel( x, y, c );
		}
	else {

		ASSERT( false );
		AfxDebugBreak( );
		}
	}
#endif

void CTreemap::AddRidge( _In_ const CRect& rc, _Inout_ _Inout_updates_( 4 ) DOUBLE* surface, _In_ const DOUBLE h ) {
	auto width  = ( rc.Width( ) );
	auto height = ( rc.Height( ) );

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
	m_root = NULL;
	BuildDemoData();
	}

CTreemapPreview::~CTreemapPreview( ) {
	if ( m_root != NULL ) {
		delete m_root;
		m_root = NULL;
		}
	}

void CTreemapPreview::SetOptions(_In_ const CTreemap::Options *options)
{
	m_treemap.SetOptions(options);
	Invalidate();
}

void CTreemapPreview::BuildDemoData( ) {
	//CTreemap::GetDefaultPalette( m_colors );
	m_vectorOfColors = CTreemap::GetDefaultPaletteAsVector( );
	size_t col = 0;
	COLORREF color;
	INT i = 0;

#ifdef CHILDVEC
	std::vector<CItem*> c4;
	std::vector<CItem*> c1;
	std::vector<CItem*> c2;
	std::vector<CItem *> c3;
	std::vector<CItem *> c10;
	std::vector<CItem*> c0;
#else
	CArray<CItem *, CItem *> c4;
	CArray<CItem *, CItem *> c0;
	CArray<CItem *, CItem *> c1;
	CArray<CItem *, CItem *> c2;
	CArray<CItem *, CItem *> c3;
	CArray<CItem *, CItem *> c10;
#endif
	
	color = GetNextColor( col );
	for ( i = 0; i < 20; i++ ) {
		c4.polyAdd( new CItem { 7 * i, color } );
		}
	for ( i = 0; i < 9; i++ ) {
		c0.polyAdd( new CItem { 13 * i, GetNextColor( col ) } );
		}

	color = GetNextColor( col );
	for ( i = 0; i < 7; i++ ) {
		c1.polyAdd( new CItem { 23 * i, color } );
		}
	c0.polyAdd( new CItem { c1 } );

	color = GetNextColor( col );
	for ( i = 0; i < 53; i++ ) {
		c2.polyAdd( new CItem { 1 + i, color } );
		}
	c3.polyAdd( new CItem { 457, GetNextColor( col ) } );
	c3.polyAdd( new CItem { c4 } );
	c3.polyAdd( new CItem { c2 } );
	c3.polyAdd( new CItem { 601, GetNextColor( col ) } );
	c3.polyAdd( new CItem { 151, GetNextColor( col ) } );

	
	c10.polyAdd( new CItem { c0 } );
	c10.polyAdd( new CItem { c3 } );;
	m_root = new CItem { c10 };
	}

COLORREF CTreemapPreview::GetNextColor( _Inout_ size_t& i ) {
	++i;
	i %= m_vectorOfColors.size( );
	return m_vectorOfColors.at( i );
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
