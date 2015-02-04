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
		WINDOWPLACEMENT wp;
		wp.length = sizeof( wp );
		const INT r = swscanf_s( s.c_str( ), _T( "%u,%u," ) _T( "%ld,%ld,%ld,%ld," ) _T( "%ld,%ld,%ld,%ld" ), &wp.flags, &wp.showCmd, &wp.ptMinPosition.x, &wp.ptMinPosition.y, &wp.ptMaxPosition.x, &wp.ptMaxPosition.y, &wp.rcNormalPosition.left, &wp.rcNormalPosition.right, &wp.rcNormalPosition.top, &wp.rcNormalPosition.bottom );
		TRACE( _T( "swscanf_s result: %i\r\n" ), r );
		TRACE( _T( "WINDOWPLACEMENT:\r\n\twp.flags: %u,\r\n\twp.showCmd: %u,\r\n\twp.ptMinPosition.x: %ld,\r\n\twp.ptMinPosition.y: %ld,\r\n\twp.ptMaxPosition.x: %ld,\r\n\twp.ptMaxPosition.y: %ld,\r\n\twp.rcNormalPosition.left: %ld,\r\n\twp.rcNormalPosition.right: %ld,\r\n\twp.rcNormalPosition.top: %ld,\r\n\twp.rcNormalPosition.bottom: %ld\r\n" ), wp.flags, wp.showCmd, wp.ptMinPosition.x, wp.ptMinPosition.y, wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.right, wp.rcNormalPosition.top, wp.rcNormalPosition.bottom );
		if ( r == 10 ) {
			rwp = wp;
			}
		}

	std::wstring generalized_make_entry( _In_z_ const PCTSTR name, _In_z_ const PCTSTR entry_fmt_str ) {
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
		OutputDebugStringW( name );

		return L"";
		}

	std::wstring MakeSplitterPosEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, entrySplitterPosS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeSplitterPosEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( name );
			}
		return ws_entry;
		}

	std::wstring MakeColumnOrderEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, entryColumnOrderS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeColumnOrderEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( name );
			}
		return ws_entry;
		}

	std::wstring MakeDialogRectangleEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, entryDialogRectangleS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"MakeDialogRectangleEntry failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( name );
			}
		return ws_entry;
		}

	std::wstring MakeColumnWidthsEntry( _In_z_ const PCTSTR name ) {
		const auto ws_entry = generalized_make_entry( name, entryColumnWidthsS );
		if ( ws_entry.empty( ) ) {
			displayWindowsMsgBoxWithMessage( L"entryColumnWidthsS failed to format the string! OutputDebugString has the name string. Will return empty string." );
			OutputDebugStringW( name );
			}
		return ws_entry;
		}

	void SetProfileString( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR value ) {
		TRACE( _T( "Setting profile string\r\n\tsection: `%s`,\r\n\tentry: `%s`,\r\n\tvalue: `%s`\r\n" ), section, entry, value );
		VERIFY( AfxGetApp( )->WriteProfileStringW( section, entry, value ) );
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
	
	//const DWORD prof_string_size = MAX_PATH;
	//wchar_t prof_string[ prof_string_size ] = { 0 };

	//const auto s_2 = CRegistryUser::CStyle_GetProfileString( prof_string, prof_string_size, sectionPersistence, entryMainWindowPlacement, _T( "" ) );
	const auto s = CRegistryUser::GetProfileString_( sectionPersistence, entryMainWindowPlacement, _T( "" ) );
	DecodeWindowPlacement( s.c_str( ), wp );
	CRect rect_to_sanify = wp.rcNormalPosition;
	SanifyRect( rect_to_sanify );
	wp.rcNormalPosition = rect_to_sanify;
	//SanifyRect( ( CRect & ) wp.rcNormalPosition );
	}

void CPersistence::SetMainWindowPlacement( _In_ const WINDOWPLACEMENT& wp ) {
	const auto s = EncodeWindowPlacement( wp );
	SetProfileString( sectionPersistence, entryMainWindowPlacement, s.c_str( ) );
	}

