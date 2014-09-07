// pacman.cpp	- Implementation of CPacman
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

#include "stdafx.h"
//#include "pacman.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#ifdef DRAW_PACMAN

namespace
{
	DWORD UPDATEINTERVAL = 500;	// ms
	DOUBLE MOUTHSPEED    = 0.0030;	// aperture alteration / ms
}

CPacman::CPacman( ) {
	m_bgcolor     = GetSysColor( COLOR_WINDOW );
	m_readJobs    = 0;
	m_moving      = false;
	m_lastUpdate  = 0;
	Reset( );
	}

void CPacman::Reset( ) {
	m_toTheRight   = true;
	m_position     = 0;
	m_mouthOpening = true;
	m_aperture     = 0;
	}

void CPacman::SetBackgroundColor( _In_ const COLORREF color ) {
	m_bgcolor = color;
	}

void CPacman::Start( _In_ const bool start ) {
	m_moving     = start;
	m_lastUpdate = GetTickCount64();
	}

bool CPacman::Drive( _In_ const std::int32_t readJobs ) {
	/*
	  returns false if (pacman) is moving, or the time passed since the last call to Drive is less than UPDATEINTERVAL.
	  returns true otherwise (successfully updates pacman's position).
	*/
	m_readJobs = std::int32_t(readJobs);
	if ( !m_moving ) {
		return false;
		}

	auto now    = GetTickCount64( );
	auto delta  = now - m_lastUpdate;

	if ( delta < UPDATEINTERVAL ) {
		return false;
		}

	m_lastUpdate = now;

	return true;
	}

bool CPacman::Drive( _In_ const LONGLONG readJobs ) {
	/*
	  returns false if (pacman) is moving, or the time passed since the last call to Drive is less than UPDATEINTERVAL.
	  returns true otherwise (successfully updates pacman's position).
	*/
	m_readJobs = std::int32_t(readJobs);
	if ( !m_moving ) {
		return false;
		}

	auto now    = GetTickCount64( );
	auto delta  = now - m_lastUpdate;

	if ( delta < UPDATEINTERVAL ) {
		return false;
		}

	m_lastUpdate = now;

	return true;
	}


void CPacman::Draw( _In_ CDC *pdc, _In_ const CRect& rect ) {
	return;
	ASSERT_VALID( pdc );
	pdc->FillSolidRect( rect, m_bgcolor );
	
	CRect rc = rect;
	rc.DeflateRect( 5, 1 );

	if ( rc.Height( ) % 2 == 0 ) {
		rc.bottom--;
		}
	INT diameter = rc.Height( );
	INT left = rc.left + INT( m_position * ( rc.Width( ) - diameter ) );
	rc.left  = left;
	rc.right = left + diameter;
	CPen pen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	CSelectObject sopen( pdc, &pen );
	CBrush brush( CalculateColor( ) );
	CSelectObject sobrush( pdc, &brush );
	CPoint ptStart;
	CPoint ptEnd;
	INT hmiddle = rc.top + diameter / 2;
	INT mouthcy = INT( m_aperture * m_aperture * diameter );
	INT upperMouthcy = mouthcy;
	INT lowerMouthcy = mouthcy;
	/*
	  It's the sad truth, that CDC::Pie() behaves different on Windows 9x than on NT.
	  support dropped 5/1/2014. I think that's reasonable.
	*/
	lowerMouthcy++;
	if (m_toTheRight) {
		ptStart.x   = ptEnd.x = rc.right;
		ptStart.y	= hmiddle - upperMouthcy;
		ptEnd.y		= hmiddle + lowerMouthcy;
		}
	else {
		ptStart.x   = ptEnd.x = rc.left;
		ptStart.y	= hmiddle + lowerMouthcy;
		ptEnd.y		= hmiddle - upperMouthcy;
		}
	pdc->Pie( rc, ptStart, ptEnd );
	}

COLORREF CPacman::CalculateColor( ) {

	ASSERT(m_readJobs >= 0);
	DOUBLE a = atan( m_readJobs / 18 ) / pi2;
	ASSERT(a >= 0.0);
	ASSERT(a <= 1.0);

	// a == 1 --> yellow
	// a == 0 --> bgcolor
	//red & green typically never are less than 240, never greater than 254
	//TRACE( _T( "a: %f, a*255: %f, (1-a * GetRValue): %f, (1-a * GetGValue): %f \r\n" ), a, ( a*255.00 ), ( ( 1 - a ) * GetRValue( m_bgcolor ) ), ( ( 1 - a ) * GetGValue( m_bgcolor ) ));
	//INT red		= (INT)(a * 255 + (1 - a) * GetRValue(m_bgcolor));
	//INT green	= (INT)(a * 255 + (1 - a) * GetGValue(m_bgcolor));
	//245 is fine on all themes that I've tested.
	INT red = 245;
	INT green = 245;
	INT blue = INT( ( 1 - a ) * GetBValue( m_bgcolor ) );

#ifdef COLOR_DEBUG
	TRACE( _T("red: %i, green: %i, blue: %i\r\n"), red, green, blue);
	std::shared_ptr<colorRecord> newRecord = std::make_shared<colorRecord>();
	newRecord->blue = blue;
	newRecord->green = green;
	newRecord->red = red;
	colorRecords.push_back(std::move(newRecord));
#endif
		return RGB(red, green, blue);
	}

#endif
// $Log$
// Revision 1.6  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
