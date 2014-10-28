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
		//COL_SUBDIRS,
		COL_LASTCHANGE,
		COL_ATTRIBUTES
		};
	}


void AddFileExtensionData( _Out_ _Pre_satisfies_( (extensionRecords._Mylast - extensionRecords._Myfirst) == 0 ) std::vector<SExtensionRecord>& extensionRecords, _Inout_ std::map<std::wstring, SExtensionRecord>& extensionMap );

class CItemBranch;//God I hate C++

void    addDIRINFO                    ( _Inout_ std::vector<DIRINFO>& directories, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t );
void    addFILEINFO                   ( _Inout_ std::vector<FILEINFO>& files, _Pre_valid_ _Post_invalid_ FILEINFO& fi, _In_ CFileFindWDS& CFFWDS, _Post_invalid_ FILETIME& t );
void    FindFilesLoop                 ( _Inout_ std::vector<FILEINFO>& files, _Inout_ std::vector<DIRINFO>& directories, const CString& path );

//std::pair<std::vector<std::pair<CItemBranch*, CString>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>
//std::vector<std::pair<CItemBranch*, CString>>
_Pre_satisfies_( !ThisCItem->m_done ) std::pair<std::vector<std::pair<CItemBranch*, CString>>,std::vector<std::pair<CItemBranch*, std::future<std::uint64_t>>>>    readJobNotDoneWork            ( _In_ CItemBranch* const ThisCItem, const CString& path );

//std::vector<std::pair<CItemBranch*, CString>>    findWorkToDo           ( _In_ const CItemBranch* const ThisCItem );

_Pre_satisfies_( ThisCItem->m_type == IT_DIRECTORY ) void    DoSomeWork                    ( _In_ CItemBranch* const ThisCItem, const CString& path );

CString GetFindPattern                ( _In_ const CString& path );

class CItemBranch : public CTreeListItem {
	/*
	  CItemBranch. This is the object, from which the whole tree is built.
	  For every directory, file etc., we find on the Harddisks, there is one CItemBranch.
	  It is derived from CTreeListItem because it _may_ become "visible" and therefore may be inserted in the TreeList view (we don't clone any data).
 
	  It may have been better to design a class hierarchy for this.

	  We collect data of files in FILEINFOs before we create items for them, because we need to know their count before we can decide whether or not we have to create a <Files> item. A <Files> item is only created, when both apply:
		(a) there are more than one files
		(b) there are subdirectories.
	*/
	static_assert( sizeof( unsigned long long ) == sizeof( std::uint64_t ), "Bad parameter size! Check all functions that accept an unsigned long long or a std::uint64_t!" );

	public:
		CItemBranch  ( ITEMTYPE type, _In_ CString name, std::uint64_t size, FILETIME time, DWORD attr, bool done );
		virtual ~CItemBranch (                                                         );

		bool operator<( const CItemBranch& rhs ) const {
			return size_recurse( ) < rhs.size_recurse( );
			}

		//Recursive size
		std::uint64_t size_recurse( ) const {
			std::uint64_t total = m_size;
			const auto childCount = m_children.size( );
			for ( size_t i = 0; i < childCount; ++i ) {
				//using operator[ ] here because performance is critical
				total += m_children[ i ]->size_recurse( );
				}
			//for ( const auto& child : m_children ) {
			//	total += child->size_recurse( );
			//	}
			//total += m_size;
			return total;
			}

		//4,294,967,295  (4294967295 ) is the maximum number of files in an NTFS filesystem according to http://technet.microsoft.com/en-us/library/cc781134(v=ws.10).aspx
		_Ret_range_( 0, 4294967295 ) std::uint32_t files_recurse( ) const {
			std::uint32_t total = 0;
			for ( const auto& child : m_children ) {
				total += child->files_recurse( );
				}
			total += 1;
			return total;
			}

		FILETIME FILETIME_recurse( ) const {
			auto ft = zeroInitFILETIME( );
			if ( Compare_FILETIME_cast( ft, m_lastChange ) ) {
				ft = m_lastChange;
				}
			for ( const auto& child : m_children ) {
				auto ft_child = child->FILETIME_recurse( );
				if ( Compare_FILETIME_cast( ft, ft_child ) ) {
					ft = ft_child;
					}
				}
			return ft;
			}

		// CTreeListItem Interface
		virtual COLORREF         GetItemTextColor    ( ) const override final;
		virtual size_t           GetChildrenCount    ( ) const override final { return m_children.size( ); }

		virtual CString          GetText             ( _In_ _In_range_( 0, 7 ) const INT subitem ) const override final;
		INT CompareSibling                           ( _In_ const CTreeListItem* const tlib, _In_ _In_range_( 0, INT32_MAX ) const INT subitem ) const;
#ifdef ITEM_DRAW_SUBITEM
		//virtual INT              GetImageToCache     ( ) const override;
		virtual bool             DrawSubitem         ( _In_ _In_range_( 0, 7 ) const ENUM_COL subitem, _In_ CDC& pdc, _Inout_ CRect& rc, _In_ const UINT state, _Out_opt_ INT* width, _Inout_ INT* focusLeft ) const;
		        COLORREF         GetPercentageColor  (                                          ) const;
#endif

