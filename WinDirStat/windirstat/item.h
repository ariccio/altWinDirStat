// item.h	- Declaration of CItem
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

#include "Treelistcontrol.h"
#include "treemap.h"
#include "dirstatdoc.h"		// CExtensionData
#include "FileFindWDS.h"		// CFileFindWDS


// Columns
enum {
	COL_NAME,
	COL_SUBTREEPERCENTAGE,
	COL_PERCENTAGE,
	COL_SUBTREETOTAL,
	COL_ITEMS,
	COL_FILES,
	COL_SUBDIRS,
	COL_LASTCHANGE,
	COL_ATTRIBUTES
	};

// Item types
enum ITEMTYPE {
	IT_MYCOMPUTER,		// Pseudo Container "My Computer"
	IT_DRIVE,			// C:\, D:\ etc.
	IT_DIRECTORY,		// Folder
	IT_FILE,			// Regular file
	IT_FILESFOLDER,		// Pseudo Folder "<Files>"
	IT_FREESPACE,		// Pseudo File "<Free Space>"
	IT_UNKNOWN,			// Pseudo File "<Unknown>"

	ITF_FLAGS	 = 0xF000,
	ITF_ROOTITEM = 0x8000	// This is an additional flag, not a type.
	};

// Whether an item type is a leaf type
inline bool IsLeaf( const ITEMTYPE t ) { return t == IT_FILE || t == IT_FREESPACE || t == IT_UNKNOWN; }

// Compare FILETIMEs
inline bool operator< ( const FILETIME& t1, const FILETIME& t2 )
{
	ULARGE_INTEGER& u1 = ( ULARGE_INTEGER& ) t1;
	ULARGE_INTEGER& u2 = ( ULARGE_INTEGER& ) t2;

	return ( u1.QuadPart < u2.QuadPart );
}

// Compare FILETIMEs
inline bool operator== ( const FILETIME& t1, const FILETIME& t2 )
{
	return t1.dwLowDateTime == t2.dwLowDateTime && t1.dwHighDateTime == t2.dwHighDateTime;
}

class CItem : public CTreeListItem, public CTreemap::Item {
	/*
	  CItem. This is the object, from which the whole tree is built.
	  For every directory, file etc., we find on the Harddisks, there is one CItem.
	  It is derived from CTreeListItem because it _may_ become "visible" and therefore may be inserted in the TreeList view (we don't clone any data).
 
	  Of course, this class and the base classes are optimized rather for size than for speed.
 
	  The m_type indicates whether we are a file or a folder or a drive etc.
	  It may have been better to design a class hierarchy for this, but I can't help it, rather than browsing to virtual functions I like to flatly see what's going on.
	  But, of course, now we have quite many switch statements in the member functions.
 
	  Naming convention:
	  Methods which recurse down to every child (expensive) are named "RecurseDoSomething".
	  Methods which recurse up to the parent (not so expensive) are named "UpwardDoSomething".

	  We collect data of files in FILEINFOs before we create items for them, because we need to know their count before we can decide whether or not we have to create a <Files> item. (A <Files> item is only created, when
		(a) there are more than one files and
		(b) there are subdirectories.)
	*/

	public:
		struct FILEINFO {
			CString      name;
			std::int64_t length;
			FILETIME     lastWriteTime;
			DWORD        attributes;
			};
		
		CItem  ( ITEMTYPE type, LPCTSTR name, bool dontFollow = false );
		~CItem (                                                      );
		CItem  ( CItem&&  in                                          );

		bool operator<( const CItem& rhs ) const {
			return ( ( GetSize( ) ) < ( rhs.GetSize( ) ) );
			}

		bool operator>( const CItem& rhs ) const {
			return ( ( GetSize( ) ) > ( rhs.GetSize( ) ) );
			}


