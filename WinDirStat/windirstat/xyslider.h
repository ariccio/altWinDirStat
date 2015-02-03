// xyslider.h	- Declaration of CXySlider
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

#pragma once

#ifndef XYSLIDER_H
#define XYSLIDER_H


#include "stdafx.h"

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

	//void GetRange( _Out_ CSize& range ) const {
	//	range = m_externalRange;
	//	}

	//void SetRange( _In_ const CSize range ) {
	//	m_externalRange = range;
	//	}
	
	//CPoint GetPos( ) const {
	//	return m_externalPos;
	//	}

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
		m_timer = SetTimer( 4711, 500, NULL );
		}


	
	RECT GetGripperRect( ) const {
		RECT rc { -m_gripperRadius.cx, -m_gripperRadius.cy, m_gripperRadius.cx + 1, m_gripperRadius.cy + 1 };
		
		/*
		inline void CRect::OffsetRect(_In_ POINT point) throw()
		{
			::OffsetRect(this, point.x, point.y);
		}
		*/
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::OffsetRect( &rc, m_zero.x, m_zero.y ) );

		//rc.OffsetRect( m_zero );
		
		VERIFY( ::OffsetRect( &rc, m_pos.x, m_pos.y ) );
		//rc.OffsetRect( m_pos  );

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
		CStatic::OnDestroy( );
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

	_At_( nChar, _Const_ )
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point ) {
		RemoveTimer( );
		CStatic::OnLButtonUp( nFlags, point );
		}
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSetPos( WPARAM, LPARAM lparam ) {
		const auto point = reinterpret_cast<const POINT*>( lparam );
		SetPos( *point );
		return 0;
		}
	afx_msg LRESULT OnGetPos( WPARAM, LPARAM lparam ) {
		auto point = reinterpret_cast<POINT*>( lparam );
		//*point = GetPos( );
		*point = m_externalPos;
		return 0;
		}
	};

void AFXAPI DDX_XySlider(CDataExchange* pDX, INT nIDC, POINT& value);


// $Log$
// Revision 1.4  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
#else
#error ass
#endif
