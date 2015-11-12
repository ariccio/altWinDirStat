// treemap.h	- Declaration of CColorSpace, CTreemap and CTreemapPreview
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_TREEMAP_H_INCLUDED
#define WDS_TREEMAP_H_INCLUDED

WDS_FILE_INCLUDE_MESSAGE

#include "globalhelpers.h"

class CTreeListItem;
class CGraphView;
class CDirstatDoc;

void trace_typeview_used_stack( );
void trace_typeview_used_heap( );
void trace_draw_cushion_stack_uses( _In_ const rsize_t num_times_stack_used );
void trace_draw_cushion_heap__uses( _In_ const rsize_t num_times_heap__used );
void trace_stack_uses_percent( _In_ const double stack_v_total );
void trace_stack_size_alloc( _In_ const double stack_size_av );
void trace_heap__uses_percent( _In_ const double heap__v_total );
void trace_heap__size_alloc( _In_ const double heap__size_av );
// CTreemap. Can create a treemap. Knows 2 squarification methods: KDirStat-like, SequoiaView-like.
class CTreemap {
public:
	CTreemap( );

#ifdef DEBUG
	~CTreemap( ) {
		const double stack_v_total = ( static_cast< double >( num_times_stack_used ) / static_cast< double >( num_times_heap__used + num_times_stack_used ) );
		const double heap__v_total = ( static_cast< double >( num_times_heap__used ) / static_cast< double >( num_times_heap__used + num_times_stack_used ) );
		const double stack_size_av = ( ( num_times_stack_used != 0 ) ? ( static_cast< double >( total_size_stack_vector ) / static_cast< double >( num_times_stack_used ) ) : 0 );
		const double heap__size_av = ( ( num_times_heap__used != 0 ) ? ( static_cast< double >( total_size_heap__vector ) / static_cast< double >( num_times_heap__used ) ) : 0 );

#ifdef WDS_TYPEVIEW_TREEMAP_MEMORY_USAGE_DEBUGGING
		if ( m_is_typeview ) {
			if ( num_times_stack_used > 0 ) {
				trace_typeview_used_stack( );
				}
			if ( num_times_heap__used > 0 ) {
				trace_typeview_used_heap( );
				}
			return;
			}
#endif

		trace_draw_cushion_stack_uses( num_times_stack_used );
		trace_draw_cushion_heap__uses( num_times_heap__used );

		if ( ( stack_v_total != 0 ) && ( stack_size_av > 0 ) ) {
			trace_stack_uses_percent( stack_v_total );
			trace_stack_size_alloc( stack_size_av );
			}
		if ( ( heap__v_total != 0 ) && ( heap__size_av > 0 ) ) {
			trace_heap__uses_percent( heap__v_total );
			trace_heap__size_alloc( heap__size_av );
			}
		}
#else
	~CTreemap( ) = default;
#endif

	void UpdateCushionShading      ( _In_ const bool               newVal                                   );
	void SetOptions                ( _In_ const Treemap_Options&           options                                  );
	void RecurseCheckTree          ( _In_ const CTreeListItem* const item                                     ) const;
#ifdef DEBUG
	void validateRectangle         ( _In_ const CTreeListItem* const child, _In_ const RECT             rc  ) const;
#endif
	void compensateForGrid         ( _Inout_    RECT* const             rc,    _In_ const HDC hDC, _In_ const HDC hAttribDC ) const;

	void DrawTreemap               ( _In_ const HDC hOffscreen_buffer, _Inout_    RECT* const rc, _In_ const CTreeListItem* const root,  _In_ const Treemap_Options& options, _In_ const HDC hAttribDC );

	void DrawColorPreview          ( _In_ HDC hDC, _In_ const RECT rc, _In_ const COLORREF           color, _In_     const Treemap_Options* const options = NULL );

