// options.h		- Declaration of CRegistryUser, COptions and CPersistence
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_OPTIONS_H
#define WDS_OPTIONS_H

struct COptions;
class CDirstatApp;


namespace registry_strings {
	_Null_terminated_ const wchar_t sectionPersistence[ ] = { _T( "persistence" ) };
	_Null_terminated_ const wchar_t entryShowFileTypes[ ] = { _T( "showFileTypes" ) };
	_Null_terminated_ const wchar_t entryShowStatusbar[ ] = { _T( "showStatusbar" ) };
	_Null_terminated_ const wchar_t entryShowTreemap[ ] = { _T( "showTreemap" ) };
	_Null_terminated_ const wchar_t sectionBarState[ ] = { _T( "persistence\\barstate" ) };
	_Null_terminated_ const wchar_t entryConfigPositionX[ ] = { _T( "configPositionX" ) };
	_Null_terminated_ const wchar_t entryConfigPositionY[ ] = { _T( "configPositionY" ) };
	_Null_terminated_ const wchar_t entryConfigPage[ ] = { _T( "configPage" ) };
	_Null_terminated_ const wchar_t entrySelectDrivesFolder[ ] = { _T( "selectDrivesFolder" ) };
	_Null_terminated_ const wchar_t entrySelectDrivesRadio[ ] = { _T( "selectDrivesRadio" ) };
	_Null_terminated_ const wchar_t entryShowDeleteWarning[ ] = { _T( "showDeleteWarning" ) };
	_Null_terminated_ const wchar_t entryColumnWidthsS[ ] = { _T( "%s-columnWidths" ) };
	}

namespace helpers {
	std::wstring generalized_make_entry( _In_z_ const PCTSTR name, _In_z_ const PCTSTR entry_fmt_str );
	std::wstring MakeColumnOrderEntry( _In_z_ const PCTSTR name );
	std::wstring MakeColumnWidthsEntry( _In_z_ const PCTSTR name );
	}

namespace {
	
	
	
	void SetProfileString( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR value ) {
		TRACE( _T( "Setting profile string\r\n\tsection: `%s`,\r\n\tentry: `%s`,\r\n\tvalue: `%s`\r\n" ), section, entry, value );
		VERIFY( AfxGetApp( )->WriteProfileStringW( section, entry, value ) );
		}

	}


// CRegistryUser. (Base class for COptions and CPersistence.)
// Can read from and write to the registry.
namespace CRegistryUser {
	std::wstring GetProfileString_ ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR defaultValue  );
	UINT         GetProfileInt_    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const INT  defaultValue    );
	bool         GetProfileBool    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const bool defaultValue    );
	void         SetProfileInt     ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const INT  value           );
	void         SetProfileBool    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const bool value           );
	//void         CheckRange        ( _Inout_      INT&   value,   _In_   const INT    min,   _In_   const INT  max             );
	};


// CPersistence. Reads from and writes to the registry all the persistent settings like window position, column order etc.
class CPersistence {
public:
	
	
	static void  SetDialogRectangle       ( _In_z_  const PCTSTR name,        _In_ const RECT rc                                         );
	static void  SetMainWindowPlacement   ( _In_    const WINDOWPLACEMENT& wp                                                               );
	static void  SetSelectDrivesDrives    ( _In_    const std::vector<std::wstring>& drives                                                              );
	
	
	
	static void  SetSplitterPos           ( _In_z_  const PCTSTR name,        _In_ const bool valid,             _In_ const DOUBLE userpos );

