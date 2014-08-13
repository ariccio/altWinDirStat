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
#include "windirstat.h"
#include "dirstatdoc.h"
#include "options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	COptions _theOptions;

	const LPCTSTR sectionPersistence             = _T( "persistence" );
	const LPCTSTR entryShowFreeSpace             = _T( "showFreeSpace" );
	const LPCTSTR entryShowUnknown               = _T( "showUnknown" );
	const LPCTSTR entryShowFileTypes             = _T( "showFileTypes" );
	const LPCTSTR entryShowTreemap               = _T( "showTreemap" );
	const LPCTSTR entryShowToolbar               = _T( "showToolbar" );
	const LPCTSTR entryShowStatusbar             = _T( "showStatusbar" );
	const LPCTSTR entryMainWindowPlacement       = _T( "mainWindowPlacement" );
	const LPCTSTR entrySplitterPosS              = _T( "%s-splitterPos" );
	const LPCTSTR entryColumnOrderS              = _T( "%s-columnOrder" );
	const LPCTSTR entryColumnWidthsS             = _T( "%s-columnWidths" );
	const LPCTSTR entryDialogRectangleS          = _T( "%s-rectangle" );
	const LPCTSTR entryConfigPage                = _T( "configPage" );
	const LPCTSTR entryConfigPositionX           = _T( "configPositionX" );
	const LPCTSTR entryConfigPositionY           = _T( "configPositionY" );
	const LPCTSTR entrySelectDrivesRadio         = _T( "selectDrivesRadio" );
	const LPCTSTR entrySelectDrivesFolder        = _T( "selectDrivesFolder" );
	const LPCTSTR entrySelectDrivesDrives        = _T( "selectDrivesDrives" );
	const LPCTSTR entryShowDeleteWarning         = _T( "showDeleteWarning" );
	const LPCTSTR sectionBarState                = _T( "persistence\\barstate" );


	const LPCTSTR sectionOptions                 = _T( "options" );
	const LPCTSTR entryListGrid                  = _T( "treelistGrid" ); // for compatibility with 1.0.1, this entry is named treelistGrid.
	const LPCTSTR entryListStripes               = _T( "listStripes" );
	const LPCTSTR entryListFullRowSelection      = _T( "listFullRowSelection" );
	const LPCTSTR entryTreelistColorCount        = _T( "treelistColorCount" );
	const LPCTSTR entryTreelistColorN            = _T( "treelistColor%d" );
	const LPCTSTR entryHumanFormat               = _T( "humanFormat" );
	const LPCTSTR entryPacmanAnimation           = _T( "pacmanAnimation" );
	const LPCTSTR entryShowTimeSpent             = _T( "showTimeSpent" );
	const LPCTSTR entryTreemapHighlightColor     = _T( "treemapHighlightColor" );
	const LPCTSTR entryTreemapStyle              = _T( "treemapStyle" );
	const LPCTSTR entryTreemapGrid               = _T( "treemapGrid" );
	const LPCTSTR entryTreemapGridColor          = _T( "treemapGridColor" );
	const LPCTSTR entryBrightness                = _T( "brightness" );
	const LPCTSTR entryHeightFactor              = _T( "heightFactor" );
	const LPCTSTR entryScaleFactor               = _T( "scaleFactor" );
	const LPCTSTR entryAmbientLight              = _T( "ambientLight" );
	const LPCTSTR entryLightSourceX              = _T( "lightSourceX" );
	const LPCTSTR entryLightSourceY              = _T( "lightSourceY" );
	const LPCTSTR entryFollowMountPoints         = _T( "followMountPoints" );
	const LPCTSTR entryFollowJunctionPoints      = _T( "followJunctionPoints" );
	const LPCTSTR entryUseWdsLocale              = _T( "useWdsLocale" );

	const LPCTSTR entryEnabled                   = _T( "enabled" );
	const LPCTSTR entryTitle                     = _T( "title" );
	const LPCTSTR entryWorksForDrives            = _T( "worksForDrives" );
	const LPCTSTR entryWorksForDirectories       = _T( "worksForDirectories" );
	const LPCTSTR entryWorksForFilesFolder       = _T( "worksForFilesFolder" );
	const LPCTSTR entryWorksForFiles             = _T( "worksForFiles" );
	const LPCTSTR entryWorksForUncPaths          = _T( "worksForUncPaths" );
	const LPCTSTR entryCommandLine               = _T( "commandLine" );
	const LPCTSTR entryRecurseIntoSubdirectories = _T( "recurseIntoSubdirectories" );
	const LPCTSTR entryAskForConfirmation        = _T( "askForConfirmation" );
	const LPCTSTR entryShowConsoleWindow         = _T( "showConsoleWindow" );
	const LPCTSTR entryWaitForCompletion         = _T( "waitForCompletion" );
	const LPCTSTR entryRefreshPolicy             = _T( "refreshPolicy" );
	const LPCTSTR entryReportSubject             = _T( "reportSubject" );
	const LPCTSTR entryReportPrefix              = _T( "reportPrefix" );
	const LPCTSTR entryReportSuffix              = _T( "reportSuffix" );

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

}