	_Success_( return != NULL ) _Ret_maybenull_ _Must_inspect_result_ const CTreeListItem* FindItemByPoint( _In_ const CTreeListItem* const root, _In_ const POINT point, _In_opt_ const CDirstatDoc* const test_doc ) const;


protected:

	void SetPixels        ( _In_ const HDC offscreen_buffer, _In_reads_( maxIndex ) _Pre_readable_size_( maxIndex ) const COLORREF* const pixles, _In_ const int&   yStart, _In_ const int& xStart, _In_ const int& yEnd, _In_ const int& xEnd,   _In_ _In_range_( >=, 0 ) const int rcWidth, _In_ const size_t offset, const size_t maxIndex, _In_ _In_range_( >=, 0 ) const int rcHeight ) const;

	void RecurseDrawGraph ( _In_ const HDC hOffscreen_buffer, _In_ const CTreeListItem* const     item,   _In_ const RECT* const rc,     _In_ const bool asroot, _In_ const DOUBLE ( &psurface )[ 4 ], _In_ const DOUBLE h ) const;

	void RecurseDrawGraph_CushionShading( _In_ const bool asroot, _Out_ DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE ( &psurface )[ 4 ], _In_ const RECT rc, _In_ const DOUBLE height, _In_ const CTreeListItem* const item ) const;

	void DrawCushion      ( _In_ const HDC offscreen_buffer, _In_ const RECT&              rc,        _In_ const DOUBLE ( &surface )[ 4 ], _In_                    const COLORREF col,       _In_ _In_range_( 0, 1 ) const DOUBLE  brightness ) const;
	void DrawSolidRect    ( _In_ const HDC hDC, _In_ const RECT&              rc,        _In_ const COLORREF        col,            _In_ _In_range_( 0, 1 ) const DOUBLE   brightness ) const;
	void DrawChildren     ( _In_ const HDC hOffscreen_buffer, _In_ const CTreeListItem*  const parent,    _In_ const DOUBLE ( &surface )[ 4 ], _In_                    const DOUBLE   h          ) const;
	


	//KDS -> KDirStat
	DOUBLE KDS_CalcNextRow ( _In_ const CTreeListItem* const parent, _In_ _In_range_( 0, INT_MAX ) const size_t nextChild,  _In_ _In_range_( 0, 32767 ) const DOUBLE width, _Out_ size_t* const childrenUsed, _Inout_ std::vector<DOUBLE>* const childWidth, const std::uint64_t parentSize ) const;
		
	bool KDS_PlaceChildren ( _In_ const CTreeListItem* const parent, _Inout_    std::vector<double>* const childWidth, _Inout_ std::vector<double>* const rows,            _Inout_    std::vector<size_t>* const childrenPerRow ) const;
	void KDS_DrawChildren  ( _In_ const HDC hOffscreen_buffer,                       _In_ const CTreeListItem* const parent,          _In_ const DOUBLE       ( &surface )[ 4 ], _In_ const DOUBLE h ) const;
	
	void KDS_DrawSingleRow( _In_ const std::vector<size_t>& childrenPerRow, _In_ _In_range_( 0, SIZE_T_MAX ) const size_t& row, _In_ const std::vector<const CTreeListItem*>& parent_vector_of_children, _Inout_ _In_range_( 0, SIZE_T_MAX ) size_t* const c, _In_ const std::vector<double>& childWidth, _In_ const int& width, _In_ const bool& horizontalRows, _In_ const int& bottom, _In_ const double& top, _In_ const RECT& rc, _In_ const HDC hOffscreen_buffer, _In_ const DOUBLE( &surface )[ 4 ], _In_ const DOUBLE& h, _In_ const CTreeListItem* const parent ) const;


