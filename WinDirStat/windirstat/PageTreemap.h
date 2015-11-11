// pagetreemap.h	- Declaration of CDemoControl and CPageTreemap
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.


#pragma once

#include "stdafx.h"

#ifndef WDS_PAGETREEMAP_H
#define WDS_PAGETREEMAP_H

WDS_FILE_INCLUDE_MESSAGE


#include "colorbutton.h"
#include "xyslider.h" //for CXySlider, else we'd have to use PIMPL, which I hate.
#include "globalhelpers.h"
#include "macros_that_scare_small_children.h"

//
// CPageTreemap. "Settings" property page "Treemap".
//
//CTreemap::Options;

class CDirstatApp;

class CPageTreemap final : public CPropertyPage {
	DECLARE_DYNAMIC(CPageTreemap)
	enum {
		IDD = IDD_PAGE_TREEMAP
		};

public:
	CPageTreemap( ) : CPropertyPage( CPageTreemap::IDD ) { }

	DISALLOW_COPY_AND_ASSIGN( CPageTreemap );

protected:
	void UpdateOptions          ( _In_ const bool save = true    );
	void UpdateStatics          (                           );
	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
	virtual BOOL OnInitDialog   (                           ) override final;
	virtual void OnOK           (                           ) override final;


	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	void OnSomethingChanged( ) {
		VERIFY( CWnd::UpdateData( ) );
		VERIFY( CWnd::UpdateData( false ) );
		SetModified( );
		}

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
	POINT             m_ptLightSource;
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
							//C4820: 'CPageTreemap' : '4' bytes padding added after data member 'CPageTreemap::m_nScaleFactor'
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
#endif
