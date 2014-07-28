// pacman.h	- Declaration of CPacman
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


#ifdef _DEBUG
//#define COLOR_DEBUG
#endif

#ifdef COLOR_DEBUG
struct colorRecord {
	INT red;
	INT green;
	INT blue;
	};
#endif



//
// CPacman. Pacman animation.
//
#ifdef DRAW_PACMAN
class CPacman {
public:
	CPacman();
	void SetBackgroundColor ( _In_ const COLORREF color                    );
	void Reset              (                                         );
	void Start              ( _In_ const bool start                        );
	bool Drive              ( _In_ const std::int32_t readJobs                 );	// return: true -> should be redrawn.
	bool Drive              ( _In_ const LONGLONG readJobs                 );
	void Draw               ( _In_ CDC *pdc,              _In_ const CRect& rc );
#ifdef COLOR_DEBUG
	~CPacman( );
#endif

private:

	COLORREF CalculateColor (                                         );

	COLORREF       m_bgcolor;		    // Background color
	bool           m_moving;			// Whether pacman is moving
	bool           m_mouthOpening;		// Mouth is opening
	bool           m_toTheRight;		// moving right
	std::int32_t   m_readJobs;			// # of read jobs determines our color 
	DOUBLE         m_position;			// 0...1
	DOUBLE         m_aperture;			// 0...1
	std::uint64_t  m_lastUpdate;		// TickCount

#ifdef COLOR_DEBUG
	std::vector<std::shared_ptr<colorRecord>> colorRecords;
#endif


	};
#endif

// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
