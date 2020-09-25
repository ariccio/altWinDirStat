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

struct WTLTreemapPage final : public WTL::CPropertyPageImpl<WTLTreemapPage>, public WTL::CWinDataExchange<WTLTreemapPage> {

	DISALLOW_COPY_AND_ASSIGN(WTLTreemapPage);

	enum {
		IDD = IDD_PAGE_TREEMAP
		};

	WTLTreemapPage() : m_resetButton(this, 1) {
		}

	BEGIN_MSG_MAP_EX(WTLTreemapPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<WTLTreemapPage>)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_NOTIFY(onWM_NOTIFY)
		ALT_MSG_MAP(1)
			MSG_WM_SETCURSOR(OnSetCursor_Reset)
	END_MSG_MAP()

	BEGIN_DDX_MAP()
		DDX_CONTROL(IDC_TREEMAPHIGHLIGHTCOLOR, m_highlightColor)
		DDX_CONTROL(IDC_TREEMAPGRIDCOLOR, m_gridColor)
		DDX_CONTROL_HANDLE(IDC_BRIGHTNESS, m_brightness)
		DDX_CONTROL_HANDLE(IDC_CUSHIONSHADING, m_cushionShading)
		DDX_CONTROL_HANDLE(IDC_HEIGHT, m_height)
		DDX_CONTROL_HANDLE(IDC_SCALEFACTOR, m_scaleFactor)
		//DDX_CONTROL_HANDLE(IDC_LIGHTSOURCE, m_lightSource)
		DDX_CONTROL(IDC_RESET, m_resetButton)
		DDX_RADIO(IDC_KDIRSTAT, m_style)
		DDX_TEXT_LEN(IDC_STATICBRIGHTNESS, m_sBrightness, str_size)
		//DDX_CONTROL(IDC_BRIGHTNESS, m_nBrightness)
		DDX_INT(IDC_BRIGHTNESS, m_nBrightness)
		DDX_TEXT_LEN(IDC_STATICCUSHIONSHADING, m_sCushionShading, str_size)
		//DDX_CONTROL(IDC_CUSHIONSHADING, m_nCushionShading)
		DDX_TEXT_LEN(IDC_STATICHEIGHT, m_sHeight, str_size)
		//DDX_CONTROL(IDC_HEIGHT, m_nHeight)
		DDX_TEXT_LEN(IDC_STATICSCALEFACTOR, m_sScaleFactor, str_size)
		//DDX_CONTROL(IDC_SCALEFACTOR, m_nScaleFactor)

		//DDX_CONTROL(IDC_LIGHTSOURCE, m_ptLightSource)

	END_DDX_MAP()
	void UpdateOptions          (_In_ const bool save = true) noexcept;
	void UpdateStatics          ();

	LRESULT OnSetCursor_Reset(const HWND hwndCtrl, UINT uHitTest, UINT uMouseMsg);

	BOOL OnInitDialog(const HWND focus, const LPARAM lparam);

	void ValuesAltered(_In_ const bool altered = true) noexcept;
	int OnApply();


	void OnVScroll(UINT nSBCode, UINT nPos, HWND /*pScrollBar*/);
	void OnSomethingChanged() noexcept;


	LRESULT onWM_NOTIFY(const int wParam, const NMHDR* const lParam);

	void OnColorChangedTreemapGrid(const NMHDR*) noexcept;
	void OnColorChangedTreemapHighlight(const NMHDR*) noexcept;

	Treemap_Options   m_options;	// Current options
	Treemap_Options   m_undo;	    // Valid, if m_altered = false


	BOOL              m_altered = FALSE;	// Values have been altered. Button reads "Reset to defaults".
	BOOL              m_grid;

	CColorButton      m_highlightColor;
	CColorButton      m_gridColor;

	WTL::CTrackBarCtrl       m_brightness;
	WTL::CTrackBarCtrl       m_cushionShading;
	WTL::CTrackBarCtrl       m_height;
	WTL::CTrackBarCtrl       m_scaleFactor;

	//CXySlider         m_lightSource;
	//POINT             m_ptLightSource;
	ATL::CContainedWindow      m_resetButton;

	static const rsize_t str_size = 10;

	_Field_z_               wchar_t m_sBrightness[str_size] = {};
	_Field_z_               wchar_t m_sCushionShading[str_size] = {};
	_Field_z_               wchar_t m_sHeight[str_size] = {};
	_Field_z_               wchar_t m_sScaleFactor[str_size] = {};

	_Field_range_(0, 2) INT       m_style;
	_Field_range_(0, 100) INT     m_nBrightness;

	//TODO: m_nCushionShading is never explicitly initialized.
	_Field_range_(0, 100) INT     m_nCushionShading;

	//TODO: m_nHeight is never explicitly initialized.
	_Field_range_(0, 100) INT     m_nHeight;

	//TODO: m_nScaleFactor is never explicitly initialized.
							//C4820: 'CPageTreemap' : '4' bytes padding added after data member 'CPageTreemap::m_nScaleFactor'
	_Field_range_(0, 100) INT     m_nScaleFactor;

	};

class CPageTreemap final : public CPropertyPage {
	DISALLOW_COPY_AND_ASSIGN(CPageTreemap);

// Fuckin' OOP sucks.
public:
	enum {
		IDD = IDD_PAGE_TREEMAP
		};

	/*
	#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \
	*/
	//DECLARE_DYNAMIC(CPageTreemap)
public:
	static const CRuntimeClass classCPageTreemap;
	virtual CRuntimeClass* GetRuntimeClass() const;

public:
	CPageTreemap( ) noexcept : CPropertyPage( CPageTreemap::IDD ) { }


protected:
	void UpdateOptions          ( _In_ const bool save = true    ) noexcept;
	void UpdateStatics          (                           );
	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
	virtual BOOL OnInitDialog   (                           ) override final;
	virtual void OnOK           (                           ) override final;


	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
	void OnSomethingChanged( ) noexcept {
		VERIFY( CWnd::UpdateData( ) );
		VERIFY( CWnd::UpdateData( false ) );
		SetModified( );
		}

	void ValuesAltered( _In_ const bool altered = true ) noexcept;


	Treemap_Options   m_options;	// Current options
	Treemap_Options   m_undo;	    // Valid, if m_altered = false


	BOOL              m_altered = FALSE;	// Values have been altered. Button reads "Reset to defaults".
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

	//TODO: m_nCushionShading is never explicitly initialized.
	_Field_range_( 0, 100 ) INT     m_nCushionShading;

	//TODO: m_nHeight is never explicitly initialized.
	_Field_range_( 0, 100 ) INT     m_nHeight;

	//TODO: m_nScaleFactor is never explicitly initialized.
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
