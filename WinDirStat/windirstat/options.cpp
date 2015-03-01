// options.cpp	- Implementation of CPersistence, COptions and CRegistryUser
//
// see `file_header_text.txt` for licensing & contact info.


#pragma once

#include "stdafx.h"

#ifndef WDS_OPTIONS_CPP
#define WDS_OPTIONS_CPP

#include "dirstatdoc.h"
#include "globalhelpers.h"
#include "windirstat.h"
#include "options.h"



namespace registry_strings {
	
	
	
	_Null_terminated_ const wchar_t entrySplitterPosS[ ] = { _T( "%s-splitterPos" ) };
	_Null_terminated_ const wchar_t entryColumnOrderS[ ] = { _T( "%s-columnOrder" ) };
	
	_Null_terminated_ const wchar_t entryDialogRectangleS[ ] = { _T( "%s-rectangle" ) };
	
	
	
	_Null_terminated_ const wchar_t entrySelectDrivesDrives[ ] = { _T( "selectDrivesDrives" ) };
	
	
	_Null_terminated_ const wchar_t sectionOptions[ ] = { _T( "options" ) };
	_Null_terminated_ const wchar_t entryListGrid[ ] = { _T( "treelistGrid" ) }; // for compatibility with 1.0.1, this entry is named treelistGrid.
	_Null_terminated_ const wchar_t entryListStripes[ ] = { _T( "listStripes" ) };
	_Null_terminated_ const wchar_t entryListFullRowSelection[ ] = { _T( "listFullRowSelection" ) };
	_Null_terminated_ const wchar_t entryTreelistColorCount[ ] = { _T( "treelistColorCount" ) };
	_Null_terminated_ const wchar_t entryTreelistColorN[ ] = { _T( "treelistColor%d" ) };
	_Null_terminated_ const wchar_t entryHumanFormat[ ] = { _T( "humanFormat" ) };
	_Null_terminated_ const wchar_t entryShowTimeSpent[ ] = { _T( "showTimeSpent" ) };
	_Null_terminated_ const wchar_t entryTreemapHighlightColor[ ] = { _T( "treemapHighlightColor" ) };
	_Null_terminated_ const wchar_t entryTreemapStyle[ ] = { _T( "treemapStyle" ) };
	_Null_terminated_ const wchar_t entryTreemapGrid[ ] = { _T( "treemapGrid" ) };
	_Null_terminated_ const wchar_t entryTreemapGridColor[ ] = { _T( "treemapGridColor" ) };
	_Null_terminated_ const wchar_t entryBrightness[ ] = { _T( "brightness" ) };
	_Null_terminated_ const wchar_t entryHeightFactor[ ] = { _T( "heightFactor" ) };
	_Null_terminated_ const wchar_t entryScaleFactor[ ] = { _T( "scaleFactor" ) };
	_Null_terminated_ const wchar_t entryAmbientLight[ ] = { _T( "ambientLight" ) };
	_Null_terminated_ const wchar_t entryLightSourceX[ ] = { _T( "lightSourceX" ) };
	_Null_terminated_ const wchar_t entryLightSourceY[ ] = { _T( "lightSourceY" ) };
	_Null_terminated_ const wchar_t entryFollowMountPoints[ ] = { _T( "followMountPoints" ) };
	_Null_terminated_ const wchar_t entryFollowJunctionPoints[ ] = { _T( "followJunctionPoints" ) };
	_Null_terminated_ const wchar_t entryEnabled[ ] = { _T( "enabled" ) };
	_Null_terminated_ const wchar_t entryTitle[ ] = { _T( "title" ) };
	_Null_terminated_ const wchar_t entryWorksForDrives[ ] = { _T( "worksForDrives" ) };
	_Null_terminated_ const wchar_t entryWorksForDirectories[ ] = { _T( "worksForDirectories" ) };
	_Null_terminated_ const wchar_t entryWorksForFilesFolder[ ] = { _T( "worksForFilesFolder" ) };
	_Null_terminated_ const wchar_t entryWorksForFiles[ ] = { _T( "worksForFiles" ) };
	_Null_terminated_ const wchar_t entryWorksForUncPaths[ ] = { _T( "worksForUncPaths" ) };
	_Null_terminated_ const wchar_t entryCommandLine[ ] = { _T( "commandLine" ) };
	_Null_terminated_ const wchar_t entryRecurseIntoSubdirectories[ ] = { _T( "recurseIntoSubdirectories" ) };
	_Null_terminated_ const wchar_t entryAskForConfirmation[ ] = { _T( "askForConfirmation" ) };
	_Null_terminated_ const wchar_t entryShowConsoleWindow[ ] = { _T( "showConsoleWindow" ) };
	_Null_terminated_ const wchar_t entryWaitForCompletion[ ] = { _T( "waitForCompletion" ) };
	_Null_terminated_ const wchar_t entryRefreshPolicy[ ] = { _T( "refreshPolicy" ) };
	_Null_terminated_ const wchar_t entryMainWindowPlacement[ ] = { _T( "mainWindowPlacement" ) };
	
	}