		void             TmiSetRectangle     ( _In_ const CRect& rc          );
		CRect            TmiGetRectangle     (                               ) const { return BuildCRect( m_rect ); };

		// Branch/Leaf shared functions
		_Must_inspect_result_ _Ret_maybenull_    CItemBranch* GetParent                         (                                                  ) const { return static_cast< CItemBranch* >( m_parent ); };

		INT     GetSortAttributes             (                                                                   ) const;
		DOUBLE  averageNameLength             (                                                                   ) const;
		DOUBLE  GetFraction                   (                                                                   ) const;

		void    stdRecurseCollectExtensionData( _Inout_    std::map<std::wstring, SExtensionRecord>& extensionMap ) const;

		void    SetAttributes                 ( _In_ const DWORD         attr                                );
		
		//DWORD   GetAttributes                 ( ) const;
		CString GetPath                       ( ) const;

		void    UpwardGetPathWithoutBackslash ( CString& pathBuf ) const;

		_Pre_satisfies_(  this->m_type   == IT_FILE      )                                  const std::wstring GetExtension             ( ) const;
		_Pre_satisfies_(  this->m_type   == IT_FILE      )                                        PCWSTR       CStyle_GetExtensionStrPtr( ) const;
		_Pre_satisfies_(  this->m_type   == IT_FILE      )                                        COLORREF     GetGraphColor            ( ) const;
		_Pre_satisfies_(  this->m_type   == IT_FILE      ) _Success_( SUCCEEDED( return ) )       HRESULT      CStyle_GetExtension      (  _Out_writes_z_( strSize ) PWSTR psz_extension, const rsize_t strSize ) const;
		_Post_satisfies_( return->m_type == IT_DIRECTORY )                                        CItemBranch* AddDirectory             ( const CString thisFilePath, const DWORD thisFileAttributes, const CString thisFileName, const FILETIME& thisFileTime );


		CString GetTextCOL_ATTRIBUTES( ) const;
		CString GetTextCOL_LASTCHANGE( ) const;
		CString GetTextCOL_FILES( ) const;
		CString GetTextCOL_ITEMS ( ) const;
		CString GetTextCOL_PERCENTAGE( ) const;

		//INT CompareName              ( _In_ const CItemBranch* const other ) const;
		//INT CompareLastChange        ( _In_ const CItemBranch* const other ) const;

	public:
		//Branch only functions
		//_Pre_satisfies_ isn't actually useful for static analysis, but including anyways
		CItemBranch* AddChild        ( _In_ _Post_satisfies_( child->m_parent == this ) CItemBranch*       const child       );
		void SortAndSetDone          (                                           );

		//these `Get` and `Find` functions should be virtual when refactoring as branch
		_Ret_notnull_ CItemBranch*    GetChildGuaranteedValid ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const;
		_Ret_notnull_ CItemBranch*    TmiGetChild             ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t c ) const { return GetChildGuaranteedValid( c ); }
		_Success_( return != NULL ) _Must_inspect_result_ _Ret_maybenull_ CTreeListItem*  GetTreeListChild        ( _In_ _In_range_( 0, SIZE_T_MAX ) const size_t i ) const { return m_children.at( i ); }

		bool IsAncestorOf                ( _In_ const CItemBranch* const item     ) const;
		
		void AddChildren( );

		//Functions that should be virtually overridden for a Leaf
		//these `Has` and `Is` functions should be virtual when refactoring as branch
		//the compiler is too stupid to de-virtualize these calls, so I'm guarding them with preprocessor #ifdefs, for now - and yes, it does make a big difference!
		
		bool IsTreeDone                      (                                  ) const { return m_done; };
	
		//data members//DON'T FUCK WITH LAYOUT! It's tweaked for good memory layout!
		

	public:
			                                     std::vector<CItemBranch*>      m_children;

		//18446744073709551615 is the maximum theoretical size of an NTFS file according to http://blogs.msdn.com/b/oldnewthing/archive/2007/12/04/6648243.aspx
		_Field_range_( 0, 18446744073709551615 ) std::uint64_t                  m_size;                // OwnSize, if IT_FILE or IT_FREESPACE, or IT_UNKNOWN; SubtreeTotal else.

		                                         ITEMTYPE                       m_type;                // Indicates our type. See ITEMTYPE.
												 attribs                        m_attr;
												 bool                           m_done        : 1;     // Whole Subtree is done.
	private:
		                                         CString                        m_name;                // Display name
	public:
											     FILETIME                       m_lastChange;          // Last modification time OF SUBTREE
		                                         SRECT                          m_rect;                // Finally, this is our coordinates in the Treemap view. (For GraphView)

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
