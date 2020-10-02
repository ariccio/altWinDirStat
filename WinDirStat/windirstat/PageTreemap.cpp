// PageTreemap.cpp		- Implementation of CDemoControl and CPageTreemap
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_PAGETREEMAP_CPP
#define WDS_PAGETREEMAP_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "pagetreemap.h"
#include "options.h"
#include "globalhelpers.h"

//#include "windirstat.h"

namespace {
	constexpr const int CPageTreemap_maxHeight = 200;

	template<typename slidT>
	void initSlid(slidT slid, const int minimum, const int maximum, const int ticFreq, const int lineSize, const int pageSize, const int initValue) noexcept {
		slid.SetRange(minimum, maximum);
		slid.SetPageSize(pageSize);
		slid.SetTicFreq(ticFreq);
		slid.SetLineSize(lineSize);
		slid.SetPos(initValue);
		}
	}

/*
#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL, NULL)

	becomes

	#define IMPLEMENT_DYNAMIC(CPageTreemap, CPropertyPage) \
	IMPLEMENT_RUNTIMECLASS(CPageTreemap, CPropertyPage, 0xFFFF, NULL, NULL)

	becomes

	#define IMPLEMENT_RUNTIMECLASS(CPageTreemap, CPropertyPage, 0xFFFF, NULL, NULL) \
	AFX_COMDAT const CRuntimeClass CPageTreemap::class##CPageTreemap = { \
		"CPageTreemap", sizeof(class class_name), 0xFFFF, NULL, \
			RUNTIME_CLASS(CPropertyPage), NULL, NULL }; \
	CRuntimeClass* CPageTreemap::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(CPageTreemap); }

and this:
	AFX_COMDAT const CRuntimeClass CPageTreemap::class##CPageTreemap = { \
		"CPageTreemap", sizeof(class class_name), 0xFFFF, NULL, \
			RUNTIME_CLASS(CPropertyPage), NULL, NULL }; \
	CRuntimeClass* CPageTreemap::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(CPageTreemap); }

		becomes

	AFX_COMDAT const CRuntimeClass CPageTreemap::class##CPageTreemap = { \
		"CPageTreemap", sizeof(class class_name), 0xFFFF, NULL, \
			((CRuntimeClass*)(&CPropertyPage::classCPropertyPage), NULL, NULL }; \
	CRuntimeClass* CPageTreemap::GetRuntimeClass() const \
		{ return (CRuntimeClass*)(&CPageTreemap::classCPageTreemap); }

		#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)



*/
//IMPLEMENT_DYNAMIC( CPageTreemap, CPropertyPage )
AFX_COMDAT const CRuntimeClass CPageTreemap::classCPageTreemap = {
	"CPageTreemap", sizeof(class CPageTreemap), 0xFFFF, NULL, const_cast<CRuntimeClass*>(&CPropertyPage::classCPropertyPage), NULL, NULL
	};
CRuntimeClass* CPageTreemap::GetRuntimeClass() const {
	return const_cast<CRuntimeClass*>(&CPageTreemap::classCPageTreemap);
	}


void CPageTreemap::DoDataExchange( CDataExchange* pDX ) {
	CWnd::DoDataExchange( pDX );

	DDX_Control ( pDX, IDC_TREEMAPHIGHLIGHTCOLOR, m_highlightColor );
	DDX_Control ( pDX, IDC_TREEMAPGRIDCOLOR,      m_gridColor );
	DDX_Control ( pDX, IDC_BRIGHTNESS,            m_brightness );
	DDX_Control ( pDX, IDC_CUSHIONSHADING,        m_cushionShading );
	DDX_Control ( pDX, IDC_HEIGHT,                m_height );
	DDX_Control ( pDX, IDC_SCALEFACTOR,           m_scaleFactor );
	DDX_Control ( pDX, IDC_LIGHTSOURCE,           m_lightSource );
	DDX_Control ( pDX, IDC_RESET,                 m_resetButton );

	if ( !pDX->m_bSaveAndValidate ) {
		UpdateOptions( false );
		UpdateStatics( );
		}

	DDX_Radio   ( pDX, IDC_KDIRSTAT, ( int & ) m_style );
	DDX_Text    ( pDX, IDC_STATICBRIGHTNESS,     m_sBrightness,     str_size );
	DDX_Slider  ( pDX, IDC_BRIGHTNESS,           m_nBrightness );

	DDX_Text    ( pDX, IDC_STATICCUSHIONSHADING, m_sCushionShading, str_size );
	DDX_Slider  ( pDX, IDC_CUSHIONSHADING,       m_nCushionShading );

	DDX_Text    ( pDX, IDC_STATICHEIGHT,         m_sHeight,         str_size );
	DDX_Slider  ( pDX, IDC_HEIGHT,               m_nHeight );

	DDX_Text    ( pDX, IDC_STATICSCALEFACTOR,    m_sScaleFactor,    str_size );
	DDX_Slider  ( pDX, IDC_SCALEFACTOR,          m_nScaleFactor );

	DDX_XySlider( pDX, IDC_LIGHTSOURCE,          m_ptLightSource );


	if ( pDX->m_bSaveAndValidate ) {
		UpdateOptions( );
		}
	}


