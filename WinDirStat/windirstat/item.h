// item.h	- Declaration of CItemBranch
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

#ifndef ITEM_H
#define ITEM_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"
//#include "Treelistcontrol.h"
//#include "treemap.h"
//#include "dirstatdoc.h"		// CExtensionData
//#include "FileFindWDS.h"		// CFileFindWDS


namespace column {
	enum {
		COL_NAME,
		//COL_SUBTREEPERCENTAGE,
		COL_PERCENTAGE,
		COL_SUBTREETOTAL,
		COL_ITEMS,
		COL_FILES,
		COL_SUBDIRS,
		COL_LASTCHANGE,
		COL_ATTRIBUTES
		};
	}


void AddFileExtensionData( _Inout_ std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap );

class CItemBranch;//God I hate C++

void    addDIRINFO                    ( _Inout_ std::vector<DIRINFO>& directories, _Pre_valid_ _Post_invalid_ DIRINFO& di, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t );
void    addFILEINFO                   ( _Inout_ std::vector<FILEINFO>& files, _Pre_valid_ _Post_invalid_ FILEINFO& fi, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t );
void    FindFilesLoop                 ( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, CString path );
void    readJobNotDoneWork            ( _In_ CItemBranch* ThisCItem, CString path );
std::vector<CItemBranch*>    StillHaveTimeToWork           ( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t start );
void    DoSomeWork                    ( _In_ CItemBranch* ThisCItem, _In_ _In_range_( 0, UINT64_MAX ) const std::uint64_t ticks                           );
CString GetFindPattern                ( _In_ const CString path );

_Ret_range_( 0, UINT64_MAX ) std::uint64_t GetProgressRangeDrive( CString path );

class CItemBranch : public CTreeListItem, public CTreemap::Item {
	/*
	  CItemBranch. This is the object, from which the whole tree is built.
	  For every directory, file etc., we find on the Harddisks, there is one CItemBranch.
	  It is derived from CTreeListItem because it _may_ become "visible" and therefore may be inserted in the TreeList view (we don't clone any data).
 
	  Of course, this class and the base classes are optimized rather for size than for speed.
 
	  It may have been better to design a class hierarchy for this.
 
	  Naming convention:
	  Methods which recurse down to every child (expensive) are named "RecurseDoSomething".
	  Methods which recurse up to the parent (not so expensive) are named "UpwardDoSomething".

	  We collect data of files in FILEINFOs before we create items for them, because we need to know their count before we can decide whether or not we have to create a <Files> item. (A <Files> item is only created, when
		(a) there are more than one files and
		(b) there are subdirectories.)
	*/
	static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "Bad parameter size! Check all functions that accept an unsigned long long or a std::uint64_t!" );

	public:
		CItemBranch  ( ITEMTYPE type, _In_ CString name, std::uint64_t size, FILETIME time, DWORD attr, bool done, bool dontFollow = false );
		~CItemBranch (                                                         );

		bool operator<( const CItemBranch& rhs ) const {
			return m_size < rhs.m_size;
			}

		// CTreeListItem Interface
		
		virtual COLORREF         GetItemTextColor    ( ) const override;
		virtual size_t           GetChildrenCount    ( ) const override { return m_children.size( ); }

		virtual CString          GetText             ( _In_ _In_range_( 0, INT32_MAX ) const INT            subitem                                                 ) const override;
		virtual INT              CompareSibling      ( _In_                            const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const override;
#ifdef ITEM_DRAW_SUBITEM
		virtual INT              GetImageToCache     ( ) const override;
		virtual bool             DrawSubitem         ( _In_ _In_range_( 0, INT32_MAX ) const INT subitem, _In_ CDC* pdc, _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft ) const;
		        COLORREF         GetPercentageColor  (                                          ) const;
#endif
		
		
		// CTreemap::Item interface
		virtual void             TmiSetRectangle     ( _In_ const CRect& rc          )       override;
		virtual CRect            TmiGetRectangle     (                               ) const override { return BuildCRect( m_rect ); };
		virtual COLORREF         TmiGetGraphColor    (                               ) const override { return GetGraphColor   (            ); }
		virtual size_t           TmiGetChildrenCount (                               ) const override { return m_children.size (            ); }
		virtual std::uint64_t    TmiGetSize          (                               ) const override { return m_size; }
		virtual bool             TmiIsLeaf           (                               ) const override { return m_type == IT_FILE; }



		// Branch/Leaf shared functions
		_Must_inspect_result_              CItemBranch* GetParent                         (                                                  ) const { return static_cast< CItemBranch* >( CTreeListItem::GetParent( ) ); };

		INT GetSortAttributes              (                                                               ) const;
		DOUBLE averageNameLength( ) const;
		
		void SetAttributes                 (      const DWORD              attr                            );
		
		void stdRecurseCollectExtensionData( _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap ) const;

		
		void UpwardAddSubdirs              ( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      dirCount                        );
		void UpwardAddFiles                ( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      fileCount                       );
		void UpwardAddSize                 ( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      bytes                           );
		void UpwardAddReadJobs             ( _In_ _In_range_( -INT32_MAX, INT32_MAX ) const std::int64_t      count                           );
		void UpwardUpdateLastChange        ( _In_ const FILETIME&          t                               );
		
		ITEMTYPE                  GetType                       ( ) const { return m_type; };
		DOUBLE                    GetFraction                   ( ) const;
		DWORD                     GetAttributes                 ( ) const;

		const std::wstring        GetExtension                  ( ) const;
		CString                   GetPath                       ( ) const;
		CString                   GetFolderPath                 ( ) const;
		void                      UpwardGetPathWithoutBackslash ( CString& pathBuf ) const;

		_Success_( SUCCEEDED( return ) ) HRESULT CStyle_GetExtension(  _Out_writes_z_( strSize ) PWSTR psz_extension, const rsize_t strSize ) const;

		PCWSTR CStyle_GetExtensionStrPtr( ) const;

		CString GetTextCOL_ATTRIBUTES( ) const;
		CString GetTextCOL_LASTCHANGE( ) const;
		CString GetTextCOL_SUBDIRS( ) const;
		CString GetTextCOL_FILES( ) const;
		CString GetTextCOL_ITEMS ( ) const;
		CString GetTextCOL_PERCENTAGE( ) const;//COL_ITEMS
		
		//static INT __cdecl CItem_compareBySize ( _In_ const void* const p1, _In_ const void* const p2 );
		COLORREF GetGraphColor                 (                                          ) const;
		
		bool     MustShowReadJobs              (                                          ) const;
		
	
		CItemBranch* AddDirectory                      ( const CString thisFilePath, const DWORD thisFileAttributes, const CString thisFileName, const FILETIME& thisFileTime );
		void DriveVisualUpdateDuringWork       (                                          );

		INT CompareName              ( _In_ const CItemBranch* const other ) const;
		INT CompareLastChange        ( _In_ const CItemBranch* const other ) const;

	public:
		//Branch only functions
		CItemBranch* AddChild                      ( _In_       CItemBranch*       child       );
		void SortAndSetDone                (                                           );

		//these `Get` and `Find` functions should be virtual when refactoring as branch
		//_Success_( return != NULL ) _Must_inspect_result_ virtual CItemBranch*    FindDirectoryByPath     ( _In_ const CString& path                         ) const;
		_Success_( return != NULL )                       virtual CItemBranch*    GetChildGuaranteedValid ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i  ) const;
		_Success_( return != NULL ) _Must_inspect_result_ virtual CTreeListItem*  GetTreeListChild        ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t            i ) const override;
		_Success_( return != NULL ) _Must_inspect_result_ virtual CTreemap::Item* TmiGetChild             (      const size_t            c   ) const override { return GetChildGuaranteedValid( c          ); }

		bool IsAncestorOf                ( _In_ const CItemBranch* const item     ) const;
		
		
		//Functions that should be virtually overrided for a Leaf
		//these `Has` and `Is` functions should be virtual when refactoring as branch
		//the compiler is too stupid to de-virtualize these calls, so I'm guarding them with preprocessor #ifdefs, for now - and yes, it does make a big difference!
		
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			bool IsTreeDone                      (                                  ) const { return m_done; };
		
		
		//these should NOT be virtual
		//std::uint64_t      GetProgressRange   (                                  ) const;

		//these `Get` functions should be virtual when refactoring as branch
			
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			std::uint64_t      GetProgressPos     (                                  ) const;
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			std::uint32_t GetReadJobs        (                                  ) const { return m_readJobs; };
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			std::uint32_t GetFilesCount      (                                  ) const { return m_files; };
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			std::uint32_t GetSubdirsCount    (                                  ) const { return m_subdirs; };
#ifdef LEAF_VIRTUAL_FUNCTIONS
		virtual 