		// CTreeListItem Interface
		virtual void                                   DrawAdditionalState       ( _In_       CDC*           pdc,        _In_ const CRect& rcLabel                                                                                         ) const;
		virtual bool                                   DrawSubitem               ( _In_ const INT            subitem,    _In_       CDC*   pdc,    _Inout_ CRect& rc, _In_ const UINT state, _Inout_opt_ INT* width, _Inout_ INT* focusLeft) const;
		virtual INT                                    CompareSibling            ( _In_ const CTreeListItem* tlib,       _In_ const INT    subitem                                                                                         ) const;
		virtual INT                                    GetChildrenCount          (                                                                                                                                                         ) const;
		virtual INT                                    GetImageToCache           (                                                                                                                                                         ) const;
		virtual COLORREF                               GetItemTextColor          (                                                                                                                                                         ) const;
		virtual CString                                GetText                   ( _In_ const INT            subitem                                                                                                                       ) const;
		_Must_inspect_result_ virtual CTreeListItem*   GetTreeListChild          ( _In_ const INT            i                                                                                                                             ) const;

		// CTreemap::Item interface
		virtual CRect            TmiGetRectangle                                 (                                                                                                  ) const;
		virtual void             TmiSetRectangle                                 ( _In_ const CRect&         rc                                                                             );

		// CTreemap::Item interface -> header-implemented functions
		_Must_inspect_result_ virtual CTreemap::Item  *TmiGetChild               (      const INT            c )   const { return GetChild        ( c          ); }
		virtual bool                                   TmiIsLeaf                 (                             )   const { return IsLeaf          ( GetType( ) ); }
		virtual COLORREF                               TmiGetGraphColor          (                             )   const { return GetGraphColor   (            ); }
		virtual INT                                    TmiGetChildrenCount       (                             )   const { return GetChildrenCount(            ); }
		virtual LONGLONG                               TmiGetSize                (                             )   const { return GetSize         (            ); }

		// CItem
		static  INT                                    GetSubtreePercentageWidth (                             );
		

		SRECT GetSRECT( ) const;

		bool HasUncPath                  (                                  ) const;
		bool IsAncestorOf                ( _In_ const CItem *item           ) const;
		bool IsDone                      (                                  ) const;
		bool IsRootItem                  (                                  ) const;
		bool IsReadJobDone               (                                  ) const;
		bool StartRefresh                (                                  );
		bool StartRefreshIT_MYCOMPUTER   ( );
		bool StartRefreshIT_FILESFOLDER  ( _In_ bool wasExpanded );
		bool StartRefreshIT_FILE         ( );
		bool StartRefreshIsDeleted       ( _In_ ITEMTYPE typeOf_thisItem    );
		
		LONGLONG GetProgressRange        (                                  ) const;
		LONGLONG GetProgressPos          (                                  ) const;
		LONGLONG GetSize                 (                                  ) const;
		LONGLONG GetReadJobs             (                                  ) const;
		LONGLONG GetFilesCount           (                                  ) const;
		LONGLONG GetSubdirsCount         (                                  ) const;
		LONGLONG GetItemsCount           (                                  ) const;

		_Must_inspect_result_                     bool   StartRefreshIsMountOrJunction    ( _In_ ITEMTYPE typeOf_thisItem );
		_Must_inspect_result_                     static CItem *FindCommonAncestor        ( _In_ const CItem *item1, _In_ const CItem *item2 );
		_Must_inspect_result_                     const  CItem *UpwardGetRoot             (                                                  ) const;
		_Must_inspect_result_                            CItem *GetParent                 (                                                  ) const;
		_Success_(return != NULL) _Must_inspect_result_  CItem *FindDirectoryByPath       ( _In_ const CString& path                         );
		_Success_(return != NULL) _Must_inspect_result_  CItem *FindFreeSpaceItem         (                                                  ) const;
		_Success_(return != NULL) _Must_inspect_result_  CItem *FindUnknownItem           (                                                  ) const;
		_Success_(return != NULL) _Must_inspect_result_  CItem *GetChild                  ( _In_ const INT i                                 ) const;
		_Success_(return != NULL)                        CItem *GetChildGuaranteedValid   ( _In_ const INT i                                 ) const;
		