BEGIN_MESSAGE_MAP(CPageTreemap, CPropertyPage)
	ON_WM_VSCROLL()
	ON_NOTIFY(COLBN_CHANGED, IDC_TREEMAPGRIDCOLOR, &( CPageTreemap::OnColorChangedTreemapGrid ) )
	ON_NOTIFY(COLBN_CHANGED, IDC_TREEMAPHIGHLIGHTCOLOR, &( CPageTreemap::OnColorChangedTreemapHighlight ) )
	ON_BN_CLICKED(IDC_KDIRSTAT,    &( CPageTreemap::OnSomethingChanged ) )
	ON_BN_CLICKED(IDC_SEQUOIAVIEW, &( CPageTreemap::OnSomethingChanged ) )
	ON_BN_CLICKED(IDC_TREEMAPGRID, &( CPageTreemap::OnSomethingChanged ) )
	ON_BN_CLICKED(IDC_RESET, &( CPageTreemap::OnBnClickedReset ) )
	ON_NOTIFY(XYSLIDER_CHANGED, IDC_LIGHTSOURCE, &( CPageTreemap::OnLightSourceChanged ) )
END_MESSAGE_MAP()


//WM_INITDIALOG message: https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
//The dialog box procedure should return TRUE to direct the system to set the keyboard focus to the control specified by wParam.
//Otherwise, it should return FALSE to prevent the system from setting the default keyboard focus.
BOOL WTLTreemapPage::OnInitDialog(const HWND hWnd, const LPARAM /*lparam*/) {
	TRACE(L"Initialize WTL dialog\r\n-------\r\n");
	debugDataExchangeMembers();
#pragma push_macro("SubclassWindow")
#undef SubclassWindow
	m_resetButton.SubclassWindow(hwnd::GetDlgItem(m_hWnd, IDC_RESET));
	//m_cushionShading.Attach(hwnd::GetDlgItem(m_hWnd, IDC_CUSHIONSHADING));
	//m_brightness.Attach(hwnd::GetDlgItem(m_hWnd, IDC_BRIGHTNESS));
	//m_height.Attach(hwnd::GetDlgItem(m_hWnd, IDC_HEIGHT));
	//m_scaleFactor.Attach(hwnd::GetDlgItem(m_hWnd, IDC_HEIGHT));
	

	m_highlightColor.SubclassWindow(hwnd::GetDlgItem(m_hWnd, IDC_TREEMAPHIGHLIGHTCOLOR));
	//m_highlightColor.Attach(GetDlgItem(IDC_TREEMAPHIGHLIGHTCOLOR));
	//m_gridColor.Attach(GetDlgItem(IDC_TREEMAPGRIDCOLOR));
	m_gridColor.SubclassWindow(hwnd::GetDlgItem(m_hWnd, IDC_TREEMAPGRIDCOLOR));
#pragma pop_macro("SubclassWindow")

	VERIFY(DoDataExchange(DDX_LOAD));

	const COptions* const Options = GetOptions();
	m_options = Options->m_treemapOptions;

	debugDataExchangeMembers();
	//debugSliderPosition();
	m_brightness = GetDlgItem(IDC_BRIGHTNESS);
	initSlid(m_brightness, 0, 100, 1, 1, 10, 100 - m_options.brightness);

	m_cushionShading = GetDlgItem(IDC_CUSHIONSHADING);
	initSlid(m_cushionShading, 0, 100, 1, 1, 10, m_options.GetAmbientLightPercent());

	m_height = GetDlgItem(IDC_HEIGHT);
	initSlid(m_height, 0, CPageTreemap_maxHeight, 1, 1, CPageTreemap_maxHeight / 10, (CPageTreemap_maxHeight - m_options.GetHeightPercent()));
	//m_height.SetRange(0, CPageTreemap_maxHeight, true);
	//m_height.SetPageSize(CPageTreemap_maxHeight / 10);

	m_scaleFactor = GetDlgItem(IDC_SCALEFACTOR);
	initSlid(m_scaleFactor, 0, 100, 1, 1, 10, (100 - m_options.GetScaleFactorPercent()));
	//m_lightSource.SetRange( CSize { 400, 400 } );
	//m_lightSource.m_externalRange = WTL::CSize{ 400, 400 };

	//m_highlightColor.Attach(GetDlgItem(IDC_TREEMAPHIGHLIGHTCOLOR));
	m_highlightColor.m_preview.SetColor(Options->m_treemapHighlightColor);

	//must DDX_LOAD after UpdateOptions(false)
	UpdateOptions(false);
	VERIFY(DoDataExchange(DDX_LOAD));
	debugDataExchangeMembers();
	debugSliderPosition();
	TRACE(L"initialization complete\r\n-------\r\n");
	//ValuesAltered(true); // m_undo is invalid
	return TRUE;
}



