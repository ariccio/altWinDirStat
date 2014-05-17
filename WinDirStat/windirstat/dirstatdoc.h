// dirstatdoc.h		- Declaration of the CDirstatDoc class
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

#include "selectdrivesdlg.h"
#include <vector>

class CItem;

//
// The treemap colors as calculated in CDirstatDoc::SetExtensionColors()
// all have the "brightness" BASE_BRIGHTNESS.
// I define brightness as a number from 0 to 3.0: (r+g+b)/255.
// RGB(127, 255, 0), for example, has a brightness of 2.5.
// 
#define BASE_BRIGHTNESS 1.8

//
// Data stored for each extension.
//
struct SExtensionRecord
{
	LONGLONG files;
	LONGLONG bytes;
	COLORREF color;
};

//
// Maps an extension (".bmp") to an SExtensionRecord.
//
typedef CMap<CString, LPCTSTR, SExtensionRecord, SExtensionRecord&> CExtensionData;

//
// Hints for UpdateAllViews()
//
enum
{
	HINT_NULL,				        // General update
	HINT_NEWROOT,			        // Root item has changed - clear everything.
	HINT_SELECTIONCHANGED,	        // The selection has changed, EnsureVisible.
	HINT_SHOWNEWSELECTION,	        // The selection has changed, Show Path
	HINT_SELECTIONSTYLECHANGED,	    // Only update selection in Graphview
	HINT_EXTENSIONSELECTIONCHANGED,	// Type list selected a new extension
	HINT_ZOOMCHANGED,		        // Only zoom item has changed.
	HINT_REDRAWWINDOW,		        // Only graphically redraw views.
	HINT_SOMEWORKDONE,		        // Directory list shall process mouse messages first, then re-sort.
	HINT_LISTSTYLECHANGED,	        // Options: List style (grid/stripes) or treelist colors changed
	HINT_TREEMAPSTYLECHANGED	    // Options: Treemap style (grid, colors etc.) changed
};

// CDirstatDoc. The "Document" class. 
// Owner of the root item and various other data (see data members).
class CDirstatDoc : public CDocument
{
protected:
	CDirstatDoc();	// Created by MFC only
	DECLARE_DYNCREATE(CDirstatDoc)

public:
	virtual ~CDirstatDoc();

	static void    DecodeSelection      ( const CString s,   _Inout_ CString& folder,  _Inout_ CStringArray& drives      );
	virtual void   DeleteContents       (                                                                                );
	static CString EncodeSelection      ( const RADIO radio,            const CString folder, const CStringArray& drives );
	static TCHAR   GetEncodingSeparator (                                                                                );
	virtual BOOL   OnNewDocument        (                                                                                );
	virtual BOOL   OnOpenDocument       ( const LPCTSTR   lpszPathName                                                   );
	virtual void   SetPathName          ( const LPCTSTR   lpszPathName, BOOL bAddToMRU                                   );
	virtual void   Serialize            ( const CArchive& ar                                                             );
	CExtensionData* GetExtensionDataPtr (                                                                                );


	COLORREF        GetCushionColor     ( LPCTSTR ext );
	COLORREF        GetZoomColor        (             ) const;


	//const CExtensionData *GetExtensionData(); TODO: investigate failure
	CExtensionData *GetExtensionData    (             );
	LONGLONG        GetRootSize         (             ) const;


	bool IsDrive                        ( CString spec                                     );
	bool IsRootDone                     (                                                  )      const;
	bool IsZoomed                       (                                                  )      const;
	bool OptionShowFreeSpace            (                                                  )      const;
	bool OptionShowUnknown              (                                                  )      const;
	bool UserDefinedCleanupWorksForItem ( const USERDEFINEDCLEANUP *udc, const CItem *item );
	bool Work                           ( DWORD ticks                                      ); // return: true if done.


	void ForgetItemTree                 (                                                                );
	void OpenItem                       ( const CItem *item                                              );
	void RefreshJunctionItems           (                                                                );
	void RefreshMountPointItems         (                                                                );
	void SetHighlightExtension          ( const LPCTSTR ext                                              );
	void SetSelection                   ( const CItem *item,   const bool keepReselectChildStack = false );
	void SetTitlePrefix                 ( const CString prefix                                           );
	void UnlinkRoot                     (                                                                );

	CItem  *GetRootItem                 ( ) const;
	CItem  *GetSelection                ( ) const;
	CItem  *GetZoomItem                 ( ) const;

	CString GetHighlightExtension       ( ) const;

	LONGLONG GetWorkingItemReadJobs     ( ) const;


protected:

	static CExtensionData *_pqsortExtensionData;
	static int __cdecl _compareExtensions     ( const void *ext1, const void *ext2 );
	bool stdCompareExtensions                 ( const CString *stringOne, const CString *stringTwo );

	CString BuildUserDefinedCleanupCommandLine( const LPCTSTR format, const LPCTSTR rootPath, const LPCTSTR currentPath );
		
	bool DeletePhysicalItem                   ( CItem *item, const bool toTrashBin );
	bool DirectoryListHasFocus                (                                    ) const;
	bool IsReselectChildAvailable             (                                    ) const;

	CItem *PopReselectChild                   (                                    );

	void AskForConfirmation                   ( const USERDEFINEDCLEANUP *udc, CItem *item ) throw ( CUserException * );
	void PerformUserDefinedCleanup            ( const USERDEFINEDCLEANUP *udc, CItem *item ) throw ( CException *     );
	
