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
	}


IMPLEMENT_DYNAMIC( CPageTreemap, CPropertyPage )

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

void CPageTreemap::ValuesAltered( _In_ const bool altered ) noexcept {
	m_altered = ( altered ? TRUE : FALSE );
	m_resetButton.SetWindowTextW( m_altered ? L"&Reset to\r\nDefaults" : L"Back to\r\n&User Settings" );
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


#else

#endif