	//SQV -> SequoiaView
	void SQV_DrawChildren  ( _In_ const HDC hOffscreen_buffer,                       _In_ const CTreeListItem* const parent, _In_ const DOUBLE ( &surface )[ 4 ], _In_ const DOUBLE h ) const;
	void RenderLeaf        ( _In_ const HDC hOffscreen_buffer,          _In_ const CTreeListItem* const item,   _In_ const DOUBLE ( &surface )[ 4 ]                   ) const;
	void RenderRectangle   ( _In_ const HDC  offscreen_buffer,          _In_ const RECT&             rc,     _In_ const DOUBLE ( &surface )[ 4 ], _In_ DWORD color ) const;

	//if we pass horizontal by reference, compiler produces `cmp    BYTE PTR [r15], 0` for `if ( horizontal )`, pass by value generates `test    r15b, r15b`
	void SQV_put_children_into_their_places( _In_ const size_t& rowBegin, _In_ const size_t& rowEnd, _In_ const std::vector<const CTreeListItem*>& parent_vector_of_children, _Inout_ std::map<std::uint64_t, std::uint64_t>* const sizes, _In_ const std::uint64_t& sumOfSizesOfChildrenInRow, _In_ const int& heightOfNewRow, _In_ const bool horizontal, _In_ const RECT& remaining, _In_ const HDC hOffscreen_buffer, _In_ const DOUBLE( &surface )[ 4 ], _In_ const DOUBLE& scaleFactor, _In_ const DOUBLE h, _In_ const int& widthOfRow ) const;

	bool IsCushionShading( ) const;

private:

	void DrawCushion_with_heap( _In_ const size_t loop_rect_start_outer, _In_ const size_t loop_rect__end__outer, _In_ const size_t loop_rect_start_inner, _In_ const size_t loop_rect__end__inner, _In_ const size_t inner_stride, _In_ const size_t offset, _In_ _In_range_( 1024, SIZE_T_MAX ) const size_t vecSize, _In_ const HDC offscreen_buffer, const _In_ RECT& rc, _In_ _In_range_( 0, 1 ) const DOUBLE brightness, _In_ const size_t largestIndexWritten, _In_ const DOUBLE surface_0, _In_ const DOUBLE surface_1, _In_ const DOUBLE surface_2, _In_ const DOUBLE surface_3, _In_ const DOUBLE Is, _In_ const DOUBLE Ia, _In_ const DOUBLE colR, _In_ const DOUBLE colG, _In_ const DOUBLE colB ) const;

	void DrawCushion_with_stack( _In_ const size_t loop_rect_start_outer, _In_ const size_t loop_rect__end__outer, _In_ const size_t loop_rect_start_inner, _In_ const size_t loop_rect__end__inner, _In_ const size_t inner_stride, _In_ const size_t offset, _In_ _In_range_( 1, 1024 ) const size_t vecSize, _In_ HDC offscreen_buffer, const _In_ RECT& rc, _In_ _In_range_( 0, 1 ) const DOUBLE brightness, _In_ const size_t largestIndexWritten, _In_ const DOUBLE surface_0, _In_ const DOUBLE surface_1, _In_ const DOUBLE surface_2, _In_ const DOUBLE surface_3, _In_ const DOUBLE Is, _In_ const DOUBLE Ia, _In_ const DOUBLE colR, _In_ const DOUBLE colG, _In_ const DOUBLE colB ) const;

public:
	
	//C4820: 'CTreemap' : '7' bytes padding added after data member 'CTreemap::IsCushionShading_current'
	bool IsCushionShading_current : 1;

	Treemap_Options   m_options;	// Current options
#ifdef DEBUG
	mutable std::uint64_t total_size_stack_vector = 0;
	mutable std::uint64_t total_size_heap__vector = 0;
	mutable rsize_t       num_times_heap__used    = 0;
	mutable rsize_t       num_times_stack_used    = 0;
	bool                  m_is_typeview           = false;
#endif

protected:

	DOUBLE    m_Lx;// Derived parameters
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


#endif

// $Log$
// Revision 1.6  2004/11/29 07:07:47  bseifert
// Introduced SRECT. Saves 8 Bytes in sizeof(CItem). Formatting changes.