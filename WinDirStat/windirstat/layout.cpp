// layout.cpp	- Implementation of CLayout
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_LAYOUT_CPP
#define WDS_LAYOUT_CPP

#include "options.h"
#include "layout.h"
void CLayout::CSizeGripper::DrawShadowLine( _In_ HDC& hDC, _In_ WTL::CPoint start, _In_ WTL::CPoint end ) {
	//ASSERT_VALID( pdc );
	CPen lightPen( PS_SOLID, 1, GetSysColor( COLOR_3DHIGHLIGHT ) );
	HGDIOBJ old_obj_1 = SelectObject( hDC, lightPen );
	//CSelectObject sopen( pdc, lightPen );

	//pdc.MoveTo( start );
	/*
_AFXWIN_INLINE CPoint CDC::MoveTo(POINT point)
	{ ASSERT(m_hDC != NULL); return MoveTo(point.x, point.y); }
	
CPoint CDC::MoveTo(int x, int y)
{
	ASSERT(m_hDC != NULL);
	CPoint point;

	if (m_hDC != m_hAttribDC)
		VERIFY(::MoveToEx(m_hDC, x, y, &point));
	if (m_hAttribDC != NULL)
		VERIFY(::MoveToEx(m_hAttribDC, x, y, &point));
	return point;
}

	*/

	ASSERT( hDC != NULL );
	POINT point_1 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_1 ) );

	//VERIFY( pdc.LineTo( end ) );
	/*
BOOL CDC::LineTo(int x, int y)
{
	ASSERT(m_hDC != NULL);
	if (m_hAttribDC != NULL && m_hDC != m_hAttribDC)
		::MoveToEx(m_hAttribDC, x, y, NULL);
	return ::LineTo(m_hDC, x, y);
}

	*/
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	start.x++;
	end.y++;

	CPen darkPen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) );
	
	//unselect object
	const auto old_obj_1_unselect_res = SelectObject( hDC, old_obj_1 );
	ASSERT( old_obj_1_unselect_res != NULL );

	HGDIOBJ old_obj_2 = SelectObject( hDC, darkPen );
	//CSelectObject sopen2( pdc, darkPen );

	//pdc.MoveTo( start );
	ASSERT( hDC != NULL );
	POINT point_2 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_2 ) );
	
	//VERIFY( pdc.LineTo( end ) );
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	start.x++;
	end.y++;

	//pdc.MoveTo( start );
	ASSERT( hDC != NULL );
	POINT point_3 = { 0, 0 };
	VERIFY( ::MoveToEx( hDC, start.x, start.y, &point_3 ) );

	//VERIFY( pdc.LineTo( end ) );
	ASSERT( hDC != NULL );
	VERIFY( ::LineTo( hDC, end.x, end.y ) );

	//unselect object
	const auto old_obj_2_unselect_res = SelectObject( hDC, old_obj_2 );
	ASSERT( old_obj_2_unselect_res != NULL );
	}

#else

#endif