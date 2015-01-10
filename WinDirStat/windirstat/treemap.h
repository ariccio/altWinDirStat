// treemap.h	- Declaration of CColorSpace, CTreemap and CTreemapPreview
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

#ifndef TREEMAP_H_INCLUDED
#define TREEMAP_H_INCLUDED

#include "stdafx.h"

class CItemBranch;
class CTreeListItem;


//#define DBL_MAX_100 1.79769e+306
//struct setPixStruct {
//	setPixStruct( int in_x, int in_y, COLORREF in_color ) : ix( std::move( in_x ) ), iy( std::move( in_y ) ), color( std::move( in_color ) ) { }
//	int ix;
//	int iy;
//	COLORREF color;
//	static_assert( sizeof( std::int_fast32_t ) == sizeof( DWORD ), "whoops! need a different color size!" );
//	};
//
//#ifdef GRAPH_LAYOUT_DEBUG
//struct pixBitsSet {
//	
//	};
//#endif

// CTreemap. Can create a treemap. Knows 2 squarification methods: KDirStat-like, SequoiaView-like.
class CTreemap {
public:


public:
	CTreemap( );
	void UpdateCushionShading      ( _In_ const bool               newVal                                   );
	void SetOptions                ( _In_ const Treemap_Options&           options                                  );
	void RecurseCheckTree          ( _In_ const CItemBranch* const item                                     ) const;
	void validateRectangle         ( _In_ const CItemBranch* const child, _In_ const CRect&             rc  ) const;
	void compensateForGrid         ( _Inout_    CRect&             rc,    _In_       CDC&               pdc ) const;

	void DrawTreemap               ( _In_ CDC& offscreen_buffer, _Inout_    CRect& rc, _In_ const CItemBranch* const root,  _In_opt_ const Treemap_Options* const options = NULL );
	void DrawTreemapDoubleBuffered ( _In_ CDC& pdc, _In_ const CRect& rc, _In_       CItemBranch* const root,  _In_opt_ const Treemap_Options* const options = NULL );
	void DrawColorPreview          ( _In_ CDC& pdc, _In_ const CRect& rc, _In_ const COLORREF           color, _In_     const Treemap_Options* const options = NULL );

	_Success_( return != NULL ) _Ret_maybenull_ _Must_inspect_result_ CItemBranch* FindItemByPoint( _In_ const CItemBranch* const root, _In_ const CPoint point ) const;

	

protected:

	void SetPixels        ( _In_ CDC& offscreen_buffer, _In_reads_( maxIndex ) _Pre_readable_size_( maxIndex ) const COLORREF* const pixles, _In_ const int&   yStart, _In_ const int& xStart, _In_ const int& yEnd, _In_ const int& xEnd,   _In_ const int rcWidth, _In_ const size_t offset, const size_t maxIndex, _In_ const int rcHeight ) const;

	void RecurseDrawGraph ( _In_ CDC& offscreen_buffer, _In_ const CItemBranch* const     item,   _In_ const CRect& rc,     _In_ const bool asroot, _In_ const DOUBLE ( &psurface )[ 4 ], _In_ const DOUBLE h ) const;


	void DrawCushion      ( _In_ CDC& offscreen_buffer, _In_ const CRect&              rc,        _In_ const DOUBLE ( &surface )[ 4 ], _In_                    const COLORREF col,       _In_ _In_range_( 0, 1 ) const DOUBLE  brightness ) const;
	void DrawSolidRect    ( _In_ CDC& pdc, _In_ const CRect&              rc,        _In_ const COLORREF        col,            _In_ _In_range_( 0, 1 ) const DOUBLE   brightness ) const;
	void DrawChildren     ( _In_ CDC& pdc, _In_ const CItemBranch*  const parent,    _In_ const DOUBLE ( &surface )[ 4 ], _In_                    const DOUBLE   h          ) const;
	


	//KDS -> KDirStat
	DOUBLE KDS_CalcNextRow ( _In_ const CItemBranch* const parent, _In_ _In_range_( 0, INT_MAX ) const size_t nextChild,  _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Out_ INT_PTR& childrenUsed, _Inout_ std::vector<DOUBLE>& childWidth, const std::uint64_t parentSize ) const;
		
	bool KDS_PlaceChildren ( _In_ const CItemBranch* const parent, _Inout_    std::vector<double>& childWidth, _Inout_ std::vector<double>& rows,            _Inout_    std::vector<INT_PTR>& childrenPerRow ) const;
	void KDS_DrawChildren  ( _In_ CDC&  pdc,                       _In_ const CItemBranch* const parent,          _In_ const DOUBLE       ( &surface )[ 4 ], _In_ const DOUBLE h ) const;
	
	void KDS_DrawSingleRow( _In_ const std::vector<INT_PTR>& childrenPerRow, _In_ _In_range_( 0, SIZE_T_MAX ) const size_t& row, _In_ const std::vector<CTreeListItem*>& parent_vector_of_children, _Inout_ _In_range_( 0, SIZE_T_MAX ) size_t& c, _In_ const std::vector<double>& childWidth, _In_ const int& width, _In_ const bool& horizontalRows, _In_ const int& bottom, _In_ const double& top, _In_ const CRect& rc, _In_ CDC& pdc, _In_ const DOUBLE( &surface )[ 4 ], _In_ const DOUBLE& h, _In_ const CItemBranch* const parent ) const;


	//SQV -> SequoiaView
	void SQV_DrawChildren  ( _In_ CDC&  pdc,                       _In_ const CItemBranch* const parent, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const;
	void RenderLeaf        ( _In_ CDC&  offscreen_buffer,          _In_ const CItemBranch* const item,   _In_ const DOUBLE ( &surface )[ 4 ]                   ) const;
	void RenderRectangle   ( _In_ CDC&  offscreen_buffer,          _In_ const CRect&             rc,     _In_ const DOUBLE ( &surface )[ 4 ], _In_ DWORD color ) const;

	void SQV_put_children_into_their_places( _In_ const size_t& rowBegin, _In_ const size_t& rowEnd, _In_ const std::vector<CTreeListItem*>& parent_vector_of_children, _Inout_ std::map<std::uint64_t, std::uint64_t>& sizes, _In_ const std::uint64_t& sumOfSizesOfChildrenInRow, _In_ const int& heightOfNewRow, _In_ const bool& horizontal, _In_ const CRect& remaining, _In_ CDC& pdc, _In_ const DOUBLE( &surface )[ 4 ], _In_ const DOUBLE& scaleFactor, _In_ const DOUBLE h, _In_ const int& widthOfRow ) const;

	void AddRidge( _In_ const CRect& rc, _Inout_ DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const;
	
	bool IsCushionShading( ) const;
public:
	
	bool IsCushionShading_current : 1;

	Treemap_Options   m_options;	// Current options

protected:

	DOUBLE    m_Lx;			// Derived parameters
	DOUBLE    m_Ly;
	DOUBLE    m_Lz;
public:

#ifdef GRAPH_LAYOUT_DEBUG
	void debugSetPixel( CDC& pdc, int a, int b, COLORREF c ) const;
	mutable std::unique_ptr<std::vector<std::vector<bool>>> bitSetMask;
	mutable int numCalls;
#endif

	};


#else
#error 555
#endif

// $Log$
// Revision 1.6  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.