		INT FindChildIndex                 ( _In_ const CItem *child                                       ) const;
		INT GetSortAttributes              (                                                               ) const;

		void AddChild                      ( _In_       CItem*             child                           );
#ifdef CHILDVEC
		void AddChildToVec                 ( _In_       CItem&             child                           );
#endif
		void AddTicksWorked                ( _In_ const unsigned long long more                            );
		void CreateFreeSpaceItem           (                                                               );
		void CreateUnknownItem             (                                                               );
		
		void DoSomeWork                    ( _In_ const unsigned long long ticks                           );
		//void FindFilesLoop                 ( _In_ const unsigned long long ticks, _In_ unsigned long long start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ CList<FILEINFO, FILEINFO>& files );
		void readJobNotDoneWork            ( _In_ const unsigned long long ticks, _In_ unsigned long long start );
		void FindFilesLoop                 ( _In_ const unsigned long long ticks, _In_ unsigned long long start, _Inout_ LONGLONG& dirCount, _Inout_ LONGLONG& fileCount, _Inout_ std::vector<FILEINFO>& files );
		void RecurseCollectExtensionData   ( _Inout_ CExtensionData       *ed                              );
		void RefreshRecycler               (                                                               );
		void RemoveAllChildren             (                                                               );
		void RemoveAllChildrenFromVec      (                                                               );
		void RemoveChild                   ( _In_ const INT                i                               );
		void RemoveChildFromVec            ( _In_ const size_t             i                               );
		void RemoveFreeSpaceItem           (                                                               );
		void RemoveUnknownItem             (                                                               );
		void SetAttributes                 (      const DWORD              attr                            );
		void SetDone                       (                                                               );
		void SetLastChange                 ( _In_ const FILETIME&          t                               );
		void SetReadJobDone                ( _In_ const bool               done = true                     );
		void SetSize                       ( _In_ const LONGLONG           ownSize                         );
		void StartRefreshHandleDeletedItem ( );
		void StartRefreshRecreateFSandUnknw( );
		void StartRefreshHandleWasExpanded ( );
		void StartRefreshUpwardClearItem   ( _In_ ITEMTYPE typeOf_thisItem );
		void stdRecurseCollectExtensionData( _Inout_ std::map<CString, SExtensionRecord>& stdExtensionData );
		void StillHaveTimeToWork           ( _In_ const unsigned long long ticks, _In_ unsigned long long start );
		void UpdateFreeSpaceItem           (                                                               );
		void UpdateLastChange              (                                                               );
		void UpwardAddSubdirs              ( _In_ const std::int64_t      dirCount                        );
		void UpwardAddFiles                ( _In_ const std::int64_t      fileCount                       );
		void UpwardAddSize                 ( _In_ const std::int64_t      bytes                           );
		void UpwardAddReadJobs             ( _In_ const std::int64_t      count                           );
		void UpwardUpdateLastChange        ( _In_ const FILETIME&          t                               );
		void UpwardRecalcLastChange        (                                                               );
		void UpwardSetUndone               (                                                               );
		void UpwardSetUndoneIT_DRIVE       (                                                               );
		void UpwardParentSetUndone         (                                                               );
		//CArray<CItem *, CItem *>* getChildrenPtr              ( );

		FILETIME                  GetLastChange               ( ) const;
		DWORD                     GetAttributes               ( ) const;
		unsigned long long        GetTicksWorked              ( ) const;

		LONG                      TmiGetRectLeft              ( ) const;
		
		DOUBLE                    GetFraction                 ( ) const;
	
		ITEMTYPE                  GetType                     ( ) const;

