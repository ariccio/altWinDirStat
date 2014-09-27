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
//typedef CMap<CString, LPCTSTR, SExtensionRecord, SExtensionRecord&> CExtensionData;

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
	HINT_HIDEUNKNOWN,
	HINT_UNHIDEUNKNOWN,
	//HINT_HIDEFREESPACE,
	//HINT_UNHIDEFREESPACE
	};


class CItemBranchWorker {
	public:
	CItemBranchWorker( CItemBranch* initialItem ) : startingTimeTicks( GetTickCount64( ) ) {
		notDoneItems.emplace_back( initialItem );
		}

	bool DoSomeWork( _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks ) {
		if ( notDoneItems.empty( ) ) {
			return true;
			}
		for ( const auto& item : notDoneItems ) {
			TRACE( _T( "notDoneItem: %s\r\n" ), item->UpwardGetPathWithoutBackslash( ) );
			}
		auto ThisCItem = notDoneItems.at( notDoneItems.size( ) -1 );
		if ( ThisCItem->IsDone( ) ) {
			notDoneItems.pop_back( );
			return false;
			}

		auto typeOfThisItem = ThisCItem->m_type;
		if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY ) {
			if ( !ThisCItem->IsReadJobDone( ) ) {
				worker_readJobNotDoneWork( ThisCItem, ticks, startingTimeTicks );
				}
			if ( ( GetTickCount64( ) - startingTimeTicks ) > ticks ) {
				return false;
				}
			}
		if ( typeOfThisItem == IT_DRIVE || typeOfThisItem == IT_DIRECTORY || typeOfThisItem == IT_MYCOMPUTER ) {
			ASSERT( ThisCItem->IsReadJobDone( ) );
			if ( ThisCItem->GetChildrenCount( ) == 0 ) {
				ASSERT( !ThisCItem->IsDone( ) );
				ThisCItem->SortAndSetDone( );
				return false;
				}
			auto startChildren = GetTickCount64( );
			worker_StillHaveTimeToWork( ThisCItem, ticks, startingTimeTicks );
			auto TicksWorked = GetTickCount64( ) - startChildren;
			ThisCItem->AddTicksWorked( TicksWorked );
			return false;
			}
		else {
			ASSERT( !ThisCItem->IsDone( ) );
			ThisCItem->SortAndSetDone( );
			return true;
			}
		return false;
		}

	void worker_StillHaveTimeToWork( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t start ) {
		bool timeExpired = true;
		while ( ( GetTickCount64( ) - start < ticks ) && (!ThisCItem->IsDone( ) ) ) {
			unsigned long long minticks = UINT_MAX;
			CItemBranch* minchild = NULL;
		
		
			//Interestingly, the old-style, non-ranged loop is faster here ( in debug mode )
			auto sizeOf_m_children = ThisCItem->m_children.size( );
			for ( size_t i = 0; i < sizeOf_m_children; ++i ) {
				if ( !ThisCItem->m_children.at( i )->IsDone( ) ) {
					//minticks = ThisCItem->m_children[ i ]->GetTicksWorked( );
					notDoneItems.emplace_back( ThisCItem->m_children[ i ] );
					}
				}

			if ( minchild == NULL ) {
				//Either no children ( a file ) or all children are done!
				ThisCItem->SortAndSetDone( );
				ASSERT( ( ThisCItem->m_children.size( ) == 0 ) || ( ThisCItem->IsDone( ) ) );
				timeExpired = false;
				break;
				}
			auto tickssofar = GetTickCount64( ) - start;
			if ( ticks > tickssofar ) {
				if ( !minchild->IsDone( ) ) {
					//notDoneItems.emplace_back( minChild );
					//DoSomeWork( ticks - tickssofar );
					}
				}
			}
		if ( timeExpired ) {
			ThisCItem->DriveVisualUpdateDuringWork( );
			}
		}

	void worker_FindFilesLoop( _In_ CItemBranch* ThisCItem, _In_ const std::uint64_t ticks, _In_ const std::uint64_t start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files ) {
		ASSERT( ThisCItem->GetType( ) != IT_FILE );
		CFileFindWDS finder;
		BOOL b = finder.FindFile( GetFindPattern( ThisCItem->GetPath( ) ) );
		bool didUpdateHack = false;
		FILEINFO fi;
		FILETIME t;
		while ( b ) {
			b = finder.FindNextFile( );
			if ( finder.IsDots( ) ) {
				continue;//Skip the rest of the block. No point in operating on ourselves!
				}
			if ( finder.IsDirectory( ) ) {
				dirCount++;
				finder.GetLastWriteTime( &t );
				auto newDir = ThisCItem->AddDirectory( finder.GetFilePath( ), finder.GetAttributes( ), finder.altGetFileName( ), t );
				notDoneItems.emplace_back( newDir );
				}
			else {
				fileCount++;
				fi.attributes = 0;
				fi.lastWriteTime.dwHighDateTime = 0;
				fi.lastWriteTime.dwLowDateTime = 0;
				fi.length = 0;
				fi.name.Empty( );
				PWSTR namePtr = finder.altGetFileName( );
				if ( namePtr != NULL ) {
					fi.name = namePtr;
					}
				else {
					fi.name = finder.GetFileName( );
					//GetString returns a const C-style string pointer, and the compiler bitches if we try to assign it to a PWSTR
					namePtr = const_cast<PWSTR>( fi.name.GetString( ) );
					}
				fi.attributes = finder.GetAttributes( );
				if ( fi.attributes & FILE_ATTRIBUTE_COMPRESSED ) {//ONLY do GetCompressed Length if file is actually compressed
					//fi.length = finder.GetCompressedLength( namePtr );
					fi.length = finder.GetCompressedLength( );
					}
				else {

	#ifdef _DEBUG
					if ( !( finder.GetLength( ) == finder.GetCompressedLength( ) ) ) {
						static_assert( sizeof( unsigned long long ) == 8, "bad format specifiers!" );
						TRACE( _T( "GetLength: %llu != GetCompressedLength: %llu !!! Path: %s\r\n" ), finder.GetLength( ), finder.GetCompressedLength( ), finder.GetFilePath( ) );
						}
	#endif
					fi.length = finder.GetLength( );//temp
					}
				finder.GetLastWriteTime( &fi.lastWriteTime ); // (We don't use GetLastWriteTime(CTime&) here, because, if the file has an invalid timestamp, that function would ASSERT and throw an Exception.)
				files.emplace_back( std::move( fi ) );
				}
			if ( ( ( fileCount + dirCount ) > 0 ) && ( ( ( fileCount + dirCount ) % 1000 ) == 0 ) /*&& ( !didUpdateHack ) */ ) {
				ThisCItem->DriveVisualUpdateDuringWork( );
				//didUpdateHack = true;
				}
			}	

		}


	void worker_readJobNotDoneWork( _In_ CItemBranch* ThisCItem, _In_ const std::uint64_t ticks, _In_ const std::uint64_t start ) {
		LONGLONG dirCount  = 0;
		LONGLONG fileCount = 0;
		std::vector<FILEINFO> vecFiles;
		CItemBranch* filesFolder = NULL;

		vecFiles.reserve( 50 );//pseudo-arbitrary number

		worker_FindFilesLoop( ThisCItem, ticks, start, dirCount, fileCount, vecFiles );

		if ( dirCount > 0 && fileCount > 1 ) {
			filesFolder = new CItemBranch { IT_FILESFOLDER, _T( "<Files>" ), 0, zeroInitFILETIME( ), 0, false };
			filesFolder->SetReadJobDone( false );
			ThisCItem->AddChild( filesFolder );
			}
		else if ( fileCount > 0 ) {
			filesFolder = ThisCItem;
			}
		if ( filesFolder != NULL ) {
			for ( const auto& aFile : vecFiles ) {
				filesFolder->AddChild( new CItemBranch { IT_FILE, aFile.name, aFile.length, aFile.lastWriteTime, aFile.attributes, true } );
				}
			filesFolder->UpwardAddFiles( fileCount );
			if ( dirCount > 0 && fileCount > 1 ) {
				filesFolder->SortAndSetDone( );
				}
			}
		ThisCItem->UpwardAddSubdirs( dirCount );
		ThisCItem->UpwardAddReadJobs( -1 );
		ThisCItem->SetReadJobDone( true );
		auto TicksWorked = GetTickCount64( ) - start;
		ThisCItem->AddTicksWorked( TicksWorked );
		}

	std::vector<CItemBranch*> notDoneItems;
	std::uint64_t startingTimeTicks;
	};