#endif
			std::uint32_t GetItemsCount      (                                  ) const { return m_files + m_subdirs; };

		static_assert( sizeof( LONGLONG ) == sizeof( std::int64_t ), "y'all ought to check m_size, m_files, m_subdirs, m_readJobs, m_freeDiskSpace, m_totalDiskSpace, and FILEINFO!!" );
		static_assert( sizeof( unsigned char ) == 1, "y'all ought to check m_attributes" );
	
		//data members//DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!
	public:
		ITEMTYPE                 m_type;                // Indicates our type. See ITEMTYPE.
		bool                     m_readJobDone : 1;     // FindFiles() (our own read job) is finished.
		bool                     m_done        : 1;     // Whole Subtree is done.
	public:
		unsigned char            m_attributes;          // Packed file attributes of the item
	private:
		_Field_range_( 0, 4294967295 )           std::uint32_t                  m_files;			// # Files in subtree

		                                         CString                        m_name;                // Display name
		
		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		_Field_range_( 0, 4294967295 )           std::uint32_t                  m_subdirs;		// # Folder in subtree
		_Field_range_( 0, 4294967295 )           std::uint32_t                  m_readJobs;		// # "read jobs" in subtree.
	public:
		                                         std::vector<CItemBranch*>      m_children;


		//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t                  m_size;			// OwnSize, if IT_FILE or IT_FREESPACE, or IT_UNKNOWN; SubtreeTotal else.
											     FILETIME                       m_lastChange;		// Last modification time OF SUBTREE
	public:
		// For GraphView:
		                                         SRECT                          m_rect;				// Finally, this is our coordinates in the Treemap view.
#ifdef _DEBUG
											   //static int                     LongestName;
#endif
	};


// $Log$
// Revision 1.15  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.14  2004/11/28 19:20:46  assarbad
// - Fixing strange behavior of logical operators by rearranging code in
//   CItem::SetAttributes() and CItem::GetAttributes()
//
// Revision 1.13  2004/11/28 15:38:42  assarbad
// - Possible sorting implementation (using bit-order in m_attributes)
//
// Revision 1.12  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.11  2004/11/25 23:07:24  assarbad
// - Derived CFileFindWDS from CFileFind to correct a problem of the ANSI version
//
// Revision 1.10  2004/11/15 19:50:39  assarbad
// - Minor corrections
//
// Revision 1.9  2004/11/12 00:47:42  assarbad
// - Fixed the code for coloring of compressed/encrypted items. Now the coloring spans the full row!
//
// Revision 1.8  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.7  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
