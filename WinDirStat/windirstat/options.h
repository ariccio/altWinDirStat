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

struct USERDEFINEDCLEANUP
{
	bool enabled;
	bool virginTitle;
	CString title;
	bool worksForDrives;
	bool worksForDirectories;
	bool worksForFilesFolder;
	bool worksForFiles;
	bool worksForUncPaths;
	CString commandLine;
	bool recurseIntoSubdirectories;
	bool askForConfirmation;
	bool showConsoleWindow;
	bool waitForCompletion;
	REFRESHPOLICY refreshPolicy;
};

#define USERDEFINEDCLEANUPCOUNT 10

#define TREELISTCOLORCOUNT 8


//
// CRegistryUser. (Base class for COptions and CPersistence.)
// Can read from and write to the registry.
//
class CRegistryUser
{
public:
	static void    SetProfileString(const LPCTSTR section, const LPCTSTR entry, const LPCTSTR value);
	static CString GetProfileString( const LPCTSTR section, const LPCTSTR entry, const LPCTSTR defaultValue );

	static void SetProfileInt(const LPCTSTR section, const LPCTSTR entry, const int value);
	static int  GetProfileInt( const LPCTSTR section, const LPCTSTR entry, const int defaultValue );

	static void SetProfileBool(const LPCTSTR section, const LPCTSTR entry, const bool value);
	static bool GetProfileBool( const LPCTSTR section, const LPCTSTR entry, const bool defaultValue );

	static void CheckRange( int& value, const int min, const int max );
};


//
// CPersistence. Reads from and writes to the registry all the persistent settings
// like window position, column order etc.
//
class CPersistence: private CRegistryUser
{
public:
	static bool GetShowFreeSpace();
	static void SetShowFreeSpace( const bool show );

	static bool GetShowUnknown();
	static void SetShowUnknown( const bool show );

	static bool GetShowFileTypes();
	static void SetShowFileTypes( const bool show );

	static bool GetShowTreemap();
	static void SetShowTreemap( const bool show );

	static bool GetShowToolbar();
	static void SetShowToolbar( const bool show );

	static bool GetShowStatusbar();
	static void SetShowStatusbar( const bool show );

	static void GetMainWindowPlacement(/* [in/out] */ WINDOWPLACEMENT& wp);
	static void SetMainWindowPlacement(const WINDOWPLACEMENT& wp);

	static void SetSplitterPos( const LPCTSTR name, const bool valid, const double userpos );
	static void GetSplitterPos( const LPCTSTR name, bool& valid, double& userpos );

	static void SetColumnOrder(const LPCTSTR name, const CArray<int, int>& arr);
	static void GetColumnOrder( const LPCTSTR name, /* in/out */ CArray<int, int>& arr );

	static void SetColumnWidths(const LPCTSTR name, const CArray<int, int>& arr);
	static void GetColumnWidths( const LPCTSTR name, /* in/out */ CArray<int, int>& arr );

	static void SetDialogRectangle(const LPCTSTR name, const CRect& rc);
	static void GetDialogRectangle( const LPCTSTR name, CRect& rc );

//	static void SetSorting(LPCTSTR name, int column1, bool ascending1, int column2, bool ascending2);
//	static void GetSorting(LPCTSTR name, int columnCount, int& column1, bool& ascending1, int& column2, bool& ascending2);

	static int  GetConfigPage( const int max );
	static void SetConfigPage( const int page );

	static void GetConfigPosition(/* in/out */ CPoint& pt);
	static void SetConfigPosition(const CPoint pt);

	static CString GetBarStateSection();

	static int  GetSelectDrivesRadio();
	static void SetSelectDrivesRadio(const int radio);

	static CString GetSelectDrivesFolder();
	static void    SetSelectDrivesFolder(const LPCTSTR folder);

	static void GetSelectDrivesDrives(CStringArray& drives);
	static void SetSelectDrivesDrives(const CStringArray& drives);

	static bool GetShowDeleteWarning();
	static void SetShowDeleteWarning(const bool show);

private:
	static void SetArray(const LPCTSTR entry, const CArray<int, int>& arr);
	static void GetArray(const LPCTSTR entry, /* in/out */ CArray<int, int>& arr);
	static void SetRect(const LPCTSTR entry, const CRect& rc);
	static void GetRect( const LPCTSTR entry, CRect& rc );
	static void SanifyRect(CRect& rc);