namespace helpers {
	std::wstring generalized_make_entry( _In_z_ const PCTSTR name, _In_z_ const PCTSTR entry_fmt_str ) {
		//TODO: uses heap!
		const auto chars_needed = ( _scwprintf( entry_fmt_str, name ) + 1 );
		std::unique_ptr<_Null_terminated_ wchar_t[]> char_buffer_ptr = std::make_unique<wchar_t[ ]>( static_cast<size_t>( chars_needed ) );
		
		//This is so that the `Locals`/`Autos` window shows an actual string!
		PWSTR buffer_ptr = char_buffer_ptr.get( );
		const HRESULT fmt_res_1 = StringCchPrintfW( buffer_ptr, static_cast<size_t>( chars_needed ), entry_fmt_str, name );
		ASSERT( SUCCEEDED( fmt_res_1 ) );
		if ( SUCCEEDED( fmt_res_1 ) ) {
			return std::wstring( buffer_ptr );
			}
		if ( fmt_res_1 == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			const auto double_chars_needed = static_cast< size_t >( chars_needed ) * 2u;
			char_buffer_ptr.reset( new wchar_t[ double_chars_needed ] );
			PWSTR double_buffer_ptr = char_buffer_ptr.get( );
			const HRESULT fmt_res_2 = StringCchPrintfW( double_buffer_ptr, double_chars_needed, entry_fmt_str, name );
			if ( SUCCEEDED( fmt_res_2 ) ) {
				return std::wstring( double_buffer_ptr );
				}
			}
		displayWindowsMsgBoxWithMessage( L"generalized_make_entry failed to format the string! OutputDebugString has the name string. Will return empty string." );
		OutputDebugStringW( L"generalized_make_entry failed to format the string! OutputDebugString has the name string. Will return empty string.\r\n" );

		return L"";
		}
	std::wstring MakeColumnOrderEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, registry_strings::entryColumnOrderS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeColumnOrderEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( L"MakeColumnOrderEntry failed to format the string! OutputDebugString has the name string. Will return empty string.\r\n" );
			}
		return ws_entry;
		}
	std::wstring MakeColumnWidthsEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, registry_strings::entryColumnWidthsS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"entryColumnWidthsS failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( L"entryColumnWidthsS failed to format the string! OutputDebugString has the name string. Will return empty string.\r\n" );
			}
		return ws_entry;
		}
	}

namespace {
	COptions _theOptions;

	
	
	
	
	//const PCTSTR entryMainWindowPlacement       = _T( "mainWindowPlacement" );
	//const PCTSTR entrySplitterPosS              = _T( "%s-splitterPos" );
	//const PCTSTR entryColumnOrderS              = _T( "%s-columnOrder" );
	//const PCTSTR entryColumnWidthsS             = _T( "%s-columnWidths" );
	//const PCTSTR entryDialogRectangleS          = _T( "%s-rectangle" );
	//const PCTSTR entryConfigPage                = _T( "configPage" );
	//const PCTSTR entryConfigPositionX           = _T( "configPositionX" );
	//const PCTSTR entryConfigPositionY           = _T( "configPositionY" );
	//const PCTSTR entrySelectDrivesRadio         = _T( "selectDrivesRadio" );
	//const PCTSTR entrySelectDrivesFolder        = _T( "selectDrivesFolder" );
	//const PCTSTR entrySelectDrivesDrives        = _T( "selectDrivesDrives" );
	//const PCTSTR entryShowDeleteWarning         = _T( "showDeleteWarning" );
	//const PCTSTR sectionBarState                = _T( "persistence\\barstate" );
	//const PCTSTR sectionOptions                 = _T( "options" );
	//const PCTSTR entryListGrid                  = _T( "treelistGrid" ); // for compatibility with 1.0.1, this entry is named treelistGrid.
	//const PCTSTR entryListStripes               = _T( "listStripes" );
	//const PCTSTR entryListFullRowSelection      = _T( "listFullRowSelection" );
	//const PCTSTR entryTreelistColorCount        = _T( "treelistColorCount" );
	//const PCTSTR entryTreelistColorN            = _T( "treelistColor%d" );
	//const PCTSTR entryHumanFormat               = _T( "humanFormat" );
	//const PCTSTR entryShowTimeSpent             = _T( "showTimeSpent" );
	//const PCTSTR entryTreemapHighlightColor     = _T( "treemapHighlightColor" );
	//const PCTSTR entryTreemapStyle              = _T( "treemapStyle" );
	//const PCTSTR entryTreemapGrid               = _T( "treemapGrid" );
	//const PCTSTR entryTreemapGridColor          = _T( "treemapGridColor" );
	//const PCTSTR entryBrightness                = _T( "brightness" );
	//const PCTSTR entryHeightFactor              = _T( "heightFactor" );
	//const PCTSTR entryScaleFactor               = _T( "scaleFactor" );
	//const PCTSTR entryAmbientLight              = _T( "ambientLight" );
	//const PCTSTR entryLightSourceX              = _T( "lightSourceX" );
	//const PCTSTR entryLightSourceY              = _T( "lightSourceY" );
	//const PCTSTR entryFollowMountPoints         = _T( "followMountPoints" );
	//const PCTSTR entryFollowJunctionPoints      = _T( "followJunctionPoints" );
	//const PCTSTR entryEnabled                   = _T( "enabled" );
	//const PCTSTR entryTitle                     = _T( "title" );
	//const PCTSTR entryWorksForDrives            = _T( "worksForDrives" );
	//const PCTSTR entryWorksForDirectories       = _T( "worksForDirectories" );
	//const PCTSTR entryWorksForFilesFolder       = _T( "worksForFilesFolder" );
	//const PCTSTR entryWorksForFiles             = _T( "worksForFiles" );
	//const PCTSTR entryWorksForUncPaths          = _T( "worksForUncPaths" );
	//const PCTSTR entryCommandLine               = _T( "commandLine" );
	//const PCTSTR entryRecurseIntoSubdirectories = _T( "recurseIntoSubdirectories" );
	//const PCTSTR entryAskForConfirmation        = _T( "askForConfirmation" );
	//const PCTSTR entryShowConsoleWindow         = _T( "showConsoleWindow" );
	//const PCTSTR entryWaitForCompletion         = _T( "waitForCompletion" );
	//const PCTSTR entryRefreshPolicy             = _T( "refreshPolicy" );

