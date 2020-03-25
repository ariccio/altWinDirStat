// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once

#ifndef WDS_COLORBUTTON_CPP
#define WDS_COLORBUTTON_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "colorbutton.h"
#include "globalhelpers.h"
#include "hwnd_funcs.h"

BEGIN_MESSAGE_MAP( CColorButton, CButton )
	ON_WM_PAINT( )
	ON_WM_DESTROY( )
	ON_CONTROL_REFLECT( BN_CLICKED, &( CColorButton::OnBnClicked ) )
	ON_WM_ENABLE( )
END_MESSAGE_MAP( )

void CPreview::SetColor(_In_ const COLORREF color) noexcept {
	m_color = color;
	hwnd::InvalidateErase(m_hWnd, false, __FUNCTION__);
	}

LRESULT CPreview::OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ ) {
	//CPaintDC dc( this );
	PAINTSTRUCT ps = { 0 };

	HDC hDC = ATL::CWindow::BeginPaint( &ps );
	ASSERT( hDC != nullptr);

	RECT rc;

	//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	ASSERT( ::IsWindow( m_hWnd ) );

	//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
	//Return value: If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	VERIFY( ::GetClientRect( m_hWnd, &rc ) );

	//DrawEdge function: https://msdn.microsoft.com/en-us/library/dd162477.aspx
	//If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	VERIFY( ::DrawEdge( hDC, &rc, EDGE_BUMP, BF_RECT bitor BF_ADJUST ) );

	auto color_scope_holder = m_color;

	//GetParent function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633510.aspx
	//If the window is a child window, the return value is a handle to the parent window.
	//If the window is a top-level window with the WS_POPUP style, the return value is a handle to the owner window.
	//If the function fails, the return value is NULL.
	//To get extended error information, call GetLastError.
	const HWND parent = ::GetParent( m_hWnd );
	ASSERT( parent != NULL );

	WINDOWINFO window_info = { };
	window_info.cbSize = sizeof( WINDOWINFO );

	//GetWindowInfo function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633516.aspx
	//If [GetWindowInfo] succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	const BOOL window_info_res = ::GetWindowInfo( parent, &window_info );
	if ( window_info_res == 0 ) {
		const DWORD err = ::GetLastError( );
		TRACE( _T( "GetWindowInfo failed! Error: %u\r\n" ), static_cast<unsigned>( err ) );
		displayWindowsMsgBoxWithError( err );
		const auto color = color_scope_holder;

//		//SetBkColor function: https://msdn.microsoft.com/en-us/library/dd162964.aspx
//		//If the [SetBkColor] succeeds, the return value specifies the previous background color as a COLORREF value.
//		//If [SetBkColor] fails, the return value is CLR_INVALID.
//		const auto set_color_res_1 = ::SetBkColor( hDC, color );
//		ASSERT( set_color_res_1 != CLR_INVALID );
//#ifndef DEBUG
//		UNREFERENCED_PARAMETER( set_color_res_1 );
//#endif
//		//ExtTextOut function: https://msdn.microsoft.com/en-us/library/dd162713.aspx
//		//If the string is drawn, the return value [of ExtTextOutW] is nonzero.
//		//However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
//		//If the function fails, the return value is zero.
//		VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

		fill_solid_RECT( hDC, &rc, color );

		ATL::CWindow::EndPaint( &ps );
		return 0;
		}

	if ( ( window_info.dwStyle bitand WS_DISABLED ) != 0 ) {
		//GetSysColor function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724371.aspx
		//The function returns the red, green, blue (RGB) color value of the given element.
		//If the nIndex parameter is out of range, the return value is zero.
			//Because zero is also a valid RGB value, you cannot use GetSysColor to determine whether a system color is supported by the current platform.
			//Instead, use the GetSysColorBrush function, which returns NULL if the color is not supported.
		color_scope_holder = ::GetSysColor( COLOR_BTNFACE );
		}
	const auto color = color_scope_holder;
			
//	//SetBkColor function: https://msdn.microsoft.com/en-us/library/dd162964.aspx
//	//If the function succeeds, the return value specifies the previous background color as a COLORREF value.
//	//If [SetBkColor] fails, the return value is CLR_INVALID.
//	const auto set_color_res_2 = ::SetBkColor( hDC, color );
//	ASSERT( set_color_res_2 != CLR_INVALID );
//#ifndef DEBUG
//	UNREFERENCED_PARAMETER( set_color_res_2 );
//#endif
//
//	//ExtTextOut function: https://msdn.microsoft.com/en-us/library/dd162713.aspx
//	//If the string is drawn, the return value [of ExtTextOutW] is nonzero.
//	//However, if the ANSI version of ExtTextOut is called with ETO_GLYPH_INDEX, the function returns TRUE even though the function does nothing.
//	//If the function fails, the return value is zero.
//	VERIFY( ::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL ) );

	fill_solid_RECT( hDC, &rc, color );

	EndPaint( &ps );
	return 0;
	}

afx_msg void CColorButton::OnEnable(const BOOL bEnable) noexcept {
	hwnd::InvalidateErase(m_preview.m_hWnd);
	/*
	_AFXWIN_INLINE void CWnd::OnEnable(BOOL)
	{ Default(); }
		*/

	CWnd::OnEnable(bEnable);
	}
#endif