	static void  GetConfigPosition        ( _Inout_ WTL::CPoint& pt                                                                              );
	static void  GetDialogRectangle       ( _In_z_  const PCTSTR name,        _Out_ RECT& rc                                            );
	static void  GetSplitterPos           ( _In_z_ const PCTSTR name,         _Inout_ bool& valid,               _Inout_ DOUBLE& userpos   );
	static void  GetMainWindowPlacement   ( _Out_ WINDOWPLACEMENT& wp                                                                     );
	static void  GetSelectDrivesDrives    ( _Inout_ std::vector<std::wstring>& drives                                                                    );
	static INT   GetConfigPage            ( _In_    const INT max                                                                           );
	
	
	static void  SetShowDeleteWarning( _In_    const bool show ) {
		CRegistryUser::SetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowDeleteWarning, show );
		}

	static bool  GetShowDeleteWarning( ) {
		return CRegistryUser::GetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowDeleteWarning, true );
		}


	static RADIO GetSelectDrivesRadio     (                                                                                                 );


	static void  SetSelectDrivesRadio( _In_    const INT radio ) {
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesRadio, radio );
		}


	static void  SetConfigPage( _In_    const INT page ) {
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entryConfigPage, page );
		}


	static void  SetSelectDrivesFolder( _In_z_  const PCTSTR folder ) {
		SetProfileString( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesFolder, folder );
		}

	static void  SetConfigPosition( _In_    const WTL::CPoint pt ) {
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entryConfigPositionX, pt.x );
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entryConfigPositionY, pt.y );
		}

	static void  SetShowFileTypes( _In_    const bool show ) {
		CRegistryUser::SetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowFileTypes, show );
		}
	static void  SetShowStatusbar( _In_    const bool show ) {
		CRegistryUser::SetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowStatusbar, show );
		}
	static void  SetShowTreemap( _In_    const bool show ) {
		CRegistryUser::SetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowTreemap, show );
		}

	static bool  GetShowFileTypes( ) {
		return CRegistryUser::GetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowFileTypes, true );
		}
	static bool  GetShowStatusbar( ) {
		return CRegistryUser::GetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowStatusbar, true );
		}
	static bool  GetShowTreemap( ) {
		return CRegistryUser::GetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowTreemap, true );
		}


	//static DWORD CStyle_GetSelectDrivesFolder( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( return ) PWSTR psz_text, _In_ const DWORD strSize );

	static std::wstring GetSelectDrivesFolder( ) {
		return CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesFolder, _T( "" ) );
		}

	static PCTSTR GetBarStateSection( ) {
		return registry_strings::sectionBarState;
		}

	static void SetColumnWidths( _In_z_  const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
		SetArray( helpers::MakeColumnWidthsEntry( name ), arr, arrSize );
		}

	static void SetColumnOrder( _In_z_  const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
		SetArray( helpers::MakeColumnOrderEntry( name ), arr, arrSize );
		}

	static void GetColumnOrder( _In_z_ const PCTSTR name, _Out_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
		GetArray( helpers::MakeColumnOrderEntry( name ), arr, arrSize );
		}
	
	
	static void GetColumnWidths( _In_z_ const PCTSTR name, _Out_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize ) {
		GetArray( helpers::MakeColumnWidthsEntry( name ), arr, arrSize );
		}
private:
	
	_Pre_satisfies_( arrSize > 1 )
	static void    GetArray               ( _In_ const std::wstring entry, _Out_ _Pre_writable_size_( arrSize ) INT* arr_, const rsize_t arrSize );
	
	_Success_( SUCCEEDED( return ) )
	static const HRESULT GetRect                ( _In_ const std::wstring entry, _Out_ RECT& rc                  );


	static void    SetRect                ( _In_z_ const PCTSTR entry, _In_ const RECT rc               );
	static void    SetArray               ( _In_ const std::wstring name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );

	};

// COptions. Represents all the data which can be viewed and modified in the "Configure WinDirStat" dialog.
// COptions is a singleton.
_Success_( return != NULL ) COptions *GetOptions();

struct COptions {
	COptions( ) = default;

	void SetHumanFormat              ( _In_ const bool human, CDirstatApp* app_ptr );
	void SetListFullRowSelection     ( _In_ const bool show                                         );
	void SetListGrid                 ( _In_ const bool show                                         );
	void SetListStripes              ( _In_ const bool show                                         );
	void SetTreemapHighlightColor    ( _In_ const COLORREF color                                    );
	void SetTreemapOptions           ( _In_ const Treemap_Options& options                        );
	void ReadTreemapOptions          ( );
	void SaveTreemapOptions          ( );
	void LoadFromRegistry            ( );
	void SaveToRegistry              ( );

	                                       bool              m_humanFormat          = true;
	                                       bool              m_listGrid             = true;
	                                       bool              m_listStripes          = true;
	                                       bool              m_listFullRowSelection = true;
	                                       bool              m_followMountPoints    = false;
	                                       bool              m_followJunctionPoints = false;
										   //C4820: 'COptions' : '1' bytes padding added after data member 'COptions::m_showTimeSpent'
	                                       bool              m_showTimeSpent        = false;

	_Field_size_( TREELISTCOLORCOUNT )     COLORREF          m_treelistColor[ TREELISTCOLORCOUNT ];
	_Field_range_( 1, TREELISTCOLORCOUNT ) rsize_t           m_treelistColorCount;
	                                       //C4820: 'COptions' : '4' bytes padding added after data member 'COptions::m_treemapHighlightColor'
	                                       COLORREF          m_treemapHighlightColor;
	                                       Treemap_Options   m_treemapOptions;
	};
#else
#error ass
#endif