	const COLORREF treelistColorDefault[TREELISTCOLORCOUNT] = {
		RGB(  64,  64, 140 ),
		RGB( 140,  64,  64 ),
		RGB(  64, 140,  64 ),
		RGB( 140, 140,  64 ),
		RGB(   0,   0, 255 ),
		RGB( 255,   0,   0 ),
		RGB(   0, 255,   0 ),
		RGB( 255, 255,   0 )
		};

	void SanifyRect( _Inout_ RECT& rc ) {
		const INT visible = 30;

		//rc.NormalizeRect( );
		normalize_RECT( rc );

		RECT rcDesktop_temp = { 0, 0, 0, 0 };

		/*
_AFXWIN_INLINE CWnd* PASCAL CWnd::GetDesktopWindow()
	{ return CWnd::FromHandle(::GetDesktopWindow()); }

_AFXWIN_INLINE void CWnd::GetWindowRect(LPRECT lpRect) const
	{ ASSERT(::IsWindow(m_hWnd)); ::GetWindowRect(m_hWnd, lpRect); }

		*/
		const auto desktop_window = CWnd::FromHandle( ::GetDesktopWindow( ) );
		ASSERT( ::IsWindow( desktop_window->m_hWnd ) );
		
		//If the function succeeds, the return value is nonzero.
		VERIFY( ::GetWindowRect( desktop_window->m_hWnd, &rcDesktop_temp ) );

		//CWnd::GetDesktopWindow( )->GetWindowRect( &rcDesktop_temp );

		const RECT& rcDesktop = rcDesktop_temp;

		if ( ( rc.right - rc.left ) > ( rcDesktop.right - rcDesktop.left ) ) {
			rc.right = rc.left + ( rcDesktop.right - rcDesktop.left );
			}
		if ( ( rc.bottom - rc.top ) > ( rcDesktop.bottom - rcDesktop.top ) ) {
			rc.bottom = rc.top + ( rcDesktop.bottom - rcDesktop.top );
			}
		if ( rc.left < 0 ) {
			VERIFY( ::OffsetRect( &rc, -( rc.left ), 0 ) );
			}
		if ( rc.left > rcDesktop.right - visible ) {
			VERIFY( ::OffsetRect( &rc, -( visible ), 0 ) );
			//rc.OffsetRect( -visible, 0 );
			}
		if ( rc.top < 0 ) {
			//rc.OffsetRect( -rc.top, 0 );
			VERIFY( ::OffsetRect( &rc, -( rc.top ), 0 ) );
			}
		if ( rc.top > rcDesktop.bottom - visible ) {
			//rc.OffsetRect( 0, -visible );
			VERIFY( ::OffsetRect( &rc, 0, -( visible ) ) );
			}
		}

