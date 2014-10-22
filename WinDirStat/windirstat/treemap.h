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

#define DBL_MAX_100 1.79769e+306
struct setPixStruct {
	setPixStruct( std::int_fast32_t in_x, std::int_fast32_t in_y, std::int_fast32_t in_color ) : ix( std::move( in_x ) ), iy( std::move( in_y ) ), color( std::move( in_color ) ) { }
	std::int_fast32_t ix;
	std::int_fast32_t iy;
	std::int_fast32_t color;
	static_assert( sizeof( std::int_fast32_t ) == sizeof( DWORD ), "whoops! need a different color size!" );
	};

#ifdef GRAPH_LAYOUT_DEBUG
struct pixBitsSet {
	
	};
#endif

class CColorSpace {
	public:	
	// Returns the brightness of color. Brightness is a value between 0 and 1.0.
	_Ret_range_( 0, 1 ) static DOUBLE GetColorBrightness( _In_ const COLORREF color ) {
		return ( GetRValue( color ) + GetGValue( color ) + GetBValue( color ) ) / 255.0 / 3.0;
		}

	// Gives a color a defined brightness.
	static COLORREF MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_(0, 1) const DOUBLE brightness );

	};


class CItemBranch;

// CTreemap. Can create a treemap. Knows 2 squarification methods: KDirStat-like, SequoiaView-like.
class CTreemap {
public:
	enum STYLE : std::uint8_t {
		KDirStatStyle,		// Children are layed out in rows. Similar to the style used by KDirStat.
		SequoiaViewStyle	// The 'classical' squarification as described in `squarified treemaps` (stm.pdf)
		};

	// Collection of all treemap options.
	struct Options {
		STYLE    style;                                             // Squarification method
		bool     grid;                                              // Whether or not to draw grid lines
		COLORREF gridColor;                                         // Color of grid lines
		_Field_range_(  0, 1          ) DOUBLE brightness;          // (default = 0.84)
	    _Field_range_(  0, UINT64_MAX ) DOUBLE height;              // (default = 0.40)  Factor "H (really range should be 0...std::numeric_limits<double>::max/100"
		_Field_range_(  0, 1          ) DOUBLE scaleFactor;         // (default = 0.90)  Factor "F"
		_Field_range_(  0, 1          ) DOUBLE ambientLight;        // (default = 0.15)  Factor "Ia"
		_Field_range_( -4, 4          ) DOUBLE lightSourceX;        // (default = -1.0), negative = left
		_Field_range_( -4, 4          ) DOUBLE lightSourceY;        // (default = -1.0), negative = top

		_Ret_range_( 0, 100 ) INT    GetBrightnessPercent  ( ) const { return RoundDouble( brightness   * 100 );                               }
		_Ret_range_( 0, 100 ) INT    GetHeightPercent      ( ) const { return RoundDouble( height       * 100 );                               }
		_Ret_range_( 0, 100 ) INT    GetScaleFactorPercent ( ) const { return RoundDouble( scaleFactor  * 100 );                               }
		_Ret_range_( 0, 100 ) INT    GetAmbientLightPercent( ) const { return RoundDouble( ambientLight * 100 );                               }
		_Ret_range_( 0, 100 ) INT    GetLightSourceXPercent( ) const { return RoundDouble( lightSourceX * 100 );                               }
		_Ret_range_( 0, 100 ) INT    GetLightSourceYPercent( ) const { return RoundDouble( lightSourceY * 100 );                               }
		                      CPoint GetLightSourcePoint   ( ) const { return CPoint { GetLightSourceXPercent( ), GetLightSourceYPercent( ) }; }

		_Ret_range_( 0, 100 ) INT    RoundDouble ( const DOUBLE d ) const { return signum( d ) * INT( abs( d ) + 0.5 ); }

		void SetBrightnessPercent  ( const INT    n   ) { brightness   = n / 100.0; }
		void SetHeightPercent      ( const INT    n   ) { height       = n / 100.0; }
		void SetScaleFactorPercent ( const INT    n   ) { scaleFactor  = n / 100.0; }
		void SetAmbientLightPercent( const INT    n   ) { ambientLight = n / 100.0; }
		void SetLightSourceXPercent( const INT    n   ) { lightSourceX = n / 100.0; }
		void SetLightSourceYPercent( const INT    n   ) { lightSourceY = n / 100.0; }
		void SetLightSourcePoint   ( const CPoint pt  ) {
			 SetLightSourceXPercent( pt.x );
			 SetLightSourceYPercent( pt.y );
			}
		};

public:
	void UpdateCushionShading( _In_ const bool newVal );
	CTreemap( );

