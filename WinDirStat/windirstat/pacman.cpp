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
#include "pacman.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	DWORD UPDATEINTERVAL = 500;	// ms
	DOUBLE MOUTHSPEED    = 0.0030;	// aperture alteration / ms
}

CPacman::CPacman()
{
	m_isWindows9x = false;
	m_bgcolor     = GetSysColor( COLOR_WINDOW );
	m_readJobs    = 0;
	m_speed       = 0.0005;
	m_moving      = false;
	m_lastUpdate  = 0;
#ifdef COLOR_DEBUG
	std::vector<std::shared_ptr<colorRecord>> colorRecords;
#endif
	Reset( );
}

#ifdef COLOR_DEBUG

CPacman::~CPacman( ) {
	long long INT totalRed = 0;
	long long INT totalGreen = 0;
	long long INT totalBlue = 0;

	INT smallestRed = 256;
	INT smallestGreen =256;
	INT smallestBlue = 256;
	
	INT largestRed = 0;
	INT largestGreen = 0;
	INT largestBlue = 0;
	
	for ( auto aRecord : colorRecords ) {
		
		if ( aRecord->red > largestRed ) {
			largestRed = aRecord->red;
			}

		if ( aRecord->red < smallestRed ) {
			smallestRed = aRecord->red;
			}
		

		if ( aRecord->green > largestGreen ) {
			largestGreen = aRecord->green;
			}
		if ( aRecord->green < smallestGreen ) {
			smallestGreen = aRecord->green;
			}
		

		if ( aRecord->blue > largestBlue ) {
			largestBlue = aRecord->blue;
			}
		if ( aRecord->blue < smallestBlue ) {
			smallestBlue = aRecord->blue;
			}
		totalRed += aRecord->red;
		totalGreen += aRecord->green;
		totalBlue += aRecord->blue;
		}
	long long  sizeVect = colorRecords.size( );
	if ( sizeVect == 0 ) {
		sizeVect = 1;//prevents division by zero crash if exiting before scanning directory
		}
	TRACE(_T("totalRed: %lld\r\n"), totalRed );
	TRACE( _T( "totalGreen: %lld\r\n" ), totalGreen );
	TRACE( _T( "totalBlue: %lld\r\n" ), totalBlue );
	long DOUBLE RedAvg   = totalRed   / sizeVect + 1;
	long DOUBLE GreenAvg = totalGreen / sizeVect + 1;
	long DOUBLE BlueAvg  = totalBlue  / sizeVect + 1;
	TRACE( _T( "Total number of color records: %llu\r\n"), sizeVect);
	TRACE( _T( "Average of all Green color values: %i, average of all Red color values: %i, average of all Blue color values: %i\r\n" ), ( INT ) GreenAvg, ( INT ) RedAvg, ( INT ) BlueAvg );
	
	TRACE( _T( "Largest Red   value observed: %d\r\n"), largestRed    );
	TRACE( _T( "Largest Green value observed: %d\r\n" ), largestGreen );
	TRACE( _T( "Largest Blue  value observed: %d\r\n" ), largestBlue  );
	
	TRACE( _T( "Smallest Red   value observed: %d\r\n" ), smallestRed );
	TRACE( _T( "Smallest Green value observed: %d\r\n" ), smallestGreen );
	TRACE( _T( "Smallest Blue  value observed: %d\r\n" ), smallestBlue );
	}

#endif


void CPacman::Reset()
{
	m_toTheRight   = true;
	m_position     = 0;
	m_mouthOpening = true;
	m_aperture     = 0;
}

void CPacman::SetBackgroundColor( _In_ const COLORREF color )
{
	m_bgcolor = color;
}

void CPacman::SetSpeed( _In_ const DOUBLE speed )
{
	m_speed = speed;
}

void CPacman::Start( _In_ const bool start )
{
	m_moving     = start;
	m_lastUpdate = GetTickCount64();
}

