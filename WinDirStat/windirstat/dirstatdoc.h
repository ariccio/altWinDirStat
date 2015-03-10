// dirstatdoc.h		- Declaration of the CDirstatDoc class
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"
//#include "globalhelpers.h"


#ifndef WDS_DIRSTATDOC_H
#define WDS_DIRSTATDOC_H

#pragma message( "Including `" __FILE__ "`..." )

#include "datastructures.h"
#include "ChildrenHeapManager.h"


class CTreeListItem;
class CMainFrame;
class CDirstatApp;

// The treemap colors as calculated in CDirstatDoc::SetExtensionColors() all have the "brightness" BASE_BRIGHTNESS. I define brightness as a number from 0 to 3.0: (r+g+b)/255. RGB(127, 255, 0), for example, has a brightness of 2.5.
#define BASE_BRIGHTNESS 1.8

// The "Document" class. Owner of the root item and various other data (see data members).
class CDirstatDoc final : public CDocument {
protected:
	_Pre_satisfies_( _theDocument == NULL ) _Post_satisfies_( _theDocument == this ) CDirstatDoc( );	// Created by MFC only
	
	/*
	DECLARE_DYNCREATE(CDirstatDoc)
	--becomes--
	  DECLARE_DYNAMIC(CDirstatDoc) \
	  static CObject* PASCAL CreateObject();
	--becomes--
  public: \
	  static const CRuntimeClass classCDirstatDoc; \
	  virtual CRuntimeClass* GetRuntimeClass() const; \
	  static CObject* PASCAL CreateObject();
	*/
	DECLARE_DYNCREATE(CDirstatDoc)

public:

	virtual ~CDirstatDoc( ) final;
	CDirstatDoc& operator=( const CDirstatDoc& in ) = delete;
	CDirstatDoc( const CDirstatDoc& in ) = delete;

	virtual void     DeleteContents        (                                                      ) override final;
	virtual BOOL     OnNewDocument         (                                                      ) override final;

#pragma warning(disable:4373)//Previous versions of MSVC violated the standard.//MSVC bitches nonstop if we DO NOT disable.
	virtual BOOL     OnOpenDocument        ( _In_z_     PCWSTR const pszPathName                      ) override final;
	COLORREF         GetCushionColor       ( _In_z_     PCWSTR const ext                              );
	
	bool Work                              ( ); // return: true if done.
	bool OnWorkFinished                    ( );
	
	_Pre_satisfies_( item.m_child_info._Myptr == nullptr )
	void OpenItem ( _In_     const CTreeListItem& item                                                  );
	void SetSelection ( _In_ const CTreeListItem& item );


	void SetHighlightExtension             ( _In_ const std::wstring       ext                                                   );
	//void ForgetItemTree                    ( );
	void SortTreeList                      ( );	


	_Ret_notnull_ const std::vector<SExtensionRecord>* GetExtensionRecords ( );
	
	bool   IsRootDone    ( ) const;

protected:
	
	//std::vector<std::wstring> buildRootFolders( _In_     const std::vector<std::wstring>& drives,        _In_    std::wstring& folder );
	void buildDriveItems                      ( _In_z_     PCWSTR const pszPathName );
	//void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>& extensionsToSet );

	//void VectorExtensionRecordsToMap          ( );
	void RebuildExtensionData                 ( );
	
	//bool                                      m_showMyComputer;       // True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool                                      m_extensionDataValid;   // If this is false, m_extensionData must be rebuilt
	//C4820: 'CDirstatDoc' : '5' bytes padding added after data member 'CDirstatDoc::m_timeTextWritten' (dirstatdoc.cpp)
	bool                                      m_timeTextWritten;
	//moved std::uint16_t to before m_highlightExtension --- 3 bytes padding left?


public:
	//std::uint16_t                             m_iterations;
	std::wstring                              m_highlightExtension;   // Currently highlighted extension
	std::unique_ptr<CTreeListItem>              m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	CTreeListItem const*                        m_selectedItem;         // Currently selected item, or NULL
	//CItemBranch const*                        m_workingItem;          // Current item we are working on. For progress indication
	//C4820: 'CDirstatDoc' : '6' bytes padding added after data member 'CDirstatDoc::m_iterations' (dirstatdoc.cpp)
	Children_String_Heap_Manager              m_name_pool;

protected:	
	std::vector<SExtensionRecord>             m_extensionRecords;
	std::unordered_map<std::wstring, COLORREF>m_colorMap;

public:
	DOUBLE                                    m_searchTime;
	DOUBLE                                    m_compressed_file_timing;
	LARGE_INTEGER                             m_searchStartTime;
	CMainFrame*                               m_frameptr;
	CDirstatApp*                              m_appptr;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateEditCopy( _In_ CCmdUI* pCmdUI );
	afx_msg void OnEditCopy( );
	
public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
		afx_msg void OnUpdateFileOpen( CCmdUI *pCmdUI );
		afx_msg void OnUpdateFileOpenLight( CCmdUI *pCmdUI );
	};

// BUGBUG: TODO: EVIL GLOBAL! The document is needed in many places.
extern CDirstatDoc *GetDocument();

#else

#endif
