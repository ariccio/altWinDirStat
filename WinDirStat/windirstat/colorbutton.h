// colorbutton.h - Declaration of CColorButton
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#ifndef WDS_COLORBUTTON_H
#define WDS_COLORBUTTON_H


#include "stdafx.h"
#include "globalhelpers.h"

#define COLBN_CHANGED	0x87	// this is a value, I hope, that is nowhere used as notification code.


// The color preview is an own little child window of the button.
class CPreview final : public ATL::CWindowImpl<CPreview> {
public:
	COLORREF m_color;

	CPreview& operator=( const CPreview& in ) = delete;
	CPreview( const CPreview& in ) = delete;

	CPreview( ) : m_color{ 0u } { }
	void SetColor( _In_ const COLORREF color ) {
		m_color = color;
		if ( IsWindow( ) ) {
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
			//InvalidateRect( NULL );
			}
		}

#pragma warning( push )
#pragma warning( disable: 4365 )
	BEGIN_MSG_MAP(CPreview)
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_PAINT ), ( CPreview::OnPaint ) )
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_LBUTTONDOWN ), ( CPreview::OnLButtonDown ) )
		DEFAULT_REFLECTION_HANDLER( )
	END_MSG_MAP()
#pragma warning( pop )

	LRESULT OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ ) {
		//CPaintDC dc( this );
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint( &ps );

		RECT rc;
		ASSERT( ::IsWindow( m_hWnd ) );

		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );
		//GetClientRect( &rc );
		//VERIFY( dc.DrawEdge( &rc, EDGE_BUMP, BF_RECT bitor BF_ADJUST ) );
		/*
_AFXWIN_INLINE BOOL CDC::DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags)
{ ASSERT(m_hDC != NULL); return ::DrawEdge(m_hDC, lpRect, nEdge, nFlags); }
		*/

		VERIFY( DrawEdge( hDC, &rc, EDGE_BUMP, BF_RECT bitor BF_ADJUST ) );

		auto color_scope_holder = m_color;
		const HWND parent = GetParent( );
			
		auto window_info = zero_init_struct<WINDOWINFO>( );
		window_info.cbSize = sizeof( WINDOWINFO );

			
		//If [GetWindowInfo] succeeds, the return value is nonzero.
		const BOOL window_info_res = ::GetWindowInfo( parent, &window_info );
		if ( window_info_res == 0 ) {
			const DWORD err = GetLastError( );
			TRACE( _T( "GetWindowInfo failed! Error: %u\r\n" ), unsigned( err ) );
			displayWindowsMsgBoxWithError( err );
			const auto color = color_scope_holder;

			//dc.FillSolidRect( &rc, color );
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

			//If [SetBkColor] fails, the return value is CLR_INVALID.
			const auto set_color_res_1 = ::SetBkColor( hDC, color );
			ASSERT( set_color_res_1 != CLR_INVALID );
#ifndef DEBUG
			UNREFERENCED_PARAMETER( set_color_res_1 );
#endif

			//If the string is drawn, the return value [of ExtTextOutW] is nonzero. However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
			VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

			EndPaint( &ps );
			return 0;
			}

		if ( ( window_info.dwStyle bitand WS_DISABLED ) != 0 ) {
			color_scope_holder = GetSysColor( COLOR_BTNFACE );
			}
		const auto color = color_scope_holder;
			
		//dc.FillSolidRect( &rc, color );
		//If [SetBkColor] fails, the return value is CLR_INVALID.
		const auto set_color_res_2 = ::SetBkColor( hDC, color );
		ASSERT( set_color_res_2 != CLR_INVALID );
#ifndef DEBUG
		UNREFERENCED_PARAMETER( set_color_res_2 );
#endif

		VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

		EndPaint( &ps );
		return 0;
		}

	//UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/
	//UINT nFlags, CPoint point
	LRESULT OnLButtonDown( UINT /*nMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ ) {
		const UINT nFlags = static_cast< UINT >( wParam );
		//const POINT point = static_cast< POINT >( lParam );
		POINT point_temp;
		point_temp.x = GET_X_LPARAM( lParam );
		point_temp.y = GET_Y_LPARAM( lParam );
		POINT point = point_temp;
		ASSERT( ::IsWindow( m_hWnd ) );
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::ClientToScreen( m_hWnd, &point ) );
		//ClientToScreen( &point );

		const auto this_parent = GetParent( );

		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::ScreenToClient( this_parent.m_hWnd, &point ) );
			
		//this_parent->ScreenToClient( &point );

		TRACE( _T( "User clicked x:%ld, y:%ld! Sending WM_LBUTTONDOWN!\r\n" ), point.x, point.y );
			
		/*
		_AFXWIN_INLINE CWnd* CWnd::GetParent() const
		{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::GetParent(m_hWnd)); }	
		*/
		/*
_AFXWIN_INLINE LRESULT CWnd::_AFX_FUNCNAME(SendMessage)(UINT message, WPARAM wParam, LPARAM lParam) const
{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, message, wParam, lParam); }
#pragma push_macro("SendMessage")
#undef SendMessage
_AFXWIN_INLINE LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{ return _AFX_FUNCNAME(SendMessage)(message, wParam, lParam); }
#pragma pop_macro("SendMessage")

		*/
		//this_parent->SendMessageW( WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
			
		const HWND parent = GetParent( );
		::SendMessageW( parent, WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
		return 0;
	}
	};


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

			VERIFY( m_preview.Create( m_hWnd, rc, _T( "" ), WS_CHILD | WS_VISIBLE, 0, 4711, NULL ) );

			VERIFY( CWnd::ModifyStyle( 0, WS_CLIPCHILDREN ) );
			}
		CButton::OnPaint( );
		}

	afx_msg void OnDestroy( ) {
		if ( ::IsWindow( m_preview.m_hWnd ) ) {
			VERIFY( m_preview.DestroyWindow( ) );
			}
		CButton::OnDestroy( );
		}

	afx_msg void OnBnClicked( ) {
		WTL::CColorDialog dlg( m_preview.m_color );
		if ( IDOK == dlg.DoModal( ) ) {
			m_preview.SetColor( dlg.GetColor( ) );
			NMHDR hdr;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = static_cast<UINT_PTR>( GetDlgCtrlID( ) );
			hdr.code = COLBN_CHANGED;
			TRACE( _T( "Color button clicked! Sending WM_NOTIFY to Dialog with Ctrl ID: %llu\r\n" ), static_cast<ULONGLONG>( hdr.idFrom ) );
			CWnd::GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( GetDlgCtrlID( ) ), ( LPARAM ) &hdr );
			}
		}

	afx_msg void OnEnable( const BOOL bEnable ) {
		if ( ::IsWindow( m_preview.m_hWnd ) ) {

			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_preview.m_hWnd, NULL, TRUE ) );
			//m_preview.InvalidateRect( NULL );
			
			}
		CButton::OnEnable( bEnable );
		}
	};

#else


#endif
