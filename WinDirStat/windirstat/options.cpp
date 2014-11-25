// options.cpp	- Implementation of CPersistence, COptions and CRegistryUser
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	COptions _theOptions;

	const PCTSTR sectionPersistence             = _T( "persistence" );
	const PCTSTR entryShowFileTypes             = _T( "showFileTypes" );
	const PCTSTR entryShowTreemap               = _T( "showTreemap" );
	const PCTSTR entryShowStatusbar             = _T( "showStatusbar" );
	const PCTSTR entryMainWindowPlacement       = _T( "mainWindowPlacement" );
	const PCTSTR entrySplitterPosS              = _T( "%s-splitterPos" );
	const PCTSTR entryColumnOrderS              = _T( "%s-columnOrder" );
	const PCTSTR entryColumnWidthsS             = _T( "%s-columnWidths" );
	const PCTSTR entryDialogRectangleS          = _T( "%s-rectangle" );
	const PCTSTR entryConfigPage                = _T( "configPage" );
	const PCTSTR entryConfigPositionX           = _T( "configPositionX" );
	const PCTSTR entryConfigPositionY           = _T( "configPositionY" );
	const PCTSTR entrySelectDrivesRadio         = _T( "selectDrivesRadio" );
	const PCTSTR entrySelectDrivesFolder        = _T( "selectDrivesFolder" );
	const PCTSTR entrySelectDrivesDrives        = _T( "selectDrivesDrives" );
	const PCTSTR entryShowDeleteWarning         = _T( "showDeleteWarning" );
	const PCTSTR sectionBarState                = _T( "persistence\\barstate" );
	const PCTSTR sectionOptions                 = _T( "options" );
	const PCTSTR entryListGrid                  = _T( "treelistGrid" ); // for compatibility with 1.0.1, this entry is named treelistGrid.
	const PCTSTR entryListStripes               = _T( "listStripes" );
	const PCTSTR entryListFullRowSelection      = _T( "listFullRowSelection" );
	const PCTSTR entryTreelistColorCount        = _T( "treelistColorCount" );
	const PCTSTR entryTreelistColorN            = _T( "treelistColor%d" );
	const PCTSTR entryHumanFormat               = _T( "humanFormat" );
	const PCTSTR entryShowTimeSpent             = _T( "showTimeSpent" );
	const PCTSTR entryTreemapHighlightColor     = _T( "treemapHighlightColor" );
	const PCTSTR entryTreemapStyle              = _T( "treemapStyle" );
	const PCTSTR entryTreemapGrid               = _T( "treemapGrid" );
	const PCTSTR entryTreemapGridColor          = _T( "treemapGridColor" );
	const PCTSTR entryBrightness                = _T( "brightness" );
	const PCTSTR entryHeightFactor              = _T( "heightFactor" );
	const PCTSTR entryScaleFactor               = _T( "scaleFactor" );
	const PCTSTR entryAmbientLight              = _T( "ambientLight" );
	const PCTSTR entryLightSourceX              = _T( "lightSourceX" );
	const PCTSTR entryLightSourceY              = _T( "lightSourceY" );
	const PCTSTR entryFollowMountPoints         = _T( "followMountPoints" );
	const PCTSTR entryFollowJunctionPoints      = _T( "followJunctionPoints" );
	const PCTSTR entryEnabled                   = _T( "enabled" );
	const PCTSTR entryTitle                     = _T( "title" );
	const PCTSTR entryWorksForDrives            = _T( "worksForDrives" );
	const PCTSTR entryWorksForDirectories       = _T( "worksForDirectories" );
	const PCTSTR entryWorksForFilesFolder       = _T( "worksForFilesFolder" );
	const PCTSTR entryWorksForFiles             = _T( "worksForFiles" );
	const PCTSTR entryWorksForUncPaths          = _T( "worksForUncPaths" );
	const PCTSTR entryCommandLine               = _T( "commandLine" );
	const PCTSTR entryRecurseIntoSubdirectories = _T( "recurseIntoSubdirectories" );
	const PCTSTR entryAskForConfirmation        = _T( "askForConfirmation" );
	const PCTSTR entryShowConsoleWindow         = _T( "showConsoleWindow" );
	const PCTSTR entryWaitForCompletion         = _T( "waitForCompletion" );
	const PCTSTR entryRefreshPolicy             = _T( "refreshPolicy" );

	COLORREF treelistColorDefault[TREELISTCOLORCOUNT] = {
		RGB(  64,  64, 140 ),
		RGB( 140,  64,  64 ),
		RGB(  64, 140,  64 ),
		RGB( 140, 140,  64 ),
		RGB(   0,   0, 255 ),
		RGB( 255,   0,   0 ),
		RGB(   0, 255,   0 ),
		RGB( 255, 255,   0 )
		};

	void SanifyRect( _Inout_ CRect& rc ) {
		const INT visible = 30;

		rc.NormalizeRect( );

		CRect rcDesktop;
		CWnd::GetDesktopWindow( )->GetWindowRect( rcDesktop );

		if ( rc.Width( ) > rcDesktop.Width( ) ) {
			rc.right = rc.left + rcDesktop.Width( );
			}
		if ( rc.Height( ) > rcDesktop.Height( ) ) {
			rc.bottom = rc.top + rcDesktop.Height( );
			}
		if ( rc.left < 0 ) {
			rc.OffsetRect( -rc.left, 0 );
			}
		if ( rc.left > rcDesktop.right - visible ) {
			rc.OffsetRect( -visible, 0 );
			}
		if ( rc.top < 0 ) {
			rc.OffsetRect( -rc.top, 0 );
			}
		if ( rc.top > rcDesktop.bottom - visible ) {
			rc.OffsetRect( 0, -visible );
			}
		}

	CString EncodeWindowPlacement( _In_ const WINDOWPLACEMENT& wp ) {
		CString s;
		s.Format(
			_T( "%u,%u," )
			_T( "%ld,%ld,%ld,%ld," )
			_T( "%ld,%ld,%ld,%ld" ),
			wp.flags, wp.showCmd,
			wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y,
			wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom
		);
		return s;
		}

	void DecodeWindowPlacement( _In_ const CString& s, _Inout_ WINDOWPLACEMENT& rwp ) {
		WINDOWPLACEMENT wp;
		wp.length = sizeof( wp );
		INT r = swscanf_s( s, _T( "%u,%u," ) _T( "%ld,%ld,%ld,%ld," ) _T( "%ld,%ld,%ld,%ld" ), &wp.flags, &wp.showCmd, &wp.ptMinPosition.x, &wp.ptMinPosition.y, &wp.ptMaxPosition.x, &wp.ptMaxPosition.y, &wp.rcNormalPosition.left, &wp.rcNormalPosition.right, &wp.rcNormalPosition.top, &wp.rcNormalPosition.bottom );
		if ( r == 10 ) {
			rwp = wp;
			}
		}

	CString MakeSplitterPosEntry( _In_z_ const PCTSTR name ) {
		CString entry;
		entry.Format( entrySplitterPosS, name );
		return entry;
		}

	CString MakeColumnOrderEntry( _In_z_ const PCTSTR name ) {
		CString entry;
		entry.Format( entryColumnOrderS, name );
		return entry;
		}

	CString MakeDialogRectangleEntry( _In_z_ const PCTSTR name ) {
		CString entry;
		entry.Format( entryDialogRectangleS, name );
		return entry;
		}

	CString MakeColumnWidthsEntry( _In_z_ const PCTSTR name ) {
		CString entry;
		entry.Format( entryColumnWidthsS, name );
		return entry;
		}

	void SetProfileString( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR value ) {
		AfxGetApp( )->WriteProfileString( section, entry, value );
		}
	}

