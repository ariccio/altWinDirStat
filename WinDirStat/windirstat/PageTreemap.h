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

class CPageTreemap final : public CPropertyPage {
	DECLARE_DYNAMIC(CPageTreemap)
	enum {
		IDD = IDD_PAGE_TREEMAP
		};

public:
	CPageTreemap( ) : CPropertyPage( CPageTreemap::IDD ) { }
	CPageTreemap& operator=( const CPageTreemap& in ) = delete;
	CPageTreemap( const CPageTreemap& in ) = delete;

protected:
	void UpdateOptions          ( _In_ const bool save = true    );
	void UpdateStatics          (                           );
	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
	virtual BOOL OnInitDialog   (                           ) override final;
	virtual void OnOK           (                           ) override final;


	void OnSomethingChanged( );

	void ValuesAltered( _In_ const bool altered = true );


	Treemap_Options   m_options;	// Current options
	Treemap_Options   m_undo;	    // Valid, if m_altered = false

	BOOL              m_altered;	// Values have been altered. Button reads "Reset to defaults".
	BOOL              m_grid;

	CColorButton      m_highlightColor;
	CColorButton      m_gridColor;

	CSliderCtrl       m_brightness;
	CSliderCtrl       m_cushionShading;
	CSliderCtrl       m_height;
	CSliderCtrl       m_scaleFactor;

	CXySlider         m_lightSource;
	WTL::CPoint       m_ptLightSource;
	CButton           m_resetButton;

	static const rsize_t str_size = 10;

	_Field_z_               wchar_t m_sBrightness     [ str_size ];
	_Field_z_               wchar_t m_sCushionShading [ str_size ];
	_Field_z_               wchar_t m_sHeight         [ str_size ];
	_Field_z_               wchar_t m_sScaleFactor    [ str_size ];

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
