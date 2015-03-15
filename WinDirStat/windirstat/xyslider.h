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

// CXySlider. A two-dimensional slider. CXySlider is used in the options dialog!
class CXySlider final : public CStatic {
	DECLARE_DYNAMIC(CXySlider)

public:
	CXySlider( ) : m_inited { false }, m_gripperHighlight { false }, m_timer { 0u } {
		m_externalPos.x = 0;
		m_externalPos.y = 0;
		m_externalRange.cx = 100;
		m_externalRange.cy = 100;
		m_pos.x = 0;
		m_pos.y = 0;
		}

	CXySlider& operator=( const CXySlider& in ) = delete;
	CXySlider( const CXySlider& in ) = delete;

	void SetPos( const POINT pt );

	// "Line size" is always 1 Pixel
	// "Page size" is always 10 Pixel

protected:
	void Initialize       (                             );
	void CalcSizes        (                             ) ;
	void NotifyParent     (                             ) const;
	void RemoveTimer      (                                  );
	void PaintBackground  ( _In_ CDC& pdc                    );
	void PaintGripper     ( _In_ CDC& pdc                    );
	void DoMoveBy         ( _In_ const INT cx, _In_ const INT cy              );
	void DoDrag           ( _In_ const POINT point                );
	void DoPage           ( _In_ const POINT point                );
	void HighlightGripper ( _In_ const bool on );
	void Handle_WM_MOUSEMOVE( _In_ const POINT& ptMin, _In_ const POINT& ptMax, _In_ const MSG& msg, _Inout_ POINT& pt0 );

	void InternToExtern( ) {
		m_externalPos.x = static_cast<INT>( static_cast<DOUBLE>( abs( m_pos.x ) ) * static_cast<DOUBLE>( m_externalRange.cx ) / static_cast<DOUBLE>( m_range.cx ) + 0.5 ) * signum( m_pos.x );
		m_externalPos.y = static_cast<INT>( static_cast<DOUBLE>( abs( m_pos.y ) ) * static_cast<DOUBLE>( m_externalRange.cy ) / static_cast<DOUBLE>( m_range.cy ) + 0.5 ) * signum( m_pos.y );
		}

	void ExternToIntern( ) {
		m_pos.x = static_cast<INT>( static_cast<DOUBLE>( abs( m_externalPos.x ) ) * static_cast<DOUBLE>( m_range.cx ) / static_cast<DOUBLE>( m_externalRange.cx ) + 0.5 ) * signum( m_externalPos.x );
		m_pos.y = static_cast<INT>( static_cast<DOUBLE>( abs( m_externalPos.y ) ) * static_cast<DOUBLE>( m_range.cy ) / static_cast<DOUBLE>( m_externalRange.cy ) + 0.5 ) * signum( m_externalPos.y );
		}

	void InstallTimer( ) {
		RemoveTimer( );
		/*
_AFXWIN_INLINE UINT_PTR CWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD))
	{ ASSERT(::IsWindow(m_hWnd)); return ::SetTimer(m_hWnd, nIDEvent, nElapse,
		lpfnTimer); }

		*/
		//If the window handle identifies an existing window, [IsWindow] returns [a nonzero value].
		ASSERT( ::IsWindow( m_hWnd ) );
		ASSERT( m_hWnd != NULL );
		

		//If [SetTimer] succeeds and the hWnd parameter is NULL, the return value is an integer identifying the new timer. An application can pass this value to the KillTimer function to destroy the timer.
		//If [SetTimer] succeeds and the hWnd parameter is not NULL, then the return value is a nonzero integer. An application can pass the value of the nIDEvent parameter to the KillTimer function to destroy the timer.
		//If [SetTimer] fails to create a timer, the return value is zero. To get extended error information, call GetLastError.
		//m_timer = SetTimer( 4711, 500, NULL );

		const auto temp_timer_value = ::SetTimer( m_hWnd, 4711u, 500u, NULL );
		//TODO: check this value!
		ASSERT( temp_timer_value != 0 );
		m_timer = temp_timer_value;
		}

	RECT GetGripperRect( ) const {
		RECT rc { -m_gripperRadius.cx, -m_gripperRadius.cy, m_gripperRadius.cx + 1, m_gripperRadius.cy + 1 };
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::OffsetRect( &rc, m_zero.x, m_zero.y ) );
		VERIFY( ::OffsetRect( &rc, m_pos.x, m_pos.y ) );
		return rc;
		}

	//C4820: 'CXySlider' : '3' bytes padding added after data member 'CXySlider::m_inited'
	bool     m_inited;
public:
	// These are in external scale
	SIZE          m_externalRange;

protected:
	POINT         m_externalPos;

	// These are in pixels
	SIZE          m_range;
	POINT         m_pos;	// relative to m_zero

	// Constants (in pixels)
	RECT          m_rcAll;
	RECT          m_rcInner;
	POINT         m_zero;
	SIZE          m_radius;
	//C4820: 'CXySlider' : '4' bytes padding added after data member 'CXySlider::m_gripperRadius'
	SIZE          m_gripperRadius;

	UINT_PTR m_timer;
	//C4820: 'CXySlider' : '7' bytes padding added after data member 'CXySlider::m_gripperHighlight'
	bool     m_gripperHighlight;

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
	afx_msg void OnSetFocus( CWnd* pOldWnd );
	afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnPaint( );
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point ) {
		RemoveTimer( );
		CWnd::OnLButtonUp( nFlags, point );
		}
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSetPos( WPARAM, LPARAM lparam ) {
		const auto point = reinterpret_cast<const POINT*>( lparam );
		SetPos( *point );
		return 0;
		}
	afx_msg LRESULT OnGetPos( WPARAM, LPARAM lparam ) {
		auto point = reinterpret_cast<POINT*>( lparam );
		*point = m_externalPos;
		return 0;
		}
	};

void AFXAPI DDX_XySlider(CDataExchange* pDX, INT nIDC, POINT& value);

#else

#endif