/////////////////////////////////////////////////////////////////////////////
bool CPersistence::GetShowFreeSpace() {
	return GetProfileBool(sectionPersistence, entryShowFreeSpace, false);
	}

void CPersistence::SetShowFreeSpace( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowFreeSpace, show);
	}

bool CPersistence::GetShowUnknown() {
	return GetProfileBool(sectionPersistence, entryShowUnknown, false);
	}

void CPersistence::SetShowUnknown( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowUnknown, show);
	}

bool CPersistence::GetShowFileTypes() {
	return GetProfileBool(sectionPersistence, entryShowFileTypes, true);
	}

void CPersistence::SetShowFileTypes( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowFileTypes, show);
	}

bool CPersistence::GetShowTreemap() {
	return GetProfileBool(sectionPersistence, entryShowTreemap, true);
	}

void CPersistence::SetShowTreemap( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowTreemap, show);
	}

bool CPersistence::GetShowToolbar() {
	return GetProfileBool(sectionPersistence, entryShowToolbar, true);
	}

void CPersistence::SetShowToolbar( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowToolbar, show);
	}

bool CPersistence::GetShowStatusbar() {
	return GetProfileBool(sectionPersistence, entryShowStatusbar, true);
	}

void CPersistence::SetShowStatusbar( _In_ const bool show ) {
	SetProfileBool(sectionPersistence, entryShowStatusbar, show);
	}

void CPersistence::GetMainWindowPlacement( _Inout_ WINDOWPLACEMENT& wp) {
	ASSERT( wp.length == sizeof( wp ) );
	auto s = GetProfileString( sectionPersistence, entryMainWindowPlacement, _T( "" ) );
	DecodeWindowPlacement( s, wp );
	SanifyRect( ( CRect & ) wp.rcNormalPosition );
	}

void CPersistence::SetMainWindowPlacement(_In_ const WINDOWPLACEMENT& wp) {
	CString s = EncodeWindowPlacement( wp );
	SetProfileString( sectionPersistence, entryMainWindowPlacement, s );
	}

void CPersistence::SetSplitterPos( _In_ const LPCTSTR name, _In_ const bool valid, _In_ const DOUBLE userpos ) {
	INT pos = 0;
	if ( valid ) {
		pos = INT( userpos * 100 );
		}
	else {
		pos = -1;
		}
	SetProfileInt( sectionPersistence, MakeSplitterPosEntry( name ), pos );
	}

void CPersistence::GetSplitterPos( _In_ const LPCTSTR name, _Inout_ bool& valid, _Inout_ DOUBLE& userpos ) {
	auto pos = GetProfileInt( sectionPersistence, MakeSplitterPosEntry( name ), -1 );
	if (pos < 0 || pos > 100) {
		valid = false;
		userpos = 0.5;
		}
	else {
		valid = true;
		userpos = ( DOUBLE ) pos / 100;
		}
	}