void CPersistence::SetSplitterPos( _In_z_ const PCTSTR name, _In_ const bool valid, _In_ const DOUBLE userpos ) {
	INT pos = 0;
	if ( valid ) {
		pos = static_cast<INT>( userpos * 100 );
		}
	else {
		pos = -1;
		}
	CRegistryUser::SetProfileInt( sectionPersistence, MakeSplitterPosEntry( name ).c_str( ), pos );
	}

void CPersistence::GetSplitterPos( _In_z_  const PCTSTR name, _Inout_ bool& valid, _Inout_ DOUBLE& userpos ) {
	auto pos = CRegistryUser::GetProfileInt_( sectionPersistence, MakeSplitterPosEntry( name ).c_str( ), -1 );
	if ( pos > 100 ) {
		valid = false;
		userpos = 0.5;
		}
	else {
		valid = true;
		userpos = ( DOUBLE ) pos / 100;
		}
	}
//void CPersistence::GetColumnOrder( _In_z_  const PCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
//	GetArray( MakeColumnOrderEntry( name ), arr );
//	}

void CPersistence::GetDialogRectangle( _In_z_ const PCTSTR name, _Inout_ RECT& rc ) {
	GetRect( MakeDialogRectangleEntry( name ).c_str( ), rc );
	CRect temp( rc );
	SanifyRect( temp );
	rc = temp;
	}

//void CPersistence::GetColumnWidths( _In_z_  const PCTSTR name, _Inout_ CArray<INT, INT>& arr ) {
//	GetArray( MakeColumnWidthsEntry( name ), arr );
//	}

//void CPersistence::SetColumnWidths( _In_z_ const PCTSTR name, _In_ const CArray<INT, INT>& arr ) {
//	SetArray( MakeColumnWidthsEntry( name ), arr );
//	}

void CPersistence::SetColumnWidths( _In_z_ const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	SetArray( MakeColumnWidthsEntry( name ).c_str( ), arr, arrSize );
	}

//void CPersistence::SetColumnOrder( _In_z_ const PCTSTR name, _In_ const CArray<INT, INT>& arr ) {
//	SetArray( MakeColumnOrderEntry( name ), arr );
//	}

void CPersistence::SetColumnOrder( _In_z_ const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	SetArray( MakeColumnOrderEntry( name ).c_str( ), arr, arrSize );
	}

void CPersistence::SetDialogRectangle( _In_z_  const PCTSTR name, _In_ const RECT& rc ) {
	SetRect( MakeDialogRectangleEntry( name ).c_str( ), rc );
	}

void CPersistence::GetColumnOrder( _In_z_ const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	GetArray( MakeColumnOrderEntry( name ), arr, arrSize );
	}

