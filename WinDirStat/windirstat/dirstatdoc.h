// dirstatdoc.h		- Declaration of the CDirstatDoc class
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"
//#include "globalhelpers.h"


#ifndef WDS_DIRSTATDOC_H
#define WDS_DIRSTATDOC_H

WDS_FILE_INCLUDE_MESSAGE

#include "datastructures.h"
#include "ChildrenHeapManager.h"
#include "macros_that_scare_small_children.h"


class CTreeListItem;
class CMainFrame;
class CDirstatApp;

// The "Document" class. Owner of the root item and various other data (see data members).
//CDocument decls are in:
//	C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afxwin.h:5764
//CDocument source is in:
//	C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\src\mfc\doccore.cpp
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
	//DECLARE_DYNCREATE(CDirstatDoc)

public: 
	static const CRuntimeClass classCDirstatDoc;
	virtual CRuntimeClass* GetRuntimeClass() const;

	static CObject* PASCAL CreateObject();

private:
	DISALLOW_COPY_AND_ASSIGN(CDirstatDoc);

public:

	virtual ~CDirstatDoc( ) final;


	virtual void     DeleteContents        (                                                      ) override final;
	virtual BOOL     OnNewDocument         (                                                      ) override final;

#pragma warning(disable:4373)//Previous versions of MSVC violated the standard.//MSVC bitches nonstop if we DO NOT disable.
	virtual BOOL     OnOpenDocument        ( _In_z_     PCWSTR const pszPathName                      ) override final;
	COLORREF         GetCushionColor       ( _In_z_     PCWSTR const ext                              );
	
	bool Work                              ( ); // return: true if done.
	bool OnWorkFinished                    ( );
	
	_Pre_satisfies_( item.m_child_info.m_child_info_ptr == nullptr )
	void OpenItem ( _In_     const CTreeListItem& item                                                  );

	void SetSelection ( _In_ const CTreeListItem& item );


	void SetHighlightExtension             ( _In_ const std::wstring       ext                                                   );
	void SortTreeList                      ( );	


	//We need a getter (NOT public data member) because we may need to do work before accessing.
	_Ret_notnull_ const std::vector<SExtensionRecord>* GetExtensionRecords ( );
	
	bool   IsRootDone    ( ) const noexcept;

protected:
	
	void buildDriveItems                      ( _In_z_     PCWSTR const pszPathName );
	void RebuildExtensionData                 ( );
	
	bool                                      m_extensionDataValid        = false;   // If this is false, m_extensionData must be rebuilt
	//C4820: 'CDirstatDoc' : '5' bytes padding added after data member 'CDirstatDoc::m_timeTextWritten' (dirstatdoc.cpp)
	bool                                      m_timeTextWritten           = false;
	//moved std::uint16_t to before m_highlightExtension --- 3 bytes padding left?


public:
	std::wstring                                 m_highlightExtension;   // Currently highlighted extension
	std::unique_ptr<CTreeListItem>               m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	CTreeListItem const*                         m_selectedItem           = nullptr;         // Currently selected item, or NULL
	//C4820: 'CDirstatDoc' : '6' bytes padding added after data member 'CDirstatDoc::m_iterations' (dirstatdoc.cpp)
	std::unique_ptr<Children_String_Heap_Manager>m_name_pool;

protected:	
	std::vector<SExtensionRecord>                m_extensionRecords; //needs to be protected (NOT public) because we may need to do work before accessing.
	std::unordered_map<std::wstring, COLORREF>   m_colorMap;

public:
	DOUBLE                                       m_searchTime             = DBL_MAX;
	DOUBLE                                       m_compressed_file_timing = -1;
	LARGE_INTEGER                                m_searchStartTime;
	CMainFrame*                                  m_frameptr;
	CDirstatApp*                                 m_appptr;
	/*
	DECLARE_MESSAGE_MAP()
	--becomes--
protected: \
	static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
	virtual const AFX_MSGMAP* GetMessageMap() const; \

	*/
	//DECLARE_MESSAGE_MAP()
protected:
	static const AFX_MSGMAP* PASCAL GetThisMessageMap();
	virtual const AFX_MSGMAP* GetMessageMap() const;

public:
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
extern CDirstatDoc *GetDocument() noexcept;

#endif
