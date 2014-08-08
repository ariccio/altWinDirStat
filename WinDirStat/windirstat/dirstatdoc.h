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
//#include <vector>

class CItem;
class CItemBranch;


#ifdef USE_USN_JOURNAL
struct relevantUpdateSequenceNumberChangeJournalInformation {
	DWORDLONG fileReferenceNumber;
	DWORDLONG parentFileReferenceNumber;
	USN fileUSN;
	DWORD fileAttributes;
	WORD fileNameLength;
	WORD fileNameOffset;
	std::wstring fileName;
	};

typedef relevantUpdateSequenceNumberChangeJournalInformation relUSNInfo;

struct childrenOfUSN {
	relUSNInfo thisUSN;
	std::vector<relUSNInfo> children;
#ifdef DEBUG
	void display( ) const {
		for ( const auto& aChild : children ) {
			TRACE( _T( "%s/%s\r\n" ), thisUSN.fileName, aChild.fileName );
			}
		}
#endif
	};
#endif

//
// The treemap colors as calculated in CDirstatDoc::SetExtensionColors()
// all have the "brightness" BASE_BRIGHTNESS.
// I define brightness as a number from 0 to 3.0: (r+g+b)/255.
// RGB(127, 255, 0), for example, has a brightness of 2.5.
// 
#define BASE_BRIGHTNESS 1.8


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

	static  void    DecodeSelection       ( _In_ const CString s,              _Inout_       CString& folder,  _Inout_        CStringArray& drives );
	virtual void    DeleteContents        (                                                                                                        );
	static  CString EncodeSelection       ( _In_ const RADIO radio,            _In_    const CString  folder,   _In_    const CStringArray& drives );
	static  TCHAR   GetEncodingSeparator  (                                                                                                        );
	virtual BOOL    OnNewDocument         (                                                                                                        );
	virtual BOOL    OnOpenDocument        ( _In_ LPCTSTR   lpszPathName                                                                            );
	virtual void    SetPathName           ( _In_ LPCTSTR   lpszPathName,                     BOOL     bAddToMRU                                    );
	virtual void    Serialize             ( _In_ const CArchive& ar                                                                                );

	_Must_inspect_result_ std::vector<SExtensionRecord>*       GetExtensionRecords    ( );
	_Must_inspect_result_ CItemBranch*                               GetRootItem            ( ) const;
	_Must_inspect_result_ CItemBranch*                               GetSelection           ( ) const;
	_Must_inspect_result_ CItemBranch*                               GetZoomItem            ( ) const;


	COLORREF        GetCushionColor     ( _In_ LPCWSTR ext  );
	COLORREF        GetZoomColor        (                   ) const;
	LONGLONG        GetRootSize         (                   ) const;
	std::int64_t    GetFreeDiskSpace    ( _In_ CString path );
	std::int64_t    GetTotlDiskSpace    ( _In_ CString path );

	bool IsDrive                        ( _In_ const CString spec                                      ) const;
	bool IsRootDone                     (                                                              ) const;
	bool IsZoomed                       (                                                              ) const;
	bool OptionShowFreeSpace            (                                                              ) const;
	bool OptionShowUnknown              (                                                              ) const;
	bool Work                           ( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t               ticks                         ); // return: true if done.


	void ForgetItemTree                 (                                                                                        );
	void OpenItem                       ( _In_ const CItemBranch*              item                                                    );
	void RefreshJunctionItems           (                                                                                        );
	void RefreshMountPointItems         (                                                                                        );
	void SetHighlightExtension          ( _In_ const LPCTSTR             ext                                                     );
	void SetSelection                   ( _In_ const CItemBranch*              item,  _In_ const bool   keepReselectChildStack = false );
	void SetTitlePrefix                 ( const CString prefix                                                                   );
	void UnlinkRoot                     (                                                                                        );

	
	void clearZoomItem              ( );
	void clearRootItem              ( );
	void clearSelection             ( );

#ifdef USE_USN_JOURNAL
	void experimentalFunc           ( );
#endif

	void experimentalSection        ( _In_ CStringArray& drives );
	CString GetHighlightExtension   ( ) const;
	LONGLONG GetWorkingItemReadJobs ( ) const;