void CPersistence::SetColumnOrder(_In_ const LPCTSTR name, _In_ const CArray<INT, INT>& arr) {
	SetArray( MakeColumnOrderEntry( name ), arr );
	}

void CPersistence::GetColumnOrder( _In_ const LPCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
	GetArray( MakeColumnOrderEntry( name ), arr );
	}

void CPersistence::SetColumnWidths(_In_ const LPCTSTR name, _In_ const CArray<INT, INT>& arr) {
	SetArray( MakeColumnWidthsEntry( name ), arr );
	}

void CPersistence::GetColumnWidths( _In_ const LPCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
	GetArray( MakeColumnWidthsEntry( name ), arr );
	}

void CPersistence::SetDialogRectangle(_In_ const LPCTSTR name, _In_ const CRect& rc) {
	SetRect( MakeDialogRectangleEntry( name ), rc );
	}

void CPersistence::GetDialogRectangle( _In_ const LPCTSTR name, _Inout_ CRect& rc ) {
	GetRect( MakeDialogRectangleEntry( name ), rc );
	SanifyRect( rc );
	}

INT CPersistence::GetConfigPage( _In_ const INT max_val ) {
	/* changed max to max_val to avoid conflict in ASSERT macro*/
	auto n = GetProfileInt( sectionPersistence, entryConfigPage, 0 );
	CheckRange( n, 0, max_val );
	ASSERT( ( n >= 0 ) && ( n <= max_val ) );
	return n;
	}

void CPersistence::SetConfigPage( _In_ const INT page ) {
	SetProfileInt( sectionPersistence, entryConfigPage, page );
	}

void CPersistence::GetConfigPosition(_Inout_ CPoint& pt) {
	pt.x = GetProfileInt( sectionPersistence, entryConfigPositionX, pt.x );
	pt.y = GetProfileInt( sectionPersistence, entryConfigPositionY, pt.y );
	
	CRect rc { pt, CSize( 100, 100 ) };
	SanifyRect( rc );
	pt = rc.TopLeft( );
	}

void CPersistence::SetConfigPosition(_In_ const CPoint pt) {
	SetProfileInt( sectionPersistence, entryConfigPositionX, pt.x );
	SetProfileInt( sectionPersistence, entryConfigPositionY, pt.y );
	}

CString CPersistence::GetBarStateSection() {
	return sectionBarState;
	}

INT CPersistence::GetSelectDrivesRadio() {
	auto radio = GetProfileInt( sectionPersistence, entrySelectDrivesRadio, 0 );
	CheckRange( radio, 0, 2 );
	ASSERT( (radio >= 0)&&(radio <= 2));
	return radio;
	}

void CPersistence::SetSelectDrivesRadio(_In_ const INT radio) {
	SetProfileInt( sectionPersistence, entrySelectDrivesRadio, radio );
	}

CString CPersistence::GetSelectDrivesFolder() {
	return GetProfileString( sectionPersistence, entrySelectDrivesFolder, _T( "" ) );
	}

void CPersistence::SetSelectDrivesFolder(_In_ const LPCTSTR folder) {
	SetProfileString(sectionPersistence, entrySelectDrivesFolder, folder);
	}