class CTreemap;

bool CPersistence::GetShowFileTypes( ) {
	return CRegistryUser::GetProfileBool( sectionPersistence, entryShowFileTypes, true );
	}

bool CPersistence::GetShowTreemap( ) {
	return CRegistryUser::GetProfileBool( sectionPersistence, entryShowTreemap, true );
	}

bool CPersistence::GetShowStatusbar( ) {
	return CRegistryUser::GetProfileBool( sectionPersistence, entryShowStatusbar, true );
	}

void CPersistence::SetShowFileTypes( _In_ const bool show ) {
	CRegistryUser::SetProfileBool( sectionPersistence, entryShowFileTypes, show );
	}

void CPersistence::SetShowTreemap( _In_ const bool show ) {
	CRegistryUser::SetProfileBool( sectionPersistence, entryShowTreemap, show );
	}

void CPersistence::SetShowStatusbar( _In_ const bool show ) {
	CRegistryUser::SetProfileBool( sectionPersistence, entryShowStatusbar, show );
	}

void CPersistence::GetMainWindowPlacement( _Inout_ WINDOWPLACEMENT& wp ) {
	ASSERT( wp.length == sizeof( wp ) );
	auto s = CRegistryUser::GetProfileString_( sectionPersistence, entryMainWindowPlacement, _T( "" ) );
	DecodeWindowPlacement( s, wp );
	SanifyRect( ( CRect & ) wp.rcNormalPosition );
	}

