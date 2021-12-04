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

//struct TrackbarDDX : public WTL::CTrackBarCtrl, WTL::CWinDataExchange<TrackbarDDX> {
//	
//};




// Only temporarily a template.
template <typename PageDialogT>
inline void ValuesAltered(_In_ const bool altered, _Inout_ PageDialogT* const dialog) noexcept {
	dialog->m_altered = (altered ? TRUE : FALSE);
	dialog->m_resetButton.SetWindowTextW(dialog->m_altered ? L"&Reset to\r\nDefaults" : L"Back to\r\n&User Settings");
}


struct WTLTreemapPage final : public WTL::CPropertyPageImpl<WTLTreemapPage>, public WTL::CWinDataExchange<WTLTreemapPage> {

	DISALLOW_COPY_AND_ASSIGN(WTLTreemapPage);

	enum {
		IDD = IDD_PAGE_TREEMAP
		};

	//WTLTreemapPage() : m_resetButton(this, 1), m_brightness(this, 2), m_cushionShading(this, 3), m_height(this, 4), m_scaleFactor(this, 5) {
	//	}

	WTLTreemapPage() : WTL::CPropertyPageImpl<WTLTreemapPage>::CPropertyPageImpl(L"Treemap Options"), m_resetButton(L"reset", this, 1), m_brightness(this), m_cushionShading(this), m_height(this), m_scaleFactor(this) {
		}

	//WTLTreemapPage() = default;