	void CallUserDefinedCleanup               ( const bool isDirectory, const CString& format, const CString& rootPath, const CString& currentPath, const bool showConsoleWindow, const bool wait );
	void ClearReselectChildStack              (                                                                                                                                                   );
	void GetDriveItems                        ( CArray<CItem *, CItem *>& drives                                                                                                                  );
	void PushReselectChild                    ( CItem *item                                                                                                                                       );
	void RefreshAfterUserDefinedCleanup       ( const USERDEFINEDCLEANUP *udc, CItem *item                                                                                                        );
	void RecursiveUserDefinedCleanup          ( const USERDEFINEDCLEANUP *udc, const CString& rootPath, const CString& currentPath                                                                );
	void RecurseRefreshMountPointItems        ( CItem *item                                                                                                                                       );
	void RecurseRefreshJunctionItems          ( CItem *item                                                                                                                                       );
	void RefreshItem                          ( CItem *item                                                                                                                                       );
	//void RefreshRecyclers                     (                                                                                                                                                   );
	void RebuildExtensionData                 (                                                                                                                                                   );
	std::vector<CString> stdSortExtData       ( CStringArray& sortedExtensions                                                                                                                    );
	void SortExtensionData                    ( CStringArray& sortedExtensions                                                                                                                    );
	void SetExtensionColors                   ( const CStringArray& sortedExtensions                                                                                                              );
	void stdSetExtensionColors                ( const std::vector<CString>& extensionsToSet                                                                                                       );
	void SetWorkingItemAncestor               ( CItem *item                                                                                                                                       );
	void SetWorkingItem                       ( CItem *item                                                                                                                                       );
	void SetZoomItem                          ( CItem *item                                                                                                                                       );

	bool    m_showFreeSpace;		// Whether to show the <Free Space> item
	bool    m_showUnknown;			// Whether to show the <Unknown> item
	bool    m_showMyComputer;		// True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool    m_extensionDataValid;   // If this is false, m_extensionData must be rebuilt

	CItem  *m_rootItem;			    // The very root item
	CItem  *m_selectedItem;		    // Currently selected item, or NULL
	CString m_highlightExtension;	// Currently highlighted extension
	CItem  *m_zoomItem;			    // Current "zoom root"
	CItem  *m_workingItem;		   // Current item we are working on. For progress indication

	CExtensionData m_extensionData;		// Base for the extension view and cushion colors

	CList<CItem *, CItem *> m_reselectChildStack; // Stack for the "Re-select Child"-Feature

#ifdef DEBUG
	void traceOut_ColorExtensionSetDebugLog( );
	bool isColorInVector( DWORD aColor, std::vector<DWORD>& colorVector );
	struct debuggingLogger {
		bool iLessThan_Colors_GetSize;
		int iterator;
		DWORD color;
		CString extension;
		};
	std::vector<debuggingLogger> ColorExtensionSetDebugLog;
	std::vector<DWORD> workDone;
#endif
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateRefreshselected(CCmdUI *pCmdUI);
	afx_msg void OnRefreshselected();
	afx_msg void OnUpdateRefreshall(CCmdUI *pCmdUI);
	afx_msg void OnRefreshall();
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnCleanupEmptyrecyclebin();
	afx_msg void OnUpdateViewShowfreespace(CCmdUI *pCmdUI);
	afx_msg void OnViewShowfreespace();
	afx_msg void OnUpdateViewShowunknown(CCmdUI *pCmdUI);
	afx_msg void OnViewShowunknown();
	afx_msg void OnUpdateTreemapZoomin(CCmdUI *pCmdUI);
	afx_msg void OnTreemapZoomin();
	afx_msg void OnUpdateTreemapZoomout(CCmdUI *pCmdUI);
	afx_msg void OnTreemapZoomout();
	afx_msg void OnUpdateExplorerHere(CCmdUI *pCmdUI);
	afx_msg void OnExplorerHere();
	afx_msg void OnUpdateCommandPromptHere(CCmdUI *pCmdUI);
	afx_msg void OnCommandPromptHere();
	afx_msg void OnUpdateCleanupDeletetotrashbin(CCmdUI *pCmdUI);
	afx_msg void OnCleanupDeletetotrashbin();
	afx_msg void OnUpdateCleanupDelete(CCmdUI *pCmdUI);
	afx_msg void OnCleanupDelete();
	//afx_msg void OnUpdateUserdefinedcleanup(CCmdUI *pCmdUI);
	//afx_msg void OnUserdefinedcleanup(UINT id);
	afx_msg void OnUpdateTreemapSelectparent(CCmdUI *pCmdUI);
	afx_msg void OnTreemapSelectparent();
	afx_msg void OnUpdateTreemapReselectchild(CCmdUI *pCmdUI);
	afx_msg void OnTreemapReselectchild();
	afx_msg void OnUpdateCleanupOpen(CCmdUI *pCmdUI);
	afx_msg void OnCleanupOpen();
	afx_msg void OnUpdateCleanupProperties(CCmdUI *pCmdUI);
	afx_msg void OnCleanupProperties();

public:
	double m_searchTime;
	LARGE_INTEGER m_searchStartTime;
	LARGE_INTEGER m_timerFrequency;
	
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
};

//
// The document is needed in many places.
//
extern CDirstatDoc *GetDocument();



// $Log$
// Revision 1.9  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