// CDirstatDoc. The "Document" class. 
// Owner of the root item and various other data (see data members).
class CDirstatDoc : public CDocument {
protected:
	CDirstatDoc();	// Created by MFC only
	DECLARE_DYNCREATE(CDirstatDoc)

public:

	virtual ~CDirstatDoc();
	
	virtual void     DeleteContents        (                                                                                 ) override;
	virtual void     Serialize             ( _In_ const CArchive& ar                                                         );
	virtual BOOL     OnNewDocument         (                                                                                 ) override;
	virtual BOOL     OnOpenDocument        ( _In_z_     LPCTSTR   lpszPathName                                               ) override;
	
	COLORREF         GetCushionColor       ( _In_z_ PCWSTR ext   );
	COLORREF         GetZoomColor          ( ) const;
	
	bool Work                              ( _In_ _In_range_( 0, UINT64_MAX ) std::uint64_t ticks ); // return: true if done.
	bool IsDrive                           ( _In_                       const CString       spec  ) const;
	bool OnWorkFinished                    ( );
	
	void OpenItem                          ( _In_   const CItemBranch* item                                                  );
	void SetHighlightExtension             ( _In_z_ const LPCTSTR      ext                                                   );
	void SetSelection                      ( _In_   const CItemBranch* item,  _In_ const bool keepReselectChildStack = false );
	void SetTitlePrefix                    ( _In_   const CString      prefix                                                );
	void ForgetItemTree                    ( );
	void SortTreeList                      ( );	

