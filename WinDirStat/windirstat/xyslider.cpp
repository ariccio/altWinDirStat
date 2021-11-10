// XySlider.cpp	- Implementation of CXySlider
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"
#pragma once

#ifndef WDS_XYSLIDER_CPP
#define WDS_XYSLIDER_CPP

WDS_FILE_INCLUDE_MESSAGE


#include "datastructures.h"
#include "xyslider.h"
#include "globalhelpers.h"
#include "hwnd_funcs.h"
#include "macros_that_scare_small_children.h"
#include "ScopeGuard.h"

namespace {

	[[nodiscard]] std::wstring pts(const POINT& pt) {
		return L"pt.x: " + std::to_wstring(pt.x) + L", pt.y: " + std::to_wstring(pt.y);
		}

	[[nodiscard]] bool pts_eq(const POINT& ptl, const POINT& ptr) noexcept {
		if (ptl.x != ptr.x) {
			return false;
			}
		if (ptl.y != ptr.y) {
			return false;
			}
		// I could do this above, but I don't like undefined behavior.
		// Is POINT even packed?
		ASSERT(memcmp(&ptl, &ptr, sizeof(POINT)) == 0);
		return true;
		}

	constexpr const int GRIPPER_RADIUS = 8;

	void move_to_coord( _In_ const HDC hDC, _In_ const int rc_x, _In_ const int rc_y) {
		//A little bit like lifting the pen so we can draw the circle.
		//pdc.MoveTo( rc.left,  m_zero.y ); <---Not handling the return value means that WE DO NOT care about the previous "current position", thus the fourth parameter to MoveToEx should be NULL.
		//If [MoveToEx] succeeds, the return value is nonzero.
		//If [MoveToEx] fails, the return value is zero.
		//Reverse engineering shows that it can set ERROR_INVALID_PARAMETER.
		const BOOL move_result = ::MoveToEx(hDC, rc_x, rc_y, nullptr);
		if (move_result == 0) {
			std::terminate();
			}
		}
	
	void line_to_coord( _In_ const HDC hDC, _In_ const int rc_x, _In_ const int rc_y) {
		ASSERT( hDC != nullptr );
		//If [LineTo] succeeds, the return value is nonzero.
		//If [LineTo] fails, the return value is zero.
		//Reverse Engineering shows that it can set ERROR_INVALID_HANDLE.
		VERIFY( ::LineTo( hDC, rc_x, rc_y ) );
		}

