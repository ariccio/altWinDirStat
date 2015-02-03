// colorbutton.h - Declaration of CColorButton
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#ifndef WDS_COLORBUTTON_H
#define WDS_COLORBUTTON_H


#include "stdafx.h"

#define COLBN_CHANGED	0x87	// this is a value, I hope, that is nowhere used as notification code.

//
// CColorButton. A Pushbutton which allows to choose a color and shows this color on its surface.
// In the resource editor, the button should be set to "right align text", as the color will be shown in the left third.
// When the user chose a color, the parent is notified via WM_NOTIFY and the notification code COLBN_CHANGED.
//
class CColorButton final : public CButton {
public:

	CColorButton( ) = default;
	CColorButton& operator=( const CColorButton& in ) = delete;
	CColorButton( const CColorButton& in ) = delete;
	// The color preview is an own little child window of the button.
	class CPreview final : public CWnd {
	public:
		COLORREF m_color;

		CPreview& operator=( const CPreview& in ) = delete;
		CPreview( const CPreview& in ) = delete;

		CPreview( ) : m_color{ 0u } { }
		void SetColor( _In_ const COLORREF color ) {
			m_color = color;
			if ( IsWindow( m_hWnd ) ) {
				//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
				VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
				//InvalidateRect( NULL );
				}
			}

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint( ) {
			CPaintDC dc( this );
			RECT rc;
			ASSERT( ::IsWindow( m_hWnd ) );

			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
			VERIFY( ::GetClientRect( m_hWnd, &rc ) );
			//GetClientRect( &rc );

			VERIFY( dc.DrawEdge( &rc, EDGE_BUMP, BF_RECT bitor BF_ADJUST ) );

			auto color_scope_holder = m_color;
			if ( ( GetParent( )->GetStyle( ) bitand WS_DISABLED ) != 0 ) {
				color_scope_holder = GetSysColor( COLOR_BTNFACE );
				}
			const auto color = color_scope_holder;
			dc.FillSolidRect( &rc, color );
			}

		afx_msg void OnLButtonDown( UINT nFlags, CPoint point ) {
			ASSERT( ::IsWindow( m_hWnd ) );
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::ClientToScreen( m_hWnd, &point ) );
			//ClientToScreen( &point );

			const auto this_parent = GetParent( );

			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::ScreenToClient( this_parent->m_hWnd, &point ) );
			//this_parent->ScreenToClient( &point );

			TRACE( _T( "User clicked x:%ld, y:%ld! Sending WM_LBUTTONDOWN!\r\n" ), point.x, point.y );
			
			/*
			_AFXWIN_INLINE CWnd* CWnd::GetParent() const
			{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::GetParent(m_hWnd)); }	
			*/
			this_parent->SendMessageW( WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
		}
		};

	//C4820: 'CColorButton::CPreview' : '4' bytes padding added after data member 'CColorButton::CPreview::m_color'

	CPreview m_preview;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint( ) {
		if ( m_preview.m_hWnd == NULL ) {
			RECT rc;
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
			VERIFY( ::GetClientRect( m_hWnd, &rc ) );
			//GetClientRect( &rc );

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

			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_preview.m_hWnd, NULL, TRUE ) );
			//m_preview.InvalidateRect( NULL );
			
			}
		CButton::OnEnable( bEnable );
		}
	};

#else


#endif
