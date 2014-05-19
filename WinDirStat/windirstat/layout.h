// layout.h	- Declaration of CLayout
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

//
// CLayout. A poor men's dialog layout mechanism.
// Simple, flat, and sufficient for our purposes.
//
class CLayout
{
	struct SControlInfo
	{
		CWnd *control;
		double movex;
		double movey;
		double stretchx;
		double stretchy;

		CRect originalRectangle;
	};

public:
	class CSizeGripper: public CWnd
	{
	public:
		static const int _width;

		CSizeGripper();
		void Create(_Inout_ CWnd *parent, _In_ CRect rc);

	private:
		void DrawShadowLine(_In_ CDC *pdc, _In_ CPoint start, _In_ CPoint end);

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg LRESULT OnNcHitTest( CPoint point );
	};


	CLayout(CWnd *dialog, LPCTSTR name);
	int AddControl( _In_ CWnd *control, _In_ const double movex, _In_ const double movey, _In_ const double stretchx, _In_ const double stretchy );
	void AddControl( _In_ const UINT id, _In_ const double movex, _In_ const double movey, _In_ const double stretchx, _In_ const double stretchy );

	void OnInitDialog( _In_ const bool centerWindow );
	void OnSize();
	void OnGetMinMaxInfo(_Inout_ MINMAXINFO *mmi);
	void OnDestroy();

protected:
	CWnd *m_dialog;
	CString m_name;
	CSize m_originalDialogSize;
	CArray<SControlInfo, SControlInfo&> m_control;
	CSizeGripper m_sizeGripper;
};

// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