	void paint_background(_In_ const HDC hDC, const RECT& m_rcInner, const RECT& m_rcAll, const POINT& m_zero, const SIZE& m_gripperRadius, _In_ const HWND m_hWnd) noexcept {

		fill_solid_RECT(hDC, &m_rcAll, ::GetSysColor(COLOR_BTNFACE));

		RECT rc = m_rcInner;

		ASSERT(hDC != NULL);

		//If [DrawEdge] succeeds, the return value is nonzero. If [DrawEdge] fails, the return value is zero.
		VERIFY(::DrawEdge(hDC, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST));

		fill_solid_RECT(hDC, &rc, RGB(255, 255, 255));

		const HPEN hPen = gdi::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DLIGHT));
		HGDIOBJ_wrapper pen(hPen);

		SelectObject_wrapper sopen(hDC, pen.m_hObject);
		//ASSERT(hAttribDC == nullptr);


		// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
		// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
		move_to_coord(hDC, rc.left, m_zero.y);

		line_to_coord(hDC, rc.right, m_zero.y);

		move_to_coord(hDC, m_zero.x, rc.top);

		line_to_coord(hDC, m_zero.x, rc.bottom);


		RECT circle = m_rcAll;

		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		//circle.DeflateRect( m_gripperRadius );
		VERIFY(::InflateRect(&circle, -(m_gripperRadius.cx), -(m_gripperRadius.cy)));

		SelectStockObject_wrapper sobrush(hDC, NULL_BRUSH);

		ASSERT(hDC != NULL);
		//If [Ellipse] succeeds, the return value is nonzero. If [Ellipse] fails, the return value is zero.
		VERIFY(::Ellipse(hDC, circle.left, circle.top, circle.right, circle.bottom));
	
		//--------------------------------
		if (::GetFocus() == m_hWnd) {
			//TODO: what function?
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			ASSERT(::IsWindow(m_hWnd));
			VERIFY(::DrawFocusRect(hDC, &m_rcAll));
		}
	}

	HPEN gripper_pen() noexcept {
		//CreatePen function: https://msdn.microsoft.com/en-us/library/dd183509.aspx
		//The CreatePen function creates a logical pen that has the specified style, width, and color.
		//The pen can subsequently be selected into a device context and used to draw lines and curves.
		//If the function succeeds, the return value is a handle that identifies a logical pen.
		//If the function fails, the return value is NULL.
		//When you no longer need the pen, call the DeleteObject function to delete it.
		//TODO: GetSysColorBrush?
		return gdi::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));

		}

	COLORREF gripper_color(const bool m_gripperHighlight) noexcept {
		COLORREF color_scopeholder = ::GetSysColor(COLOR_BTNFACE);
		if (m_gripperHighlight) {
			INT r = GetRValue(color_scopeholder);
			INT g = GetGValue(color_scopeholder);
			INT b = GetBValue(color_scopeholder);;
			r += (255 - r) / 3;
			g += (255 - g) / 3;
			b += (255 - b) / 3;
			color_scopeholder = RGB(r, g, b);
			}
		return color_scopeholder;
		}
	
	void paint_gripper(_In_ const HDC hDC, const bool m_gripperHighlight, RECT& gripper_rect) noexcept {
		ASSERT(hDC != NULL);

		const COLORREF color = gripper_color(m_gripperHighlight);
		fill_solid_RECT(hDC, &gripper_rect, color);


		//If [DrawEdge] succeeds, the return value is nonzero.
		//If [DrawEdge] fails, the return value is zero.
		VERIFY(::DrawEdge(hDC, &gripper_rect, EDGE_RAISED, BF_RECT));

		HGDIOBJ_wrapper pen(gripper_pen());

		SelectObject_wrapper sopen(hDC, pen.m_hObject);
		move_to_coord(hDC, gripper_rect.left, (gripper_rect.top + (gripper_rect.bottom - gripper_rect.top) / 2));

		line_to_coord(hDC, gripper_rect.right, (gripper_rect.top + (gripper_rect.bottom - gripper_rect.top) / 2));

		move_to_coord(hDC, gripper_rect.left + (gripper_rect.right - gripper_rect.left) / 2, gripper_rect.top);

		line_to_coord(hDC, gripper_rect.left + (gripper_rect.right - gripper_rect.left) / 2, gripper_rect.bottom);

		}

	void verify_m_rect_all_size(const RECT& m_rcAll) noexcept {
		ASSERT(m_rcAll.left == 0);
		ASSERT(m_rcAll.top == 0);
		ASSERT((m_rcAll.right - m_rcAll.left) % 2 == 1);
		ASSERT((m_rcAll.bottom - m_rcAll.top) % 2 == 1);
		ASSERT((m_rcAll.right - m_rcAll.left) >= GRIPPER_RADIUS * 2);	// Control must be large enough
		ASSERT((m_rcAll.bottom - m_rcAll.top) >= GRIPPER_RADIUS * 2);
		DEBUG_ONLY(m_rcAll);
		}


	POINT m_zero_size(const RECT& m_rcAll) noexcept {
		POINT m_zero = {
			.x = ((m_rcAll.right - m_rcAll.left) / 2),
			.y = ((m_rcAll.bottom - m_rcAll.top) / 2)
			};
		return m_zero;
		}

	SIZE m_radius_size(const RECT& m_rcAll) noexcept {
		SIZE m_radius = {
			.cx = ((m_rcAll.right - m_rcAll.left) / 2 - 1),
			.cy = ((m_rcAll.bottom - m_rcAll.top) / 2 - 1)
			};
		return m_radius;
		}

	void shrink_rect_slightly(_Inout_ RECT* const rc) {
		// Make size odd, so that zero lines are central
		if ((rc->right - rc->left) % 2 == 0) {
			rc->right--;
			}
		if ((rc->bottom - rc->top) % 2 == 0) {
			rc->bottom--;
			}
		}

	SIZE center_gripper(_In_ const RECT& grip_rect, _In_ const POINT& pt0) noexcept {
		const auto grip_rect_center_x = ((grip_rect.left + grip_rect.right) / 2);
		const auto grip_rect_center_y = ((grip_rect.top + grip_rect.bottom) / 2);
		const auto new_point_x = (pt0.x - grip_rect_center_x);
		const auto new_point_y = (pt0.y - grip_rect_center_y);
		const SIZE inGripper = {
			.cx = new_point_x,
			.cy = new_point_y 
			};
		return inGripper;
		}

	POINT pt_min(const POINT& m_zero, const SIZE& m_range, const SIZE& inGripper) noexcept {
		const POINT ptMin_holder = {
			.x = (m_zero.x - m_range.cx + inGripper.cx),
			.y = (m_zero.y - m_range.cy + inGripper.cy)
			};
		return ptMin_holder;
		}

	POINT pt_max(const POINT& m_zero, const SIZE& m_range, const SIZE& inGripper) noexcept {
		POINT ptMax_holder = {
			.x = (m_zero.x + m_range.cx + inGripper.cx),
			.y = (m_zero.y + m_range.cy + inGripper.cy)
			};
		return ptMax_holder;
		}

	void notify_parent(_In_ const HWND m_hWnd) noexcept {
		NMHDR hdr = { };
		hdr.hwndFrom = m_hWnd;

		/*
		Maybe:
		int GetDlgCtrlID() const throw()
		{
			ATLASSERT(::IsWindow(m_hWnd));
			return ::GetDlgCtrlID(m_hWnd);
		}
		*/
		const int ID = hwnd::GetDlgCtrlID(m_hWnd);
		hdr.idFrom = static_cast<UINT_PTR>(ID);
		hdr.code = XYSLIDER_CHANGED;
		TRACE(_T("NotifyParent called! Sending WM_NOTIFY!\r\n"));
		//CWnd::GetParent( )->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( ID ), reinterpret_cast<LPARAM>(&hdr) );
		::SendMessageW(::GetParent(m_hWnd), WM_NOTIFY, static_cast<WPARAM>(ID), reinterpret_cast<LPARAM>(&hdr));

		}

	DOUBLE internal_x_to_external_x(const POINT m_pos, const SIZE m_externalRange, const SIZE m_range) noexcept {
		const DOUBLE m_pos_x_abs = static_cast<DOUBLE>(abs(m_pos.x));
		const DOUBLE external_cx = static_cast<DOUBLE>(m_externalRange.cx);
		const DOUBLE range_cx = static_cast<DOUBLE>(m_range.cx);
		const DOUBLE scaled_position_x = m_pos_x_abs * external_cx;
		const DOUBLE x = (scaled_position_x / range_cx) + 0.5;
		return x;
		}

	DOUBLE internal_y_to_external_y(const POINT m_pos, const SIZE m_externalRange, const SIZE m_range) noexcept {
		const DOUBLE m_pos_y_abs = static_cast<DOUBLE>(abs(m_pos.y));
		const DOUBLE external_cy = static_cast<DOUBLE>(m_externalRange.cy);
		const DOUBLE range_cy = static_cast<DOUBLE>(m_range.cy);
		const DOUBLE scaled_position_y = m_pos_y_abs * external_cy;
		const DOUBLE y = (scaled_position_y / range_cy) + 0.5;
		return y;
		}

	DOUBLE external_y_to_internal_y(const POINT m_externalPos, const SIZE m_range, const SIZE m_externalRange) noexcept {
		const DOUBLE m_external_y_abs = static_cast<DOUBLE>(abs(m_externalPos.y));
		const DOUBLE range_cy = static_cast<DOUBLE>(m_range.cy);
		const DOUBLE external_cy = static_cast<DOUBLE>(m_externalRange.cy);
		const DOUBLE scaled_position_y = m_external_y_abs * m_range.cy;
		const DOUBLE y = (scaled_position_y / external_cy) + 0.5;
		return y;
		}

	DOUBLE external_x_to_internal_x(const POINT m_externalPos, const SIZE m_range, const SIZE m_externalRange) noexcept {
		const DOUBLE m_external_x_abs = static_cast<DOUBLE>(abs(m_externalPos.x));
		const DOUBLE range_cx = static_cast<DOUBLE>(m_range.cx);
		const DOUBLE external_cx = static_cast<DOUBLE>(m_externalRange.cx);
		const DOUBLE scaled_position_x = m_external_x_abs * range_cx;
		const DOUBLE x = (scaled_position_x / external_cx) + 0.5;
		return x;
		}

	UINT_PTR set_timer(_In_ const HWND m_hWnd) noexcept {
		//If the window handle identifies an existing window, [IsWindow] returns [a nonzero value].
		ASSERT(::IsWindow(m_hWnd));

		//SetTimer function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-settimer
		//If [SetTimer] succeeds and the hWnd parameter is NULL, the return value is an integer identifying the new timer. An application can pass this value to the KillTimer function to destroy the timer.
		//If [SetTimer] succeeds and the hWnd parameter is not NULL, then the return value is a nonzero integer. An application can pass the value of the nIDEvent parameter to the KillTimer function to destroy the timer.
		//If [SetTimer] fails to create a timer, the return value is zero. To get extended error information, call GetLastError.
		//If lpTimerFunc is NULL, the system posts a WM_TIMER message to the application queue.
		const UINT_PTR temp_timer_value = ::SetTimer(m_hWnd, 4711u, 500u, nullptr);
		//TODO: check this value!
		if (temp_timer_value == 0) {
			TRACE(L"SetTimer failed.\r\n");
			displayWindowsMsgBoxWithMessage(L"SetTimer returned 0, indicating unexpected failure.");
			displayWindowsMsgBoxWithError();
			std::terminate();
			}
		return temp_timer_value;
		}

	void on_paint(const RECT& m_rcAll, _In_ const HWND m_hWnd, _In_ const RECT& m_rcInner, _In_ const POINT& m_zero, _In_ const SIZE& m_gripperRadius, _In_ const bool m_gripperHighlight, _In_ RECT gripper_rect) noexcept {
		const INT w = (m_rcAll.right - m_rcAll.left);
		const INT h = (m_rcAll.bottom - m_rcAll.top);


		PAINTSTRUCT ps = { 0 };
		const HDC dc = hwnd::BeginPaint(m_hWnd, &ps);
		auto paint_guard = WDS_SCOPEGUARD_INSTANCE([&]() { hwnd::EndPaint(m_hWnd, ps); });
		const HDC dcmem = gdi::CreateCompatibleDeviceContext(dc);
		auto paint_mem_guard = WDS_SCOPEGUARD_INSTANCE([&]() { gdi::DeleteDeviceContext(dcmem); });
		const HGDIOBJ bm = gdi::CreateCompatibleBitmap(dc, w, h);
		HGDIOBJ_wrapper bm_(bm);

		SelectObject_wrapper sobm(dcmem, bm);

		// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
		// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
		paint_background(dcmem, m_rcInner, m_rcAll, m_zero, m_gripperRadius, m_hWnd);
		
		paint_gripper(dcmem, m_gripperHighlight, gripper_rect);

		VERIFY(::BitBlt(dc, 0, 0, w, h, dcmem, 0, 0, SRCCOPY));

		}


	}
