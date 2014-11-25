// pagetreemap.h	- Declaration of CDemoControl and CPageTreemap
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


#ifndef PAGETREEMAP_H
#define PAGETREEMAP_H



#include "stdafx.h"


#ifndef COLORBUTTON_H
#include "colorbutton.h"
#else
#error ass!
#endif

//#include "treemap.h"

#ifndef XYSLIDER_H
#include "xyslider.h"
#else
#error ass!
#endif


//#include "afxwin.h"

//
// CPageTreemap. "Settings" property page "Treemap".
//
//CTreemap::Options;

class CPageTreemap : public CPropertyPage {
	DECLARE_DYNAMIC(CPageTreemap)
	enum {
		IDD = IDD_PAGE_TREEMAP
		};

public:
	CPageTreemap( ) : CPropertyPage( CPageTreemap::IDD ) { }
	//virtual ~CPageTreemap ( );

protected:
	void UpdateOptions          ( _In_ const bool save = true    );
	void UpdateStatics          (                           );
	void OnSomethingChanged( ) {
		UpdateData( );
		UpdateData( false );
		SetModified( );
		}
	void ValuesAltered( _In_ const bool altered = true ) {
		m_altered = altered;
		auto s = MAKEINTRESOURCE( m_altered ? IDS_RESETTO_DEFAULTS : IDS_BACKTO_USERSETTINGS );
		m_resetButton.SetWindowText( s );
		}

	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
	virtual BOOL OnInitDialog   (                           ) override final;
	virtual void OnOK           (                           ) override final;

	Treemap_Options m_options;	// Current options
	Treemap_Options m_undo;	    // Valid, if m_altered = false

	bool              m_altered;	// Values have been altered. Button reads "Reset to defaults".
	bool              m_grid;

	//CTreemapPreview   m_preview;

	CColorButton      m_highlightColor;
	CColorButton      m_gridColor;

	CSliderCtrl       m_brightness;
	CSliderCtrl       m_cushionShading;
	CSliderCtrl       m_height;
	CSliderCtrl       m_scaleFactor;

	CString           m_sBrightness;
	CString           m_sCushionShading;
	CString           m_sHeight;
	CString           m_sScaleFactor;

	_Field_range_( 0, 2 )   INT               m_style;
	_Field_range_( 0, 100 ) INT               m_nBrightness;
	_Field_range_( 0, 100 ) INT               m_nCushionShading;
	_Field_range_( 0, 100 ) INT               m_nHeight;
	_Field_range_( 0, 100 ) INT               m_nScaleFactor;

	CXySlider         m_lightSource;
	CPoint            m_ptLightSource;

	CButton           m_resetButton;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnColorChangedTreemapGrid( NMHDR *, LRESULT* result ) {
		*result = 0;
		OnSomethingChanged( );
		}
	afx_msg void OnColorChangedTreemapHighlight( NMHDR*, LRESULT* result ) {
		*result = 0;
		OnSomethingChanged( );
		}
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
		UNREFERENCED_PARAMETER( nPos );
		UNREFERENCED_PARAMETER( nSBCode );
		UNREFERENCED_PARAMETER( pScrollBar );
		OnSomethingChanged( );
		ValuesAltered( );
		}
	afx_msg void OnLightSourceChanged( NMHDR *, LRESULT * ) {
		OnSomethingChanged( );
		ValuesAltered( );
		}
	afx_msg void OnBnClickedReset();

};

// $Log$
// Revision 1.6  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.5  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.4  2004/11/05 16:53:06  assarbad
// Added Date and History tag where appropriate.
//
#else
#error ass
#endif
