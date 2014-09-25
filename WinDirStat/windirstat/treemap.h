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

//bool WillGetWorse( _In_ const std::uint64_t sumOfSizeOfChilrenInThisRow, _In_ const LONGLONG minSizeOfChildrenInThisRow, _In_ const LONGLONG maxSizeOfChildrenInThisRow, _In_ const DOUBLE worstRatioSoFar, _In_ const DOUBLE hh, _Out_ DOUBLE& nextWorst );
//void assign_rc_and_fBegin_horizontalOrVertical( _In_ const CRect& remainingRectangleToFill, _Inout_ CRect& rc, _Inout_ DOUBLE& fBegin, _In_ const bool divideHorizontally, _In_ const int widthOfThisRow );
//void addChild_rowEnd_toRow( _Inout_ std::uint64_t& sumOfSizeOfChilrenInThisRow, _In_ const LONGLONG minSizeOfChildrenInThisRow, _Inout_ INT_PTR& rowEnd, _In_ const DOUBLE& nextWorst, _Inout_ DOUBLE& worstRatioSoFar );

// CColorSpace. Helper class for manipulating colors. Static members only.
class CColorSpace {
public:
	// Returns the brightness of color. Brightness is a value between 0 and 1.0.
	_Ret_range_(0, 1) static DOUBLE GetColorBrightness( _In_ const COLORREF color );

	// Gives a color a defined brightness.
	static COLORREF MakeBrightColor( _In_ const COLORREF color, _In_ _In_range_(0, 1) const DOUBLE brightness );

	// Returns true, if the system has <= 256 colors
	//static bool Is256Colors();

	// Swaps values above 255 to the other two values
	//static void NormalizeColor( _Inout_ _Out_range_( 0, 255 ) INT& red, _Inout_ _Out_range_( 0, 255 ) INT& green, _Inout_ _Out_range_( 0, 255 ) INT& blue );

protected:
	// Helper function for NormalizeColor()
//static void DistributeFirst( _Inout_ _Out_range_( 0, 255 ) INT& first, _Inout_ _Out_range_( 0, 255 ) INT& second, _Inout_ _Out_range_( 0, 255 ) INT& third );
	};

//
// CTreemap. Can create a treemap. Knows 2 squarification methods:
// KDirStat-like, SequoiaView-like.
//
// This class is fairly reusable.
//
class CTreemap {
public:
	// One of these flags can be added to the COLORREF returned by TmiGetGraphColor(). Used for <Free space> (darker) and <Unknown> (brighter).
	//
	static const DWORD COLORFLAG_DARKER  = 0x01000000;
	static const DWORD COLORFLAG_LIGHTER = 0x02000000;
	static const DWORD COLORFLAG_MASK	 = 0x03000000;

	// Item. Interface which must be supported by the tree items.
	// If you prefer to use the getHead()/getNext() pattern rather than using an array for the children, you will have to rewrite CTreemap.
	class Item {
	public:
		virtual                              bool       TmiIsLeaf          (                      ) const = 0;
		virtual                              CRect      TmiGetRectangle    (                      ) const = 0;
		virtual                              void       TmiSetRectangle    ( _In_ const CRect& rc )       = 0;
		virtual                              COLORREF   TmiGetGraphColor   (                      ) const = 0;
		virtual                              size_t     TmiGetChildrenCount(                      ) const = 0;
		_Must_inspect_result_ virtual        Item*      TmiGetChild        ( const size_t c       ) const = 0;
		virtual                              LONGLONG   TmiGetSize         (                      ) const = 0;
		//virtual                              Item       TmiGetParent       (                      ) const = 0;
		//virtual                              ITEMTYPE   TmiGetType         (                      ) const = 0;
		};

	// Callback. Interface with 1 "callback" method. Can be given to the CTreemap-constructor. The CTreemap will call the method very frequently during building the treemap.
	// It's because, if the tree has been paged out by the system, building the treemap can last long (> 30 seconds).
	// TreemapDrawingCallback() gives the chance to provide at least a little visual feedback (Update of RAM usage indicator, for instance).
	struct Callback {
		virtual void TreemapDrawingCallback( ) = 0;
		};

	// Treemap squarification style.
	enum STYLE : std::uint8_t {
		KDirStatStyle,		// Children are layed out in rows. Similar to the style used by KDirStat.
		SequoiaViewStyle	// The 'classical' squarification as described in at http://www.win.tue.nl/~vanwijk/.
		};

	// Collection of all treemap options.
	struct Options {
		STYLE    style;         // Squarification method
		bool     grid;           // Whether or not to draw grid lines
		COLORREF gridColor;  // Color of grid lines
		_Field_range_(  0, 1                                   ) DOUBLE brightness;          // (default = 0.84)
	    _Field_range_(  0, UINT64_MAX                          ) DOUBLE height;              // (default = 0.40)  Factor "H (really range should be 0...std::numeric_limits<double>::max/100"
		_Field_range_(  0, 1                                   ) DOUBLE scaleFactor;         // (default = 0.90)  Factor "F"
		_Field_range_(  0, 1                                   ) DOUBLE ambientLight;        // (default = 0.15)  Factor "Ia"
		_Field_range_( -4, 4                                   ) DOUBLE lightSourceX;        // (default = -1.0), negative = left
		_Field_range_( -4, 4                                   ) DOUBLE lightSourceY;        // (default = -1.0), negative = top