	BEGIN_MSG_MAP_EX(WTLTreemapPage)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<WTLTreemapPage>)
		MSG_WM_VSCROLL(OnVScroll)
		//MSG_WM_NOTIFY(onWM_NOTIFY)
		//MESSAGE_HANDLER_EX(BN_CLICKED, OnMessageHandlerEX)
		MESSAGE_HANDLER_EX(XY_SETPOS, setXY)
		NOTIFY_HANDLER_EX(IDC_LIGHTSOURCE, XYSLIDER_CHANGED, OnXYNotifyHandlerEX)
		COMMAND_HANDLER(IDC_RESET, BN_CLICKED, OnCommandIDCReset)
		COMMAND_HANDLER(IDC_TREEMAPGRIDCOLOR, BN_CLICKED, on_WM_COMMAND_Treemap_colorbutton)
		COMMAND_HANDLER(IDC_TREEMAPHIGHLIGHTCOLOR, BN_CLICKED, on_WM_COMMAND_Treemap_colorbutton)
		//REFLECTED_COMMAND_CODE_HANDLER()
		//CHAIN_MSG_MAP_MEMBER(m_lightSource)
		ALT_MSG_MAP(1)
			//MSG_WM_SETCURSOR(OnSetCursor_Reset)
			//MSG_WM_COMMAND(onResetCommand)
		ALT_MSG_MAP(2)
		ALT_MSG_MAP(3)
		ALT_MSG_MAP(4)
		ALT_MSG_MAP(5)
	END_MSG_MAP()

	BEGIN_DDX_MAP(WTLTreemapPage)
		DDX_CONTROL(IDC_TREEMAPHIGHLIGHTCOLOR, m_highlightColor)
		DDX_CONTROL(IDC_TREEMAPGRIDCOLOR, m_gridColor)
		DDX_CONTROL(IDC_BRIGHTNESS, m_brightness)
		DDX_CONTROL(IDC_CUSHIONSHADING, m_cushionShading)
		DDX_CONTROL(IDC_HEIGHT, m_height)
		DDX_CONTROL(IDC_SCALEFACTOR, m_scaleFactor)
		DDX_CONTROL(IDC_LIGHTSOURCE, m_lightSource)
		DDX_CONTROL(IDC_RESET, m_resetButton)
		DDX_RADIO(IDC_KDIRSTAT, m_style)
		DDX_TEXT_LEN(IDC_STATICBRIGHTNESS, m_sBrightness, str_size)
		//DDX_CONTROL(IDC_BRIGHTNESS, m_nBrightness)
		DDX_INT(IDC_BRIGHTNESS, m_nBrightness)
		DDX_TEXT_LEN(IDC_STATICCUSHIONSHADING, m_sCushionShading, str_size)
		DDX_INT(IDC_CUSHIONSHADING, m_nCushionShading)
		DDX_TEXT_LEN(IDC_STATICHEIGHT, m_sHeight, str_size)
		DDX_INT(IDC_HEIGHT, m_nHeight)
		DDX_TEXT_LEN(IDC_STATICSCALEFACTOR, m_sScaleFactor, str_size)
		DDX_INT(IDC_SCALEFACTOR, m_nScaleFactor)

		//DDX_CONTROL(IDC_LIGHTSOURCE, m_ptLightSource)

	END_DDX_MAP()
	void UpdateOptions          (_In_ const bool save = true) noexcept;
	//void UpdateStatics          ();


	LRESULT OnCommandIDCReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	BOOL OnInitDialog(const HWND focus, const LPARAM lparam);

	//void ValuesAltered(_In_ const bool altered = true) noexcept;
	int OnApply();
	
	//Note to self: WTL defines nPos as an int here!?
	void setPosition(int nPos, HWND /*pScrollBar*/);

	//Note to self: WTL defines nPos as a short!?
	//Also note to self, WTL defines nSBcode as an int
	void OnVScroll(int nSBCode, short nPos, HWND /*pScrollBar*/);

	LRESULT on_WM_COMMAND_Treemap_colorbutton(const WORD wNotifyCode, const WORD wID, HWND hWndCtl, BOOL& bHandled);


	void updateControls() noexcept;
	void updateSliders() noexcept;
	void variablesToOptions() noexcept;
	void optionsToVariables() noexcept;


	void debugDataExchangeMembers() const noexcept;
	void debugSliderPosition() const noexcept;

	LRESULT setXY(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT OnXYNotifyHandlerEX(const NMHDR* const pnmh) noexcept;


	Treemap_Options   m_options = {};	// Current options
	Treemap_Options   m_undo = {};	    // Valid, if m_altered = false


	BOOL              m_altered = FALSE;	// Values have been altered. Button reads "Reset to defaults".
	BOOL              m_grid = FALSE;

	CColorButton      m_highlightColor;
	CColorButton      m_gridColor;

	ATL::CContainedWindowT<WTL::CTrackBarCtrl>       m_brightness;
	ATL::CContainedWindowT<WTL::CTrackBarCtrl>       m_cushionShading;
	ATL::CContainedWindowT<WTL::CTrackBarCtrl>       m_height;
	ATL::CContainedWindowT<WTL::CTrackBarCtrl>       m_scaleFactor;

	WTLXySlider         m_lightSource;
	POINT             m_ptLightSource;
	ATL::CContainedWindow      m_resetButton;

	static constexpr const rsize_t str_size = 14;

	_Field_z_               wchar_t m_sBrightness[str_size] = {};
	_Field_z_               wchar_t m_sCushionShading[str_size] = {};
	_Field_z_               wchar_t m_sHeight[str_size] = {};
	_Field_z_               wchar_t m_sScaleFactor[str_size] = {};

	_Field_range_(0, 2) INT       m_style = 0;
	_Field_range_(0, 100) INT     m_nBrightness = 0;

	_Field_range_(0, 100) INT     m_nCushionShading = 0;

	_Field_range_(0, 100) INT     m_nHeight = 0;

							//C4820: 'CPageTreemap' : '4' bytes padding added after data member 'CPageTreemap::m_nScaleFactor'
	_Field_range_(0, 100) INT     m_nScaleFactor = 0;

	};