void CPersistence::GetSelectDrivesDrives(_Inout_ CStringArray& drives) {
	drives.RemoveAll( );
	auto s = GetProfileString( sectionPersistence, entrySelectDrivesDrives, _T( "" ) );
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

void CPersistence::SetSelectDrivesDrives(_In_ const CStringArray& drives) {
	CString s;
	auto sizeDrives = drives.GetSize( );
	for (INT i = 0; i < sizeDrives; i++) {
		if ( i > 0 ) {
			s += _T( "|" );
			}
		s += drives[ i ];
		}
	SetProfileString( sectionPersistence, entrySelectDrivesDrives, s );
	}

bool CPersistence::GetShowDeleteWarning() {
	return GetProfileBool( sectionPersistence, entryShowDeleteWarning, true );
	}

void CPersistence::SetShowDeleteWarning(_In_ const bool show) {
	SetProfileBool( sectionPersistence, entryShowDeleteWarning, show );
	}

void CPersistence::SetArray(_In_ const LPCTSTR entry, _In_ const CArray<INT, INT>& arr) {
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

void CPersistence::GetArray( _In_ const LPCTSTR entry, _Inout_ CArray<INT, INT>& rarr ) {
	auto s = GetProfileString( sectionPersistence, entry, _T( "" ) );
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

void CPersistence::SetRect(_In_ const LPCTSTR entry, _In_ const CRect& rc) {
	CString s;
	s.Format( _T( "%d,%d,%d,%d" ), rc.left, rc.top, rc.right, rc.bottom );
	SetProfileString( sectionPersistence, entry, s );
	}

void CPersistence::GetRect( _In_ const LPCTSTR entry, _Inout_ CRect& rc ) {
	auto s = GetProfileString( sectionPersistence, entry, _T( "" ) );
	CRect tmp;
	INT r = swscanf_s( s, _T( "%d,%d,%d,%d" ), &tmp.left, &tmp.top, &tmp.right, &tmp.bottom );
	if ( r == 4 ) {
		rc = tmp;
		}
	}

void CPersistence::SanifyRect(_Inout_ CRect& rc) {
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

CString CPersistence::MakeSplitterPosEntry(_In_ const LPCTSTR name) {
	CString entry;
	entry.Format( entrySplitterPosS, name );
	return entry;
	}

CString CPersistence::MakeColumnOrderEntry(_In_ const LPCTSTR name) {
	CString entry;
	entry.Format( entryColumnOrderS, name );
	return entry;
	}

CString CPersistence::MakeDialogRectangleEntry(_In_ const LPCTSTR name) {
	CString entry;
	entry.Format( entryDialogRectangleS, name );
	return entry;
	}

CString CPersistence::MakeColumnWidthsEntry(_In_ const LPCTSTR name) {
	CString entry;
	entry.Format( entryColumnWidthsS, name );
	return entry;
	}

CString CPersistence::EncodeWindowPlacement(_In_ const WINDOWPLACEMENT& wp) {
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

void CPersistence::DecodeWindowPlacement(_In_ const CString& s, _Inout_ WINDOWPLACEMENT& rwp) {
	WINDOWPLACEMENT wp;
	wp.length = sizeof( wp );
	AfxCheckMemory( );
	INT r = swscanf_s( s, _T( "%u,%u," ) _T( "%ld,%ld,%ld,%ld," ) _T( "%ld,%ld,%ld,%ld" ), &wp.flags, &wp.showCmd, &wp.ptMinPosition.x, &wp.ptMinPosition.y, &wp.ptMaxPosition.x, &wp.ptMaxPosition.y, &wp.rcNormalPosition.left, &wp.rcNormalPosition.right, &wp.rcNormalPosition.top, &wp.rcNormalPosition.bottom );
	AfxCheckMemory( );
	if ( r == 10 ) {
		rwp = wp;
		}
	}

/////////////////////////////////////////////////////////////////////////////
_Success_( return != NULL ) COptions *GetOptions() {
	return &_theOptions;
	}


COptions::COptions( ) : m_listGrid(true), m_followJunctionPoints(false), m_followMountPoints(false), m_humanFormat(true), m_listFullRowSelection(true), m_listStripes(true), m_showTimeSpent(false) { }//TODO: check defaults!

bool COptions::IsListGrid() const {
	return m_listGrid;
	}

void COptions::SetListGrid(_In_ const bool show) {
	if ( m_listGrid != show ) {
		m_listGrid = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}

bool COptions::IsListStripes() const {
	return m_listStripes;
	}

void COptions::SetListStripes(_In_ const bool show) {
	if ( m_listStripes != show ) {
		m_listStripes = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}

bool COptions::IsListFullRowSelection() const {
	return m_listFullRowSelection;
	}

void COptions::SetListFullRowSelection(_In_ const bool show) {
	if ( m_listFullRowSelection != show ) {
		m_listFullRowSelection = show;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}

void COptions::GetTreelistColors(_Inout_ _Inout_updates_( TREELISTCOLORCOUNT ) COLORREF color[TREELISTCOLORCOUNT]) {//typo?
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		color[ i ] = m_treelistColor[ i ];
		}
	}

void COptions::SetTreelistColors(_In_ _In_reads_( TREELISTCOLORCOUNT ) const COLORREF color[TREELISTCOLORCOUNT]) {
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		m_treelistColor[ i ] = color[ i ];
		}
	GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
	}

COLORREF COptions::GetTreelistColor( _In_ _In_range_( 0, TREELISTCOLORCOUNT ) const INT i ) const {
	ASSERT( i >= 0 );
	ASSERT( i < m_treelistColorCount );
	return m_treelistColor[ i ];
	}

INT COptions::GetTreelistColorCount() const {
	return m_treelistColorCount;
	}

void COptions::SetTreelistColorCount(_In_ const INT count) {
	if ( m_treelistColorCount != count ) {
		m_treelistColorCount = count;
		GetDocument( )->UpdateAllViews( NULL, HINT_LISTSTYLECHANGED );
		}
	}

bool COptions::IsHumanFormat() const {
	return m_humanFormat;
	}

void COptions::SetHumanFormat(_In_ const bool human) {
	if ( m_humanFormat != human ) {
		m_humanFormat = human;
		GetDocument( )->UpdateAllViews( NULL, HINT_NULL );
		GetApp( )->UpdateRamUsage( );
		}
	}

bool COptions::IsShowTimeSpent() const {
	return m_showTimeSpent;
	}

void COptions::SetShowTimeSpent(_In_ const bool show) {
	if ( m_showTimeSpent != show ) {
		m_showTimeSpent = show;
		}
	}

COLORREF COptions::GetTreemapHighlightColor() const {
	return m_treemapHighlightColor;
	}

void COptions::SetTreemapHighlightColor(_In_ const COLORREF color) {
	if ( m_treemapHighlightColor != color ) {
		m_treemapHighlightColor = color;
		GetDocument( )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		}
	}

_Must_inspect_result_ const CTreemap::Options *COptions::GetTreemapOptions() const {
	return &m_treemapOptions;
	}

void COptions::SetTreemapOptions(_In_ const CTreemap::Options& options) {
	if ( options.style        != m_treemapOptions.style
	 ||  options.grid         != m_treemapOptions.grid
	 ||  options.gridColor    != m_treemapOptions.gridColor
	 ||  options.brightness   != m_treemapOptions.brightness
	 ||  options.height       != m_treemapOptions.height
	 ||  options.scaleFactor  != m_treemapOptions.scaleFactor
	 ||  options.ambientLight != m_treemapOptions.ambientLight
	 ||  options.lightSourceX != m_treemapOptions.lightSourceX
	 ||  options.lightSourceY != m_treemapOptions.lightSourceY ) {
		m_treemapOptions = options;
		GetDocument( )->UpdateAllViews( NULL, HINT_TREEMAPSTYLECHANGED );
		}
	}

bool COptions::IsFollowMountPoints() const {
	return m_followMountPoints;
	}

void COptions::SetFollowMountPoints(_In_ const bool follow) {
	if ( m_followMountPoints != follow ) {
		m_followMountPoints = follow;
		GetDocument( )->RefreshMountPointItems( );
		}
	}

bool COptions::IsFollowJunctionPoints( ) const {
	return m_followJunctionPoints;
	}

void COptions::SetFollowJunctionPoints(_In_ const bool follow) {
	if ( m_followJunctionPoints != follow ) {
		m_followJunctionPoints = follow;
		GetDocument( )->RefreshJunctionItems( );
		}
	}

void COptions::SaveToRegistry() {
	SetProfileBool( sectionOptions, entryListGrid, m_listGrid );
	SetProfileBool( sectionOptions, entryListStripes, m_listStripes );
	SetProfileBool( sectionOptions, entryListFullRowSelection, m_listFullRowSelection );

	SetProfileInt( sectionOptions, entryTreelistColorCount, m_treelistColorCount );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		CString entry;
		entry.Format( entryTreelistColorN, i );
		SetProfileInt( sectionOptions, entry, m_treelistColor[ i ] );
		}
	SetProfileBool( sectionOptions, entryHumanFormat, m_humanFormat );

	SetProfileBool( sectionOptions, entryShowTimeSpent, m_showTimeSpent );
	SetProfileInt( sectionOptions, entryTreemapHighlightColor, m_treemapHighlightColor );

	SaveTreemapOptions( );

	SetProfileBool( sectionOptions, entryFollowMountPoints, m_followMountPoints );
	SetProfileBool( sectionOptions, entryFollowJunctionPoints, m_followJunctionPoints );
	// We must distinguish between 'empty' and 'default'. 'Default' will read "", 'Empty' will read "$", Others will read "$text.."
	//const LPCTSTR stringPrefix = _T( "$" );

	CString s;
	}

void COptions::LoadFromRegistry( ) {
	m_listGrid             = GetProfileBool( sectionOptions, entryListGrid, false );
	m_listStripes          = GetProfileBool( sectionOptions, entryListStripes, false );
	m_listFullRowSelection = GetProfileBool( sectionOptions, entryListFullRowSelection, true );

	m_treelistColorCount = GetProfileInt( sectionOptions, entryTreelistColorCount, 4 );
	CheckRange( m_treelistColorCount, 1, TREELISTCOLORCOUNT );
	ASSERT( ( m_treelistColorCount >= 1 ) && ( m_treelistColorCount <= TREELISTCOLORCOUNT ) );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		CString entry;
		entry.Format( entryTreelistColorN, i );
		m_treelistColor[ i ] = GetProfileInt( sectionOptions, entry, treelistColorDefault[ i ] );
		}
	m_humanFormat = GetProfileBool( sectionOptions, entryHumanFormat, true );

	m_showTimeSpent = GetProfileBool( sectionOptions, entryShowTimeSpent, false );
	m_treemapHighlightColor = GetProfileInt( sectionOptions, entryTreemapHighlightColor, RGB( 255, 255, 255 ) );

	ReadTreemapOptions( );

	m_followMountPoints = GetProfileBool( sectionOptions, entryFollowMountPoints, false );
	// Ignore junctions by default
	m_followJunctionPoints = GetProfileBool( sectionOptions, entryFollowJunctionPoints, false );

	}


void COptions::ReadTreemapOptions() {
	CTreemap::Options standard = CTreemap::GetDefaultOptions();

	auto style = GetProfileInt(sectionOptions, entryTreemapStyle, standard.style);
	if ( style != CTreemap::KDirStatStyle && style != CTreemap::SequoiaViewStyle ) {
		style = CTreemap::KDirStatStyle;
		}
	else {
		//ASSERT( false ); //always executes?
		}
	m_treemapOptions.style     = ( CTreemap::STYLE )style;

	m_treemapOptions.grid      =    GetProfileBool( sectionOptions, entryTreemapGrid,      standard.grid                      );
	
	m_treemapOptions.gridColor =    GetProfileInt(  sectionOptions, entryTreemapGridColor, standard.gridColor                 );

	auto         brightness     =    GetProfileInt(  sectionOptions, entryBrightness,	   standard.GetBrightnessPercent ( )  );
	CheckRange( brightness,     0,							 100   );
	ASSERT(   ( brightness    >=0    ) && (  brightness   <= 100 ) );
	m_treemapOptions.SetBrightnessPercent(   brightness  );

	auto         height         =    GetProfileInt(  sectionOptions, entryHeightFactor,     standard.GetHeightPercent      ( ) );
	CheckRange( height,         0,							 100   );
	ASSERT(   ( height        >=0    ) && (  height       <= 100 ) );
	m_treemapOptions.SetHeightPercent(       height);

	auto         scaleFactor    =    GetProfileInt(  sectionOptions, entryScaleFactor,      standard.GetScaleFactorPercent ( ) );
	CheckRange( scaleFactor,    0,							 100   );
	ASSERT(   ( scaleFactor   >=0    ) && (  scaleFactor  <= 100 ) );
	m_treemapOptions.SetScaleFactorPercent(  scaleFactor );

	auto        ambientLight   =    GetProfileInt(  sectionOptions, entryAmbientLight,     standard.GetAmbientLightPercent( ) );
	CheckRange( ambientLight,   0,							 100   );
	ASSERT(   ( ambientLight  >=0    ) && (  ambientLight <= 100 ) );
	m_treemapOptions.SetAmbientLightPercent( ambientLight);

	auto         lightSourceX   =    GetProfileInt(  sectionOptions, entryLightSourceX,     standard.GetLightSourceXPercent( ) );
	CheckRange( lightSourceX,   -200,						 200   );
	ASSERT(   ( lightSourceX  >=-200 ) && (  lightSourceX <= 200 ) );
	m_treemapOptions.SetLightSourceXPercent( lightSourceX);

	auto         lightSourceY   =    GetProfileInt(  sectionOptions, entryLightSourceY,     standard.GetLightSourceYPercent( ) );
	CheckRange( lightSourceY,   -200,						 200   );
	ASSERT(   ( lightSourceY  >=-200 ) && (  lightSourceY <= 200 ) );
	m_treemapOptions.SetLightSourceYPercent( lightSourceY);
	}

void COptions::SaveTreemapOptions() {
	SetProfileInt (  sectionOptions,  entryTreemapStyle,      m_treemapOptions.style                     );
	SetProfileBool(  sectionOptions,  entryTreemapGrid,       m_treemapOptions.grid                      );
	SetProfileInt (  sectionOptions,  entryTreemapGridColor,  m_treemapOptions.gridColor                 );
	SetProfileInt (  sectionOptions,  entryBrightness,        m_treemapOptions.GetBrightnessPercent  ( ) );
	SetProfileInt (  sectionOptions,  entryHeightFactor,      m_treemapOptions.GetHeightPercent      ( ) );
	SetProfileInt (  sectionOptions,  entryScaleFactor,       m_treemapOptions.GetScaleFactorPercent ( ) );
	SetProfileInt (  sectionOptions,  entryAmbientLight,      m_treemapOptions.GetAmbientLightPercent( ) );
	SetProfileInt (  sectionOptions,  entryLightSourceX,      m_treemapOptions.GetLightSourceXPercent( ) );
	SetProfileInt (  sectionOptions,  entryLightSourceY,      m_treemapOptions.GetLightSourceYPercent( ) );
	}


/////////////////////////////////////////////////////////////////////////////

void CRegistryUser::SetProfileString(_In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const LPCTSTR value) {
	AfxGetApp()->WriteProfileString(section, entry, value);
	}

CString CRegistryUser::GetProfileString( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const LPCTSTR defaultValue ) {
	return AfxGetApp()->GetProfileString(section, entry, defaultValue);
	}

void CRegistryUser::SetProfileInt(_In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const INT value) {
	AfxGetApp()->WriteProfileInt(section, entry, value);
	}

INT CRegistryUser::GetProfileInt( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const INT defaultValue ) {
	return AfxGetApp( )->GetProfileInt( section, entry, defaultValue );
	}

void CRegistryUser::SetProfileBool(_In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const bool value) {
	SetProfileInt(section, entry, (INT)value);
	}

bool CRegistryUser::GetProfileBool( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const bool defaultValue ) {
	return GetProfileInt( section, entry, defaultValue ) != 0;
	}

void CRegistryUser::CheckRange( _Inout_ INT& value, _In_ const INT min_val, _In_ const INT max_val ) {
	/*changed min and max to min_val and max_val to avoid name collision (with min() & max()) in ASSERT macro*/
	ASSERT( min_val < max_val);
	if ( value < min_val ) {
		 value = min_val;
		}
	if ( value > max_val ) {
		 value = max_val;
		}
	ASSERT( (value>=min_val)&&(value <= max_val));
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