BOOL CPageTreemap::OnInitDialog( ) {
	VERIFY( CDialog::OnInitDialog( ) );
	ValuesAltered( ); // m_undo is invalid

	m_brightness.SetPageSize( 10 );
	m_cushionShading.SetPageSize( 10 );
	m_height.SetRange( 0, CPageTreemap_maxHeight, true );
	m_height.SetPageSize( CPageTreemap_maxHeight / 10 );
	m_scaleFactor.SetPageSize( 10 );
	//m_lightSource.SetRange( CSize { 400, 400 } );
	m_lightSource.m_externalRange = WTL::CSize { 400, 400 };
	
	const COptions* const Options = GetOptions( );
	m_options = Options->m_treemapOptions;
	m_highlightColor.m_preview.SetColor( Options->m_treemapHighlightColor );

	VERIFY( CWnd::UpdateData( false ) );
	return TRUE;
	}

void CPageTreemap::OnOK( ) {
	VERIFY( CWnd::UpdateData( ) );
	const auto Options = GetOptions( );
	Options->SetTreemapOptions( m_options );
	Options->SetTreemapHighlightColor( m_highlightColor.m_preview.m_color );
	CPropertyPage::OnOK( );
	}

int WTLTreemapPage::OnApply() {
	const BOOL result = DoDataExchange(DDX_SAVE);
	//if (result == PSNRET_NOERROR) {
	if (!result) {
		return PSNRET_INVALID;
		}
	//const auto Options = GetOptions();
	//Options->SetTreemapOptions(m_options);
	//Options->SetTreemapHighlightColor(m_highlightColor.m_preview.m_color);
	return PSNRET_NOERROR;
	}


void WTLTreemapPage::ValuesAltered( _In_ const bool altered ) noexcept {
	m_altered = (altered ? TRUE : FALSE);
	m_resetButton.SetWindowTextW(m_altered ? L"&Reset to\r\nDefaults" : L"Back to\r\n&User Settings");
	}

void CPageTreemap::ValuesAltered( _In_ const bool altered ) noexcept {
	m_altered = ( altered ? TRUE : FALSE );
	m_resetButton.SetWindowTextW( m_altered ? L"&Reset to\r\nDefaults" : L"Back to\r\n&User Settings" );
	}

