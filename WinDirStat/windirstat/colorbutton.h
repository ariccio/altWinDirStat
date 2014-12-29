// colorbutton.h - Declaration of CColorButton
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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"

#define COLBN_CHANGED	0x87	// this is a value, I hope, that is nowhere used as notification code.

//
// CColorButton. A Pushbutton which allows to choose a color and shows this color on its surface.
// In the resource editor, the button should be set to "right align text", as the color will be shown in the left third.
// When the user chose a color, the parent is notified via WM_NOTIFY and the notification code COLBN_CHANGED.
//
class CColorButton : public CButton {
public:

	CColorButton( ) { }
	CColorButton& operator=( const CColorButton& in ) = delete;
	CColorButton( const CColorButton& in ) = delete;
	// The color preview is an own little child window of the button.
	class CPreview: public CWnd {
	public:
		CPreview& operator=( const CPreview& in ) = delete;
		CPreview( const CPreview& in ) = delete;

		CPreview( ) : m_color( 0 ) { }
		void SetColor( _In_ const COLORREF color ) {
			m_color = color;
			if ( IsWindow( m_hWnd ) ) {
				InvalidateRect( NULL );
				}
			}
		COLORREF m_color;

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint( ) {
			CPaintDC dc( this );
			RECT rc;
			GetClientRect( &rc );

			VERIFY( dc.DrawEdge( &rc, EDGE_BUMP, BF_RECT bitor BF_ADJUST ) );

			auto color = m_color;
			if ( ( GetParent( )->GetStyle( ) bitand WS_DISABLED ) != 0 ) {
				color = GetSysColor( COLOR_BTNFACE );
				}
			dc.FillSolidRect( &rc, color );
			}

		afx_msg void OnLButtonDown( UINT nFlags, CPoint point ) {
			ClientToScreen( &point );
			GetParent( )->ScreenToClient( &point );
			TRACE( _T( "User clicked x:%ld, y:%ld! Sending WM_LBUTTONDOWN!\r\n" ), point.x, point.y );
			
			/*
			_AFXWIN_INLINE CWnd* CWnd::GetParent() const
			{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::GetParent(m_hWnd)); }	
			*/
			GetParent( )->SendMessageW( WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
		}
		};

	CPreview m_preview;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint( ) {
		if ( m_preview.m_hWnd == NULL ) {
			RECT rc;
			GetClientRect( &rc );

			rc.right = rc.left + ( rc.right - rc.left ) / 3;
			//rc.DeflateRect( 4, 4 );
			VERIFY( ::InflateRect( &rc, -4, -4 ) );

			VERIFY( m_preview.Create( AfxRegisterWndClass( 0, 0, 0, 0 ), _T( "" ), WS_CHILD | WS_VISIBLE, rc, this, 4711 ) );

			VERIFY( ModifyStyle( 0, WS_CLIPCHILDREN ) );
			}
		CButton::OnPaint( );
		}

	afx_msg void OnDestroy( ) {
		if ( IsWindow( m_preview.m_hWnd ) ) {
			VERIFY( m_preview.DestroyWindow( ) );
			}
		CButton::OnDestroy( );
		}

	afx_msg void OnBnClicked( ) {
		CColorDialog dlg( m_preview.m_color );
		if ( IDOK == dlg.DoModal( ) ) {
			m_preview.SetColor( dlg.GetColor( ) );
			NMHDR hdr;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = static_cast<UINT_PTR>( GetDlgCtrlID( ) );
			hdr.code = COLBN_CHANGED;
			TRACE( _T( "Color button clicked! Sending WM_NOTIFY to Dialog with Ctrl ID: %llu\r\n" ), static_cast<ULONGLONG>( hdr.idFrom ) );
			GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), ( LPARAM ) &hdr );
			}
		}

	afx_msg void OnEnable( const BOOL bEnable ) {
		if ( IsWindow( m_preview.m_hWnd ) ) {
			m_preview.InvalidateRect( NULL );
			}
		CButton::OnEnable( bEnable );
		}
	};





// $Log$
// Revision 1.3  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