	std::wstring EncodeWindowPlacement( _In_ const WINDOWPLACEMENT& wp ) {
		TRACE( _T( "Encoding window placement....\r\n" ) );
		TRACE( _T( "Placement that we're encoding:\r\n\twp.flags: %u,\r\n\twp.showCmd: %u,\r\n\twp.ptMinPosition.x: %ld,\r\n\twp.ptMinPosition.y: %ld,\r\n\twp.ptMaxPosition.x: %ld,\r\n\twp.ptMaxPosition.y: %ld,\r\n\twp.rcNormalPosition.left: %ld,\r\n\twp.rcNormalPosition.right: %ld,\r\n\twp.rcNormalPosition.top: %ld,\r\n\twp.rcNormalPosition.bottom: %ld\r\n" ), wp.flags, wp.showCmd, wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom );
		//CString s;
		//s.Format(
		//	_T( "%u,%u," )
		//	_T( "%ld,%ld,%ld,%ld," )
		//	_T( "%ld,%ld,%ld,%ld" ),
		//	wp.flags, wp.showCmd,
		//	wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y,
		//	wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom
		//);

		std::wstring fmt_str;
		fmt_str += std::to_wstring( wp.flags );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.showCmd );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.ptMinPosition.x );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.ptMinPosition.y );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.ptMaxPosition.x );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.ptMaxPosition.y );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.rcNormalPosition.left );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.rcNormalPosition.right );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.rcNormalPosition.top );
		fmt_str += L',';
		fmt_str += std::to_wstring( wp.rcNormalPosition.bottom );
		//ASSERT( fmt_str.compare( s ) == 0 );

		TRACE( _T( "Encoded string: %s\r\n" ), fmt_str.c_str( ) );
		return fmt_str;
		}

	void DecodeWindowPlacement( _In_ const std::wstring s, _Inout_ WINDOWPLACEMENT& rwp ) {
		TRACE( _T( "Decoding window placement! wp.flags, wp.showCmd, wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom: %s\r\n" ), s.c_str( ) );
		
		auto wp = zero_init_struct<WINDOWPLACEMENT>( );

		wp.length = sizeof( wp );
		
		const INT r = swscanf_s( s.c_str( ), _T( "%u,%u," ) _T( "%ld,%ld,%ld,%ld," ) _T( "%ld,%ld,%ld,%ld" ), &wp.flags, &wp.showCmd, &wp.ptMinPosition.x, &wp.ptMinPosition.y, &wp.ptMaxPosition.x, &wp.ptMaxPosition.y, &wp.rcNormalPosition.left, &wp.rcNormalPosition.right, &wp.rcNormalPosition.top, &wp.rcNormalPosition.bottom );
		TRACE( _T( "swscanf_s result: %i\r\n" ), r );
		TRACE( _T( "WINDOWPLACEMENT:\r\n\twp.flags: %u,\r\n\twp.showCmd: %u,\r\n\twp.ptMinPosition.x: %ld,\r\n\twp.ptMinPosition.y: %ld,\r\n\twp.ptMaxPosition.x: %ld,\r\n\twp.ptMaxPosition.y: %ld,\r\n\twp.rcNormalPosition.left: %ld,\r\n\twp.rcNormalPosition.right: %ld,\r\n\twp.rcNormalPosition.top: %ld,\r\n\twp.rcNormalPosition.bottom: %ld\r\n" ), wp.flags, wp.showCmd, wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom );
		if ( r == 10 ) {
			rwp = wp;
			}
		}


	std::wstring MakeSplitterPosEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = helpers::generalized_make_entry( name, registry_strings::entrySplitterPosS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeSplitterPosEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( L"MakeSplitterPosEntry failed to format the string! OutputDebugString has the name string. Will return empty string.\r\n" );
			}
		return ws_entry;
		}


	std::wstring MakeDialogRectangleEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = helpers::generalized_make_entry( name, registry_strings::entryDialogRectangleS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeDialogRectangleEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( L"MakeDialogRectangleEntry failed to format the string! OutputDebugString has the name string. Will return empty string.\r\n" );
			}
		return ws_entry;
		}

	}

class CTreemap;

void CPersistence::GetMainWindowPlacement( _Out_ WINDOWPLACEMENT& wp ) {
	//ASSERT( wp.length == sizeof( wp ) );
	
	//const DWORD prof_string_size = MAX_PATH;
	//wchar_t prof_string[ prof_string_size ] = { 0 };

	//const auto s_2 = CRegistryUser::CStyle_GetProfileString( prof_string, prof_string_size, registry_strings::sectionPersistence, entryMainWindowPlacement, _T( "" ) );
	const auto s = CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, registry_strings::entryMainWindowPlacement, _T( "" ) );
	DecodeWindowPlacement( s.c_str( ), wp );
	
	RECT rect_to_sanify = wp.rcNormalPosition;
	SanifyRect( rect_to_sanify );
	wp.rcNormalPosition = rect_to_sanify;
	//SanifyRect( ( CRect & ) wp.rcNormalPosition );
	}

void CPersistence::SetMainWindowPlacement( _In_ const WINDOWPLACEMENT& wp ) {
	const auto s = EncodeWindowPlacement( wp );
	SetProfileString( registry_strings::sectionPersistence, registry_strings::entryMainWindowPlacement, s.c_str( ) );
	}

void CPersistence::SetSplitterPos( _In_z_ const PCTSTR name, _In_ const bool valid, _In_ const DOUBLE userpos ) {
	INT pos = 0;
	if ( valid ) {
		pos = static_cast<INT>( userpos * 100 );
		}
	else {
		pos = -1;
		}
	CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, MakeSplitterPosEntry( name ).c_str( ), pos );
	}

void CPersistence::GetSplitterPos( _In_z_  const PCTSTR name, _Inout_ bool& valid, _Inout_ DOUBLE& userpos ) {
	const auto pos = CRegistryUser::GetProfileInt_( registry_strings::sectionPersistence, MakeSplitterPosEntry( name ).c_str( ), -1 );
	if ( pos > 100 ) {
		valid = false;
		userpos = 0.5;
		}
	else {
		valid = true;
		userpos = ( static_cast<DOUBLE>( pos ) / static_cast<DOUBLE>( 100 ) );
		}
	}