	CString GetHighlightExtension          ( ) const;


	_Must_inspect_result_ std::vector<SExtensionRecord>* GetExtensionRecords ( );
	_Must_inspect_result_ CItemBranch*                   GetSelection        ( ) const;
	_Must_inspect_result_ CItemBranch*                   GetZoomItem         ( ) const;

	_Must_inspect_result_             CItemBranch*  GetRootItem ( );
	_Success_( return != UINT64_MAX ) std::uint64_t GetRootSize ( );
	
	bool   IsRootDone    ( );
	bool   IsZoomed      ( );
	_Requires_lock_held_( m_rootItemCriticalSection ) DOUBLE GetNameLength ( ) const;

	
protected:
	void buildDriveItems                      ( _In_           CStringArray& rootFolders );
	void buildRootFolders                     ( _In_           CStringArray& drives,                _In_    CString& folder,    _Inout_ CStringArray& rootFolders );
	void SetWorkingItem                       ( _In_opt_       CItemBranch*                   item, _In_    bool     hideTiming                                   );
	
	void PushReselectChild                    ( _In_           CItemBranch*                   item            );
	void stdSetExtensionColors                ( _Inout_        std::vector<SExtensionRecord>& extensionsToSet );
	void SetWorkingItem                       ( _In_opt_       CItemBranch*                   item            );
	void SetZoomItem                          ( _In_           CItemBranch*                   item            );
	
	void VectorExtensionRecordsToMap          ( );
	void RebuildExtensionData                 ( );
	void ClearReselectChildStack              ( );

	bool stdCompareExtensions                 ( _In_ const CString* stringOne, _In_ const CString* stringTwo                           );	
	bool DeletePhysicalItem                   ( _In_       CItemBranch* item,  _In_ const bool     toTrashBin                          );
	bool DirectoryListHasFocus                (                                                                                        ) const;
	bool IsReselectChildAvailable             (                                                                                        ) const;

	_Must_inspect_result_ CItemBranch *PopReselectChild                   (                                    );	
	
	
	
	bool    m_showMyComputer     : 1;   // True, if the user selected more than one drive for scanning. In this case, we need a root pseudo item ("My Computer").
	bool    m_extensionDataValid : 1;   // If this is false, m_extensionData must be rebuilt
	bool    m_timeTextWritten    : 1;

	CString                                   m_highlightExtension;   // Currently highlighted extension

	CRITICAL_SECTION                          m_rootItemCriticalSection;
	_Guarded_by_( m_rootItemCriticalSection ) std::unique_ptr<CItemBranch>              m_rootItem;             // The very root item. CDirstatDoc owns this item and all of it's children - the whole tree.
	
	CItemBranch*                              m_selectedItem;         // Currently selected item, or NULL
	CItemBranch*                              m_zoomItem;             // Current "zoom root"
	CItemBranch*                              m_workingItem;          // Current item we are working on. For progress indication

	CList<CItemBranch *, CItemBranch *>       m_reselectChildStack;   // Stack for the "Re-select Child"-Feature

	std::uint64_t                             m_freeDiskSpace;
	std::uint64_t                             m_totalDiskSpace;
	std::vector<SExtensionRecord>             m_extensionRecords;
	std::map<CString, COLORREF>               m_colorMap;
	std::unique_ptr<CItemBranchWorker>       m_theWorker;

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