void WTLTreemapPage::UpdateOptions(_In_ const bool save) noexcept {
	static_assert(std::is_convertible< decltype(m_style), std::underlying_type< decltype(m_options.style) >::type>::value, "");
	if (save) {
		m_options.SetBrightnessPercent(100 - m_nBrightness);
		m_options.SetScaleFactorPercent(100 - m_nScaleFactor);
		m_options.SetAmbientLightPercent(m_nCushionShading);
		m_options.SetHeightPercent(CPageTreemap_maxHeight - m_nHeight);

		//m_options.SetLightSourcePoint(m_ptLightSource);
		m_options.style = ((m_style == 0) ? Treemap_STYLE::KDirStatStyle : Treemap_STYLE::SequoiaViewStyle);
		m_options.grid = ((m_grid == FALSE) ? false : true);
		m_options.gridColor = m_gridColor.m_preview.m_color;

	}
	else {
		m_nBrightness = (100 - m_options.GetBrightnessPercent());
		m_nScaleFactor = (100 - m_options.GetScaleFactorPercent());
		m_nCushionShading = m_options.GetAmbientLightPercent();
		m_nHeight = CPageTreemap_maxHeight - m_options.GetHeightPercent();
		//m_ptLightSource = m_options.GetLightSourcePoint();
		m_style = ((m_options.style == Treemap_STYLE::KDirStatStyle) ? 0 : 1);
		m_grid = (m_options.grid ? TRUE : FALSE);
		m_gridColor.m_preview.SetColor(m_options.gridColor);
	}
}

void WTLTreemapPage::UpdateStatics() {
	int format_results[4] = { -1, -1, -1, -1 };

	format_results[0] = ::swprintf_s(m_sBrightness, str_size, L"%d", (100 - m_nBrightness));
	format_results[1] = ::swprintf_s(m_sCushionShading, str_size, L"%d", (100 - m_nCushionShading));
	format_results[2] = ::swprintf_s(m_sHeight, str_size, L"%d", ((CPageTreemap_maxHeight - m_nHeight) / (CPageTreemap_maxHeight / 100)));
	format_results[3] = ::swprintf_s(m_sScaleFactor, str_size, L"%d", (100 - m_nScaleFactor));


	TRACE(L"Statics:\r\n");
	TRACE(L"\tm_sBrightness: %s\r\n", m_sBrightness);
	TRACE(L"\tm_sCushionShading: %s\r\n", m_sCushionShading);
	TRACE(L"\tm_sHeight: %s\r\n", m_sHeight);
	TRACE(L"\tm_sScaleFactor: %s\r\n", m_sScaleFactor);
	//Not vectorized: 1304, loop includes assignments of different sizes
	for (rsize_t i = 0; i < 4; ++i) {
		ASSERT(format_results[i] != -1);
		if (format_results[i] == -1) {
			displayWindowsMsgBoxWithMessage(std::wstring(L"format_results[ ") + std::to_wstring(i) + std::wstring(L" ] == -1! swprintf_s failed!"));
			std::terminate();
		}
	}
}


void CPageTreemap::UpdateOptions( _In_ const bool save ) noexcept {
	static_assert( std::is_convertible< decltype( m_style ), std::underlying_type< decltype( m_options.style ) >::type>::value, "" );
	if ( save ) {
		m_options.SetBrightnessPercent( 100 - m_nBrightness );
		m_options.SetScaleFactorPercent( 100 - m_nScaleFactor );
		m_options.SetAmbientLightPercent( m_nCushionShading );
		m_options.SetHeightPercent( CPageTreemap_maxHeight - m_nHeight );
		
		m_options.SetLightSourcePoint( m_ptLightSource );
		m_options.style     = ( ( m_style == 0 ) ? Treemap_STYLE::KDirStatStyle : Treemap_STYLE::SequoiaViewStyle );
		m_options.grid      = ( ( m_grid == FALSE ) ? false : true );
		m_options.gridColor = m_gridColor.m_preview.m_color;
		}
	else {
		m_nBrightness     = ( 100 - m_options.GetBrightnessPercent( ) );
		m_nScaleFactor    = ( 100 - m_options.GetScaleFactorPercent( ) );
		m_nCushionShading = m_options.GetAmbientLightPercent( );
		m_nHeight         = CPageTreemap_maxHeight - m_options.GetHeightPercent( );
		m_ptLightSource   = m_options.GetLightSourcePoint( );
		m_style           = ( ( m_options.style == Treemap_STYLE::KDirStatStyle ) ? 0 : 1 );
		m_grid            = ( m_options.grid ? TRUE : FALSE );
		m_gridColor.m_preview.SetColor( m_options.gridColor );
		}
	}

