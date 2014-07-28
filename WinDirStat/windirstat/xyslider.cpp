// XySlider.cpp	- Implementation of CXySlider
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
#include ".\xyslider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CXySlider, CStatic)

const UINT CXySlider::XY_SETPOS = WM_USER + 100;
const UINT CXySlider::XY_GETPOS = WM_USER + 101;


void AFXAPI DDX_XySlider( CDataExchange* pDX, INT nIDC, CPoint& value ) {
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem( nIDC, &hWndCtrl );
	if ( pDX->m_bSaveAndValidate ) {
		::SendMessage( hWndCtrl, CXySlider::XY_GETPOS, 0, ( LPARAM ) &value );
		}
	else {
		::SendMessage( hWndCtrl, CXySlider::XY_SETPOS, 0, ( LPARAM ) &value );
		}
	}


CXySlider::CXySlider( ) {
	m_externalRange    = CSize( 100, 100 );
	m_externalPos      = CPoint( 0, 0 );
	m_pos              = CPoint( 0, 0 );
	m_timer            = 0;
	m_inited           = false;
	m_gripperHighlight = false;
	}

void CXySlider::Initialize( ) {
	if ( !m_inited && IsWindow( m_hWnd ) ) {
		// Make size odd, so that zero lines are central
		CRect rc;
		GetWindowRect( rc );
		GetParent( )->ScreenToClient( rc );
		if ( rc.Width( ) % 2 == 0 ) {
			rc.right--;
			}
		if ( rc.Height( ) % 2 == 0 ) {
			rc.bottom--;
			}
		MoveWindow( rc );

		// Initialize constants
		CalcSizes( );

		m_inited = true;
		}
	}

void CXySlider::GetRange( _Inout_ CSize& range ) {
	range = m_externalRange;
	}

void CXySlider::SetRange( _In_ CSize range ) {
	m_externalRange = range;
	}
	
CPoint CXySlider::GetPos( ) {
	return m_externalPos;
	}

LRESULT CXySlider::OnSetPos( WPARAM, LPARAM lparam ) {
	POINT *point = ( POINT * ) lparam;
	SetPos( *point );
	return 0;
	}

LRESULT CXySlider::OnGetPos( WPARAM, LPARAM lparam ) {
	POINT *point= (POINT *)lparam;
	*point = GetPos();
	return 0;
	}


void CXySlider::SetPos( CPoint pt ) {
	Initialize( );
	m_externalPos = pt;
	ExternToIntern( );
	Invalidate( );
	}

void CXySlider::CalcSizes( ) {
	static const INT _gripperRadius = 8;

	GetClientRect( m_rcAll );

	ASSERT( m_rcAll.left == 0 );
	ASSERT( m_rcAll.top == 0 );
	ASSERT( m_rcAll.Width( ) % 2 == 1 );
	ASSERT( m_rcAll.Height( ) % 2 == 1 );
	ASSERT( m_rcAll.Width( ) >= _gripperRadius * 2 );	// Control must be large enough
	ASSERT( m_rcAll.Height( ) >= _gripperRadius * 2 );

	m_zero.x = m_rcAll.Width ()  / 2;
	m_zero.y = m_rcAll.Height()  / 2;

	m_radius.cx = m_rcAll.Width ()  / 2 - 1;
	m_radius.cy = m_rcAll.Height()  / 2 - 1;

	m_rcInner = m_rcAll;
	m_rcInner.DeflateRect(_gripperRadius - 3, _gripperRadius - 3);

	m_gripperRadius.cx = _gripperRadius;
	m_gripperRadius.cy = _gripperRadius;

	m_range = m_radius - m_gripperRadius;
	}

CRect CXySlider::GetGripperRect( ) {
	CRect rc(- m_gripperRadius.cx, - m_gripperRadius.cy, m_gripperRadius.cx + 1, m_gripperRadius.cy + 1);
	rc.OffsetRect( m_zero );
	rc.OffsetRect( m_pos );
	return rc;
	}

void CXySlider::CheckMinMax( _Inout_ LONG& val, _In_ const INT min_val, _In_ const INT max_val ) {
	/*changed min and max to min_val and max_val to avoid conflict in ASSERT macro
	  WHY are we comparing a LONG to two ints?
	*/
	ASSERT( min_val <= max_val );

	if ( val < min_val ) {
		val = min_val;
		}
	if ( val > max_val ) {
		val = max_val;
		}
	ASSERT( val <= max_val );
	ASSERT( min_val <= val );
	}

