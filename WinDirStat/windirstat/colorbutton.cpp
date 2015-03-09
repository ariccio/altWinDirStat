// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_COLORBUTTON_CPP
#define WDS_COLORBUTTON_CPP

#pragma message( "Including `" __FILE__ "`..." )

#include "colorbutton.h"
#include "globalhelpers.h"

BEGIN_MESSAGE_MAP( CColorButton, CButton )
	ON_WM_PAINT( )
	ON_WM_DESTROY( )
	ON_CONTROL_REFLECT( BN_CLICKED, &( CColorButton::OnBnClicked ) )
	ON_WM_ENABLE( )
END_MESSAGE_MAP( )


LRESULT CPreview::OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ ) {
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
	const HWND parent = ::GetParent( m_hWnd );
			
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




#else

#endif