void CPersistence::SetMainWindowPlacement( _In_ const WINDOWPLACEMENT& wp ) {
	auto s = EncodeWindowPlacement( wp );
	SetProfileString( sectionPersistence, entryMainWindowPlacement, s );
	}

void CPersistence::SetSplitterPos( _In_z_ const PCTSTR name, _In_ const bool valid, _In_ const DOUBLE userpos ) {
	INT pos = 0;
	if ( valid ) {
		pos = INT( userpos * 100 );
		}
	else {
		pos = -1;
		}
	CRegistryUser::SetProfileInt( sectionPersistence, MakeSplitterPosEntry( name ), pos );
	}

void CPersistence::GetSplitterPos( _In_z_  const PCTSTR name, _Inout_ bool& valid, _Inout_ DOUBLE& userpos ) {
	auto pos = CRegistryUser::GetProfileInt_( sectionPersistence, MakeSplitterPosEntry( name ), -1 );
	if ( pos > 100 ) {
		valid = false;
		userpos = 0.5;
		}
	else {
		valid = true;
		userpos = ( DOUBLE ) pos / 100;
		}
	}
void CPersistence::GetColumnOrder( _In_z_  const PCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
	GetArray( MakeColumnOrderEntry( name ), arr );
	}
void CPersistence::GetDialogRectangle( _In_z_ const PCTSTR name, _Inout_ CRect& rc ) {
	GetRect( MakeDialogRectangleEntry( name ), rc );
	SanifyRect( rc );
	}
void CPersistence::GetColumnWidths( _In_z_  const PCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
	GetArray( MakeColumnWidthsEntry( name ), arr );
	}

void CPersistence::SetColumnWidths( _In_z_ const PCTSTR name, _In_ const CArray<INT, INT>& arr ) {
	SetArray( MakeColumnWidthsEntry( name ), arr );
	}
void CPersistence::SetColumnOrder( _In_z_ const PCTSTR name, _In_ const CArray<INT, INT>& arr ) {
	SetArray( MakeColumnOrderEntry( name ), arr );
	}
void CPersistence::SetDialogRectangle( _In_z_  const PCTSTR name, _In_ const CRect& rc ) {
	SetRect( MakeDialogRectangleEntry( name ), rc );
	}


INT CPersistence::GetConfigPage( _In_ const INT max_val ) {
	/* changed max to max_val to avoid conflict in ASSERT macro*/
	auto n = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionPersistence, entryConfigPage, 0 ) );
	CheckMinMax( n, 0, max_val );
	ASSERT( ( n >= 0 ) && ( n <= max_val ) );
	return n;
	}

void CPersistence::SetConfigPage( _In_ const INT page ) {
	CRegistryUser::SetProfileInt( sectionPersistence, entryConfigPage, page );
	}

