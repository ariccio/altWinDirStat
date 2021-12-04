// options.h		- Declaration of CRegistryUser, COptions and CPersistence
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"


#ifndef WDS_OPTIONS_H
#define WDS_OPTIONS_H

WDS_FILE_INCLUDE_MESSAGE


#include "globalhelpers.h"

struct COptions;
class CDirstatApp;


namespace registry_strings {
	_Null_terminated_ constexpr const wchar_t sectionPersistence[ ] = { L"persistence" };
	_Null_terminated_ constexpr const wchar_t entryShowFileTypes[ ] = { L"showFileTypes" };
	_Null_terminated_ constexpr const wchar_t entryShowStatusbar[ ] = { L"showStatusbar" };
	_Null_terminated_ constexpr const wchar_t entryShowTreemap[ ] = { L"showTreemap" };
	_Null_terminated_ constexpr const wchar_t sectionBarState[ ] = { L"persistence\\barstate" };
	_Null_terminated_ constexpr const wchar_t entryConfigPositionX[ ] = { L"configPositionX" };
	_Null_terminated_ constexpr const wchar_t entryConfigPositionY[ ] = { L"configPositionY" };
	_Null_terminated_ constexpr const wchar_t entryConfigPage[ ] = { L"configPage" };
	_Null_terminated_ constexpr const wchar_t entrySelectDrivesFolder[ ] = { L"selectDrivesFolder" };
	_Null_terminated_ constexpr const wchar_t entrySelectDrivesRadio[ ] = { L"selectDrivesRadio" };
	_Null_terminated_ constexpr const wchar_t entryShowDeleteWarning[ ] = { L"showDeleteWarning" };
	_Null_terminated_ constexpr const wchar_t entryColumnWidthsS[ ] = { L"%s-columnWidths" };
	}

namespace helpers {
	std::wstring generalized_make_entry( _In_z_ const PCWSTR name, _In_z_ const PCWSTR entry_fmt_str );
	std::wstring MakeColumnOrderEntry( _In_z_ const PCWSTR name );
	std::wstring MakeColumnWidthsEntry( _In_z_ const PCWSTR name );
	}

#ifdef DEBUG
void trace_prof_string( _In_z_ PCWSTR const section, _In_z_ PCWSTR const entry, _In_z_ PCWSTR const value ) noexcept;

#endif

namespace {
	
	
	
	void SetProfileString( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR value ) noexcept {
#ifdef DEBUG
		trace_prof_string( section, entry, value );
#endif
		VERIFY( AfxGetApp( )->WriteProfileStringW( section, entry, value ) );
		}

	}


// CRegistryUser. (Base class for COptions and CPersistence.)
// Can read from and write to the registry.
namespace CRegistryUser {
	std::wstring GetProfileString_ ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR defaultValue  );
	UINT         GetProfileInt_    ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_   const INT  defaultValue    ) noexcept;
	bool         GetProfileBool    ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_   const bool defaultValue    ) noexcept;
	void         SetProfileInt     ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_   const INT  value           ) noexcept;
	void         SetProfileBool    ( _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_   const bool value           ) noexcept;
	};


// CPersistence. Reads from and writes to the registry all the persistent settings like window position, column order etc.
struct CPersistence final {
	static void  SetDialogRectangle       ( _In_z_  const PCWSTR name,        _In_ const RECT rc                                         );
	static void  SetMainWindowPlacement   ( _In_    const WINDOWPLACEMENT& wp                                                               );
	static void  SetSplitterPos           ( _In_z_  const PCWSTR name,        _In_ const bool valid,             _In_ const DOUBLE userpos );