void CPersistence::GetDialogRectangle( _In_z_ const PCTSTR name, _Out_ RECT& rc ) {
	//TODO: BUGBUG: check return value!
	GetRect( MakeDialogRectangleEntry( name ), rc );
	RECT temp = rc;
	SanifyRect( temp );
	rc = temp;
	}

void CPersistence::SetDialogRectangle( _In_z_ const PCTSTR name, _In_ const RECT rc ) {
	SetRect( MakeDialogRectangleEntry( name ).c_str( ), rc );
	}

INT CPersistence::GetConfigPage( _In_ const INT max_val ) {
	/* changed max to max_val to avoid conflict in ASSERT macro*/
	auto n = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionPersistence, registry_strings::entryConfigPage, 0 ) );
	CheckMinMax( n, 0, max_val );
	ASSERT( ( n >= 0 ) && ( n <= max_val ) );
	return n;
	}

void CPersistence::GetConfigPosition( _Inout_ WTL::CPoint& pt ) {
	pt.x = static_cast<LONG>( CRegistryUser::GetProfileInt_( registry_strings::sectionPersistence, registry_strings::entryConfigPositionX, pt.x ) );
	pt.y = static_cast<LONG>( CRegistryUser::GetProfileInt_( registry_strings::sectionPersistence, registry_strings::entryConfigPositionY, pt.y ) );

	CRect rc { pt, WTL::CSize( 100, 100 ) };
	SanifyRect( rc );
	pt = rc.TopLeft( );
	}

RADIO CPersistence::GetSelectDrivesRadio( ) {
	auto radio = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesRadio, 0 ) );
	CheckMinMax( radio, 0, 2 );
	ASSERT( ( radio >= 0 ) && ( radio <= 2 ) );
	return static_cast<RADIO>( radio );
	}

void CPersistence::GetSelectDrivesDrives( _Inout_ std::vector<std::wstring>& drives ) {
	drives.clear( );
	const auto s = CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesDrives, _T( "" ) );
	
	//const DWORD select_buf_size = MAX_PATH;
	//wchar_t select_buf[ select_buf_size ] = { 0 };
	
	//const auto chars_written = CRegistryUser::CStyle_GetProfileString( select_buf, select_buf_size, registry_strings::sectionPersistence, entrySelectDrivesDrives, _T( "" ) );
	rsize_t i = 0;
	while ( i < s.length( ) ) {
		std::wstring drive;
		while ( i < s.length( ) && s[ i ] != _T( '|' ) ) {
			drive += s[ i ];
			i++;
			}
		if ( i < s.length( ) ) {
			i++;
			}
		drives.emplace_back( std::move( drive ) );
		}
	}

void CPersistence::SetSelectDrivesDrives( _In_ const std::vector<std::wstring>& drives ) {
	std::wstring s;
	const auto sizeDrives = drives.size( );
	
	//each drive is PROBABLY going to be more than one character in length, but we can reserve something anyways!
	s.reserve( sizeDrives );
	for ( size_t i = 0; i < sizeDrives; i++ ) {
		if ( i > 0 ) {
			s += _T( "|" );
			}
		s += drives.at( i );
		}
	SetProfileString( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesDrives, s.c_str( ) );
	}

void CPersistence::SetArray( _In_ const std::wstring entry, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	ASSERT( entry.length( ) != 0 );
	
	//TODO: BUGBUG: do something here other than just returning
	if ( entry.length( ) == 0 ) {
		return;
		}
	const rsize_t int_buf_size = 11u;
	_Null_terminated_ wchar_t int_buf[ int_buf_size ] = { 0 };
	std::wstring value;
	
	//reserve comma, plus arrSize * int_buf
	value.reserve( ( arrSize ) + ( int_buf_size * arrSize ) );
	for ( rsize_t i = 0; i < arrSize; i++ ) {
		memset_zero_struct( int_buf );
		const auto swp_res = swprintf_s( int_buf, int_buf_size, L"%d", arr[ i ] );
		if ( swp_res == -1 ) {
			displayWindowsMsgBoxWithMessage( L"swprintf_s SERIOUS error!!" );
			TRACE( _T( "swprintf_s SERIOUS error!!\r\n" ) );
			std::terminate( );
			}
		if ( i > 0 ) {
			value += L',';
			}
		value += int_buf;
		}
	SetProfileString( registry_strings::sectionPersistence, entry.c_str( ), value.c_str( ) );
	}