void CPersistence::GetColumnWidths( _In_z_ const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	GetArray( MakeColumnWidthsEntry( name ), arr, arrSize );
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

void CPersistence::GetConfigPosition( _Inout_ WTL::CPoint& pt ) {
	pt.x = static_cast<LONG>( CRegistryUser::GetProfileInt_( sectionPersistence, entryConfigPositionX, pt.x ) );
	pt.y = static_cast<LONG>( CRegistryUser::GetProfileInt_( sectionPersistence, entryConfigPositionY, pt.y ) );

	CRect rc { pt, WTL::CSize( 100, 100 ) };
	SanifyRect( rc );
	pt = rc.TopLeft( );
	}

void CPersistence::SetConfigPosition( _In_ const WTL::CPoint pt ) {
	CRegistryUser::SetProfileInt( sectionPersistence, entryConfigPositionX, pt.x );
	CRegistryUser::SetProfileInt( sectionPersistence, entryConfigPositionY, pt.y );
	}

PCTSTR CPersistence::GetBarStateSection( ) {
	return sectionBarState;
	}

RADIO CPersistence::GetSelectDrivesRadio( ) {
	auto radio = static_cast< INT >( CRegistryUser::GetProfileInt_( sectionPersistence, entrySelectDrivesRadio, 0 ) );
	CheckMinMax( radio, 0, 2 );
	ASSERT( ( radio >= 0 ) && ( radio <= 2 ) );
	return static_cast<RADIO>( radio );
	}

void CPersistence::SetSelectDrivesRadio( _In_ const INT radio ) {
	CRegistryUser::SetProfileInt( sectionPersistence, entrySelectDrivesRadio, radio );
	}

std::wstring CPersistence::GetSelectDrivesFolder( ) {
	return CRegistryUser::GetProfileString_( sectionPersistence, entrySelectDrivesFolder, _T( "" ) );
	}

//DWORD CPersistence::CStyle_GetSelectDrivesFolder( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( return ) PWSTR psz_text, _In_ const DWORD strSize ) {
//	return CRegistryUser::CStyle_GetProfileString( psz_text, strSize, sectionPersistence, entrySelectDrivesFolder, _T( "" ) );
//	}

void CPersistence::SetSelectDrivesFolder( _In_z_ const PCTSTR folder ) {
	SetProfileString( sectionPersistence, entrySelectDrivesFolder, folder );
	}

void CPersistence::GetSelectDrivesDrives( _Inout_ std::vector<std::wstring>& drives ) {
	drives.clear( );
	const auto s = CRegistryUser::GetProfileString_( sectionPersistence, entrySelectDrivesDrives, _T( "" ) );
	
	//const DWORD select_buf_size = MAX_PATH;
	//wchar_t select_buf[ select_buf_size ] = { 0 };
	
	//const auto chars_written = CRegistryUser::CStyle_GetProfileString( select_buf, select_buf_size, sectionPersistence, entrySelectDrivesDrives, _T( "" ) );
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
	for ( size_t i = 0; i < sizeDrives; i++ ) {
		if ( i > 0 ) {
			s += _T( "|" );
			}
		s += drives.at( i );
		}
	SetProfileString( sectionPersistence, entrySelectDrivesDrives, s.c_str( ) );
	}

bool CPersistence::GetShowDeleteWarning( ) {
	return CRegistryUser::GetProfileBool( sectionPersistence, entryShowDeleteWarning, true );
	}

void CPersistence::SetShowDeleteWarning( _In_ const bool show ) {
	CRegistryUser::SetProfileBool( sectionPersistence, entryShowDeleteWarning, show );
	}


void CPersistence::SetArray( _In_z_ const PCTSTR entry, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
	ASSERT( wcslen( entry ) != 0 );
	
	//TODO: BUGBUG: do something here other than just returning
	if ( wcslen( entry ) == 0 ) {
		return;
		}

	std::wstring value;
	for ( rsize_t i = 0; i < arrSize; i++ ) {

		const rsize_t int_buf_size = 11;
		_Null_terminated_ wchar_t int_buf[ int_buf_size ] = { 0 };
		const auto swp_res = swprintf_s( int_buf, int_buf_size, L"%d", arr[ i ] );
		if ( swp_res == -1 ) {
			displayWindowsMsgBoxWithMessage( L"swprintf_s SERIOUS error!!" );
			TRACE( _T( "swprintf_s SERIOUS error!!\r\n" ) );
			std::terminate( );
			}
		//s.Format( _T( "%d" ), arr[ i ] );
		if ( i > 0 ) {
			value += L',' ;
			}
		value += int_buf;
		}
	SetProfileString( sectionPersistence, entry, value.c_str( ) );
	}

//void CPersistence::GetArray( _In_z_ const PCTSTR entry, _Inout_ CArray<INT, INT>& rarr ) {
//	const auto s = CRegistryUser::GetProfileString_( sectionPersistence, entry, _T( "" ) );
//	CArray<INT, INT> arr;
//	INT i = 0;
//	while ( i < s.GetLength( ) ) {
//		INT n = 0;
//		while ( i < s.GetLength( ) && _istdigit( s[ i ] ) ) {
//			n *= 10;
//			n += s[ i ] - _T( '0' );
//			i++;
//			}
//		arr.Add( n );
//		if ( i >= s.GetLength( ) || s[ i ] != _T( ',' ) ) {
//			break;
//			}
//		i++;
//		}
//	if ( i >= s.GetLength( ) && arr.GetSize( ) == rarr.GetSize( ) ) {
//		for ( i = 0; i < rarr.GetSize( ); i++ ) {
//			rarr[ i ] = arr[ i ];
//			}
//		}
//	}

void CPersistence::GetArray( _In_ const std::wstring entry, _Inout_ _Pre_writable_size_( arrSize ) INT* arr_, const rsize_t arrSize ) {
	ASSERT( entry.length( ) != 0 );
	
	//TODO: BUGBUG: do something here other than just returning
	if ( entry.length( ) == 0 ) {
		return;
		}
	const auto s_temp = CRegistryUser::GetProfileString_( sectionPersistence, entry.c_str( ), _T( "" ) );
	//const DWORD arr_buf_size = MAX_PATH;


	const std::wstring s( s_temp );

	std::vector<INT> arr;
	rsize_t i = 0;
	while ( i < s.length( ) ) {
		INT n = 0;
		while ( i < s.length( ) && iswdigit( s[ i ] ) ) {
			n *= 10;
			n += s[ i ] - _T( '0' );
			i++;
			}
		arr.emplace_back( n );
		
		if ( i >= s.length( ) || s[ i ] != _T( ',' ) ) {
			break;
			}
		i++;
		}
	if ( i >= s.length( ) && arr.size( ) == arrSize ) {
		for ( rsize_t j = 0; j < arrSize; ++j ) {
			arr_[ j ] = arr[ j ];
			}
		}
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
		SetProfileString( sectionPersistence, entry, buffer_rect );
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
	SetProfileString( sectionPersistence, entry, dynamic_format.c_str( ) );
	}


//TODO: return by value?
void CPersistence::GetRect( _In_z_ const PCTSTR entry, _Inout_ RECT& rc ) {
	CString s = CRegistryUser::GetProfileString_( sectionPersistence, entry, _T( "" ) ).c_str( );
	RECT tmp;
	const auto r = swscanf_s( s, _T( "%d,%d,%d,%d" ), &tmp.left, &tmp.top, &tmp.right, &tmp.bottom );
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

//COLORREF COptions::GetTreelistColor( _In_ _In_range_( 0, TREELISTCOLORCOUNT ) const size_t i ) const {
//	ASSERT( i < m_treelistColorCount );
//	return m_treelistColor[ i ];
//	}

void COptions::SetHumanFormat( _In_ const bool human ) {
	if ( m_humanFormat != human ) {
		m_humanFormat = human;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_NULL );
		GetApp( )->UpdateRamUsage( );
		}
	}

void COptions::SetTreemapHighlightColor( _In_ const COLORREF color ) {
	if ( m_treemapHighlightColor != color ) {
		m_treemapHighlightColor = color;
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED );
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
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED );
		}
	}