void CPersistence::GetConfigPosition( _Inout_ CPoint& pt ) {
	pt.x = static_cast<LONG>( CRegistryUser::GetProfileInt_( sectionPersistence, entryConfigPositionX, pt.x ) );
	pt.y = static_cast<LONG>( CRegistryUser::GetProfileInt_( sectionPersistence, entryConfigPositionY, pt.y ) );

	CRect rc { pt, CSize( 100, 100 ) };
	SanifyRect( rc );
	pt = rc.TopLeft( );
	}

void CPersistence::SetConfigPosition( _In_ const CPoint pt ) {
	CRegistryUser::SetProfileInt( sectionPersistence, entryConfigPositionX, pt.x );
	CRegistryUser::SetProfileInt( sectionPersistence, entryConfigPositionY, pt.y );
	}

CString CPersistence::GetBarStateSection( ) {
	return sectionBarState;
	}

RADIO CPersistence::GetSelectDrivesRadio( ) {
	auto radio = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionPersistence, entrySelectDrivesRadio, 0 ) );
	CheckMinMax( radio, 0, 2 );
	ASSERT( ( radio >= 0 ) && ( radio <= 2 ) );
	return RADIO( radio );
	}

void CPersistence::SetSelectDrivesRadio( _In_ const INT radio ) {
	CRegistryUser::SetProfileInt( sectionPersistence, entrySelectDrivesRadio, radio );
	}

CString CPersistence::GetSelectDrivesFolder( ) {
	return CRegistryUser::GetProfileString_( sectionPersistence, entrySelectDrivesFolder, _T( "" ) );
	}

void CPersistence::SetSelectDrivesFolder( _In_z_ const PCTSTR folder ) {
	SetProfileString( sectionPersistence, entrySelectDrivesFolder, folder );
	}

void CPersistence::GetSelectDrivesDrives( _Inout_ CStringArray& drives ) {
	drives.RemoveAll( );
	auto s = CRegistryUser::GetProfileString_( sectionPersistence, entrySelectDrivesDrives, _T( "" ) );
	INT i = 0;
	while ( i < s.GetLength( ) ) {
		CString drive;
		while ( i < s.GetLength( ) && s[ i ] != _T( '|' ) ) {
			drive += s[ i ];
			i++;
			}
		if ( i < s.GetLength( ) ) {
			i++;
			}
		drives.Add( drive );
		}
	}

void CPersistence::SetSelectDrivesDrives( _In_ const CStringArray& drives ) {
	CString s;
	auto sizeDrives = drives.GetSize( );
	for ( INT i = 0; i < sizeDrives; i++ ) {
		if ( i > 0 ) {
			s += _T( "|" );
			}
		s += drives[ i ];
		}
	SetProfileString( sectionPersistence, entrySelectDrivesDrives, s );
	}

bool CPersistence::GetShowDeleteWarning( ) {
	return CRegistryUser::GetProfileBool( sectionPersistence, entryShowDeleteWarning, true );
	}

void CPersistence::SetShowDeleteWarning( _In_ const bool show ) {
	CRegistryUser::SetProfileBool( sectionPersistence, entryShowDeleteWarning, show );
	}

void CPersistence::SetArray( _In_z_ const PCTSTR entry, _In_ const CArray<INT, INT>& arr ) {
	CString value;
	for ( INT i = 0; i < arr.GetSize( ); i++ ) {
		CString s;
		s.Format( _T( "%d" ), arr[ i ] );
		if ( i > 0 ) {
			value += _T( "," );
			}
		value += s;
		}
	SetProfileString( sectionPersistence, entry, value );
	}