AFX_COMDAT const CRuntimeClass CXySlider::classCXySlider = {
	"CXySlider" /*m_lpszClassName*/,
	sizeof(class CXySlider) /*m_nObjectSize*/,
	0xFFFF /*wSchema*/,
	nullptr  /*pfnNew*/,
	(const_cast<CRuntimeClass*>(&CStatic::classCStatic)) /*RUNTIME_CLASS(CStatic)*/ /*m_pBaseClass*/,
	nullptr  /*m_pNextClass*/,
	nullptr  /*class_init*/
	};

CRuntimeClass* CXySlider::GetRuntimeClass() const {
	return (const_cast<CRuntimeClass*>(&CXySlider::classCXySlider)); /* RUNTIME_CLASS(CXySlider);*/
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
	ON_MESSAGE(XY_SETPOS, &(CXySlider::OnSetPos))
	ON_MESSAGE(XY_GETPOS, &(CXySlider::OnGetPos))
END_MESSAGE_MAP()


void AFXAPI DDX_XySlider( CDataExchange* pDX, INT nIDC, POINT& value ) {
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl = nullptr;
	pDX->m_pDlgWnd->GetDlgItem( nIDC, &hWndCtrl );
	if ( pDX->m_bSaveAndValidate ) {
		::SendMessageW( hWndCtrl, XY_GETPOS, 0, reinterpret_cast<LPARAM>(&value) );
		}
	else {
		::SendMessageW( hWndCtrl, XY_SETPOS, 0, reinterpret_cast<LPARAM>(&value) );
		}
	}

void CXySlider::Initialize( ) noexcept {
	if ( ( !m_inited ) && ::IsWindow( m_hWnd ) ) {
		
		RECT rc = hwnd::GetWindowRect(m_hWnd);
		hwnd::ScreenToClient(::GetParent(m_hWnd), &rc);
		shrink_rect_slightly(&rc);

		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY( ::MoveWindow( m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );

		CXySlider::CalcSizes( );

		m_inited = true;
		}
	}

void CXySlider::CalcSizes( ) noexcept {
	ASSERT( ::IsWindow( m_hWnd ) );
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
	VERIFY( ::GetClientRect( m_hWnd, &m_rcAll ) );

	verify_m_rect_all_size(m_rcAll);

	m_zero = m_zero_size(m_rcAll);
	m_radius = m_radius_size(m_rcAll);

	m_rcInner = m_rcAll;
	constexpr const int neg_GRIPPER_RADIUS = -(GRIPPER_RADIUS - 3);
	//"Return value: If [InflateRect] succeeds, the return value is nonzero. If [InflateRect] fails, the return value is zero."
	VERIFY( ::InflateRect( &m_rcInner, neg_GRIPPER_RADIUS, neg_GRIPPER_RADIUS ) );


	m_gripperRadius.cx = GRIPPER_RADIUS;
	m_gripperRadius.cy = GRIPPER_RADIUS;

	m_range.cx = m_radius.cx - m_gripperRadius.cx;
	m_range.cy = m_radius.cy - m_gripperRadius.cy;

	}


void CXySlider::DoMoveBy( _In_ const INT cx, _In_ const INT cy ) noexcept {
	m_pos.x += cx;
	CheckMinMax( m_pos.x, -m_range.cx, m_range.cx );

	m_pos.y += cy;
	CheckMinMax( m_pos.y, -m_range.cy, m_range.cy );

	hwnd::RedrawWindow(m_hWnd);
	const POINT oldpos = m_externalPos;
	CXySlider::InternToExtern( );
	if ( ( m_externalPos.x != oldpos.x ) || ( m_externalPos.y != oldpos.y ) ) {
		notify_parent(m_hWnd);
		}
	}

void CXySlider::Handle_WM_MOUSEMOVE( _In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT* const pt0 ) noexcept {
	POINT pt = msg.pt;
	//VERIFY( ::ScreenToClient( m_hWnd, &pt ) );
	hwnd::ScreenToClient(m_hWnd, &pt);
	//ScreenToClient( &pt );

	CheckMinMax( pt.x, ptMin.x, ptMax.x );
	CheckMinMax( pt.y, ptMin.y, ptMax.y );

	const INT dx = pt.x - pt0->x;
	const INT dy = pt.y - pt0->y;

	CXySlider::DoMoveBy( dx, dy );

	(*pt0) = pt;
	}

inline void CXySlider::InternToExtern() noexcept {
	const DOUBLE x = internal_x_to_external_x(m_pos, m_externalRange, m_range);
	const DOUBLE y = internal_y_to_external_y(m_pos, m_externalRange, m_range);

	m_externalPos.x = static_cast<INT>(x) * signum(m_pos.x);
	m_externalPos.y = static_cast<INT>(y) * signum(m_pos.y);
}

inline void CXySlider::ExternToIntern() noexcept {
	const DOUBLE x = external_x_to_internal_x(m_externalPos, m_range, m_externalRange);
	const DOUBLE y = external_y_to_internal_y(m_externalPos, m_range, m_externalRange);

	m_pos.x = static_cast<INT>(x) * signum(m_externalPos.x);
	m_pos.y = static_cast<INT>(y) * signum(m_externalPos.y);
}

inline void CXySlider::InstallTimer() noexcept {
	RemoveTimer();
	m_timer = set_timer(m_hWnd);
}

inline RECT CXySlider::GetGripperRect() const noexcept {
	RECT rc{ -m_gripperRadius.cx, -m_gripperRadius.cy, m_gripperRadius.cx + 1, m_gripperRadius.cy + 1 };
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY(::OffsetRect(&rc, m_zero.x, m_zero.y));
	VERIFY(::OffsetRect(&rc, m_pos.x, m_pos.y));
	return rc;
}

void CXySlider::DragMsgLoop(_In_ const POINT ptMin, _In_ const POINT ptMax, _Inout_ POINT* const pt0) noexcept {
	do {
		MSG msg;
		if (!winuser::GetMessageW(&msg, nullptr, 0, 0)) {
			break;
		}

		if (msg.message == WM_LBUTTONUP) {
			break;
		}
		if (::GetCapture() != m_hWnd) {
			ASSERT(CWnd::GetCapture() != this);
			break;
		}

		if (msg.message == WM_MOUSEMOVE) {
			CXySlider::Handle_WM_MOUSEMOVE(ptMin, ptMax, msg, pt0);
		}
		else {
			// Return value for DispatchMessageW is generally ignored.
			(void)::DispatchMessageW(&msg);
		}
#pragma warning(suppress:4127)//conditional expression is constant
	} while (true);
	}

void CXySlider::DoDrag( _In_ const POINT point ) noexcept {
	POINT pt0 = point;

	CXySlider::HighlightGripper( true );

	const RECT grip_rect = CXySlider::GetGripperRect( );
	const SIZE inGripper = center_gripper(grip_rect, pt0);
	const POINT ptMin = pt_min(m_zero, m_range, inGripper);
	const POINT ptMax = pt_max(m_zero, m_range, inGripper);

	ASSERT(::IsWindow(m_hWnd));

	// SetCapture function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcapture
	// The return value is a handle to the window that had previously captured the mouse. If there is no such window, the return value is NULL.
	//Don't care about return value.
	(void)::SetCapture(m_hWnd);

	CXySlider::DragMsgLoop(ptMin, ptMax, &pt0);

	VERIFY( ::ReleaseCapture( ) );

	CXySlider::HighlightGripper( false );
	}

void CXySlider::DoPage( _In_ const POINT point ) noexcept {
	const POINT relative_circle_coords = {
		(m_zero.x + m_pos.x),
		(m_zero.y + m_pos.y)
		};

	const POINT point_minus_circle_coords = { 
		.x = (point.x - relative_circle_coords.x),
		.y = (point.y - relative_circle_coords.y)
		};

	const SIZE sz = { point_minus_circle_coords.x, point_minus_circle_coords.y };

	ASSERT( sz.cx != 0 || sz.cy != 0 );
	const DOUBLE x_sq = static_cast<DOUBLE>(sz.cx) * static_cast<DOUBLE>(sz.cx);
	const DOUBLE y_sq = static_cast<DOUBLE>(sz.cy) * static_cast<DOUBLE>(sz.cy);

	//pythagorean theory yo
	const double len = ::sqrt( x_sq + y_sq );

	const INT dx = static_cast<INT>( 10 * static_cast<std::int64_t>(sz.cx) / len );
	const INT dy = static_cast<INT>( 10 * static_cast<std::int64_t>(sz.cy) / len );

	CXySlider::DoMoveBy( dx, dy );
	}

void CXySlider::HighlightGripper( _In_ const bool on ) noexcept {
	m_gripperHighlight = on;
	hwnd::RedrawWindow(m_hWnd);
	//VERIFY( CWnd::RedrawWindow( ) );
	}

void CXySlider::RemoveTimer( ) noexcept {
	if ( m_timer != 0 ) {
		ASSERT( ::IsWindow( m_hWnd ) );
		
		//If [KillTimer] succeeds, the return value is nonzero. If [KillTimer] fails, the return value is zero. To get extended error information, call GetLastError.
		VERIFY( ::KillTimer( m_hWnd, m_timer ) );
		m_timer = 0;
		}
	}


void CXySlider::OnPaint( ) {
	CXySlider::Initialize( );
	on_paint(m_rcAll, m_hWnd, m_rcInner, m_zero, m_gripperRadius, m_gripperHighlight, CXySlider::GetGripperRect());
	}


void CXySlider::OnKeyDown( UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/ ) {
	switch ( nChar )
	{
		case VK_LEFT:
			return CXySlider::DoMoveBy( -1,  0 );
		case VK_RIGHT:
			return CXySlider::DoMoveBy(  1,  0 );
		case VK_UP:
			return CXySlider::DoMoveBy(  0, -1 );
		case VK_DOWN:
			return CXySlider::DoMoveBy(  0,  1 );
	}
	}

void CXySlider::OnLButtonDown( UINT /*nFlags*/, CPoint point ) {
	//don't care about old focus
	(void)::SetFocus(m_hWnd);

	const RECT rc = GetGripperRect( );

	if ( ::PtInRect( &rc, point ) ) {
		return CXySlider::DoDrag( point );
		}
	CXySlider::DoPage( point );
	CXySlider::InstallTimer( );
	}

void CXySlider::OnLButtonDblClk( UINT /*nFlags*/, CPoint point ) {
	//don't care about old focus
	::SetFocus(m_hWnd);

	const RECT grip_rect = CXySlider::GetGripperRect( );
	if ( ::PtInRect( &grip_rect, point ) ) {
		return CXySlider::DoMoveBy( -m_pos.x, -m_pos.y );
		}
	CXySlider::DoPage( point );
	CXySlider::InstallTimer( );
	}

void CXySlider::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	TRACE(L"Timer fired...\r\n");
	POINT point;
	VERIFY( ::GetCursorPos( &point ) );
	ASSERT( ::IsWindow( m_hWnd ) );
	
	//VERIFY( ::ScreenToClient( m_hWnd, &point ) );
	hwnd::ScreenToClient(m_hWnd, &point);

	const RECT rc = CXySlider::GetGripperRect( );
	if ( !::PtInRect( &rc, point ) ) {
		CXySlider::DoPage( point );
		}
	}

void CXySlider::SetPos( const POINT pt ) {
	CXySlider::Initialize( );
	m_externalPos = pt;
	CXySlider::ExternToIntern( );

	hwnd::InvalidateErase(m_hWnd);
	}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


void WTLXySlider::Initialize() noexcept {
	if ((!m_inited) && ::IsWindow(m_hWnd)) {

		RECT rc = hwnd::GetWindowRect(m_hWnd);
		hwnd::ScreenToClient(::GetParent(m_hWnd), &rc);
		shrink_rect_slightly(&rc);

		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY(::MoveWindow(m_hWnd, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), TRUE));

		WTLXySlider::CalcSizes();

		m_inited = true;
	}
}