protected:

	static CExtensionData *_pqsortExtensionData;

	void buildDriveItems( _In_ CStringArray& rootFolders, _Inout_ std::vector<std::shared_ptr<CItemBranch>>& smart_driveItems );
	void buildRootFolders( _In_ CStringArray& drives, _In_ CString& folder, _Inout_ CStringArray& rootFolders );
	void CreateUnknownAndFreeSpaceItems( _Inout_ std::vector<std::shared_ptr<CItemBranch>>& smart_driveItems );

	static INT __cdecl _compareExtensions     ( _In_ const void*    ext1,      _In_ const void*    ext2                                );
	bool stdCompareExtensions                 ( _In_ const CString* stringOne, _In_ const CString* stringTwo                           );
		
	bool DeletePhysicalItem                   ( _In_       CItemBranch* item,        _In_ const bool     toTrashBin                          );
	bool DirectoryListHasFocus                (                                                                                        ) const;
	bool IsReselectChildAvailable             (                                                                                        ) const;

	_Must_inspect_result_ CItemBranch* PopReselectChild                   (                                    );	
	
	void ClearReselectChildStack              (                                                                                                                                 );
	void GetDriveItems                        ( _Inout_        CArray<CItemBranch*, CItemBranch*>&            drives                                                                      );
	void PushReselectChild                    (                CItemBranch*                               item                                                                        );
	void RecurseRefreshMountPointItems        ( _In_           CItemBranch*                               item                                                                        );
	void RecurseRefreshJunctionItems          ( _In_           CItemBranch*                               item                                                                        );
	void RefreshItem                          ( _In_           CItemBranch*                               item                                                                        );
	void RebuildExtensionData                 (                                                                                                                                 );
	void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>&       extensionsToSet                                                             );
	void SetWorkingItemAncestor               ( _In_           CItemBranch*                               item                                                                        );
	void SetWorkingItem                       ( _In_opt_       CItemBranch*                               item                                                                        );
	void SetWorkingItem                       ( _In_opt_       CItemBranch*                               item,            _In_    bool                                  hideTiming   );
	void SetZoomItem                          ( _In_           CItemBranch*                               item                                                                        );
	
	void VectorExtensionRecordsToMap( );
	std::vector<CItemBranch*>                 modernGetDriveItems( );
	
	bool    m_showFreeSpace;		// Whether to show the <Free Space> item
	bool    m_showUnknown;			// Whether to show the <Unknown> item
	bool    m_showMyComputer;		// True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool    m_extensionDataValid;   // If this is false, m_extensionData must be rebuilt
	bool    m_timeTextWritten;

	

	CString                             m_highlightExtension;   // Currently highlighted extension

	CItemBranch*                              m_rootItem;             // The very root item
	//std::shared_ptr<CItem>              m_smartRootItem;
	CItemBranch*                              m_selectedItem;         // Currently selected item, or NULL
	
	CItemBranch*                              m_zoomItem;             // Current "zoom root"
	CItemBranch*                              m_workingItem;          // Current item we are working on. For progress indication

	CList<CItemBranch *, CItemBranch *>             m_reselectChildStack;   // Stack for the "Re-select Child"-Feature

#ifdef USE_USN_JOURNAL
	std::vector<relUSNInfo>             USNstructs;
	std::map<DWORDLONG, relUSNInfo>     parentUSNs;
#endif

	LONGLONG                 m_freeDiskSpace;   
	LONGLONG                 m_totalDiskSpace;

	std::vector<SExtensionRecord> m_extensionRecords;

	std::map<CString, COLORREF> m_colorMap;

#ifdef _DEBUG
	void traceOut_ColorExtensionSetDebugLog( );
	bool isColorInVector( DWORD aColor, std::vector<DWORD>& colorVector );
	struct debuggingLogger {
		bool iLessThan_Colors_GetSize;
		INT iterator;
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
	afx_msg void OnUpdateTreemapSelectparent(CCmdUI *pCmdUI);
	afx_msg void OnTreemapSelectparent();
	afx_msg void OnUpdateTreemapReselectchild(CCmdUI *pCmdUI);
	afx_msg void OnTreemapReselectchild();
	afx_msg void OnUpdateCleanupOpen(CCmdUI *pCmdUI);
	afx_msg void OnCleanupOpen();
	afx_msg void OnUpdateCleanupProperties(CCmdUI *pCmdUI);
	afx_msg void OnCleanupProperties();

public:
	DOUBLE m_searchTime;
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