	static CString EncodeWindowPlacement(const WINDOWPLACEMENT& wp);
	static void    DecodeWindowPlacement(const CString& s, WINDOWPLACEMENT& wp);
	static CString MakeSplitterPosEntry(const LPCTSTR name);
	static CString MakeColumnOrderEntry(const LPCTSTR name);
	static CString MakeDialogRectangleEntry( const LPCTSTR name );
	static CString MakeColumnWidthsEntry(const LPCTSTR name);
	//static CString MakeSortingColumnEntry(LPCTSTR name, int n);
	//static CString MakeSortingAscendingEntry(LPCTSTR name, int n);
	


};

//
// CLanguageOptions. Is separated from COptions because it
// must be loaded earlier.
//

class CLanguageOptions: private CRegistryUser
{
public:
	static LANGID GetLanguage();
	static void SetLanguage(const LANGID langid);
};




//
// COptions. Represents all the data which can be viewed
// and modified in the "Configure WinDirStat" dialog.
//

// COptions is a singleton.
COptions *GetOptions();

class COptions: private CRegistryUser
{
public:
	COptions();

	void LoadFromRegistry();
	void SaveToRegistry();

	bool IsListGrid() const;
	void SetListGrid(const bool show);

	bool IsListStripes() const;
	void SetListStripes(const bool show);

	bool IsListFullRowSelection() const;
	void SetListFullRowSelection(const bool show);

	void GetTreelistColors(COLORREF color[TREELISTCOLORCOUNT]);
	void SetTreelistColors(const COLORREF color[TREELISTCOLORCOUNT]);
	COLORREF GetTreelistColor(const int i) const;

	int GetTreelistColorCount() const;
	void SetTreelistColorCount(const int count);

	bool IsHumanFormat() const;
	void SetHumanFormat(const bool human);

	bool IsPacmanAnimation() const;
	void SetPacmanAnimation(const bool animate);

	bool IsShowTimeSpent() const;
	void SetShowTimeSpent(const bool show);

	COLORREF GetTreemapHighlightColor() const;
	void SetTreemapHighlightColor(const COLORREF color);

	const CTreemap::Options *GetTreemapOptions() const;
	void SetTreemapOptions(const CTreemap::Options& options);

	bool IsFollowMountPoints() const;
	void SetFollowMountPoints(const bool follow);

	// Option to ignore junction points which are not volume mount points
	bool IsFollowJunctionPoints() const;
	void SetFollowJunctionPoints(const bool ignore);

	// Option to use CDirStatApp::m_langid for date/time and number formatting
	bool IsUseWdsLocale() const;
	void SetUseWdsLocale(const bool use);

	void GetUserDefinedCleanups(USERDEFINEDCLEANUP udc[USERDEFINEDCLEANUPCOUNT]);
	void SetUserDefinedCleanups(const USERDEFINEDCLEANUP udc[USERDEFINEDCLEANUPCOUNT]);

	void  GetEnabledUserDefinedCleanups(CArray<int, int>& indices);
	bool  IsUserDefinedCleanupEnabled(const int i) const;
	const USERDEFINEDCLEANUP *GetUserDefinedCleanup( const int i ) const;

	CString GetReportSubject() const;
	CString GetReportDefaultSubject() const;
	void    SetReportSubject( const LPCTSTR subject );

	CString GetReportPrefix() const;
	CString GetReportDefaultPrefix() const;
	void    SetReportPrefix(const LPCTSTR prefix);

	CString GetReportSuffix() const;
	CString GetReportDefaultSuffix() const;
	void    SetReportSuffix( const LPCTSTR suffix );

private:
	//void ReadUserDefinedCleanup(int i);
	//void SaveUserDefinedCleanup(int i);
	bool LooksLikeVirginCleanupTitle(const CString title);
	void ReadTreemapOptions();
	void SaveTreemapOptions();

	bool m_listGrid;
	bool m_listStripes;
	bool m_listFullRowSelection;
	COLORREF m_treelistColor[TREELISTCOLORCOUNT];
	int m_treelistColorCount;
	bool m_humanFormat;
	bool m_pacmanAnimation;
	bool m_showTimeSpent;
	COLORREF m_treemapHighlightColor;

	CTreemap::Options m_treemapOptions;
	
	bool m_followMountPoints;
	bool m_followJunctionPoints;
	bool m_useWdsLocale;

	USERDEFINEDCLEANUP m_userDefinedCleanup[USERDEFINEDCLEANUPCOUNT];

	CString m_reportSubject;
	CString m_reportPrefix;
	CString m_reportSuffix;
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