void WTLXySlider::CalcSizes() noexcept {
	ASSERT(::IsWindow(m_hWnd));
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
	VERIFY(::GetClientRect(m_hWnd, &m_rcAll));

	verify_m_rect_all_size(m_rcAll);

	m_zero = m_zero_size(m_rcAll);
	m_radius = m_radius_size(m_rcAll);

	m_rcInner = m_rcAll;
	constexpr const int neg_GRIPPER_RADIUS = -(GRIPPER_RADIUS - 3);
	//"Return value: If [InflateRect] succeeds, the return value is nonzero. If [InflateRect] fails, the return value is zero."
	VERIFY(::InflateRect(&m_rcInner, neg_GRIPPER_RADIUS, neg_GRIPPER_RADIUS));


	m_gripperRadius.cx = GRIPPER_RADIUS;
	m_gripperRadius.cy = GRIPPER_RADIUS;

	m_range.cx = m_radius.cx - m_gripperRadius.cx;
	m_range.cy = m_radius.cy - m_gripperRadius.cy;

}


void WTLXySlider::DoMoveBy(_In_ const INT cx, _In_ const INT cy) noexcept {
	m_pos.x += cx;
	CheckMinMax(m_pos.x, -m_range.cx, m_range.cx);

	m_pos.y += cy;
	CheckMinMax(m_pos.y, -m_range.cy, m_range.cy);

	hwnd::RedrawWindow(m_hWnd);
	const POINT oldpos = m_externalPos;
	WTLXySlider::InternToExtern();
	if ((m_externalPos.x != oldpos.x) || (m_externalPos.y != oldpos.y)) {
		notify_parent(m_hWnd);
		::SendMessageW(m_hWnd, XY_SETPOS, 0, reinterpret_cast<LPARAM>(&m_externalPos));
	}
}

