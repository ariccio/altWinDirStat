// PageTreema	p.cpp		- Implementation of CDemoControl and CPageTreemap
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

#include "stdafx.h"
#include "pagetreemap.h"
#include "options.h"
#include "globalhelpers.h"

//#include "windirstat.h"

IMPLEMENT_DYNAMIC( CPageTreemap, CPropertyPage )

void CPageTreemap::DoDataExchange( CDataExchange* pDX ) {
	CPropertyPage::DoDataExchange( pDX );

	//DDX_Control( pDX, IDC_PREVIEW,               m_preview );
	DDX_Control( pDX, IDC_TREEMAPHIGHLIGHTCOLOR, m_highlightColor );
	DDX_Control( pDX, IDC_TREEMAPGRIDCOLOR,      m_gridColor );
	DDX_Control( pDX, IDC_BRIGHTNESS,            m_brightness );
	DDX_Control( pDX, IDC_CUSHIONSHADING,        m_cushionShading );
	DDX_Control( pDX, IDC_HEIGHT,                m_height );
	DDX_Control( pDX, IDC_SCALEFACTOR,           m_scaleFactor );
	DDX_Control( pDX, IDC_LIGHTSOURCE,           m_lightSource );
	DDX_Control( pDX, IDC_RESET,                 m_resetButton );

	if ( !pDX->m_bSaveAndValidate ) {
		UpdateOptions( false );
		UpdateStatics( );
		//m_preview.SetOptions( &m_options );
		}

	DDX_Radio(  pDX, IDC_KDIRSTAT, m_style );
	DDX_Check(  pDX, IDC_TREEMAPGRID, ( int & ) m_grid );

	DDX_Text(   pDX, IDC_STATICBRIGHTNESS,     CString( m_sBrightness ) );
	DDX_Slider( pDX, IDC_BRIGHTNESS,           m_nBrightness );

	DDX_Text(   pDX, IDC_STATICCUSHIONSHADING, CString( m_sCushionShading ) );
	DDX_Slider( pDX, IDC_CUSHIONSHADING,       m_nCushionShading );

	DDX_Text(   pDX, IDC_STATICHEIGHT,         CString( m_sHeight ) );
	DDX_Slider( pDX, IDC_HEIGHT,               m_nHeight );

	DDX_Text(   pDX, IDC_STATICSCALEFACTOR,    CString( m_sScaleFactor ) );
	DDX_Slider( pDX, IDC_SCALEFACTOR,          m_nScaleFactor );

	DDX_XySlider( pDX, IDC_LIGHTSOURCE, m_ptLightSource );

	if ( pDX->m_bSaveAndValidate ) {
		UpdateOptions( );
		}
	}


BEGIN_MESSAGE_MAP(CPageTreemap, CPropertyPage)
	ON_WM_VSCROLL()
	ON_NOTIFY(COLBN_CHANGED, IDC_TREEMAPGRIDCOLOR, OnColorChangedTreemapGrid)
	ON_NOTIFY(COLBN_CHANGED, IDC_TREEMAPHIGHLIGHTCOLOR, OnColorChangedTreemapHighlight)
	ON_BN_CLICKED(IDC_KDIRSTAT,    OnSomethingChanged)
	ON_BN_CLICKED(IDC_SEQUOIAVIEW, OnSomethingChanged)
	ON_BN_CLICKED(IDC_TREEMAPGRID, OnSomethingChanged)
	ON_BN_CLICKED(IDC_RESET, OnBnClickedReset)
	ON_NOTIFY(XYSLIDER_CHANGED, IDC_LIGHTSOURCE, OnLightSourceChanged)
END_MESSAGE_MAP()


BOOL CPageTreemap::OnInitDialog( ) {
	CPropertyPage::OnInitDialog( );

	ValuesAltered( ); // m_undo is invalid

	m_brightness.SetPageSize( 10 );
	m_cushionShading.SetPageSize( 10 );
	m_height.SetRange( 0, CPageTreemap_maxHeight, true );
	m_height.SetPageSize( CPageTreemap_maxHeight / 10 );
	m_scaleFactor.SetPageSize( 10 );
	m_lightSource.SetRange( CSize { 400, 400 } );
	
	auto Options = GetOptions( );
	m_options = Options->m_treemapOptions;
	m_highlightColor.m_preview.SetColor( Options->m_treemapHighlightColor );

	UpdateData( false );
	return TRUE;
	}

void CPageTreemap::OnOK( ) {
	UpdateData( );
	auto Options = GetOptions( );
	Options->SetTreemapOptions( m_options );
	Options->SetTreemapHighlightColor( m_highlightColor.m_preview.m_color );
	CPropertyPage::OnOK( );
	}

void CPageTreemap::UpdateOptions( _In_ const bool save ) {
	if ( save ) {
		m_options.SetBrightnessPercent( 100 - m_nBrightness );
		m_options.SetAmbientLightPercent( m_nCushionShading );
		m_options.SetHeightPercent( CPageTreemap_maxHeight - m_nHeight );
		m_options.SetScaleFactorPercent( 100 - m_nScaleFactor );
		m_options.SetLightSourcePoint( m_ptLightSource );
		m_options.style = ( m_style == 0 ? KDirStatStyle : SequoiaViewStyle );
		m_options.grid = m_grid;
		m_options.gridColor = m_gridColor.m_preview.m_color;
		}
	else {
		m_nBrightness     = 100 - m_options.GetBrightnessPercent( );
		m_nCushionShading = m_options.GetAmbientLightPercent( );
		m_nHeight         = CPageTreemap_maxHeight - m_options.GetHeightPercent( );
		m_nScaleFactor    = 100 - m_options.GetScaleFactorPercent( );
		m_ptLightSource   = m_options.GetLightSourcePoint( );
		m_style           = ( m_options.style == KDirStatStyle ? 0 : 1 );
		m_grid            = m_options.grid;
		m_gridColor.m_preview.SetColor( m_options.gridColor );
		}
	}

void CPageTreemap::UpdateStatics( ) {
	int format_results[ 4 ] = { -1, -1, -1, -1 };
	
	format_results[ 0 ] = swprintf_s( m_sBrightness,     L"%d", ( 100 - m_nBrightness ) );
	format_results[ 1 ] = swprintf_s( m_sCushionShading, L"%d", ( 100 - m_nCushionShading ) );
	format_results[ 2 ] = swprintf_s( m_sHeight,         L"%d", ( ( CPageTreemap_maxHeight - m_nHeight ) / ( CPageTreemap_maxHeight / 100 ) ) );
	format_results[ 3 ] = swprintf_s( m_sScaleFactor,    L"%d", ( 100 - m_nScaleFactor ) );

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
	UpdateData( false );
	SetModified( );
	}