	void SetOptions       ( _In_ const Options& options                      );
	void RecurseCheckTree ( _In_ const CItemBranch*    const item                         ) const;
	void validateRectangle( _In_ const CItemBranch*    const child, _In_ const CRect& rc  ) const;
	void compensateForGrid( _Inout_    CRect&   rc,    _In_       CDC&   pdc ) const;

	void DrawTreemap               ( _In_ CDC& pdc, _In_       CRect& rc, _In_       CItemBranch* const root,  _In_opt_ const Options* const options = NULL );
	void DrawTreemapDoubleBuffered ( _In_ CDC& pdc, _In_ const CRect& rc, _In_       CItemBranch* const root,  _In_opt_ const Options* const options = NULL );
	void DrawColorPreview          ( _In_ CDC& pdc, _In_ const CRect& rc, _In_ const COLORREF color, _In_     const Options* const options = NULL );

	_Success_( return != NULL ) _Ret_maybenull_ _Must_inspect_result_ CItemBranch* FindItemByPoint( _In_ const CItemBranch* const root, _In_ const CPoint point ) const;

	

protected:
	void RecurseDrawGraph ( _In_ CDC& pdc, _In_       CItemBranch*  const item,   _In_                 const CRect&   rc,      _In_                    const bool     asroot, _In_ _In_reads_( 4 )    const DOUBLE* const psurface, _In_ const DOUBLE h ) const;
	void DrawCushion      ( _In_ CDC& pdc, _In_ const CRect& rc,     _In_ _In_reads_( 4 ) const DOUBLE* const  surface, _In_                    const COLORREF col,    _In_ _In_range_( 0, 1 ) const DOUBLE  brightness                    ) const;

	void DrawSolidRect    ( _In_ CDC& pdc, _In_ const CRect& rc,     _In_                 const COLORREF col,     _In_ _In_range_( 0, 1 ) const DOUBLE   brightness ) const;
	void DrawChildren     ( _In_ CDC& pdc, _In_       CItemBranch*  const parent, _In_ _In_reads_( 4 ) const DOUBLE*  const surface, _In_                    const DOUBLE   h          ) const;
	

	DOUBLE KDirStat_CalcutateNextRow ( _In_ const CItemBranch* const parent, _In_ _In_range_( 0, INT_MAX ) const size_t nextChild,  _In_ _In_range_( 0, 32767 ) const DOUBLE width,                 _Out_ INT_PTR& childrenUsed, _Inout_ CArray<DOUBLE, DOUBLE>& childWidth ) const;
		
	bool KDirStat_ArrangeChildren    ( _In_ const CItemBranch* const parent, _Inout_       CArray<double, double>&      childWidth, _Inout_                           CArray<double, double>& rows, _Inout_    CArray<INT_PTR, INT_PTR>& childrenPerRow ) const;
	void KDirStat_DrawChildren       ( _In_       CDC&  pdc,    _In_    const CItemBranch*                 const parent,    _In_ _In_reads_( 4 )         const DOUBLE* const surface,              _In_ const DOUBLE h ) const;
	void SequoiaView_DrawChildren    ( _In_       CDC&  pdc,    _In_    const CItemBranch*                 const parent,    _In_ _In_reads_( 4 )         const DOUBLE* const surface,              _In_ const DOUBLE h ) const;

	bool IsCushionShading( ) const;

	void RenderLeaf      ( _In_ CDC& pdc, _In_       CItemBranch*  const item, _In_ _In_reads_( 4 ) const DOUBLE* const surface                   ) const;
	void RenderRectangle ( _In_ CDC& pdc, _In_ const CRect& rc,   _In_ _In_reads_( 4 ) const DOUBLE* const surface, _In_ DWORD color ) const;

	void AddRidge( _In_ const CRect& rc, _Inout_ _Inout_updates_( 4 ) DOUBLE* const surface, _In_ const DOUBLE h ) const;
	
public:
	
	bool IsCushionShading_current : 1;
	static const Options  _defaultOptions;				// Good values. Default for WinDirStat 1.0.2

	Options   m_options;	// Current options

protected:
	

	
	DOUBLE    m_Lx;			// Derived parameters
	DOUBLE    m_Ly;
	DOUBLE    m_Lz;
	//Callback* m_callback;	// Current callback
public:

#ifdef GRAPH_LAYOUT_DEBUG
	void debugSetPixel( CDC& pdc, int a, int b, COLORREF c );
	std::unique_ptr<std::vector<std::vector<bool>>> bitSetMask;
	int numCalls;
#endif

	};


#else
#error 555
#endif

// $Log$
// Revision 1.6  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.
//
// Revision 1.5  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