void CPersistence::GetArray( _In_z_ const PCTSTR entry, _Inout_ CArray<INT, INT>& rarr ) {
	auto s = CRegistryUser::GetProfileString_( sectionPersistence, entry, _T( "" ) );
	CArray<INT, INT> arr;
	INT i = 0;
	while ( i < s.GetLength( ) ) {
		INT n = 0;
		while ( i < s.GetLength( ) && _istdigit( s[ i ] ) ) {
			n *= 10;
			n += s[ i ] - _T( '0' );
			i++;
			}
		arr.Add( n );
		if ( i >= s.GetLength( ) || s[ i ] != _T( ',' ) ) {
			break;
			}
		i++;
		}
	if ( i >= s.GetLength( ) && arr.GetSize( ) == rarr.GetSize( ) ) {
		for ( i = 0; i < rarr.GetSize( ); i++ ) {
			rarr[ i ] = arr[ i ];
			}
		}
	}

void CPersistence::SetRect( _In_z_ const PCTSTR entry, _In_ const CRect& rc ) {
	CString s;
	s.Format( _T( "%d,%d,%d,%d" ), rc.left, rc.top, rc.right, rc.bottom );
	SetProfileString( sectionPersistence, entry, s );
	}

void CPersistence::GetRect( _In_z_ const PCTSTR entry, _Inout_ CRect& rc ) {
	auto s = CRegistryUser::GetProfileString_( sectionPersistence, entry, _T( "" ) );
	CRect tmp;
	auto r = swscanf_s( s, _T( "%d,%d,%d,%d" ), &tmp.left, &tmp.top, &tmp.right, &tmp.bottom );
	if ( r == 4 ) {
		rc = tmp;
		}
	}