//struct CPageTreemap final : public CPropertyPage {
//	DISALLOW_COPY_AND_ASSIGN(CPageTreemap);
//
//// Fuckin' OOP sucks.
//	enum {
//		IDD = IDD_PAGE_TREEMAP
//		};
//
//	/*
//	#define DECLARE_DYNAMIC(class_name) \
//public: \
//	static const CRuntimeClass class##class_name; \
//	virtual CRuntimeClass* GetRuntimeClass() const; \
//	*/
//	//DECLARE_DYNAMIC(CPageTreemap)
//	static const CRuntimeClass classCPageTreemap;
//	virtual CRuntimeClass* GetRuntimeClass() const;
//
//	CPageTreemap( ) noexcept : CPropertyPage( CPageTreemap::IDD ) { }
//
//
//	void UpdateOptions          ( _In_ const bool save = true    ) noexcept;
//	//void UpdateStatics          (                           );
//	virtual void DoDataExchange ( CDataExchange* pDX        ) override final;
//	virtual BOOL OnInitDialog   (                           ) override final;
//	virtual void OnOK           (                           ) override final;
//
//
//	//The compiler will automatically inline if /Ob2 is on, so we'll ask anyways.
//	void OnSomethingChanged( ) noexcept {
//		VERIFY( CWnd::UpdateData( ) );
//		VERIFY( CWnd::UpdateData( false ) );
//		SetModified( );
//		}
//
//	void /*Values*/Altered( _In_ const bool altered = true ) noexcept;
//
//
//	Treemap_Options   m_options;	// Current options
//	Treemap_Options   m_undo;	    // Valid, if m_altered = false
//
//
//	BOOL              m_altered = FALSE;	// Values have been altered. Button reads "Reset to defaults".
//	BOOL              m_grid;
//
//	CColorButton      m_highlightColor;
//	CColorButton      m_gridColor;
//
//	CSliderCtrl       m_brightness;
//	CSliderCtrl       m_cushionShading;
//	CSliderCtrl       m_height;
//	CSliderCtrl       m_scaleFactor;
//
//	CXySlider         m_lightSource;
//	POINT             m_ptLightSource;
//	CButton           m_resetButton;
//
//	static constexpr const rsize_t str_size = 14;
//
//	_Field_z_               wchar_t m_sBrightness     [ str_size ];
//	_Field_z_               wchar_t m_sCushionShading [ str_size ];
//	_Field_z_               wchar_t m_sHeight         [ str_size ];
//	_Field_z_               wchar_t m_sScaleFactor    [ str_size ];
//
//	_Field_range_( 0,   2 ) INT     m_style = 0;
//	_Field_range_( 0, 100 ) INT     m_nBrightness = 0;
//
//	//TODO: m_nCushionShading is never explicitly initialized.
//	_Field_range_( 0, 100 ) INT     m_nCushionShading = 0;
//
//	//TODO: m_nHeight is never explicitly initialized.
//	_Field_range_( 0, 100 ) INT     m_nHeight = 0;
//
//	//TODO: m_nScaleFactor is never explicitly initialized.
//							//C4820: 'CPageTreemap' : '4' bytes padding added after data member 'CPageTreemap::m_nScaleFactor'
//	_Field_range_( 0, 100 ) INT     m_nScaleFactor = 0;
//
//	DECLARE_MESSAGE_MAP()
//	afx_msg void OnColorChangedTreemapGrid( NMHDR *, LRESULT* result ) {
//		*result = 0;
//		OnSomethingChanged( );
//		}
//	afx_msg void OnColorChangedTreemapHighlight( NMHDR*, LRESULT* result ) {
//		*result = 0;
//		OnSomethingChanged( );
//		}
//	afx_msg void OnVScroll( UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* /*pScrollBar*/ ) {
//		OnSomethingChanged( );
//		ValuesAltered(true, this);
//		}
//	afx_msg void OnLightSourceChanged( NMHDR *, LRESULT * ) {
//		OnSomethingChanged( );
//		ValuesAltered(true, this);
//		}
//	afx_msg void OnBnClickedReset();
//
//};
#endif
