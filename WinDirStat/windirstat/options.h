// options.h		- Declaration of CRegistryUser, COptions and CPersistence
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_OPTIONS_H
#define WDS_OPTIONS_H




struct COptions;

// CRegistryUser. (Base class for COptions and CPersistence.)
// Can read from and write to the registry.
namespace CRegistryUser {
	std::wstring GetProfileString_ ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_z_ const PCTSTR defaultValue  );
	UINT         GetProfileInt_    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const INT  defaultValue    );
	bool         GetProfileBool    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const bool defaultValue    );
	void         SetProfileInt     ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const INT  value           );
	void         SetProfileBool    ( _In_z_ const PCTSTR section, _In_z_ const PCTSTR entry, _In_   const bool value           );
	void         CheckRange        ( _Inout_      INT&   value,   _In_   const INT    min,   _In_   const INT  max             );

	//DWORD CStyle_GetProfileString( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( return ) PWSTR psz_text, _In_ const DWORD strSize, _In_z_ const PCWSTR section, _In_z_ const PCWSTR entry, _In_z_ const PCWSTR defaultValue );

	};


// CPersistence. Reads from and writes to the registry all the persistent settings like window position, column order etc.
class CPersistence {
public:
	static void  SetConfigPage            ( _In_    const INT page                                                                          );
	static void  SetConfigPosition        ( _In_    const WTL::CPoint pt                                                                         );
	static void  SetDialogRectangle       ( _In_z_  const PCTSTR name,        _In_ const RECT rc                                         );
	static void  SetMainWindowPlacement   ( _In_    const WINDOWPLACEMENT& wp                                                               );
	static void  SetSelectDrivesDrives    ( _In_    const std::vector<std::wstring>& drives                                                              );
	static void  SetSelectDrivesFolder    ( _In_z_  const PCTSTR folder                                                                    );
	static void  SetSelectDrivesRadio     ( _In_    const INT radio                                                                         );
	static void  SetShowDeleteWarning     ( _In_    const bool show                                                                         );
	static void  SetShowFileTypes         ( _In_    const bool show                                                                         );
	static void  SetShowStatusbar         ( _In_    const bool show                                                                         );
	static void  SetShowTreemap           ( _In_    const bool show                                                                         );
	static void  SetSplitterPos           ( _In_z_  const PCTSTR name,        _In_ const bool valid,             _In_ const DOUBLE userpos );

	static void  GetConfigPosition        ( _Inout_ WTL::CPoint& pt                                                                              );
	static void  GetDialogRectangle       ( _In_z_  const PCTSTR name,        _Out_ RECT& rc                                            );
	static void  GetSplitterPos           ( _In_z_ const PCTSTR name,         _Inout_ bool& valid,               _Inout_ DOUBLE& userpos   );
	static void  GetMainWindowPlacement   ( _Out_ WINDOWPLACEMENT& wp                                                                     );
	static void  GetSelectDrivesDrives    ( _Inout_ std::vector<std::wstring>& drives                                                                    );
	static INT   GetConfigPage            ( _In_    const INT max                                                                           );
	static bool  GetShowDeleteWarning     (                                                                                                 );
	static bool  GetShowFileTypes         (                                                                                                 );
	static bool  GetShowStatusbar         (                                                                                                 );
	static bool  GetShowTreemap           (                                                                                                 );
	static RADIO GetSelectDrivesRadio     (                                                                                                 );

	//static DWORD CStyle_GetSelectDrivesFolder( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) _Post_readable_size_( return ) PWSTR psz_text, _In_ const DWORD strSize );

	static std::wstring GetSelectDrivesFolder ( );

	static PCTSTR GetBarStateSection      ( );

	static void SetColumnWidths           ( _In_z_  const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );
	static void SetColumnOrder            ( _In_z_  const PCTSTR name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );

	static void GetColumnOrder            ( _In_z_ const PCTSTR name, _Out_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );
	static void GetColumnWidths           ( _In_z_ const PCTSTR name, _Out_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );
private:
	
	_Pre_satisfies_( arrSize > 1 )
	static void    GetArray               ( _In_ const std::wstring entry, _Out_ _Pre_writable_size_( arrSize ) INT* arr_, const rsize_t arrSize );
	
	static void    GetRect                ( _In_z_ const PCTSTR entry, _Inout_ RECT& rc                  );


	static void    SetRect                ( _In_z_ const PCTSTR entry, _In_ const RECT rc               );
	static void    SetArray               ( _In_ const std::wstring name, _Inout_ _Pre_writable_size_( arrSize ) INT* arr, const rsize_t arrSize );

	};

// COptions. Represents all the data which can be viewed and modified in the "Configure WinDirStat" dialog.
// COptions is a singleton.
_Success_( return != NULL ) COptions *GetOptions();

struct COptions {
	COptions( ) = default;

	void SetHumanFormat              ( _In_ const bool human                                        );
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