/////////////////////////////////////////////////////////////////////////////
_Success_( return != NULL ) COptions* GetOptions( ) {
	ASSERT( ( &_theOptions ) != NULL );
	if ( ( &_theOptions ) != NULL ) {
		return &_theOptions;
		}
	TRACE( _T( "&_theOptions is NULL! This should never happen!\r\n" ) );
	MessageBoxW( NULL, _T( "&_theOptions is NULL! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	std::terminate( );
	}


//COptions::COptions( ) : m_listGrid( true ), m_followJunctionPoints( false ), m_followMountPoints( false ), m_humanFormat( true ), m_listFullRowSelection( true ), m_listStripes( true ), m_showTimeSpent( false ) { }//TODO: check defaults!

void COptions::SetListGrid( _In_ const bool show ) {
	if ( m_listGrid != show ) {
		m_listGrid = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}



void COptions::SetListStripes( _In_ const bool show ) {
	if ( m_listStripes != show ) {
		m_listStripes = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}



void COptions::SetListFullRowSelection( _In_ const bool show ) {
	if ( m_listFullRowSelection != show ) {
		m_listFullRowSelection = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}

//COLORREF COptions::GetTreelistColor( _In_ _In_range_( 0, TREELISTCOLORCOUNT ) const size_t i ) const {
//	ASSERT( i < m_treelistColorCount );
//	return m_treelistColor[ i ];
//	}

void COptions::SetHumanFormat( _In_ const bool human ) {
	if ( m_humanFormat != human ) {
		m_humanFormat = human;
		GetDocument( )->UpdateAllViews( NULL, HINT_NULL );
		GetApp( )->UpdateRamUsage( );
		}
	}

void COptions::SetTreemapHighlightColor( _In_ const COLORREF color ) {
	if ( m_treemapHighlightColor != color ) {
		m_treemapHighlightColor = color;
		GetDocument( )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		}
	}

//_Must_inspect_result_ const CTreemap::Options *COptions::GetTreemapOptions( ) const {
//	return &m_treemapOptions;
//	}

void COptions::SetTreemapOptions( _In_ const Treemap_Options& options ) {
	if ( options.style       != m_treemapOptions.style
	 || options.grid         != m_treemapOptions.grid
	 || options.gridColor    != m_treemapOptions.gridColor
	 || options.brightness   != m_treemapOptions.brightness
	 || options.height       != m_treemapOptions.height
	 || options.scaleFactor  != m_treemapOptions.scaleFactor
	 || options.ambientLight != m_treemapOptions.ambientLight
	 || options.lightSourceX != m_treemapOptions.lightSourceX
	 || options.lightSourceY != m_treemapOptions.lightSourceY ) {
		m_treemapOptions = options;
		GetDocument( )->UpdateAllViews( NULL, HINT_TREEMAPSTYLECHANGED );
		}
	}

void COptions::SaveToRegistry( ) {
	CRegistryUser::SetProfileBool( sectionOptions, entryListGrid, m_listGrid );
	CRegistryUser::SetProfileBool( sectionOptions, entryListStripes, m_listStripes );
	CRegistryUser::SetProfileBool( sectionOptions, entryListFullRowSelection, m_listFullRowSelection );

	CRegistryUser::SetProfileInt( sectionOptions, entryTreelistColorCount, m_treelistColorCount );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		CString entry;
		entry.Format( entryTreelistColorN, i );
		CRegistryUser::SetProfileInt( sectionOptions, entry, static_cast<const INT>( m_treelistColor[ i ] ) );
		}
	CRegistryUser::SetProfileBool( sectionOptions, entryHumanFormat, m_humanFormat );

	CRegistryUser::SetProfileBool( sectionOptions, entryShowTimeSpent, m_showTimeSpent );
	CRegistryUser::SetProfileInt( sectionOptions, entryTreemapHighlightColor, static_cast<const INT>( m_treemapHighlightColor ) );

	SaveTreemapOptions( );

	CRegistryUser::SetProfileBool( sectionOptions, entryFollowMountPoints, m_followMountPoints );
	CRegistryUser::SetProfileBool( sectionOptions, entryFollowJunctionPoints, m_followJunctionPoints );
	// We must distinguish between 'empty' and 'default'. 'Default' will read "", 'Empty' will read "$", Others will read "$text.."
	//const PCTSTR stringPrefix = _T( "$" );

	CString s;
	}

void COptions::LoadFromRegistry( ) {
	m_listGrid = CRegistryUser::GetProfileBool( sectionOptions, entryListGrid, false );
	m_listStripes = CRegistryUser::GetProfileBool( sectionOptions, entryListStripes, false );
	m_listFullRowSelection = CRegistryUser::GetProfileBool( sectionOptions, entryListFullRowSelection, true );

	m_treelistColorCount = static_cast<rsize_t>( CRegistryUser::GetProfileInt_( sectionOptions, entryTreelistColorCount, 4 ) );
	auto temp = static_cast< INT >( m_treelistColorCount );
	CRegistryUser::CheckRange( temp, 1, TREELISTCOLORCOUNT );
	m_treelistColorCount = temp;
	ASSERT( ( m_treelistColorCount >= 1 ) && ( m_treelistColorCount <= TREELISTCOLORCOUNT ) );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		CString entry;
		entry.Format( entryTreelistColorN, i );
		m_treelistColor[ i ] = CRegistryUser::GetProfileInt_( sectionOptions, entry, static_cast<const INT>( treelistColorDefault[ i ] ) );
		}
	m_humanFormat = CRegistryUser::GetProfileBool( sectionOptions, entryHumanFormat, true );

	m_showTimeSpent = CRegistryUser::GetProfileBool( sectionOptions, entryShowTimeSpent, false );
	m_treemapHighlightColor = CRegistryUser::GetProfileInt_( sectionOptions, entryTreemapHighlightColor, RGB( 255, 255, 255 ) );

	ReadTreemapOptions( );

	m_followMountPoints = CRegistryUser::GetProfileBool( sectionOptions, entryFollowMountPoints, false );
	// Ignore junctions by default
	m_followJunctionPoints = CRegistryUser::GetProfileBool( sectionOptions, entryFollowJunctionPoints, false );

	}


void COptions::ReadTreemapOptions( ) {
	Treemap_Options standard = _defaultOptions;

	auto style = CRegistryUser::GetProfileInt_( sectionOptions, entryTreemapStyle, standard.style );
	if ( style != KDirStatStyle && style != SequoiaViewStyle ) {
		style = KDirStatStyle;
		}
	m_treemapOptions.style = ( Treemap_STYLE )style;

	m_treemapOptions.grid = CRegistryUser::GetProfileBool( sectionOptions, entryTreemapGrid, standard.grid );

	m_treemapOptions.gridColor = CRegistryUser::GetProfileInt_( sectionOptions, entryTreemapGridColor, static_cast<const INT>( standard.gridColor ) );

	auto        brightness = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryBrightness, standard.GetBrightnessPercent( ) ) );
	CheckMinMax( brightness, 0, 100 );
	m_treemapOptions.SetBrightnessPercent( brightness );

	auto        height       = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryHeightFactor, standard.GetHeightPercent( ) ) );
	CheckMinMax( height, 0, 100 );
	m_treemapOptions.SetHeightPercent( height );

	auto        scaleFactor  = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryScaleFactor, standard.GetScaleFactorPercent( ) ) );
	CheckMinMax( scaleFactor, 0, 100 );
	m_treemapOptions.SetScaleFactorPercent( scaleFactor );

	auto        ambientLight = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryAmbientLight, standard.GetAmbientLightPercent( ) ) );
	CheckMinMax( ambientLight, 0, 100 );
	m_treemapOptions.SetAmbientLightPercent( ambientLight );

	auto        lightSourceX = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryLightSourceX, standard.GetLightSourceXPercent( ) ) );
	CheckMinMax( lightSourceX, -200, 200 );
	m_treemapOptions.SetLightSourceXPercent( lightSourceX );

	auto        lightSourceY = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionOptions, entryLightSourceY, standard.GetLightSourceYPercent( ) ) );
	CheckMinMax( lightSourceY, -200, 200 );
	m_treemapOptions.SetLightSourceYPercent( lightSourceY );
	}

