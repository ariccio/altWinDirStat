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

#include "stdafx.h"

#ifndef WDS_DIRSTATDOC_H
#define WDS_DIRSTATDOC_H


class CItemBranch;
class CMainFrame;


// The treemap colors as calculated in CDirstatDoc::SetExtensionColors() all have the "brightness" BASE_BRIGHTNESS.
// I define brightness as a number from 0 to 3.0: (r+g+b)/255.
// RGB(127, 255, 0), for example, has a brightness of 2.5.
#define BASE_BRIGHTNESS 1.8




// The "Document" class. Owner of the root item and various other data (see data members).
class CDirstatDoc final : public CDocument {
protected:
	_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this ) CDirstatDoc( );	// Created by MFC only
	
	/*

	//DECLARE_DYNCREATE(CDirstatDoc)
	//--becomes--
	  //DECLARE_DYNAMIC(CDirstatDoc) \
	  //static CObject* PASCAL CreateObject();
	//--becomes--
  //public: \
	  //static const CRuntimeClass classCDirstatDoc; \
	  //virtual CRuntimeClass* GetRuntimeClass() const; \
	  //static CObject* PASCAL CreateObject();

	*/
	DECLARE_DYNCREATE(CDirstatDoc)

public:

	virtual ~CDirstatDoc( ) final;
	CDirstatDoc& operator=( const CDirstatDoc& in ) = delete;
	CDirstatDoc( const CDirstatDoc& in ) = delete;

	virtual void     DeleteContents        (                                                      ) override final;
	virtual BOOL     OnNewDocument         (                                                      ) override final;
	virtual BOOL     OnOpenDocument        ( _In_z_     PCWSTR   pszPathName                      ) override final;
	COLORREF         GetCushionColor       ( _In_z_     PCWSTR   ext                              );
	
	bool Work                              ( ); // return: true if done.
	bool OnWorkFinished                    ( );
	
	//_Pre_satisfies_( item.m_type == IT_FILE )
	_Pre_satisfies_( item.m_children._Myptr == nullptr )
	void OpenItem ( _In_     const CItemBranch& item                                                  );
	void SetSelection ( _In_ const CItemBranch& item );


	void SetHighlightExtension             ( _In_ const std::wstring       ext                                                   );
	void ForgetItemTree                    ( );
	void SortTreeList                      ( );	


	_Ret_notnull_ const std::vector<SExtensionRecord>* GetExtensionRecords ( );
	
	bool   IsRootDone    ( ) const;

protected:
	
	std::vector<std::wstring> buildRootFolders( _In_     const std::vector<std::wstring>& drives,        _In_    std::wstring& folder );
	void buildDriveItems                      ( _In_     const std::vector<std::wstring>&          rootFolders );
	void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>& extensionsToSet );

	void VectorExtensionRecordsToMap          ( );
	void RebuildExtensionData                 ( );
	
	bool                                      m_showMyComputer;       // True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool                                      m_extensionDataValid;   // If this is false, m_extensionData must be rebuilt
	//C4820: 'CDirstatDoc' : '5' bytes padding added after data member 'CDirstatDoc::m_timeTextWritten' (dirstatdoc.cpp)
	bool                                      m_timeTextWritten;
	//moved std::uint16_t to before m_highlightExtension --- 3 bytes padding left?


public:
	std::uint16_t                             m_iterations;
	std::wstring                              m_highlightExtension;   // Currently highlighted extension
	std::unique_ptr<CItemBranch>              m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	CItemBranch const*                        m_selectedItem;         // Currently selected item, or NULL
	CItemBranch const*                        m_workingItem;          // Current item we are working on. For progress indication
	//C4820: 'CDirstatDoc' : '6' bytes padding added after data member 'CDirstatDoc::m_iterations' (dirstatdoc.cpp)
	
protected:	
	std::vector<SExtensionRecord>             m_extensionRecords;
	std::unordered_map<std::wstring, COLORREF>m_colorMap;

public:
	DOUBLE                                    m_searchTime;
	DOUBLE                                    m_compressed_file_timing;
	LARGE_INTEGER                             m_searchStartTime;
	CMainFrame*                               m_frameptr;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateEditCopy( _In_ CCmdUI* pCmdUI );
	afx_msg void OnEditCopy( );
	
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
#else
#error ass
#endif