void COptions::SaveToRegistry( ) {
	CRegistryUser::SetProfileBool( sectionOptions, entryListGrid, m_listGrid );
	CRegistryUser::SetProfileBool( sectionOptions, entryListStripes, m_listStripes );
	CRegistryUser::SetProfileBool( sectionOptions, entryListFullRowSelection, m_listFullRowSelection );

	CRegistryUser::SetProfileInt( sectionOptions, entryTreelistColorCount, static_cast<const INT>( m_treelistColorCount ) );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		//CString entry;
		//entry.Format( entryTreelistColorN, i );
		std::wstring dyn_fmt_str( std::wstring( L"treelistColor" + std::to_wstring( i ) ) );
		//ASSERT( dyn_fmt_str.compare( entry ) == 0 );
		CRegistryUser::SetProfileInt( sectionOptions, dyn_fmt_str.c_str( ), static_cast<const INT>( m_treelistColor[ i ] ) );
		}
	CRegistryUser::SetProfileBool( sectionOptions, entryHumanFormat, m_humanFormat );

	CRegistryUser::SetProfileBool( sectionOptions, entryShowTimeSpent, m_showTimeSpent );
	CRegistryUser::SetProfileInt( sectionOptions, entryTreemapHighlightColor, static_cast<const INT>( m_treemapHighlightColor ) );

	SaveTreemapOptions( );

	CRegistryUser::SetProfileBool( sectionOptions, entryFollowMountPoints, m_followMountPoints );
	CRegistryUser::SetProfileBool( sectionOptions, entryFollowJunctionPoints, m_followJunctionPoints );
	// We must distinguish between 'empty' and 'default'. 'Default' will read "", 'Empty' will read "$", Others will read "$text.."
	//const PCTSTR stringPrefix = _T( "$" );

	//CString s;
	}

