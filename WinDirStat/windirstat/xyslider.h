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

#ifndef XYSLIDER_H
#define XYSLIDER_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"

#define XYSLIDER_CHANGED	0x88	// this is a value, I hope, that is nowhere used as notification code.

//
// CXySlider. A two-dimensional slider. CXySlider is used in the options dialog!
//
class CXySlider: public CStatic {
	DECLARE_DYNAMIC(CXySlider)

public:
	CXySlider( ) : m_inited( false ), m_gripperHighlight( false ), m_timer( 0 ), m_pos( 0, 0 ), m_externalPos( 0, 0 ), m_externalRange( 100, 100 ) { }

	void GetRange( _Out_ CSize& range ) const {
		range = m_externalRange;
		}
	void SetRange( _In_ CSize range ) {
		m_externalRange = range;
		}
	
	CPoint GetPos( ) const {
		return m_externalPos;
		}

	void SetPos( CPoint pt ) {
		Initialize( );
		m_externalPos = pt;
		ExternToIntern( );
		Invalidate( );
		}

	// "Line size" is always 1 Pixel
	// "Page size" is always 10 Pixel

protected:
	void Initialize       (                             );
	void CalcSizes        (                             );
	//void CheckMinMax      ( _Inout_ LONG& val, _In_ INT min, _In_ INT max ) const;
	void InternToExtern   (                             );
	void ExternToIntern   (                             );
	void NotifyParent     (                             );
	void PaintBackground  ( _In_ CDC *pdc                    );
	// void PaintValues(CDC *pdc); This is too noisy
	void PaintGripper     ( _In_ CDC *pdc                    );
	void DoMoveBy         ( _In_ const INT cx, _In_ const INT cy              );
	void DoDrag           ( _In_ CPoint point                );
	void DoPage           ( _In_ CPoint point                );
	void HighlightGripper( _In_ bool on ) {
		m_gripperHighlight = on;
		RedrawWindow( );
		}
	
	void InstallTimer( ) {
		RemoveTimer( );
		m_timer = SetTimer( 4711, 500, NULL );
		}


	void RemoveTimer( ) {
		if ( m_timer != 0 ) {
			KillTimer( m_timer );
			}
		m_timer = 0;
		}

	CRect GetGripperRect  (                             ) const;


	bool     m_inited;

	// These are in external scale
	CSize    m_externalRange;
	CPoint   m_externalPos;

	// These are in pixels
	CSize    m_range;
	CPoint   m_pos;	// relative to m_zero

	// Constants (in pixels)
	CRect    m_rcAll;
	CRect    m_rcInner;
	CPoint   m_zero;
	CSize    m_radius;
	CSize    m_gripperRadius;

	UINT_PTR m_timer;
	bool     m_gripperHighlight;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg UINT OnGetDlgCode();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSetPos(WPARAM, LPARAM lparam);
	afx_msg LRESULT OnGetPos(WPARAM, LPARAM lparam);
	};

void AFXAPI DDX_XySlider(CDataExchange* pDX, INT nIDC, CPoint& value);


// $Log$
// Revision 1.4  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