	static void  GetConfigPosition        ( _Inout_ POINT* const pt                                                                              );
	//static void  GetDialogRectangle       ( _In_z_  const PCWSTR name,        _Out_ RECT* const rc                                            );
	static void  GetSplitterPos           ( _In_z_  const PCWSTR name,        _Out_ bool* const valid,               _Out_ DOUBLE* const userpos   ) noexcept;
	static void  GetMainWindowPlacement   ( _Out_   WINDOWPLACEMENT* const wp                                                                     );
	static INT   GetConfigPage            ( _In_    const INT max                                                                           ) noexcept;
	
	
	static void  SetShowDeleteWarning( _In_    const bool show ) {
		CRegistryUser::SetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowDeleteWarning, show );
		}

	static bool  GetShowDeleteWarning( ) {
		return CRegistryUser::GetProfileBool( registry_strings::sectionPersistence, registry_strings::entryShowDeleteWarning, true );
		}

	static void  SetSelectDrivesRadio( _In_    const INT radio ) {
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesRadio, radio );
		}


	static void  SetConfigPage( _In_    const INT page ) {
		CRegistryUser::SetProfileInt( registry_strings::sectionPersistence, registry_strings::entryConfigPage, page );
		}


	static void  SetSelectDrivesFolder( _In_z_  const PCWSTR folder ) {
		SetProfileString( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesFolder, folder );
		}

	static void  SetConfigPosition( _In_    const POINT pt ) {
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

	static std::wstring GetSelectDrivesFolder( ) {
		return CRegistryUser::GetProfileString_( registry_strings::sectionPersistence, registry_strings::entrySelectDrivesFolder, _T( "" ) );
		}

	static PCWSTR GetBarStateSection( ) {
		return registry_strings::sectionBarState;
		}

	static void SetColumnWidths( _In_z_  const PCWSTR name, _In_ _In_reads_( arrSize ) const INT* const arr, _In_range_( >, 1 ) const rsize_t arrSize ) {
		SetArray( helpers::MakeColumnWidthsEntry( name ), arr, arrSize );
		}

	static void SetColumnOrder( _In_z_  const PCWSTR name, _In_ _In_reads_( arrSize ) const INT* const arr, _In_range_( >, 1 ) const rsize_t arrSize ) {
		SetArray( helpers::MakeColumnOrderEntry( name ), arr, arrSize );
		}

	static void GetColumnOrder( _In_z_ const PCWSTR name, _Out_ _Out_writes_all_( arrSize ) INT* const arr, _In_range_( >, 1 ) const rsize_t arrSize, _In_z_ const PCWSTR defaultValue ) {
		GetArray( helpers::MakeColumnOrderEntry( name ), arr, arrSize, defaultValue );
		}
	
	static void GetColumnWidths( _In_z_ const PCWSTR name, _Out_ _Out_writes_all_( arrSize ) INT* const arr, _In_range_( >, 1 ) const rsize_t arrSize, _In_z_ const PCWSTR defaultValue ) {
		//TODO: BUGBUG: doesn't check return value of ::RegQueryValueExW (in CRegistryUser::GetProfileString_) - should bubble up?
		GetArray( helpers::MakeColumnWidthsEntry( name ), arr, arrSize, defaultValue );
		}
private:
	
	static void    GetArray               ( _In_ const std::wstring entry, _Out_ _Out_writes_all_( arrSize ) INT* const arr_, _In_range_( >, 1 ) const rsize_t arrSize, _In_z_ const PCWSTR defaultValue );
	
	_Success_( SUCCEEDED( return ) )
	static const HRESULT GetRect          ( _In_ const std::wstring entry, _Out_ RECT* const rc                  );


	static void    SetRect                ( _In_z_ const PCWSTR entry, _In_ const RECT rc               );
	static void    SetArray               ( _In_ const std::wstring name, _In_ _In_reads_( arrSize ) const INT* const arr, _In_range_( >, 1 ) const rsize_t arrSize );

	};

// COptions. Represents all the data which can be viewed and modified in the "Configure WinDirStat" dialog.
// COptions is a singleton.
_Success_( return != NULL ) COptions *GetOptions() noexcept;

struct COptions final {
	COptions( ) = default;

	void SetHumanFormat              ( _In_ const bool human, _In_ CDirstatApp* const app_ptr );
	void SetListFullRowSelection     ( _In_ const bool show                                   );
	void SetListGrid                 ( _In_ const bool show                                   );
	void SetListStripes              ( _In_ const bool show                                   );
	void SetTreemapHighlightColor    ( _In_ const COLORREF color                              );
	void SetTreemapOptions           ( _In_ const Treemap_Options& options                    );
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

	_Field_size_( TREELISTCOLORCOUNT )     COLORREF          m_treelistColor[ TREELISTCOLORCOUNT ] = {};
	_Field_range_( 1, TREELISTCOLORCOUNT ) rsize_t           m_treelistColorCount = {};
	                                       //C4820: 'COptions' : '4' bytes padding added after data member 'COptions::m_treemapHighlightColor'
	                                       COLORREF          m_treemapHighlightColor = {};
										   Treemap_Options   m_treemapOptions = {};
	};
#else
#error ass
#endif
