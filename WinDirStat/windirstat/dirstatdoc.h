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

class CItemBranch;


// The treemap colors as calculated in CDirstatDoc::SetExtensionColors() all have the "brightness" BASE_BRIGHTNESS.
// I define brightness as a number from 0 to 3.0: (r+g+b)/255.
// RGB(127, 255, 0), for example, has a brightness of 2.5.
#define BASE_BRIGHTNESS 1.8

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
	};



// The "Document" class. Owner of the root item and various other data (see data members).
class CDirstatDoc : public CDocument {
protected:
	_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this ) CDirstatDoc( );	// Created by MFC only
	DECLARE_DYNCREATE(CDirstatDoc)

public:

	virtual ~CDirstatDoc();
	
	virtual void     DeleteContents        (                                                      ) override final;
	virtual BOOL     OnNewDocument         (                                                      ) override final;
	virtual BOOL     OnOpenDocument        ( _In_z_     PCWSTR   pszPathName                      ) override final;
	
	COLORREF         GetCushionColor       ( _In_z_     PCWSTR   ext                              );
	
	bool Work                              ( ); // return: true if done.
	bool OnWorkFinished                    ( );
	

	_Pre_satisfies_( item.m_type == IT_FILE ) void OpenItem ( _In_   const CItemBranch& item                                                  );
	_Pre_satisfies_( this->m_zoomItem != NULL ) _When_( ( this->m_zoomItem != NULL ), _Post_satisfies_( m_selectedItem == (&item) ) ) void SetSelection ( _In_   const CItemBranch& item );


	void SetHighlightExtension             ( _In_ const std::wstring       ext                                                   );
	void SetTitlePrefix                    ( _In_ std::wstring      prefix                                                ) const;
	void ForgetItemTree                    ( );
	void SortTreeList                      ( );	

	const std::wstring& GetHighlightExtension     ( ) const;


	_Ret_notnull_ const std::vector<SExtensionRecord>* GetExtensionRecords ( );
	_Must_inspect_result_ _Ret_maybenull_ const CItemBranch*                   GetSelection        ( ) const;
	_Must_inspect_result_ _Ret_maybenull_ const CItemBranch*                   GetZoomItem         ( ) const;
	_Must_inspect_result_ _Ret_maybenull_ const CItemBranch*                   GetRootItem         ( ) const;
	_Success_( return < UINT64_MAX ) std::uint64_t GetRootSize ( ) const;
	
	bool   IsRootDone    ( ) const;
	bool   IsZoomed      ( ) const;
	
	_Ret_range_( 0, 33000 ) 
	DOUBLE GetNameLength( ) const {
		return m_rootItem->averageNameLength( );
		}

	
protected:
	
	std::vector<std::wstring> buildRootFolders     ( _In_           std::vector<std::wstring>& drives,        _In_    std::wstring& folder );

	void SetWorkingItem                       ( _In_opt_       const CItemBranch*             const item );
	void buildDriveItems                      ( _In_     const std::vector<std::wstring>&          rootFolders );
	void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>& extensionsToSet );
	void SetZoomItem                          ( _In_     const CItemBranch&              item            );
	
	void VectorExtensionRecordsToMap          ( );
	void RebuildExtensionData                 ( );
	bool DirectoryListHasFocus                (                                                                                        ) const;
	
	bool    m_showMyComputer     : 1;   // True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool    m_extensionDataValid : 1;   // If this is false, m_extensionData must be rebuilt
	bool    m_timeTextWritten    : 1;

	std::wstring                              m_highlightExtension;   // Currently highlighted extension
	std::unique_ptr<CItemBranch>              m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	CItemBranch const*                        m_selectedItem;         // Currently selected item, or NULL
	CItemBranch const*                        m_zoomItem;             // Current "zoom root"
public:
	CItemBranch const*                        m_workingItem;          // Current item we are working on. For progress indication
protected:	
	std::uint64_t                             m_freeDiskSpace;
	std::uint64_t                             m_totalDiskSpace;
	std::vector<SExtensionRecord>             m_extensionRecords;
	std::map<std::wstring, COLORREF>          m_colorMap;

public:
	DOUBLE m_searchTime;
	LARGE_INTEGER m_searchStartTime;
	LARGE_INTEGER m_timerFrequency;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateEditCopy( _In_ CCmdUI* pCmdUI );
	afx_msg void OnEditCopy( );
	afx_msg void OnUpdateTreemapZoomin( _In_ CCmdUI* pCmdUI );
	afx_msg void OnTreemapZoomin( );
	afx_msg void OnUpdateTreemapZoomout( _In_ CCmdUI* pCmdUI );
	afx_msg _Pre_satisfies_( this->m_zoomItem != NULL ) void OnTreemapZoomout( );
	afx_msg void OnUpdateTreemapSelectparent( _In_ CCmdUI* pCmdUI );
	afx_msg _Pre_satisfies_( this->m_selectedItem != NULL ) void OnTreemapSelectparent( );
	
public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
	};

// The document is needed in many places.
extern CDirstatDoc *GetDocument();



// $Log$
// Revision 1.9  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
