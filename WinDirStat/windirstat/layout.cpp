// layout.cpp	- Implementation of CLayout
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

#pragma once

#include "stdafx.h"

#include "options.h"
#include "layout.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#ifndef WDS_LAYOUT_CPP
#define WDS_LAYOUT_CPP

CLayout::CLayout( _In_ CWnd* dialog, _In_z_ PCWSTR name ) : m_dialog( dialog ), m_name( name ) {
	ASSERT( m_dialog != NULL );
	
	// This is necessary because OnGetMinMaxInfo() will be called before OnInitDialog!
	m_originalDialogSize.cx = 0;
	m_originalDialogSize.cy = 0;
	}

//size_t CLayout::AddControl( _In_ CWnd* control, _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy ) {
//	m_control.emplace_back( SControlInfo { control, movex, movey, stretchx, stretchy, CRect( ) } );
//	return m_control.size( );
//	}

void CLayout::AddControl( _In_ const UINT id, _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy ) {
	//AddControl( m_dialog->GetDlgItem( INT( id ) ), movex, movey, stretchx, stretchy );
	m_control.emplace_back( SControlInfo { m_dialog->GetDlgItem( static_cast<INT>( id ) ), movex, movey, stretchx, stretchy, CRect( ) } );
	}

void CLayout::OnInitDialog( _In_ const bool centerWindow ) {
	//m_dialog->SetIcon( GetApp( )->LoadIcon( IDR_MAINFRAME ), false );

	CRect rcDialog;
	m_dialog->GetWindowRect( rcDialog );
	m_originalDialogSize = rcDialog.Size( );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto& aControl : m_control ) {
		RECT rc;
		aControl.control->GetWindowRect( &rc );
		m_dialog->ScreenToClient( &rc );
		aControl.originalRectangle = rc;
		}

	CRect sg;
	m_dialog->GetClientRect( sg );
	sg.left = sg.right  - m_sizeGripper._width;
	sg.top  = sg.bottom - m_sizeGripper._width;
	m_sizeGripper.Create( m_dialog, sg );

	m_control.emplace_back( SControlInfo { &m_sizeGripper, 1, 1, 0, 0, sg } );

	CPersistence::GetDialogRectangle( m_name, rcDialog );
	m_dialog->MoveWindow( rcDialog );
	if ( centerWindow ) {
		m_dialog->CenterWindow( );
		}
	}

CLayout::SControlInfo::SControlInfo( CWnd* control_in, DOUBLE movex_in, DOUBLE movey_in, DOUBLE stretchx_in, DOUBLE stretchy_in, RECT originalRectangle_in ) : control( control_in ), movex( std::move( movex_in ) ), movey( std::move( movey_in ) ), stretchx ( std::move( stretchx_in ) ), stretchy( std::move( stretchy_in ) ), originalRectangle( std::move( originalRectangle_in ) ) { }

void CLayout::OnDestroy( ) {
	CRect rc;
	m_dialog->GetWindowRect( rc );
	CPersistence::SetDialogRectangle( m_name, rc );
	}

void CLayout::OnSize( ) {
	CRect rc_outer;
	m_dialog->GetWindowRect( rc_outer );
	auto newDialogSize = rc_outer.Size( );
	auto  diff = newDialogSize - m_originalDialogSize;
	// The DeferWindowPos-stuff prevents the controls from overwriting each other.
	auto hdwp = BeginDeferWindowPos( static_cast<int>( m_control.size( ) ) );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( auto& aControl : m_control ) {
		auto rc = CRect( aControl.originalRectangle );
		const auto movex = static_cast<int>( aControl.movex );
		const auto movey = static_cast<int>( aControl.movey );
		WTL::CSize move( diff.cx * movex, diff.cy * movey );
		CRect stretch( 0, 0, diff.cx * static_cast<int>( aControl.stretchx ), diff.cy * static_cast<int>( aControl.stretchy ) );
		rc += move;
		rc += stretch;
		hdwp = DeferWindowPos( hdwp, *( aControl.control ), NULL, rc.left, rc.top, rc.Width( ), rc.Height( ), SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
	
	VERIFY( EndDeferWindowPos( hdwp ) );
	}

void CLayout::OnGetMinMaxInfo( _Out_ MINMAXINFO* mmi ) const {
	mmi->ptMinTrackSize.x = m_originalDialogSize.cx;
	mmi->ptMinTrackSize.y = m_originalDialogSize.cy;
	}


/////////////////////////////////////////////////////////////////////////////

const INT CLayout::CSizeGripper::_width = 14;

//CLayout::CSizeGripper::CSizeGripper( ) { }

void CLayout::CSizeGripper::Create( _Inout_ CWnd* parent, _In_ const RECT rc ) {
	VERIFY( CWnd::Create( AfxRegisterWndClass( 0, AfxGetApp( )->LoadStandardCursor( IDC_ARROW ), ( HBRUSH ) ( COLOR_BTNFACE + 1 ), 0 ), _T( "" ), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, rc, parent, IDC_SIZEGRIPPER ) );
	}

BEGIN_MESSAGE_MAP(CLayout::CSizeGripper, CWnd)
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

void CLayout::CSizeGripper::OnPaint( ) {
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( rc );

	ASSERT( rc.Width( ) == _width );
	ASSERT( rc.Height( ) == _width );

	WTL::CPoint start;
	WTL::CPoint end;

	start.x = 1;
	start.y = _width;
	end.x   = _width;
	end.y   = 1;

	DrawShadowLine( dc, start, end );

	start.x += 4;
	end.y   += 4;

	DrawShadowLine( dc, start, end );

	start.x += 4;
	end.y   += 4;

	DrawShadowLine( dc, start, end );

	// Do not call CWnd::OnPaint() for painting messages
	}

void CLayout::CSizeGripper::DrawShadowLine( _In_ CDC& pdc, _In_ WTL::CPoint start, _In_ WTL::CPoint end ) {
	//ASSERT_VALID( pdc );
	CPen lightPen( PS_SOLID, 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
	CSelectObject sopen( pdc, lightPen );

	pdc.MoveTo( start );
	VERIFY( pdc.LineTo( end ) );

	start.x++;
	end.y++;

	CPen darkPen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	CSelectObject sopen2( pdc, darkPen );

	pdc.MoveTo( start );
	VERIFY( pdc.LineTo( end ) );

	start.x++;
	end.y++;

	pdc.MoveTo( start );
	VERIFY( pdc.LineTo( end ) );
	}

LRESULT CLayout::CSizeGripper::OnNcHitTest( CPoint point ) {
	ScreenToClient( &point );
	if ( point.x + point.y >= _width ) {
		return HTBOTTOMRIGHT;
		}
	return 0;
	}

// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
#else

#endif