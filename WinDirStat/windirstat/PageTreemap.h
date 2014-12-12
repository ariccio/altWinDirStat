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


#ifndef PAGETREEMAP_H
#define PAGETREEMAP_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"

#include "colorbutton.h"
#include "xyslider.h"

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

protected:
	void UpdateOptions          ( _In_ const bool save = true    );
	void UpdateStatics          (                           );
	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
	virtual BOOL OnInitDialog   (                           ) override final;
	virtual void OnOK           (                           ) override final;


	void OnSomethingChanged( ) {
		UpdateData( );
		UpdateData( false );
		SetModified( );
		}
	void ValuesAltered( _In_ const bool altered = true ) {
		m_altered = altered;
		//auto s = MAKEINTRESOURCEW( m_altered ? IDS_RESETTO_DEFAULTS : IDS_BACKTO_USERSETTINGS );
		m_resetButton.SetWindowTextW( m_altered ? L"&Reset to\r\nDefaults" : L"Back to\r\n&User Settings" );
		}


	Treemap_Options   m_options;	// Current options
	Treemap_Options   m_undo;	    // Valid, if m_altered = false

	bool              m_altered;	// Values have been altered. Button reads "Reset to defaults".
	bool              m_grid;

	CColorButton      m_highlightColor;
	CColorButton      m_gridColor;

	CSliderCtrl       m_brightness;
	CSliderCtrl       m_cushionShading;
	CSliderCtrl       m_height;
	CSliderCtrl       m_scaleFactor;

	CXySlider         m_lightSource;
	CPoint            m_ptLightSource;
	CButton           m_resetButton;


	_Field_z_               wchar_t m_sBrightness     [ 4 ];
	_Field_z_               wchar_t m_sCushionShading [ 4 ];
	_Field_z_               wchar_t m_sHeight         [ 4 ];
	_Field_z_               wchar_t m_sScaleFactor    [ 4 ];

	_Field_range_( 0,   2 ) INT     m_style;
	_Field_range_( 0, 100 ) INT     m_nBrightness;
	_Field_range_( 0, 100 ) INT     m_nCushionShading;
	_Field_range_( 0, 100 ) INT     m_nHeight;
	_Field_range_( 0, 100 ) INT     m_nScaleFactor;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnColorChangedTreemapGrid( NMHDR *, LRESULT* result ) {
		*result = 0;
		OnSomethingChanged( );
		}
	afx_msg void OnColorChangedTreemapHighlight( NMHDR*, LRESULT* result ) {
		*result = 0;
		OnSomethingChanged( );
		}
	afx_msg void OnVScroll( UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/ ) {
		OnSomethingChanged( );
		ValuesAltered( );
		}
	afx_msg void OnLightSourceChanged( NMHDR *, LRESULT * ) {
		OnSomethingChanged( );
		ValuesAltered( );
		}
	afx_msg void OnBnClickedReset();

};
