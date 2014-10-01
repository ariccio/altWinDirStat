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

#ifndef DIRSTATDOC_H
#define DIRSTATDOC_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"
//#include "selectdrivesdlg.h"
//#include "deletewarningdlg.h"
//#include "modalshellapi.h"
//#include "dirstatview.h"

//#include <vector>

class CItemBranch;


//
// The treemap colors as calculated in CDirstatDoc::SetExtensionColors() all have the "brightness" BASE_BRIGHTNESS.
// I define brightness as a number from 0 to 3.0: (r+g+b)/255.
// RGB(127, 255, 0), for example, has a brightness of 2.5.
#define BASE_BRIGHTNESS 1.8

// Maps an extension (".bmp") to an SExtensionRecord.
//typedef CMap<CString, PCTSTR, SExtensionRecord, SExtensionRecord&> CExtensionData;

// Hints for UpdateAllViews()
enum {
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
	HINT_TREEMAPSTYLECHANGED,	    // Options: Treemap style (grid, colors etc.) changed
	//HINT_HIDEUNKNOWN,
	//HINT_UNHIDEUNKNOWN,
	//HINT_HIDEFREESPACE,
	//HINT_UNHIDEFREESPACE
	};



// CDirstatDoc. The "Document" class. 
// Owner of the root item and various other data (see data members).
class CDirstatDoc : public CDocument {
protected:
	CDirstatDoc( );	// Created by MFC only
	DECLARE_DYNCREATE(CDirstatDoc)

public:

	virtual ~CDirstatDoc();
	
	virtual void     DeleteContents        (                                                      ) override;
	virtual BOOL     OnNewDocument         (                                                      ) override;
	virtual BOOL     OnOpenDocument        ( _In_z_     PCTSTR   lpszPathName                     ) override;
	
	COLORREF         GetCushionColor       ( _In_z_     PCWSTR   ext                              );
	
	bool Work                              ( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t ticks ); // return: true if done.
	bool IsDrive                           ( _In_                       const CString       spec  ) const;
	bool OnWorkFinished                    ( );
	
	void OpenItem                          ( _In_   const CItemBranch* item                                                  );
	void SetHighlightExtension             ( _In_z_ const PCTSTR       ext                                                   );
	void SetSelection                      ( _In_   const CItemBranch* item,  _In_ const bool keepReselectChildStack = false );
	void SetTitlePrefix                    ( _In_   const CString      prefix                                                );
	void ForgetItemTree                    ( );
	void SortTreeList                      ( );	

	std::wstring GetHighlightExtension     ( ) const;


	_Must_inspect_result_ std::vector<SExtensionRecord>* GetExtensionRecords ( );
	_Must_inspect_result_ CItemBranch*                   GetSelection        ( ) const;
	_Must_inspect_result_ CItemBranch*                   GetZoomItem         ( ) const;

	_Must_inspect_result_             CItemBranch*  GetRootItem ( );
	_Success_( return != UINT64_MAX ) std::uint64_t GetRootSize ( );
	
	bool   IsRootDone    ( );
	bool   IsZoomed      ( );
	_Requires_lock_held_( m_rootItemCriticalSection ) DOUBLE GetNameLength ( ) const;

	
protected:
	
	std::vector<CString> buildRootFolders     ( _In_           std::vector<CString>& drives,        _In_    CString& folder );

	void SetWorkingItem                       ( _In_opt_       CItemBranch*                   item, _In_    bool     hideTiming     );
	void buildDriveItems                      ( _In_           std::vector<CString>&          rootFolders );
	void PushReselectChild                    ( _In_           CItemBranch*                   item            );
	void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>& extensionsToSet );
	void SetWorkingItem                       ( _In_opt_       CItemBranch*                   item            );
	void SetZoomItem                          ( _In_     const CItemBranch*                   item            );
	
	void VectorExtensionRecordsToMap          ( );
	void RebuildExtensionData                 ( );
	void ClearReselectChildStack              ( );

	//bool stdCompareExtensions                 ( _In_ const CString* stringOne, _In_ const CString* stringTwo                           );	
	bool DeletePhysicalItem                   ( _In_       CItemBranch* item,  _In_ const bool     toTrashBin                          );
	bool DirectoryListHasFocus                (                                                                                        ) const;
	bool IsReselectChildAvailable             (                                                                                        ) const;

	_Must_inspect_result_ CItemBranch *PopReselectChild                   (                                    );	
	
	
	
	bool    m_showMyComputer     : 1;   // True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool    m_extensionDataValid : 1;   // If this is false, m_extensionData must be rebuilt
	bool    m_timeTextWritten    : 1;

	std::wstring                              m_highlightExtension;   // Currently highlighted extension

	CRITICAL_SECTION                          m_rootItemCriticalSection;
	_Guarded_by_( m_rootItemCriticalSection ) std::unique_ptr<CItemBranch>              m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	
	CItemBranch*                              m_selectedItem;         // Currently selected item, or NULL
	CItemBranch*                              m_zoomItem;             // Current "zoom root"
	CItemBranch*                              m_workingItem;          // Current item we are working on. For progress indication

	CList<CItemBranch *, CItemBranch *>       m_reselectChildStack;   // Stack for the "Re-select Child"-Feature

	std::uint64_t                             m_freeDiskSpace;
	std::uint64_t                             m_totalDiskSpace;
	std::vector<SExtensionRecord>             m_extensionRecords;
	std::map<std::wstring, COLORREF>          m_colorMap;

public:
	DOUBLE m_searchTime;
	LARGE_INTEGER m_searchStartTime;
	LARGE_INTEGER m_timerFrequency;


#ifdef _DEBUG
	void traceOut_ColorExtensionSetDebugLog( );
	bool isColorInVector( DWORD aColor, std::vector<DWORD>& colorVector );
	struct debuggingLogger {
		INT iterator;
		DWORD color;
		CString extension;
		bool iLessThan_Colors_GetSize :1 ;
		};
	std::vector<debuggingLogger> ColorExtensionSetDebugLog;
	std::vector<DWORD> workDone;
#endif
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateTreemapZoomin(CCmdUI *pCmdUI);
	afx_msg void OnTreemapZoomin();
	afx_msg void OnUpdateTreemapZoomout(CCmdUI *pCmdUI);
	afx_msg void OnTreemapZoomout();
	afx_msg void OnUpdateExplorerHere(CCmdUI *pCmdUI);
	afx_msg void OnExplorerHere();
	afx_msg void OnUpdateCommandPromptHere(CCmdUI *pCmdUI);
	afx_msg void OnCommandPromptHere();
	afx_msg void OnUpdateTreemapSelectparent(CCmdUI *pCmdUI);
	afx_msg void OnTreemapSelectparent();
	afx_msg void OnUpdateTreemapReselectchild(CCmdUI *pCmdUI);
	afx_msg void OnTreemapReselectchild();
	
public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
	};


class DirectoryWalker : public CWinThread {

	};





// The document is needed in many places.
extern CDirstatDoc *GetDocument();



// $Log$
// Revision 1.9  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