		_Ret_range_( 0, 100 ) INT    GetBrightnessPercent  ( ) const { return RoundDouble( brightness   * 100 ); }
		_Ret_range_( 0, 100 ) INT    GetHeightPercent      ( ) const { return RoundDouble( height       * 100 ); }
		_Ret_range_( 0, 100 ) INT    GetScaleFactorPercent ( ) const { return RoundDouble( scaleFactor  * 100 ); }
		_Ret_range_( 0, 100 ) INT    GetAmbientLightPercent( ) const { return RoundDouble( ambientLight * 100 ); }
		_Ret_range_( 0, 100 ) INT    GetLightSourceXPercent( ) const { return RoundDouble( lightSourceX * 100 ); }
		_Ret_range_( 0, 100 ) INT    GetLightSourceYPercent( ) const { return RoundDouble( lightSourceY * 100 ); }
		                      CPoint GetLightSourcePoint   ( ) const { return CPoint { GetLightSourceXPercent( ), GetLightSourceYPercent( ) }; }

		_Ret_range_( 0, 100 ) INT    RoundDouble ( const DOUBLE d ) const { return signum( d ) * INT( abs( d ) + 0.5 ); }

		void SetBrightnessPercent  ( const INT    n   ) { brightness   = n / 100.0; }
		void SetHeightPercent      ( const INT    n   ) { height       = n / 100.0; }
		void SetScaleFactorPercent ( const INT    n   ) { scaleFactor  = n / 100.0; }
		void SetAmbientLightPercent( const INT    n   ) { ambientLight = n / 100.0; }
		void SetLightSourceXPercent( const INT    n   ) { lightSourceX = n / 100.0; }
		void SetLightSourceYPercent( const INT    n   ) { lightSourceY = n / 100.0; }
		void SetLightSourcePoint   ( const CPoint pt  ) { SetLightSourceXPercent( pt.x ); SetLightSourceYPercent( pt.y ); }
		};

public:
	void UpdateCushionShading( _In_ const bool newVal );
	// Get a good palette of 13 colors (7 if system has 256 colors)

	static std::vector<COLORREF> GetDefaultPaletteAsVector( );
	static Options GetDefaultOptions( );

	// Construct the treemap generator and register the callback interface.
	CTreemap( Callback *callback = NULL );

	void SetOptions( _In_ const Options* options );
	Options GetOptions( );

	// DEBUG function
	void RecurseCheckTree( _In_ const Item* item ) const;
	void validateRectangle( _In_ const Item* child, _In_ const CRect& rc) const;

	void compensateForGrid( _Inout_ CRect& rc, _In_ CDC* pdc );

	void DrawTreemap( _In_ CDC* pdc, _In_ CRect& rc, _In_ Item* root, _In_opt_ const Options* options = NULL );

	// Same as above but double buffered
	void DrawTreemapDoubleBuffered( _In_ CDC* pdc, _In_ const CRect& rc, _In_ Item* root, _In_opt_ const Options* options = NULL );

	_Success_(return != NULL) _Must_inspect_result_ const Item* FindItemByPoint( _In_ const Item* root, _In_ const CPoint point );

	// Draws a sample rectangle in the given style (for color legend)
	void DrawColorPreview( _In_ CDC* pdc, _In_ const CRect& rc, _In_ const COLORREF color, _In_ const Options* options = NULL );

protected:
	void RecurseDrawGraph( _In_ CDC* pdc, _In_ Item* item, _In_ const CRect& rc, _In_ const bool asroot, _In_ _In_reads_( 4 ) const DOUBLE* psurface, _In_ const DOUBLE h, _In_ const DWORD flags );

	// This function switches to KDirStat-, or SequoiaView
	void DrawChildren( _In_ CDC* pdc, _In_ Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD flags );

	// KDirStat-like squarification
	void KDirStat_DrawChildren( _In_ CDC* pdc, _In_ const Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD flags );
		
	DOUBLE KDirStat_CalcutateNextRow( _In_ const Item* parent, _In_ _In_range_( 0, INT_MAX ) const size_t nextChild, _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Inout_ INT& childrenUsed, _Inout_ CArray<DOUBLE, DOUBLE>& childWidth );
		
	bool KDirStat_ArrangeChildren( _In_ const Item* parent, _Inout_ CArray<double, double>& childWidth, _Inout_ CArray<double, double>& rows, _Inout_ CArray<int, int>& childrenPerRow );

	// Classical SequoiaView-like squarification
	void SequoiaView_DrawChildren( _In_ CDC* pdc, _In_ Item* parent, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const DOUBLE h, _In_ const DWORD flags );

