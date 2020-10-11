//
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once

#include "stdafx.h"
#include "hwnd_funcs.h"
#include "globalhelpers.h"

#ifndef WDS_HWND_FUNCS_CPP
WDS_FILE_INCLUDE_MESSAGE

namespace {
	[[noreturn]] void should_never_happen_endpaint() noexcept {
		// EndPaint function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
		// The return value is always nonzero.
		displayWindowsMsgBoxWithMessage(L"The return value for EndPaint should always be nonzero. You've hit a point in the program where somehow, EndPaint returned zero. This is undefined state. Bye bye, program will terminate now.\r\n");
		std::terminate();
	}
}


// Sometimes, we don't care.
void hwnd::InvalidateErase(_In_opt_ const HWND hWnd, _In_opt_ const bool do_we_care, _In_opt_z_ PCSTR source) noexcept {
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
	if (!is_window) {
		if (do_we_care) {
			TRACE(L"InvalidateErase called on invalid window HWND: `%p`!\r\n", hWnd);
			std::terminate();
			}
		else if (hWnd != nullptr) {
			TRACE(L"Source: %S\r\n", source);
			}
		}

	//InvalidateRect function: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-invalidaterect
	//[hWnd is a handle] to the window whose update region has changed.
	//	If this parameter is NULL, the system invalidates and redraws all windows, not just the windows for this application, and sends the WM_ERASEBKGND and WM_NCPAINT messages before the function returns. Setting this parameter to NULL is not recommended.
	if (hWnd == nullptr) {
		TRACE(L"Setting hWnd to NULL is not recommended.\r\n");
		}

	//The InvalidateRect function adds a rectangle to the specified window's update region. The update region represents the portion of the window's client area that must be redrawn.
	//If [InvalidateRect] succeeds, the return value is nonzero.
	//If [InvalidateRect] fails, the return value is zero.
	const BOOL invalidate_result = ::InvalidateRect(hWnd, nullptr, TRUE);
	if (!invalidate_result) {
		TRACE(L"InvalidateRect on `%p` failed!\r\n", hWnd);
		if (do_we_care) {
			std::terminate();
			}
		TRACE(L"Source: %S\r\n", source);
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
	//If [RedrawWindow] succeeds, the return value is nonzero.
	//If [RedrawWindow] fails, the return value is zero.
	constexpr const UINT mfc_default_flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE;
	const BOOL redraw_success = ::RedrawWindow(hWnd, NULL, NULL, mfc_default_flags);
	if (redraw_success == 0) {
		std::terminate();
		}
	}

void hwnd::ScreenToClient(_In_ const HWND hWnd, _Inout_ POINT* const point) noexcept {
	//IsWindow function: https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-iswindow
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	const BOOL is_window = ::IsWindow(hWnd);
	if (!is_window) {
		TRACE(L"ScreenToClient called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate();
	}

	// ScreenToClient function: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-screentoclient
	// If [ScreenToClient] succeeds, the return value is nonzero.
	// If [ScreenToClient] fails, the return value is zero.
	const BOOL screenToClientResult = ::ScreenToClient(hWnd, point);
	if (screenToClientResult == 0) {
		std::terminate();
		}
	}
void hwnd::ScreenToClient(_In_ const HWND hWnd, _Inout_ RECT* const rect) noexcept {
	/*
	When we give it a RECT, MFC decides evil pointer arithmetic is fine because they suck
	void CWnd::ScreenToClient(LPRECT lpRect) const
	{
		ASSERT(::IsWindow(m_hWnd));
		::ScreenToClient(m_hWnd, (LPPOINT)lpRect);
		::ScreenToClient(m_hWnd, ((LPPOINT)lpRect)+1);
		if (GetExStyle() & WS_EX_LAYOUTRTL)
			CRect::SwapLeftRight(lpRect);
	}

	For reference:
	inline void WINAPI CRect::SwapLeftRight(_Inout_ LPRECT lpRect) throw()
	{
		LONG temp = lpRect->left;
		lpRect->left = lpRect->right;
		lpRect->right = temp;
	}

	DWORD CWnd::GetExStyle() const
	{
		ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

		if (m_pCtrlSite == NULL)
			return (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE);
		else
			return m_pCtrlSite->GetExStyle();
	}
	...but I don't think I'm going to apply the WS_EX_LAYOUTRTL style to anything, I don't support RTL languages right now.

	A RECT is defined as such:
	typedef struct tagRECT
	{
		LONG    left;
		LONG    top;
		LONG    right;
		LONG    bottom;
	} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;
	...and POINT is defined as such:
	typedef struct tagPOINT
	{
		LONG  x;
		LONG  y;
	} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

	...therefore CWnd::ScreenToClient is like calling

	::ScreenToClient(hWnd, {left, top});
	::ScreenToClient(hWnd, {right, bottom});
	*/
	//For maximum type safety, I'm not going to type pun this. There's no POINT inside the RECT (even though they are binary compatible).
	POINT p1 = { rect->left, rect->top };
	POINT p2 = { rect->right, rect->bottom };
	hwnd::ScreenToClient(hWnd, &p1);
	hwnd::ScreenToClient(hWnd, &p2);
	rect->left = p1.x;
	rect->top = p1.y;
	rect->right = p2.x;
	rect->bottom = p2.y;

	}

void hwnd::EndPaint(_In_ const HWND hWnd, _In_ const PAINTSTRUCT& ps) noexcept {
	/*
	from: C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\atlmfc\include\atlwin.h:1184
	void EndPaint(_In_ LPPAINTSTRUCT lpPaint) throw()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::EndPaint(m_hWnd, lpPaint);
	}
	*/
	const BOOL is_window = ::IsWindow(hWnd);
	if (!is_window) {
		TRACE(L"EndPaint called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate();
		}


	// EndPaint function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
	// The EndPaint function marks the end of painting in the specified window.
	// This function is required for each call to the BeginPaint function, but only after painting is complete.
	// The return value is always nonzero.
	const BOOL end_paint_result = ::EndPaint(hWnd, &ps);
	if (end_paint_result == 0) {
		TRACE(L"This shouldn't happpen if Microsoft is correct about their own documentation.\r\n");
		should_never_happen_endpaint();
		}

	}

HDC hwnd::BeginPaint(_In_ const HWND hWnd, _Out_ PPAINTSTRUCT pPaint) noexcept {
	/*
	from: C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\atlmfc\include\atlwin.h:1178	
	HDC BeginPaint(_Out_ LPPAINTSTRUCT lpPaint) throw()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::BeginPaint(m_hWnd, lpPaint);
	}

	*/
	const BOOL is_window = ::IsWindow(hWnd);
	if (!is_window) {
		TRACE(L"BeginPaint called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate();
		}

	// BeginPaint function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
	// The BeginPaint function prepares the specified window for painting and fills a PAINTSTRUCT structure with information about the painting.
	// If the function succeeds, the return value is the handle to a display device context for the specified window.
	// If the function fails, the return value is NULL, indicating that no display device context is available.
	const HDC hDC = ::BeginPaint(hWnd, pPaint);
	if (hDC == NULL) {
		TRACE(L"BeginPaint says no display device contexts are available (by returning a NULL HDC). This means there's probably something very wrong with your computer.");
		std::terminate();
	}
	return hDC;
	}

int hwnd::GetDlgCtrlID(_In_ const HWND hWnd) noexcept {
	//GetDlgCtrlID function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	// If the function succeeds, the return value is the identifier of the control.
	// If the function fails, the return value is zero. An invalid value for the hwndCtl parameter, for example, will cause the function to fail. To get extended error information, call GetLastError.
	const BOOL is_window = ::IsWindow(hWnd);
	if (!is_window) {
		TRACE(L"GetDlgCtrlID called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate();
		}
	const int ID = ::GetDlgCtrlID(hWnd);
	if (ID == 0) {
		displayWindowsMsgBoxWithError();
		std::terminate();
		}
	return ID;
	}


HWND hwnd::GetDlgItem(_In_ const HWND hWnd, _In_ const int nIDDlgItem) noexcept {
	// GetDlgItem function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgitem
	// If the function succeeds, the return value is the window handle of the specified control.
	// If the function fails, the return value is NULL, indicating an invalid dialog box handle or a nonexistent control. To get extended error information, call GetLastError.
	const BOOL is_window = ::IsWindow(hWnd);
	if (!is_window) {
		TRACE(L"GetDlgItem called on invalid window HWND: `%p`!\r\n", hWnd);
		std::terminate();
	}
	const HWND dlgItem = ::GetDlgItem(hWnd, nIDDlgItem);
	if (nullptr == dlgItem) {
		TRACE(L"GetDlgItem returned an invalid handle.\r\n");
		displayWindowsMsgBoxWithMessage(L"GetDlgItem returned NULL, indicating an invalid dialog box handle or a nonexistent control.");
		displayWindowsMsgBoxWithError();
		std::terminate();
		}
	
	return dlgItem;
	}

#endif

