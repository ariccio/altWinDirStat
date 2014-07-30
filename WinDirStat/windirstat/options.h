// options.h		- Declaration of CRegistryUser, COptions and CPersistence
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

#pragma once

#include "treemap.h"

class COptions;

enum REFRESHPOLICY
{
	RP_NO_REFRESH,
	RP_REFRESH_THIS_ENTRY,
	RP_REFRESH_THIS_ENTRYS_PARENT,
	// RP_ASSUME_ENTRY_HAS_BEEN_DELETED, // feature not implemented.
	REFRESHPOLICYCOUNT
};

//struct USERDEFINEDCLEANUP {
//	bool          enabled;
//	bool          virginTitle;
//	CString       title;
//	bool          worksForDrives;
//	bool          worksForDirectories;
//	bool          worksForFilesFolder;
//	bool          worksForFiles;
//	bool          worksForUncPaths;
//	CString       commandLine;
//	bool          recurseIntoSubdirectories;
//	bool          askForConfirmation;
//	bool          showConsoleWindow;
//	bool          waitForCompletion;
//	REFRESHPOLICY refreshPolicy;
//	};

//#define USERDEFINEDCLEANUPCOUNT 10

#define TREELISTCOLORCOUNT 8


//
// CRegistryUser. (Base class for COptions and CPersistence.)
// Can read from and write to the registry.
//
class CRegistryUser
{
public:
	static void    SetProfileString ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const LPCTSTR value        );
	static CString GetProfileString ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const LPCTSTR defaultValue );

	static void SetProfileInt       ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const INT value        );
	static INT  GetProfileInt       ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const INT defaultValue );

	static void SetProfileBool      ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const bool value        );
	static bool GetProfileBool      ( _In_ const LPCTSTR section, _In_ const LPCTSTR entry, _In_ const bool defaultValue );

	static void CheckRange          ( _Inout_ INT& value, _In_ const INT min, _In_ const INT max );
};


//
// CPersistence. Reads from and writes to the registry all the persistent settings
// like window position, column order etc.
//
class CPersistence: private CRegistryUser
{
public:


	static void SetColumnOrder           ( _In_ const LPCTSTR name, _In_ const CArray<INT, INT>& arr            );
	static void SetColumnWidths          ( _In_ const LPCTSTR name, _In_ const CArray<INT, INT>& arr            );
	static void SetConfigPage            ( _In_ const INT page                                             );
	static void SetConfigPosition        ( _In_ const CPoint pt                                            );
	static void SetDialogRectangle       ( _In_ const LPCTSTR name, _In_ const CRect& rc                        );
	static void SetMainWindowPlacement   ( _In_ const WINDOWPLACEMENT& wp                                  );
	static void SetSelectDrivesDrives    ( _In_ const CStringArray& drives                                 );
	static void SetSelectDrivesFolder    ( _In_ const LPCTSTR folder                                       );
	static void SetSelectDrivesRadio     ( _In_ const INT radio                                            );
	static void SetShowDeleteWarning     ( _In_ const bool show                                            );
	static void SetShowFileTypes         ( _In_ const bool show                                            );
	static void SetShowFreeSpace         ( _In_ const bool show                                            );
	static void SetShowStatusbar         ( _In_ const bool show                                            );
	static void SetShowTreemap           ( _In_ const bool show                                            );
	static void SetShowToolbar           ( _In_ const bool show                                            );
	static void SetShowUnknown           ( _In_ const bool show                                            );
	static void SetSplitterPos           ( _In_ const LPCTSTR name, _In_ const bool valid, _In_ const DOUBLE userpos );


	static void GetColumnOrder           ( _In_ const LPCTSTR name, _Inout_ CArray<INT, INT>& arr     );
	static void GetColumnWidths          ( _In_ const LPCTSTR name, _Inout_ CArray<INT, INT>& arr     );
	static INT  GetConfigPage            ( _In_ const INT max                                              );
	static void GetConfigPosition        ( _Inout_ CPoint& pt                                    );
	static void GetDialogRectangle       ( _In_ const LPCTSTR name, _Inout_ CRect& rc                              );
	static void GetMainWindowPlacement   ( _Inout_ WINDOWPLACEMENT& wp                         );
	static INT  GetSelectDrivesRadio     (                                                            );
	static void GetSelectDrivesDrives    ( _Inout_ CStringArray& drives                                       );
	static bool GetShowDeleteWarning     (                                                            );
	static bool GetShowFreeSpace         (                                                            );
	static bool GetShowFileTypes         (                                                            );
	static bool GetShowStatusbar         (                                                            );
	static bool GetShowTreemap           (                                                            );
	static bool GetShowToolbar           (                                                            );
	static bool GetShowUnknown           (                                                            );
	static void GetSplitterPos           ( _In_ const LPCTSTR name, _Inout_ bool& valid, _Inout_ DOUBLE& userpos           );