void COptions::LoadFromRegistry( ) {
	m_listGrid = CRegistryUser::GetProfileBool( sectionOptions, entryListGrid, false );
	m_listStripes = CRegistryUser::GetProfileBool( sectionOptions, entryListStripes, false );
	m_listFullRowSelection = CRegistryUser::GetProfileBool( sectionOptions, entryListFullRowSelection, true );

	m_treelistColorCount = static_cast<rsize_t>( CRegistryUser::GetProfileInt_( sectionOptions, entryTreelistColorCount, 4 ) );
	auto temp = static_cast< INT >( m_treelistColorCount );
	CRegistryUser::CheckRange( temp, 1, TREELISTCOLORCOUNT );
	m_treelistColorCount = static_cast<rsize_t>( temp );
	ASSERT( ( m_treelistColorCount >= 1 ) && ( m_treelistColorCount <= TREELISTCOLORCOUNT ) );
	for ( INT i = 0; i < TREELISTCOLORCOUNT; i++ ) {
		//CString entry;
		//entry.Format( entryTreelistColorN, i );
		std::wstring dyn_fmt_str( std::wstring( L"treelistColor" + std::to_wstring( i ) ) );
		m_treelistColor[ i ] = CRegistryUser::GetProfileInt_( sectionOptions, dyn_fmt_str.c_str( ), static_cast<const INT>( treelistColorDefault[ i ] ) );
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
	static_assert( std::is_convertible< INT, std::underlying_type< decltype( standard.style ) >::type>::value, "" );
	auto style = CRegistryUser::GetProfileInt_( sectionOptions, entryTreemapStyle, static_cast<INT>( standard.style ) );
	if ( style != static_cast<decltype( style )>( Treemap_STYLE::KDirStatStyle ) && style != static_cast<decltype( style )>( Treemap_STYLE::SequoiaViewStyle ) ) {
		style = static_cast<decltype( style )>( Treemap_STYLE::KDirStatStyle );
		}
	static_assert( std::is_convertible< decltype( style ), std::underlying_type< decltype( m_treemapOptions.style ) >::type>::value, "" );
	m_treemapOptions.style = static_cast<Treemap_STYLE>( style );

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
	CRegistryUser::SetProfileInt ( sectionOptions, entryTreemapStyle,     static_cast<INT>( m_treemapOptions.style ) );
	CRegistryUser::SetProfileBool( sectionOptions, entryTreemapGrid,      m_treemapOptions.grid );
	CRegistryUser::SetProfileInt ( sectionOptions, entryTreemapGridColor, static_cast<const INT>( m_treemapOptions.gridColor ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryBrightness,       m_treemapOptions.GetBrightnessPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryHeightFactor,     m_treemapOptions.GetHeightPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryScaleFactor,      m_treemapOptions.GetScaleFactorPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryAmbientLight,     m_treemapOptions.GetAmbientLightPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryLightSourceX,     m_treemapOptions.GetLightSourceXPercent( ) );
	CRegistryUser::SetProfileInt ( sectionOptions, entryLightSourceY,     m_treemapOptions.GetLightSourceYPercent( ) );
	}

std::wstring CRegistryUser::GetProfileString_( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR defaultValue ) {
	return std::wstring( AfxGetApp( )->GetProfileStringW( section, entry, defaultValue ).GetString( ) );
	}

//DWORD CRegistryUser::CStyle_GetProfileString( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( return ) PWSTR psz_text, _In_ const DWORD strSize, _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR defaultValue ) {
//	return GetProfileStringW( section, entry, defaultValue, psz_text, strSize );
//	}

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


#else

#endif