void WTLXySlider::Handle_WM_MOUSEMOVE(_In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT* const pt0) noexcept {
	POINT pt = msg.pt;
	//VERIFY( ::ScreenToClient( m_hWnd, &pt ) );
	hwnd::ScreenToClient(m_hWnd, &pt);
	//ScreenToClient( &pt );

	CheckMinMax(pt.x, ptMin.x, ptMax.x);
	CheckMinMax(pt.y, ptMin.y, ptMax.y);

	const INT dx = pt.x - pt0->x;
	const INT dy = pt.y - pt0->y;

	WTLXySlider::DoMoveBy(dx, dy);

	(*pt0) = pt;
}

inline void WTLXySlider::InternToExtern() noexcept {
	const DOUBLE x = internal_x_to_external_x(m_pos, m_externalRange, m_range);
	const DOUBLE y = internal_y_to_external_y(m_pos, m_externalRange, m_range);

	m_externalPos.x = static_cast<INT>(x) * signum(m_pos.x);
	m_externalPos.y = static_cast<INT>(y) * signum(m_pos.y);
}

inline void WTLXySlider::ExternToIntern() noexcept {
	const DOUBLE x = external_x_to_internal_x(m_externalPos, m_range, m_externalRange);
	const DOUBLE y = external_y_to_internal_y(m_externalPos, m_range, m_externalRange);

	m_pos.x = static_cast<INT>(x) * signum(m_externalPos.x);
	m_pos.y = static_cast<INT>(y) * signum(m_externalPos.y);
}