	// Sets brightness to a good value, if system has only 256 colors
	//void SetBrightnessFor256( );

	// Returns true, if height and scaleFactor are > 0 and ambientLight is < 1.0
	bool IsCushionShading( ) const;

	// Leaves space for grid and then calls RenderRectangle()
	void RenderLeaf( _In_ CDC* pdc, _In_ Item* item, _In_ _In_reads_( 4 ) const DOUBLE* surface );

	// Either calls DrawCushion( ) or DrawSolidRect( )
	void RenderRectangle(_In_ CDC* pdc, _In_ const CRect& rc, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ DWORD color);

	// Draws the surface using SetPixel( )
	void DrawCushion( _In_ CDC *pdc, _In_ const CRect& rc, _In_ _In_reads_( 4 ) const DOUBLE* surface, _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness );

	// Draws the surface using FillSolidRect( )
	void DrawSolidRect( _In_ CDC* pdc, _In_ const CRect& rc, _In_ const COLORREF col, _In_ _In_range_( 0, 1 ) const DOUBLE brightness );

	// Adds a new ridge to surface
	static void AddRidge( _In_ const CRect& rc, _Inout_ _Inout_updates_( 4 ) DOUBLE* surface, _In_ const DOUBLE h );

public:
	
	bool IsCushionShading_current : 1;

protected:
	//           bool     m_IsSystem256Colors : 1;
	static const Options  _defaultOptions;				// Good values. Default for WinDirStat 1.0.2
	//static const COLORREF _defaultCushionColors[];		// Standard palette for WinDirStat

	Options m_options;		// Current options
	DOUBLE m_Lx;			// Derived parameters
	DOUBLE m_Ly;
	DOUBLE m_Lz;
	Callback* m_callback;	// Current callback
public:

#ifdef GRAPH_LAYOUT_DEBUG
	void debugSetPixel( CDC* pdc, int a, int b, COLORREF c );
	std::unique_ptr<std::vector<std::vector<bool>>> bitSetMask;
	int numCalls;
#endif

	};

	class CTreemapPreview : public CStatic {
		/*
		  CTreemapPreview. A child window, which demonstrates the options with an own little demo tree.
		*/
	// Element of the demo tree.
	class CItemBranch : public CTreemap::Item {
	public:
		CItemBranch( INT size, COLORREF color ) : m_size( size ), m_color( color ) { }
		CItemBranch( _In_ _In_reads_( childrenCount ) _In_count_( childrenCount ) _Const_ CItemBranch** children, const size_t childrenCount ) : m_size( 0 ), m_color( NULL ) {
			for ( size_t i = 0; i < childrenCount; ++i ) {
				m_children.emplace_back( children[ i ] );
				m_size += INT( ( children[ i ] )->TmiGetSize( ) );
				}
			qsort( m_children.data( ), m_children.size( ), sizeof( CItemBranch* ), &_compareItems );
			m_children.shrink_to_fit( );
			}
		~CItemBranch( ) {
			for ( auto& a : m_children ) {
				if ( a != NULL ) {
					delete a;
					a = NULL;
					}
				}
			m_children.clear( );
			}
		static INT _compareItems( const void* p1, const void* p2 ) {
			const auto item1 = *( const CItemBranch ** ) p1;
			const auto item2 = *( const CItemBranch ** ) p2;
			return signum( item2->m_size - item1->m_size );
			}
		              virtual     CRect    TmiGetRectangle     (                      ) const          { return   m_rect;                    }
		              virtual     void     TmiSetRectangle     ( _In_ const CRect& rc )                {          m_rect = rc;               }
		              virtual     COLORREF TmiGetGraphColor    (                      ) const override { return   m_color;                   }
		              //virtual     ITEMTYPE TmiGetType          (                      ) const          { return   IT_FILESFOLDER;            };
		              virtual     LONGLONG TmiGetSize          (                      ) const          { return   m_size;                    }
		              virtual     bool     TmiIsLeaf           (                      ) const          { return ( m_children.size( ) == 0 ); }
		              virtual     size_t   TmiGetChildrenCount (                      ) const override { return   m_children.size( );        }
_Must_inspect_result_ virtual     Item*    TmiGetChild         ( const size_t c       ) const override { return   m_children.at( c );        }
	private:
		std::vector<CItemBranch* > m_children;
		INT                        m_size;		// Our size (in fantasy units)
		COLORREF                   m_color;		// Our color
		CRect                      m_rect;		// Our Rectangle in the treemap
		};

public:
	CTreemapPreview();
	//~CTreemapPreview();
	void SetOptions( _In_ const CTreemap::Options* options );

protected:
	void BuildDemoData();
	COLORREF GetNextColor(_Inout_ size_t& i);

	// Our color palette
	std::vector<COLORREF> m_vectorOfColors;

	std::unique_ptr<CItemBranch> m_root;	    // Demo tree
	CTreemap                     m_treemap;  // Our treemap creator

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
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