		CString                   GetPath                     ( ) const;
		CString                   GetFindPattern              ( ) const;
		CString                   GetFolderPath               ( ) const;
		//CString                   GetReportPath               ( ) const;
		CString                   GetName                     ( ) const;
		CString                   GetExtension                ( ) const;
#ifdef CHILDVEC
		size_t                    GetChildVecCount            ( ) const;
#endif
		void GetTextCOL_SUBTREEPERCENTAGE( _Inout_ CString& s ) const;
		void GetTextCOL_PERCENTAGE       ( _Inout_ CString& s ) const;//COL_ITEMS
		void GetTextCOL_ITEMS            ( _Inout_ CString& s ) const;
		void GetTextCOL_FILES            ( _Inout_ CString& s ) const;
		void GetTextCOL_SUBDIRS          ( _Inout_ CString& s ) const;
		void GetTextCOL_LASTCHANGE       ( _Inout_ CString& s ) const;
		void GetTextCOL_ATTRIBUTES       ( _Inout_ CString& s ) const;

	private:

		static INT __cdecl _compareBySize      ( _In_ const void *p1, _In_ const void *p2 );
	
		LONGLONG GetProgressRangeMyComputer    (                                          ) const;//const return type?
		LONGLONG GetProgressPosMyComputer      (                                          ) const;
		LONGLONG GetProgressRangeDrive         (                                          ) const;
		LONGLONG GetProgressPosDrive           (                                          ) const;
		COLORREF GetGraphColor                 (                                          ) const;
		COLORREF GetPercentageColor            (                                          ) const;
		bool     MustShowReadJobs              (                                          ) const;
		INT      FindFreeSpaceItemIndex        (                                          ) const;
		INT      FindUnknownItemIndex          (                                          ) const;
		CString  UpwardGetPathWithoutBackslash (                                          ) const;
	
		void AddDirectory                      ( _In_ const CFileFindWDS& finder          );
		void AddFile                           ( _In_ const FILEINFO&     fi              );
		//void DrivePacman                       (                                          );
		void DriveVisualUpdateDuringWork       (                                          );
		//void UpwardDrivePacman                 (                                          );

		INT CompareName( _In_ const CItem* other ) const;
		INT CompareSubTreePercentage( _In_ const CItem* other ) const;
		INT CompareLastChange( _In_ const CItem* other ) const;

	private:
	
		static_assert( sizeof( LONGLONG ) == sizeof( std::int64_t ), "y'all ought to check FILEINFO" );

	
		//data members//DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!
		CArray<CItem *, CItem *> m_children;
		ITEMTYPE                 m_type;			    // Indicates our type. See ITEMTYPE.
		CString                  m_name;				// Display name
		
		unsigned char            m_attributes;	        // Packed file attributes of the item
		bool                     m_readJobDone;			// FindFiles() (our own read job) is finished.
		bool					 m_done;				// Whole Subtree is done.

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		//18446744073709551615 is the maximum theoretical size of an NTFS file              according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx

		_Field_range_(0, 18446744073709551615) LONGLONG				m_size;				// OwnSize, if IT_FILE or IT_FREESPACE, or IT_UNKNOWN; SubtreeTotal else.
		_Field_range_(0, 4294967295 )		   std::uint32_t        m_files;			// # Files in subtree
		_Field_range_(0, 4294967295 )		   std::uint32_t        m_subdirs;			// # Folder in subtree
											   FILETIME				m_lastChange;		// Last modification time OF SUBTREE
											   
											   std::uint32_t		m_readJobs;			// # "read jobs" in subtree.
											   std::uint64_t		m_ticksWorked;		// ms time spent on this item.

		static_assert( sizeof( LONGLONG ) == sizeof( std::int64_t ),            "y'all ought to check m_size, m_files, m_subdirs, m_readJobs, m_freeDiskSpace, m_totalDiskSpace!!" );
		static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "y'all ought to check m_ticksWorked" );

		// Our children. When "this" is set to "done", this array is sorted by child size.
		
#ifdef CHILDVEC
		std::vector<CItem>       m_vectorOfChildren;
#endif
		// For GraphView:
		SRECT                    m_rect;				// Finally, this is our coordinates in the Treemap view.
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