inline void WTLXySlider::InstallTimer() noexcept {
	RemoveTimer();
	m_timer = set_timer(m_hWnd);
}

inline RECT WTLXySlider::GetGripperRect() const noexcept {
	RECT rc{ -m_gripperRadius.cx, -m_gripperRadius.cy, m_gripperRadius.cx + 1, m_gripperRadius.cy + 1 };
	//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
	VERIFY(::OffsetRect(&rc, m_zero.x, m_zero.y));
	VERIFY(::OffsetRect(&rc, m_pos.x, m_pos.y));
	return rc;
}

void WTLXySlider::DragMsgLoop(_In_ const POINT ptMin, _In_ const POINT ptMax, _Inout_ POINT* const pt0) noexcept {
	do {
		MSG msg;
		if (!winuser::GetMessageW(&msg, nullptr, 0, 0)) {
			break;
		}

		if (msg.message == WM_LBUTTONUP) {
			break;
		}
		if (::GetCapture() != m_hWnd) {
			break;
		}

		if (msg.message == WM_MOUSEMOVE) {
			WTLXySlider::Handle_WM_MOUSEMOVE(ptMin, ptMax, msg, pt0);
		}
		else {
			// Return value for DispatchMessageW is generally ignored.
			(void)::DispatchMessageW(&msg);
		}
#pragma warning(suppress:4127)//conditional expression is constant
	} while (true);
}