void CXySlider::InternToExtern( ) {
	m_externalPos.x = ( INT ) ( ( DOUBLE ) abs( m_pos.x ) * m_externalRange.cx / m_range.cx + 0.5 ) * signum( m_pos.x );
	m_externalPos.y = ( INT ) ( ( DOUBLE ) abs( m_pos.y ) * m_externalRange.cy / m_range.cy + 0.5 ) * signum( m_pos.y );
	}

void CXySlider::ExternToIntern( ) {
	m_pos.x = ( INT ) ( ( DOUBLE ) abs( m_externalPos.x ) * m_range.cx / m_externalRange.cx + 0.5 ) * signum( m_externalPos.x );
	m_pos.y = ( INT ) ( ( DOUBLE ) abs( m_externalPos.y ) * m_range.cy / m_externalRange.cy + 0.5 ) * signum( m_externalPos.y );
	}

void CXySlider::NotifyParent( ) {
	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom   = GetDlgCtrlID();
	hdr.code     = XYSLIDER_CHANGED;

	GetParent( )->SendMessage( WM_NOTIFY, GetDlgCtrlID( ), ( LPARAM ) &hdr );
	}

void CXySlider::PaintBackground( _In_ CDC *pdc ) {
	ASSERT_VALID( pdc );
	pdc->FillSolidRect( m_rcAll, GetSysColor( COLOR_BTNFACE ) );

	CRect rc = m_rcInner;
	pdc->DrawEdge( rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST );

	pdc->FillSolidRect( rc, RGB( 255, 255, 255 ) );

	CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DLIGHT ) );
	CSelectObject sopen( pdc, &pen );

	pdc->MoveTo( rc.left, m_zero.y );
	pdc->LineTo( rc.right, m_zero.y );
	pdc->MoveTo( m_zero.x, rc.top );
	pdc->LineTo( m_zero.x, rc.bottom );

	CRect circle = m_rcAll;
	circle.DeflateRect( m_gripperRadius );

	CSelectStockObject sobrush( pdc, NULL_BRUSH );
	pdc->Ellipse( circle );

	if ( GetFocus( ) == this ) {
		pdc->DrawFocusRect( m_rcAll );
		}
	}

void CXySlider::PaintGripper( _In_ CDC *pdc ) {
	ASSERT_VALID( pdc );
	CRect rc = GetGripperRect( );

	COLORREF color = GetSysColor( COLOR_BTNFACE );
	if ( m_gripperHighlight ) {
		INT r = GetRValue( color );
		INT g = GetGValue( color );
		INT b = GetBValue( color );;
		r += ( 255 - r ) / 3;
		g += ( 255 - g ) / 3;
		b += ( 255 - b ) / 3;
		color = RGB( r, g, b );
		}
	pdc->FillSolidRect( rc, color );
	pdc->DrawEdge( rc, EDGE_RAISED, BF_RECT );

	CPen pen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	CSelectObject sopen( pdc, &pen );

	pdc->MoveTo( rc.left, rc.top + rc.Height( ) / 2 );
	pdc->LineTo( rc.right, rc.top + rc.Height( ) / 2 );
	pdc->MoveTo( rc.left + rc.Width( ) / 2, rc.top );
	pdc->LineTo( rc.left + rc.Width( ) / 2, rc.bottom );
	}

void CXySlider::DoMoveBy( _In_ const INT cx, _In_ const INT cy ) {
	m_pos.x += cx;
	CheckMinMax( m_pos.x, -m_range.cx, m_range.cx );

	m_pos.y += cy;
	CheckMinMax( m_pos.y, -m_range.cy, m_range.cy );

	RedrawWindow( );

	CPoint oldpos = m_externalPos;
	InternToExtern( );
	if ( m_externalPos != oldpos ) {
		NotifyParent( );
		}
	}

void CXySlider::DoDrag( _In_ CPoint point ) {
	CPoint pt0 = point;

	HighlightGripper( true );

	CSize inGripper = pt0 - GetGripperRect( ).CenterPoint( );
	CPoint ptMin( m_zero - m_range + inGripper );
	CPoint ptMax( m_zero + m_range + inGripper );

	SetCapture( );
	for(;;) {
		MSG msg;
		if ( !GetMessage( &msg, NULL, 0, 0 ) ) {
			break;
			}

		if ( msg.message == WM_LBUTTONUP ) {
			break;
			}

		if ( GetCapture( ) != this ) {
			break;
			}

		if (msg.message == WM_MOUSEMOVE) {
			CPoint pt = msg.pt;
			ScreenToClient( &pt );

			CheckMinMax( pt.x, ptMin.x, ptMax.x );
			CheckMinMax( pt.y, ptMin.y, ptMax.y );

			INT dx = pt.x - pt0.x;
			INT dy = pt.y - pt0.y;

			DoMoveBy( dx, dy );

			pt0 = pt;
			}
		else {
			DispatchMessage( &msg );
			}
		}
	ReleaseCapture( );

	HighlightGripper( false );
	}

