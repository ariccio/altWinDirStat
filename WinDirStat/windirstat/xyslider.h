// xyslider.h	- Declaration of CXySlider
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"


#ifndef WDS_XYSLIDER_H
#define WDS_XYSLIDER_H

WDS_FILE_INCLUDE_MESSAGE

#include "globalhelpers.h"
#include "signum.h"
#include "macros_that_scare_small_children.h"


// CXySlider. A two-dimensional slider. CXySlider is used in the options dialog!
struct CXySlider final : public CWnd {
	DECLARE_DYNAMIC(CXySlider)
	DISALLOW_COPY_AND_ASSIGN(CXySlider);

	CXySlider( ) noexcept : m_gripperHighlight{ false } {
		m_externalPos.x = 0;
		m_externalPos.y = 0;
		m_externalRange.cx = 100;
		m_externalRange.cy = 100;
		m_pos.x = 0;
		m_pos.y = 0;
		}

	void SetPos( const POINT pt );

	// "Line size" is always 1 Pixel
	// "Page size" is always 10 Pixel
	void Initialize       (                             ) noexcept;
	void CalcSizes        (                             ) noexcept;
	void RemoveTimer      (                                  ) noexcept;	
	void DoMoveBy         ( _In_ const INT cx, _In_ const INT cy              ) noexcept;
	void DoDrag           ( _In_ const POINT point                ) noexcept;
	void DragMsgLoop(_In_ const POINT ptMin, _In_ const POINT ptMax, _Inout_ POINT* const pt0 ) noexcept;
	void DoPage           ( _In_ const POINT point                ) noexcept;
	void HighlightGripper ( _In_ const bool on ) noexcept;
	void Handle_WM_MOUSEMOVE( _In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT* const pt0 ) noexcept;
	void InternToExtern() noexcept;
	void ExternToIntern() noexcept;
	void InstallTimer() noexcept;
	RECT GetGripperRect() const noexcept;

	//C4820: 'CXySlider' : '3' bytes padding added after data member 'CXySlider::m_inited'
	bool     m_inited = false;
	// These are in external scale
	SIZE          m_externalRange;

	POINT         m_externalPos;

	// These are in pixels
	SIZE          m_range;
	POINT         m_pos;	// relative to m_zero

	// Constants (in pixels)
	RECT          m_rcAll = { 0 };
	RECT          m_rcInner = { 0 };
	POINT         m_zero = { 0 };
	SIZE          m_radius;
	//C4820: 'CXySlider' : '4' bytes padding added after data member 'CXySlider::m_gripperRadius'
	SIZE          m_gripperRadius;

	UINT_PTR m_timer = { 0u };
	//C4820: 'CXySlider' : '7' bytes padding added after data member 'CXySlider::m_gripperHighlight'
	bool     m_gripperHighlight = false;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy( ) {
		RemoveTimer( );
		CWnd::OnDestroy( );
		}
	afx_msg UINT OnGetDlgCode( ) {
		return DLGC_WANTARROWS;
		}
	afx_msg LRESULT OnNcHitTest( CPoint /*point*/ ) {
		return HTCLIENT;
		}
	//afx_msg void OnSetFocus( CWnd* pOldWnd );
	//afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnPaint( );
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point ) {
		RemoveTimer( );
		/*
		_AFXWIN_INLINE void CWnd::OnLButtonUp(UINT, CPoint)
			{ Default(); }

		*/
		CWnd::OnLButtonUp( nFlags, point );
		}
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSetPos( WPARAM, LPARAM lparam ) {
		const auto point = reinterpret_cast<const POINT*>( lparam );
		ASSERT(point);
		CXySlider::SetPos( *point );
		return 0;
		}
	afx_msg LRESULT OnGetPos( WPARAM, LPARAM lparam ) {
		auto point = reinterpret_cast<POINT*>( lparam );
		ASSERT(point);
		*point = m_externalPos;
		return 0;
		}
	};

void AFXAPI DDX_XySlider(CDataExchange* pDX, INT nIDC, POINT& value);

#else

#endif