void CPageTreemap::UpdateStatics( ) {
	int format_results[ 4 ] = { -1, -1, -1, -1 };
	
	format_results[ 0 ] = ::swprintf_s( m_sBrightness,     str_size, L"%d", ( 100 - m_nBrightness ) );
	format_results[ 1 ] = ::swprintf_s( m_sCushionShading, str_size, L"%d", ( 100 - m_nCushionShading ) );
	format_results[ 2 ] = ::swprintf_s( m_sHeight,         str_size, L"%d", ( ( CPageTreemap_maxHeight - m_nHeight ) / ( CPageTreemap_maxHeight / 100 ) ) );
	format_results[ 3 ] = ::swprintf_s( m_sScaleFactor,    str_size, L"%d", ( 100 - m_nScaleFactor ) );

	//Not vectorized: 1304, loop includes assignments of different sizes
	for ( rsize_t i = 0; i < 4; ++i ) {
		ASSERT( format_results[ i ] != -1 );
		if ( format_results[ i ] == -1 ) {
			displayWindowsMsgBoxWithMessage( std::wstring( L"format_results[ " ) + std::to_wstring( i ) + std::wstring( L" ] == -1! swprintf_s failed!" ) );
			std::terminate( );
			}
		}
	}


void CPageTreemap::OnBnClickedReset( ) {
	Treemap_Options o;
	if ( m_altered ) {
		o = _defaultOptions;
		m_undo = m_options;
		}
	else {
		o = m_undo;
		}
	m_options = o;

	ValuesAltered( !m_altered );
	VERIFY( CWnd::UpdateData( false ) );
	SetModified( );
	}

//WM_COMMAND message: https://docs.microsoft.com/en-us/windows/win32/menurc/wm-command
// here, HIWORD(wPARAM) is uNotify code, LOWORD(WPARAM) is nID, and hWnd is LPARAM
void WTLTreemapPage::onResetCommand(UINT uNotifyCode, int nID, HWND hWnd) noexcept {
	TRACE(L"uNotifyCode %u, nID: %i, hWnd: %p\r\n", uNotifyCode, nID, hWnd);
	}


//WM_SETCURSOR message: https://docs.microsoft.com/en-us/windows/win32/menurc/wm-setcursor
//If an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
LRESULT WTLTreemapPage::OnSetCursor_Reset(const HWND hwndCtrl, UINT uHitTest, UINT uMouseMsg) {
	TRACE(L"uHitTest: %i, uMouseMsg: %i\r\n", uHitTest, uMouseMsg);
	Treemap_Options o;
	if (m_altered) {
		o = _defaultOptions;
		m_undo = m_options;
		}
	else {
		o = m_undo;
	}
	m_options = o;

	ValuesAltered(!m_altered);
	SetModified();
	return TRUE;
	}

void WTLTreemapPage::OnSomethingChanged() noexcept {
	TRACE(L"OnSomethingChanged()\r\n");
	debugSliderPosition();
	TRACE(L"m_nBrightness: %i\r\n", m_nBrightness);
	//VERIFY(DoDataExchange(DDX_SAVE));
	TRACE(L"m_nBrightness: %i\r\n", m_nBrightness);
	//VERIFY(DoDataExchange(false));
	TRACE(L"m_sBrightness: %s\r\n", m_sBrightness);
	UpdateStatics();
	TRACE(L"m_sBrightness: %s\r\n", m_sBrightness);
	UpdateOptions(true);
	TRACE(L"m_nBrightness: %i\r\n", m_nBrightness);
	VERIFY(DoDataExchange(DDX_LOAD));
	TRACE(L"m_nBrightness: %i\r\n", m_nBrightness);
	debugSliderPosition();
	//SetModified();
	TRACE(L"end OnSomethingChanged()\r\n\r\n");
}