void WTLXySlider::DoDrag(_In_ const POINT point) noexcept {
	POINT pt0 = point;

	WTLXySlider::HighlightGripper(true);

	const RECT grip_rect = WTLXySlider::GetGripperRect();
	const SIZE inGripper = center_gripper(grip_rect, pt0);
	const POINT ptMin = pt_min(m_zero, m_range, inGripper);
	const POINT ptMax = pt_max(m_zero, m_range, inGripper);

	ASSERT(::IsWindow(m_hWnd));

	// SetCapture function (winuser.h): https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcapture
	// The return value is a handle to the window that had previously captured the mouse. If there is no such window, the return value is NULL.
	//Don't care about return value.
	(void)::SetCapture(m_hWnd);

	WTLXySlider::DragMsgLoop(ptMin, ptMax, &pt0);

	VERIFY(::ReleaseCapture());

	WTLXySlider::HighlightGripper(false);
}

void WTLXySlider::DoPage(_In_ const POINT point) noexcept {
	const POINT relative_circle_coords = {
		(m_zero.x + m_pos.x),
		(m_zero.y + m_pos.y)
	};

	const POINT point_minus_circle_coords = {
		.x = (point.x - relative_circle_coords.x),
		.y = (point.y - relative_circle_coords.y)
	};

	const SIZE sz = { point_minus_circle_coords.x, point_minus_circle_coords.y };

	ASSERT(sz.cx != 0 || sz.cy != 0);
	const DOUBLE x_sq = static_cast<DOUBLE>(sz.cx) * static_cast<DOUBLE>(sz.cx);
	const DOUBLE y_sq = static_cast<DOUBLE>(sz.cy) * static_cast<DOUBLE>(sz.cy);

	//pythagorean theory yo
	const double len = ::sqrt(x_sq + y_sq);

	//Hmm, implicit conversion. Should I convert to double earlier?
	const INT dx = static_cast<INT>(10 * static_cast<std::int64_t>(sz.cx) / len);
	const INT dy = static_cast<INT>(10 * static_cast<std::int64_t>(sz.cy) / len);

	WTLXySlider::DoMoveBy(dx, dy);
}

