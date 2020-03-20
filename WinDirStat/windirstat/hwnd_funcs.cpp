//
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once

#include "stdafx.h"
#include "hwnd_funcs.h"

#ifndef WDS_HWND_FUNCS_CPP
WDS_FILE_INCLUDE_MESSAGE

void hwnd::InvalidateErase(_In_opt_ const HWND hWnd) noexcept {
	/*
		From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afxwin.h:2378:
			void Invalidate(BOOL bErase = TRUE);
		From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afxwin2.inl:113:
			_AFXWIN_INLINE void CWnd::Invalidate(BOOL bErase)
				{ ASSERT(::IsWindow(m_hWnd)); ::InvalidateRect(m_hWnd, NULL, bErase); }
	*/

	//IsWindow function: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-iswindow
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	const BOOL is_window = ::IsWindow(hWnd);
	ASSERT(is_window);
	if (!is_window) {
		TRACE(L"InvalidateErase called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate( );
		}

	//InvalidateRect function: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-invalidaterect
	//[hWnd is a handle] to the window whose update region has changed.
	//	If this parameter is NULL, the system invalidates and redraws all windows, not just the windows for this application, and sends the WM_ERASEBKGND and WM_NCPAINT messages before the function returns. Setting this parameter to NULL is not recommended.
	if (hWnd == nullptr) {
		TRACE(L"Setting hWnd to NULL is not recommended.\r\n");
		}

	//The InvalidateRect function adds a rectangle to the specified window's update region. The update region represents the portion of the window's client area that must be redrawn.
	//If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	const BOOL invalidate_result = ::InvalidateRect(hWnd, nullptr, TRUE);
	if (!invalidate_result) {
		TRACE(L"InvalidateRect on `%p` failed!\r\n", hWnd);
		std::terminate( );
		}
	}

void hwnd::RedrawWindow(_In_ const HWND hWnd) noexcept {
	//	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
	//		CRgn* prgnUpdate = NULL,
	//		UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	/*
	_AFXWIN_INLINE BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate, CRgn* prgnUpdate,
	UINT flags)
	{ ASSERT(::IsWindow(m_hWnd)); return ::RedrawWindow(m_hWnd, lpRectUpdate, (HRGN)prgnUpdate->GetSafeHandle(), flags); }
	*/
	ASSERT(::IsWindow(hWnd));
	//RedrawWindow function: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-redrawwindow
	//If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	constexpr const UINT mfc_default_flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE;
	const BOOL redraw_success = ::RedrawWindow(hWnd, NULL, NULL, mfc_default_flags);
	if (redraw_success == 0) {
		std::terminate();
		}
	}



#endif