void WTLTreemapPage::setPosition(const int nPos, const HWND pScrollBar) {
	debugDataExchangeMembers();
	if (pScrollBar == m_brightness.m_hWnd) {
		m_brightness.SetPos(nPos);
		m_nBrightness = nPos;
		OnSomethingChanged();
	}
	else if (pScrollBar == m_cushionShading.m_hWnd) {
		m_cushionShading.SetPos(nPos);

		m_nCushionShading = nPos;
		OnSomethingChanged();
	}
	else if (pScrollBar == m_height.m_hWnd) {
		m_height.SetPos(nPos);
		m_nHeight = nPos;
		OnSomethingChanged();
	}
	else if (pScrollBar == m_scaleFactor.m_hWnd) {
		m_scaleFactor.SetPos(nPos);
		m_nScaleFactor = nPos;
		OnSomethingChanged();
		}
	else {
		TRACE(L"WARNING: not a known scroll bar. Position %i not set for %p\r\n", nPos, pScrollBar);
		}
	debugDataExchangeMembers();

	}

void WTLTreemapPage::OnVScroll(int nSBCode, short nPos, HWND pScrollBar) {
	TRACE(L"OnVScroll\r\n");
	//WM_VSCROLL (Trackbar) notification code: https://docs.microsoft.com/en-us/windows/win32/controls/wm-vscroll--trackbar-
	//The HIWORD specifies the current position of the slider if the LOWORD is TB_THUMBPOSITION or TB_THUMBTRACK. For all other notification codes, the high-order word is zero; send the TBM_GETPOS message to determine the slider position.
	if ((nSBCode == TB_THUMBTRACK) || (nSBCode == TB_THUMBPOSITION)) {
		//TRACE(L"%u\r\n", nPos);
		setPosition(nPos, pScrollBar);
		}
	else {
		//TBM_GETPOS message: https://docs.microsoft.com/en-us/windows/win32/controls/tbm-getpos
		const auto positionResult = ::SendMessage(pScrollBar, TBM_GETPOS, 0, 0);
		const int position = static_cast<int>(positionResult);
		setPosition(position, pScrollBar);
		}
	debugSliderPosition();
	DoDataExchange(DDX_SAVE);
	//OnSomethingChanged();
	ValuesAltered();
	debugSliderPosition();
	//WTL::CTrackBarCtrl::
	TRACE(L"End OnVScroll\r\n");
	}


void WTLTreemapPage::OnColorChangedTreemapGrid(const NMHDR*) noexcept {
	OnSomethingChanged();
	}
void WTLTreemapPage::OnColorChangedTreemapHighlight(const NMHDR*) noexcept {
	OnSomethingChanged();
	}


//WM_NOTIFY message: https://docs.microsoft.com/en-us/windows/win32/controls/wm-notify
//An application should use the hwndFrom or idFrom member of the NMHDR structure (passed as the lParam parameter) to identify the control.
LRESULT WTLTreemapPage::onWM_NOTIFY(const int wParam, const NMHDR* const lParam) {
	const NMHDR* const notify_message_header = lParam;

	if (notify_message_header->code == COLBN_CHANGED) {
		__debugbreak();
		}
	//break to look for COLBN_CHANGED (0x87)
	if (IDC_TREEMAPGRIDCOLOR == notify_message_header->idFrom) {
		OnColorChangedTreemapGrid(notify_message_header);
		return TRUE;
		}
	if (IDC_TREEMAPHIGHLIGHTCOLOR == notify_message_header->idFrom) {
		OnColorChangedTreemapHighlight(notify_message_header);
		return TRUE;
		}
	return FALSE;
	}



LRESULT WTLTreemapPage::OnMessageHandlerEX(UINT uMsg, WPARAM wParam, LPARAM lParam) {

	return FALSE;
}


void WTLTreemapPage::debugDataExchangeMembers() const noexcept {
	TRACE(L"m_nBrightness: %i\r\n", m_nBrightness);
	TRACE(L"m_nCushionShading: %i\r\n", m_nCushionShading);
	TRACE(L"m_nHeight: %i\r\n", m_nHeight);
	TRACE(L"m_nScaleFactor: %i\r\n", m_nScaleFactor);
	}

void WTLTreemapPage::debugSliderPosition() const noexcept {
	TRACE(L"brightness slider: %i\r\n", m_brightness.GetPos());
	TRACE(L"cushion sh slider: %i\r\n", m_cushionShading.GetPos());
	TRACE(L"height     slider: %i\r\n", m_height.GetPos());
	TRACE(L"scale fact slider: %i\r\n", m_scaleFactor.GetPos());
}

#else

#endif