	static CString GetSelectDrivesFolder ( );
	static CString GetBarStateSection    ( );


private:
	static void    SetArray                 ( _In_ const LPCTSTR entry, _In_ const CArray<INT, INT>& arr        );
	static void    SetRect                  ( _In_ const LPCTSTR entry, _In_ const CRect& rc                    );

	static void    GetArray                 ( _In_ const LPCTSTR entry, _Inout_ CArray<INT, INT>& arr );
	static void    GetRect                  ( _In_ const LPCTSTR entry, _Inout_ CRect& rc                          );

	static void    SanifyRect               ( _Inout_ CRect& rc                                               );

	static CString EncodeWindowPlacement    ( _In_ const WINDOWPLACEMENT& wp                               );
	static void    DecodeWindowPlacement    ( _In_ const CString& s, _Inout_ WINDOWPLACEMENT& wp                   );
	static CString MakeSplitterPosEntry     ( _In_ const LPCTSTR name                                      );
	static CString MakeColumnOrderEntry     ( _In_ const LPCTSTR name                                      );
	static CString MakeDialogRectangleEntry ( _In_ const LPCTSTR name                                      );
	static CString MakeColumnWidthsEntry    ( _In_ const LPCTSTR name                                      );

};

//
// CLanguageOptions. Is separated from COptions because it
// must be loaded earlier.
//

class CLanguageOptions: private CRegistryUser
{
public:
	//static LANGID GetLanguage ( );
	//static void   SetLanguage ( const LANGID langid );
};




//
// COptions. Represents all the data which can be viewed
// and modified in the "Configure WinDirStat" dialog.
//

// COptions is a singleton.
_Success_( return != NULL ) COptions *GetOptions();

class COptions : private CRegistryUser {
public:
	COptions();

	void LoadFromRegistry            (                                                         );
	void SaveToRegistry              (                                                         );
	void SetFollowJunctionPoints     ( _In_ const bool ignore                                       );
	void SetFollowMountPoints        ( _In_ const bool follow                                       );
	void SetHumanFormat              ( _In_ const bool human                                        );
	void SetListFullRowSelection     ( _In_ const bool show                                         );
	void SetListGrid                 ( _In_ const bool show                                         );
	void SetListStripes              ( _In_ const bool show                                         );
	void SetPacmanAnimation          ( _In_ const bool animate                                      );

	void SetShowTimeSpent            ( _In_ const bool show                                         );
	void SetTreelistColorCount       ( _In_ const INT count                                         );
	void SetTreelistColors           ( _In_ _In_reads_( TREELISTCOLORCOUNT ) const COLORREF color[ TREELISTCOLORCOUNT ]              );
	void SetTreemapHighlightColor    ( _In_ const COLORREF color                                    );
	void SetTreemapOptions           ( _In_ const CTreemap::Options& options                        );

	bool IsFollowMountPoints         ( ) const;
	bool IsFollowJunctionPoints      ( ) const;// Option to ignore junction points which are not volume mount points
	bool IsHumanFormat               ( ) const;
	bool IsListGrid                  ( ) const;
	bool IsListFullRowSelection      ( ) const;
	bool IsListStripes               ( ) const;

	bool IsShowTimeSpent             ( ) const;

	void GetTreelistColors           ( _Inout_ _Inout_updates_( TREELISTCOLORCOUNT ) COLORREF color[TREELISTCOLORCOUNT] );
	
	COLORREF GetTreelistColor        ( _In_ _In_range_( 0, TREELISTCOLORCOUNT ) const INT i ) const;

	INT GetTreelistColorCount        ( ) const;
	
	COLORREF GetTreemapHighlightColor( ) const;
	
	_Must_inspect_result_ const CTreemap::Options *GetTreemapOptions( ) const;

private:
	void ReadTreemapOptions          ( );
	void SaveTreemapOptions          ( );

	bool               m_listGrid;
	bool               m_listStripes;
	bool               m_listFullRowSelection;
	bool               m_humanFormat;

	_Field_size_( TREELISTCOLORCOUNT )     COLORREF           m_treelistColor[ TREELISTCOLORCOUNT ];
	_Field_range_( 1, TREELISTCOLORCOUNT ) INT                m_treelistColorCount;
	
	bool               m_followMountPoints;
	bool               m_followJunctionPoints;	
	bool               m_showTimeSpent;
	COLORREF           m_treemapHighlightColor;

	CTreemap::Options  m_treemapOptions;
	};



// $Log$
// Revision 1.12  2004/11/24 20:28:13  bseifert
// Implemented context menu compromise.
//
// Revision 1.11  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.10  2004/11/12 09:27:01  assarbad
// - Implemented ExplorerStyle option which will not be accessible through the options dialog.
//   It handles where the context menu is being shown.
//
// Revision 1.9  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