bool CPacman::Drive( _In_ const LONGLONG readJobs )
{
	/*
	  returns false if (pacman) is moving, or the time passed since the last call to Drive is less than UPDATEINTERVAL.
	  returns true otherwise (successfully updates pacman's position).
	*/
	m_readJobs = ( DOUBLE ) readJobs;
	//TRACE( _T("Driving readJobs %lld.....\r\n"), readJobs);
	//TRACE( _T( "Number of readJobs ongoing: %f\r\n" ), m_readJobs );
	if ( !m_moving ) {
		return false;
		}

	auto now    = GetTickCount64( );
	auto delta  = now - m_lastUpdate;

	if ( delta < UPDATEINTERVAL ) {
		return false;
		}

	m_lastUpdate = now;

	UpdatePosition( m_position, m_toTheRight,   m_speed    * delta );
	UpdatePosition( m_aperture, m_mouthOpening, MOUTHSPEED * delta );

	return true;
}

void CPacman::Draw(_In_ CDC *pdc, _In_ const CRect& rect)
{
	ASSERT_VALID( pdc );
	pdc->FillSolidRect( rect, m_bgcolor );
	
	CRect rc = rect;
	rc.DeflateRect( 5, 1 );

	if ( rc.Height( ) % 2 == 0 ) {
		rc.bottom--;
		}
	INT diameter = rc.Height( );
	INT left = rc.left + ( INT ) ( m_position * ( rc.Width( ) - diameter ) );
	rc.left  = left;
	rc.right = left + diameter;
	CPen pen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	CSelectObject sopen( pdc, &pen );
	CBrush brush( CalculateColor( ) );
	CSelectObject sobrush( pdc, &brush );
	CPoint ptStart;
	CPoint ptEnd;
	INT hmiddle = rc.top + diameter / 2;
	INT mouthcy = ( INT ) ( m_aperture * m_aperture * diameter );
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


void CPacman::UpdatePosition(_Inout_ DOUBLE& position, _Inout_ bool& up, _Inout_ DOUBLE diff)
{
	ASSERT( diff >= 0.0 );
	ASSERT( position >= 0.0 );
	ASSERT( position <= 1.0 );
	//TRACE( _T("Updating position, position: %f, up: %i, diff: %f\r\n"), position, up, diff);
	while ( diff > 0.0 )
	{
		if ( up ) {
			if ( position + diff > 1.0 ) {
				//TRACE( _T("position + diff: %f\r\n"), (position+diff) );
				diff = position + diff - 1.0;
				position = 1.0;
				up = !up;
				}
			else {
				position += diff;
				diff = 0;
				}
			}
		else {
			if ( position - diff < 0.0 ) {
				//TRACE( _T( "position - diff: %f\r\n" ), ( position - diff ) );
				diff = -( position - diff );
				position = 0.0;
				up = !up;
				}
			else {
				position -= diff;
				diff = 0;
				}
			}
	}
	ASSERT(diff <= 0.00 );
}

COLORREF CPacman::CalculateColor()
{
	//static const double pi2 = (3.1415926535897932384626433832795 / 2);

	ASSERT(m_readJobs >= 0);
	DOUBLE a = atan( m_readJobs / 18 ) / pi2;
	ASSERT(a >= 0.0);
	ASSERT(a <= 1.0);

/*
	// a == 1 --> yellow
	// a == 0 --> green

	INT red= (INT)(a * 255);

	return RGB(red, 255, 0);
*/
	// a == 1 --> yellow
	// a == 0 --> bgcolor

	//red & green typically never are less than 240, never greater than 254
	//TRACE( _T( "a: %f, a*255: %f, (1-a * GetRValue): %f, (1-a * GetGValue): %f \r\n" ), a, ( a*255.00 ), ( ( 1 - a ) * GetRValue( m_bgcolor ) ), ( ( 1 - a ) * GetGValue( m_bgcolor ) ));
	//INT red		= (INT)(a * 255 + (1 - a) * GetRValue(m_bgcolor));
	//INT green	= (INT)(a * 255 + (1 - a) * GetGValue(m_bgcolor));
	
	//245 is fine on all themes that I've tested.
	INT red = 245;
	INT green = 245;
	INT blue	= (INT)(          (1 - a) * GetBValue(m_bgcolor));

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

// $Log$
// Revision 1.6  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