_Pre_satisfies_( arrSize > 1 )
void CPersistence::GetArray( _In_ const std::wstring entry, _Out_ _Pre_writable_size_( arrSize ) INT* arr_, const rsize_t arrSize ) {
	ASSERT( entry.length( ) != 0 );
	for ( rsize_t i = 0; i < arrSize; ++i ) {
		arr_[ i ] = 0;
		}
	//TODO: BUGBUG: do something here other than just returning
	if ( entry.length( ) == 0 ) {
		return;
		}
	const auto s_temp = CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, entry.c_str( ), _T( "" ) );
	//const DWORD arr_buf_size = MAX_PATH;

	rsize_t current_arr_index = 0;

	const std::wstring s( s_temp );

	//std::vector<INT> arr;
	rsize_t i = 0;

	const auto string_length = s.length( );

	while ( i < string_length ) {
		INT n = 0;
		while ( i < string_length && iswdigit( s[ i ] ) ) {
			ASSERT( s[ i ] != L',' );
			n *= 10;
			n += s[ i ] - _T( '0' );
			i++;
			}
		
		ASSERT( current_arr_index < arrSize );
		arr_[ current_arr_index ] = n;
		++current_arr_index;
		//arr.emplace_back( n );
		
		if ( i >= string_length || s[ i ] != _T( ',' ) ) {
			break;
			}
		i++;
		}
	//if ( i >= s.length( ) && arr.size( ) == arrSize ) {
	//	for ( rsize_t j = 0; j < arrSize; ++j ) {
	//		ASSERT( j <= current_arr_index );
	//		ASSERT( arr_[ j ] == arr[ j ] );
	//		//arr_[ j ] = arr[ j ];
	//		}
	//	}
	}

void CPersistence::SetRect( _In_z_ const PCTSTR entry, _In_ const RECT rc ) {
	//CString s;
	//s.Format( _T( "%d,%d,%d,%d" ), rc.left, rc.top, rc.right, rc.bottom );
	//LONG_MAX == 2147483647
	//                      ^10 characters
	//            2,147,483,647
	//                         ^13 characters
	//            -2,147,483,647
	//                          ^14 characters
	// `%d`           -> max 14 characters
	// `%d,`          -> max 15 characters
	// `%d,%d,%d,%d`  -> max ( 15 * 4 characters )
	//             ^ok, technically it's ( ( 15 * 4 ) -1 ) characters, but overshooting won't hurt. We need space for the null terminator!
	//( 15 * 4 ) -> 60 characters
	const rsize_t buffer_size = 64u;
	_Null_terminated_ wchar_t buffer_rect[ buffer_size ] = { 0 };
	const HRESULT fmt_res = StringCchPrintfW( buffer_rect, buffer_size, L"%d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom );
	if ( SUCCEEDED( fmt_res ) ) {
		//ASSERT( s.Compare( buffer_rect ) == 0 );
		SetProfileString( registry_strings::sectionPersistence, entry, buffer_rect );
		return;
		}
	std::wstring dynamic_format;
	dynamic_format += std::to_wstring( rc.left );
	dynamic_format += L',';
	dynamic_format += std::to_wstring( rc.top );
	dynamic_format += L',';
	dynamic_format += std::to_wstring( rc.right );
	dynamic_format += L',';
	dynamic_format += std::to_wstring( rc.bottom );
	SetProfileString( registry_strings::sectionPersistence, entry, dynamic_format.c_str( ) );
	}


//TODO: return by value?
_Success_( SUCCEEDED( return ) )
const HRESULT CPersistence::GetRect( _In_ const std::wstring entry, _Out_ RECT& rc ) {
	//TODO: BUGBUG: `const auto s` here resolves type of s as `wchar_t*`
	const std::wstring s = CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, entry.c_str( ), _T( "" ) ).c_str( );
	RECT tmp;
	const auto r = swscanf_s( s.c_str( ), _T( "%d,%d,%d,%d" ), &tmp.left, &tmp.top, &tmp.right, &tmp.bottom );
	static_assert( SUCCEEDED( S_OK ), "Bad success return value!" );
	if ( r == 4 ) {
		TRACE( _T( "swscanf_s succeeded! read in rectangle: %d,%d,%d,%d\r\n" ), tmp.left, tmp.top, tmp.right, tmp.bottom );
		rc = tmp;
		return S_OK;
		}
	
	static_assert( !SUCCEEDED( E_FAIL ), "Bad failure return value!" );
	TRACE( _T( "swscanf_s failed! entry: `%s`, source string: `%s`\r\n" ), entry.c_str( ), s.c_str( ) );
	return E_FAIL;
	}

/////////////////////////////////////////////////////////////////////////////
_Success_( return != NULL ) COptions* GetOptions( ) {
	ASSERT( ( &_theOptions ) != NULL );
	if ( ( &_theOptions ) != NULL ) {
		return &_theOptions;
		}
	TRACE( _T( "&_theOptions is NULL! This should never happen!\r\n" ) );
	WTL::AtlMessageBox( NULL, _T( "&_theOptions is NULL! This should never happen! Hit `OK` when you're ready to abort." ), _T( "Whoa!" ), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL );
	std::terminate( );
	//need to 'call' abort because `/analyze` doesn't understand that std::terminate DOES NOT RETURN!
	abort( );
	}

void COptions::SetListGrid( _In_ const bool show ) {
	if ( m_listGrid != show ) {
		m_listGrid = show;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED );
		}
	}



void COptions::SetListStripes( _In_ const bool show ) {
	if ( m_listStripes != show ) {
		m_listStripes = show;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED );
		}
	}



void COptions::SetListFullRowSelection( _In_ const bool show ) {
	if ( m_listFullRowSelection != show ) {
		m_listFullRowSelection = show;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED );
		}
	}