void COptions::SaveTreemapOptions( ) {
	CRegistryUser::SetProfileInt ( sectionOptions, entryTreemapStyle,     m_treemapOptions.style );
	CRegistryUser::SetProfileBool( sectionOptions, entryTreemapGrid,      m_treemapOptions.grid );
	CRegistryUser::SetProfileInt ( sectionOptions, entryTreemapGridColor, static_cast<const INT>( m_treemapOptions.gridColor ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryBrightness,       m_treemapOptions.GetBrightnessPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryHeightFactor,     m_treemapOptions.GetHeightPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryScaleFactor,      m_treemapOptions.GetScaleFactorPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryAmbientLight,     m_treemapOptions.GetAmbientLightPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryLightSourceX,     m_treemapOptions.GetLightSourceXPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryLightSourceY,     m_treemapOptions.GetLightSourceYPercent( ) );
	}

CString CRegistryUser::GetProfileString_( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR defaultValue ) {
	return AfxGetApp( )->GetProfileString( section, entry, defaultValue );
	}

void CRegistryUser::SetProfileInt( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const INT value ) {
	AfxGetApp( )->WriteProfileInt( section, entry, value );
	}

UINT CRegistryUser::GetProfileInt_( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const INT defaultValue ) {
	return AfxGetApp( )->GetProfileIntW( section, entry, defaultValue );
	}

void CRegistryUser::SetProfileBool( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const bool value ) {
	SetProfileInt( section, entry, ( INT ) value );
	}

bool CRegistryUser::GetProfileBool( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const bool defaultValue ) {
	return GetProfileInt_( section, entry, defaultValue ) != 0;
	}

void CRegistryUser::CheckRange( _Inout_ INT& value, _In_ const INT min_val, _In_ const INT max_val ) {
	/*changed min and max to min_val and max_val to avoid name collision (with min() & max()) in ASSERT macro*/
	ASSERT( min_val < max_val );
	if ( value < min_val ) {
		value = min_val;
		}
	if ( value > max_val ) {
		value = max_val;
		}
	ASSERT( ( value >= min_val ) && ( value <= max_val ) );
	}


// $Log$
// Revision 1.14  2004/11/24 20:28:13  bseifert
// Implemented context menu compromise.
//
// Revision 1.13  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.12  2004/11/12 09:27:01  assarbad
// - Implemented ExplorerStyle option which will not be accessible through the options dialog.
//   It handles where the context menu is being shown.
//
// Revision 1.11  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
