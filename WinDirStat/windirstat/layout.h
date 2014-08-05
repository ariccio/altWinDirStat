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
class CLayout {
	struct SControlInfo {
		//SControlInfo( ) {
		//	control = NULL;
		//	movex = NULL;
		//	movey = NULL;
		//	stretchx = NULL;
		//	stretchy = NULL;
		//	originalRectangle.bottom = NULL;
		//	originalRectangle.top = NULL;
		//	originalRectangle.left = NULL;
		//	originalRectangle.right = NULL;
		//	}
		//SControlInfo( SControlInfo& in ) = delete;
		//SControlInfo( SControlInfo&& other ) {
		//	control           = std::move( other.control );
		//	movex             = std::move( other.movex );
		//	movey             = std::move( other.movey );
		//	stretchx          = std::move( other.stretchx );
		//	stretchy          = std::move( other.stretchy );
		//	originalRectangle = std::move( other.originalRectangle );
		//	}

		CWnd*  control;
		DOUBLE movex;
		DOUBLE movey;
		DOUBLE stretchx;
		DOUBLE stretchy;
		CRect  originalRectangle;
		};
	

public:
	class CSizeGripper: public CWnd {
	public:
		static const INT _width;

		CSizeGripper();
		void Create( _Inout_ CWnd *parent, _In_ CRect rc );

	private:
		void DrawShadowLine( _In_ CDC *pdc, _In_ CPoint start, _In_ CPoint end );

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg LRESULT OnNcHitTest( CPoint point );
		};


	CLayout( _In_ CWnd* dialog, _In_ LPCTSTR name );
	CLayout( _In_ CLayout& other );
	CLayout( CLayout&& other );


	INT_PTR  AddControl ( _In_       CWnd*       control,  _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy );
	void AddControl     ( _In_ const UINT        id,       _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy );
	void OnInitDialog   ( _In_ const bool        centerWindow                                                                                                       );
	void OnGetMinMaxInfo( _Inout_    MINMAXINFO* mmi                                                                                                                );
	
	void OnDestroy ( );
	void OnSize    ( );
protected:
	CWnd*                               m_dialog;
	CString                             m_name;
	CSize                               m_originalDialogSize;
	
	CArray<SControlInfo, SControlInfo&> m_control;
	std::vector<SControlInfo>           m_vectorOfSControlInfo;

	CSizeGripper                        m_sizeGripper;
	};

// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