void COptions::SetHumanFormat( _In_ const bool human, CDirstatApp* app_ptr ) {
	if ( m_humanFormat != human ) {
		m_humanFormat = human;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NULL );
		app_ptr->UpdateRamUsage( );
		}
	}

void COptions::SetTreemapHighlightColor( _In_ const COLORREF color ) {
	if ( m_treemapHighlightColor != color ) {
		m_treemapHighlightColor = color;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED );
		}
	}


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
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED );
		}
	}

void COptions::SaveToRegistry( ) {
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryListGrid, m_listGrid );
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryListStripes, m_listStripes );
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryListFullRowSelection, m_listFullRowSelection );

	CRegistryUser::SetProfileInt( registry_strings::sectionOptions, registry_strings::entryTreelistColorCount, static_cast<const INT>( m_treelistColorCount ) );
	for ( INT i = 0; i < static_cast<INT>( TREELISTCOLORCOUNT ); i++ ) {
		//CString entry;
		//entry.Format( entryTreelistColorN, i );
		std::wstring dyn_fmt_str( std::wstring( L"treelistColor" + std::to_wstring( i ) ) );
		//ASSERT( dyn_fmt_str.compare( entry ) == 0 );
		CRegistryUser::SetProfileInt( registry_strings::sectionOptions, dyn_fmt_str.c_str( ), static_cast<const INT>( m_treelistColor[ i ] ) );
		}
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryHumanFormat, m_humanFormat );

	//CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryShowTimeSpent, m_showTimeSpent );
	CRegistryUser::SetProfileInt( registry_strings::sectionOptions, registry_strings::entryTreemapHighlightColor, static_cast<const INT>( m_treemapHighlightColor ) );

	SaveTreemapOptions( );

	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryFollowMountPoints, m_followMountPoints );
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryFollowJunctionPoints, m_followJunctionPoints );
	// We must distinguish between 'empty' and 'default'. 'Default' will read "", 'Empty' will read "$", Others will read "$text.."
	//const PCTSTR stringPrefix = _T( "$" );

	//CString s;
	}

void COptions::LoadFromRegistry( ) {
	m_listGrid = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryListGrid, false );
	m_listStripes = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryListStripes, false );
	m_listFullRowSelection = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryListFullRowSelection, true );

	m_treelistColorCount = static_cast<rsize_t>( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryTreelistColorCount, 4 ) );
	auto temp = static_cast< INT >( m_treelistColorCount );
	//CRegistryUser::CheckRange( temp, 1, TREELISTCOLORCOUNT );
	CheckMinMax( temp, 1, TREELISTCOLORCOUNT );
	m_treelistColorCount = static_cast<rsize_t>( temp );
	ASSERT( ( m_treelistColorCount >= 1 ) && ( m_treelistColorCount <= TREELISTCOLORCOUNT ) );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		//CString entry;
		//entry.Format( entryTreelistColorN, i );
		//TODO, uses heap!
		std::wstring dyn_fmt_str( std::wstring( L"treelistColor" + std::to_wstring( i ) ) );
		m_treelistColor[ i ] = CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, dyn_fmt_str.c_str( ), static_cast<const INT>( treelistColorDefault[ i ] ) );
		}
	m_humanFormat = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryHumanFormat, true );

	//m_showTimeSpent = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryShowTimeSpent, false );
	m_treemapHighlightColor = CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryTreemapHighlightColor, RGB( 255, 255, 255 ) );

	ReadTreemapOptions( );

	m_followMountPoints = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryFollowMountPoints, false );
	// Ignore junctions by default
	m_followJunctionPoints = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryFollowJunctionPoints, false );

	}


void COptions::ReadTreemapOptions( ) {
	Treemap_Options standard = _defaultOptions;
	static_assert( std::is_convertible< INT, std::underlying_type< decltype( standard.style ) >::type>::value, "" );
	auto style = CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryTreemapStyle, static_cast<INT>( standard.style ) );
	if ( style != static_cast<decltype( style )>( Treemap_STYLE::KDirStatStyle ) && style != static_cast<decltype( style )>( Treemap_STYLE::SequoiaViewStyle ) ) {
		style = static_cast<decltype( style )>( Treemap_STYLE::KDirStatStyle );
		}
	static_assert( std::is_convertible< decltype( style ), std::underlying_type< decltype( m_treemapOptions.style ) >::type>::value, "" );
	m_treemapOptions.style = static_cast<Treemap_STYLE>( style );

	m_treemapOptions.grid = CRegistryUser::GetProfileBool( registry_strings::sectionOptions, registry_strings::entryTreemapGrid, standard.grid );

	m_treemapOptions.gridColor = CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryTreemapGridColor, static_cast<const INT>( standard.gridColor ) );

	auto        brightness = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryBrightness, standard.GetBrightnessPercent( ) ) );
	CheckMinMax( brightness, 0, 100 );
	m_treemapOptions.SetBrightnessPercent( brightness );

	auto        height       = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryHeightFactor, standard.GetHeightPercent( ) ) );
	CheckMinMax( height, 0, 100 );
	m_treemapOptions.SetHeightPercent( height );

	auto        scaleFactor  = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryScaleFactor, standard.GetScaleFactorPercent( ) ) );
	CheckMinMax( scaleFactor, 0, 100 );
	m_treemapOptions.SetScaleFactorPercent( scaleFactor );

	auto        ambientLight = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryAmbientLight, standard.GetAmbientLightPercent( ) ) );
	CheckMinMax( ambientLight, 0, 100 );
	m_treemapOptions.SetAmbientLightPercent( ambientLight );

	auto        lightSourceX = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryLightSourceX, standard.GetLightSourceXPercent( ) ) );
	CheckMinMax( lightSourceX, -200, 200 );
	m_treemapOptions.SetLightSourceXPercent( lightSourceX );

	auto        lightSourceY = static_cast< INT >( CRegistryUser::GetProfileInt_( registry_strings::sectionOptions, registry_strings::entryLightSourceY, standard.GetLightSourceYPercent( ) ) );
	CheckMinMax( lightSourceY, -200, 200 );
	m_treemapOptions.SetLightSourceYPercent( lightSourceY );
	}