void CXySlider::DoPage( _In_ CPoint point ) {
	CSize sz = point - ( m_zero + m_pos );

	ASSERT( sz.cx != 0 || sz.cy != 0 );

	const DOUBLE len = sqrt( ( DOUBLE ) ( sz.cx * sz.cx + sz.cy * sz.cy ) );

	const DOUBLE d = 10;

	INT dx = ( INT ) ( d * sz.cx / len );
	INT dy = ( INT ) ( d * sz.cy / len );

	DoMoveBy( dx, dy );
	}

void CXySlider::HighlightGripper( _In_ bool on ) {
	m_gripperHighlight = on;
	RedrawWindow( );
	}

void CXySlider::InstallTimer( ) {
	RemoveTimer( );
	m_timer = SetTimer( 4711, 500, NULL );
	}

void CXySlider::RemoveTimer( ) {
	if ( m_timer != 0 ) {
		KillTimer( m_timer );
		}
	m_timer = 0;
	}


BEGIN_MESSAGE_MAP(CXySlider, CStatic)
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_NCHITTEST()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_MESSAGE(CXySlider::XY_SETPOS, OnSetPos)
	ON_MESSAGE(CXySlider::XY_GETPOS, OnGetPos)
END_MESSAGE_MAP()

void CXySlider::OnDestroy( ) {
	RemoveTimer();
	CStatic::OnDestroy();
	}

UINT CXySlider::OnGetDlgCode( ) {
	return DLGC_WANTARROWS;
	}

LRESULT CXySlider::OnNcHitTest( CPoint /*point*/ ) {
	return HTCLIENT;
	}

void CXySlider::OnSetFocus( CWnd* pOldWnd ) {
	CStatic::OnSetFocus( pOldWnd );
	Invalidate( );
	}

void CXySlider::OnKillFocus( CWnd* pNewWnd ) {
	CStatic::OnKillFocus( pNewWnd );
	Invalidate( );
	}

void CXySlider::OnPaint( ) {
	Initialize( );
	INT w = m_rcAll.Width( );
	INT h = m_rcAll.Height( );

	CPaintDC dc( this );
	CDC dcmem;
	dcmem.CreateCompatibleDC( &dc );
	CBitmap bm;
	bm.CreateCompatibleBitmap( &dc, w, h );
	CSelectObject sobm( &dcmem, &bm );

	PaintBackground( &dcmem );
	// PaintValues(&dcmem); This is too noisy
	PaintGripper( &dcmem );

	dc.BitBlt( 0, 0, w, h, &dcmem, 0, 0, SRCCOPY );
	}

void CXySlider::OnKeyDown( UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/ ) {
	switch (nChar)
	{
		case VK_LEFT:	DoMoveBy(-1, 0); break;
		case VK_RIGHT:	DoMoveBy(1, 0); break;
		case VK_UP:		DoMoveBy(0, -1); break;
		case VK_DOWN:	DoMoveBy(0, 1); break;
	}
	}

void CXySlider::OnLButtonDown( UINT /*nFlags*/, CPoint point ) {
	SetFocus( );

	CRect rc = GetGripperRect( );

	if ( rc.PtInRect( point ) ) {
		DoDrag( point );
		}
	else {
		DoPage( point );
		InstallTimer( );
		}
	}

void CXySlider::OnLButtonDblClk( UINT /*nFlags*/, CPoint point ) {
	SetFocus( );

	if ( GetGripperRect( ).PtInRect( point ) ) {
		DoMoveBy( -m_pos.x, -m_pos.y );
		}
	else {
		DoPage( point );
		InstallTimer( );
		}
	}

void CXySlider::OnLButtonUp( UINT nFlags, CPoint point ) {
	RemoveTimer( );
	CStatic::OnLButtonUp( nFlags, point );
	}

void CXySlider::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	CRect rc = GetGripperRect( );
	if ( !rc.PtInRect( point ) ) {
		DoPage( point );
		}
	}

// $Log$
// Revision 1.4  2004/11/07 00:06:34  assarbad
// - Fixed minor bug with ampersand (details in changelog.txt)
//
// Revision 1.3  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