void WTLXySlider::HighlightGripper(_In_ const bool on) noexcept {
	m_gripperHighlight = on;
	hwnd::RedrawWindow(m_hWnd);
	//VERIFY( CWnd::RedrawWindow( ) );
}

void WTLXySlider::RemoveTimer() noexcept {
	if (m_timer != 0) {
		ASSERT(::IsWindow(m_hWnd));

		//If [KillTimer] succeeds, the return value is nonzero. If [KillTimer] fails, the return value is zero. To get extended error information, call GetLastError.
		VERIFY(::KillTimer(m_hWnd, m_timer));
		m_timer = 0;
	}
}


void WTLXySlider::OnPaint(const HDC hDC) {
	WTLXySlider::Initialize();
	on_paint(m_rcAll, m_hWnd, m_rcInner, m_zero, m_gripperRadius, m_gripperHighlight, WTLXySlider::GetGripperRect());
}


void WTLXySlider::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) {
	switch (nChar)
	{
	case VK_LEFT:
		return WTLXySlider::DoMoveBy(-1, 0);
	case VK_RIGHT:
		return WTLXySlider::DoMoveBy(1, 0);
	case VK_UP:
		return WTLXySlider::DoMoveBy(0, -1);
	case VK_DOWN:
		return WTLXySlider::DoMoveBy(0, 1);
	}
}

void WTLXySlider::OnLButtonDown(UINT /*nFlags*/, CPoint point) {
	//don't care about old focus
	(void)::SetFocus(m_hWnd);

	const RECT rc = GetGripperRect();

	if (::PtInRect(&rc, point)) {
		return WTLXySlider::DoDrag(point);
	}
	WTLXySlider::DoPage(point);
	WTLXySlider::InstallTimer();
}

void WTLXySlider::OnLButtonDblClk(UINT /*nFlags*/, CPoint point) {
	//don't care about old focus
	::SetFocus(m_hWnd);

	const RECT grip_rect = WTLXySlider::GetGripperRect();
	if (::PtInRect(&grip_rect, point)) {
		return WTLXySlider::DoMoveBy(-m_pos.x, -m_pos.y);
	}
	WTLXySlider::DoPage(point);
	WTLXySlider::InstallTimer();
}

void WTLXySlider::OnTimer(UINT_PTR /*nIDEvent*/) {
	TRACE(L"Timer fired...\r\n");
	POINT point;
	VERIFY(::GetCursorPos(&point));
	ASSERT(::IsWindow(m_hWnd));

	//VERIFY( ::ScreenToClient( m_hWnd, &point ) );
	hwnd::ScreenToClient(m_hWnd, &point);

	const RECT rc = WTLXySlider::GetGripperRect();
	if (!::PtInRect(&rc, point)) {
		WTLXySlider::DoPage(point);
	}
}

void WTLXySlider::SetPos(const POINT pt) {
	WTLXySlider::Initialize();
	m_externalPos = pt;
	WTLXySlider::ExternToIntern();

	hwnd::InvalidateErase(m_hWnd);
	::SendMessageW(::GetParent(m_hWnd), XY_SETPOS, 0, reinterpret_cast<LPARAM>(&pt));
}



#else

#endif