void COptions::SaveTreemapOptions( ) {
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryTreemapStyle,     static_cast<INT>( m_treemapOptions.style ) );
	CRegistryUser::SetProfileBool( registry_strings::sectionOptions, registry_strings::entryTreemapGrid,      m_treemapOptions.grid );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryTreemapGridColor, static_cast<const INT>( m_treemapOptions.gridColor ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryBrightness,       m_treemapOptions.GetBrightnessPercent( ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryHeightFactor,     m_treemapOptions.GetHeightPercent( ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryScaleFactor,      m_treemapOptions.GetScaleFactorPercent( ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryAmbientLight,     m_treemapOptions.GetAmbientLightPercent( ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryLightSourceX,     m_treemapOptions.GetLightSourceXPercent( ) );
	CRegistryUser::SetProfileInt ( registry_strings::sectionOptions, registry_strings::entryLightSourceY,     m_treemapOptions.GetLightSourceYPercent( ) );
	}

std::wstring CRegistryUser::GetProfileString_( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR defaultValue ) {

	//const PCWSTR reg_key_str = GetApp( )->m_pszRegistryKey;
	const HKEY   reg_sec_key = GetApp( )->GetSectionKey( section );
	_Const_ DWORD dwType = REG_SZ;
	const rsize_t buffer_size = 512u;
	BYTE registry_data_buffer[ buffer_size ] = { 0 };
	static_assert( sizeof( registry_data_buffer ) == ( sizeof( BYTE ) * buffer_size ), "" );
	DWORD buffer_size_byte_count = sizeof( registry_data_buffer );
	const LONG   query_res = ::RegQueryValueExW( reg_sec_key, entry, NULL, &dwType, registry_data_buffer, &buffer_size_byte_count );
	
	if ( query_res == ERROR_SUCCESS ) {
		ASSERT( ( wcslen( reinterpret_cast<wchar_t*>( registry_data_buffer ) ) + 1 ) == ( buffer_size_byte_count / sizeof( wchar_t ) ) );
		
		//yeah, this is actually twice as big as the buffer size needed, but who cares?
		_Null_terminated_ wchar_t reg_data_buffer[ buffer_size ] = { 0 };
		std::memcpy( reg_data_buffer, registry_data_buffer, buffer_size_byte_count );
		//_CrtDbgBreak( );
#ifdef DEBUG
		const std::wstring normal_res( AfxGetApp( )->GetProfileStringW( section, entry, defaultValue ).GetString( ) );
		ASSERT( normal_res.compare( reg_data_buffer ) == 0 );
#endif
		return std::wstring( reg_data_buffer );
		}
	TRACE( _T( "C-Style GetProfileString failed!\r\n" ) );
	return std::wstring( AfxGetApp( )->GetProfileStringW( section, entry, defaultValue ).GetString( ) );
	}


void CRegistryUser::SetProfileInt( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const INT value ) {
	ASSERT( wcslen( entry ) != 0 );
	if ( wcslen( entry ) == 0 ) {
		displayWindowsMsgBoxWithMessage( L"can set a registry key with an empty string!" );
		return;
		}
	AfxGetApp( )->WriteProfileInt( section, entry, value );
	}

UINT CRegistryUser::GetProfileInt_( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const INT defaultValue ) {
	ASSERT( wcslen( entry ) != 0 );
	if ( wcslen( entry ) == 0 ) {
		displayWindowsMsgBoxWithMessage( L"can Get a registry key with an empty string!(aborting!)" );
		std::terminate( );
		}
	return AfxGetApp( )->GetProfileIntW( section, entry, defaultValue );
	}

void CRegistryUser::SetProfileBool( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const bool value ) {
	const BOOL value_to_set = ( ( value ) ? TRUE : FALSE );
	SetProfileInt( section, entry, value_to_set );
	}

bool CRegistryUser::GetProfileBool( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_ const bool defaultValue ) {
	return GetProfileInt_( section, entry, defaultValue ) != 0;
	}


#else

#endif