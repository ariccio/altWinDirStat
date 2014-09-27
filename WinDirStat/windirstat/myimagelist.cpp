// myimagelist.cpp	 - Implementation of CMyImageList
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
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
//#include "myimagelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	COLORREF Greenify( COLORREF c ) {
		if ( c == RGB( 255, 255, 255 ) )
			return c;
		auto b = CColorSpace::GetColorBrightness( c );
		b = b * b;
		return CColorSpace::MakeBrightColor( RGB( 0, 255, 0 ), b );
		}

	COLORREF Blueify( COLORREF c ) {
		if ( c == RGB( 255, 255, 255 ) )
			return c;
		auto b = CColorSpace::GetColorBrightness( c );
		return CColorSpace::MakeBrightColor( RGB( 0, 0, 255 ), b );
		}

	COLORREF Yellowify( COLORREF c ) {
		if ( c == RGB( 255, 255, 255 ) )
			return c;
		auto b = CColorSpace::GetColorBrightness( c );
		b = b * b;
		return CColorSpace::MakeBrightColor( RGB( 255, 255, 0 ), b );
		}
	}


//CMyImageList::CMyImageList( ) : m_filesFolderImage( 0 ), m_freeSpaceImage( 0 ), m_unknownImage( 0 ), m_emptyImage( 0 ), m_junctionImage( 0 ) { }
//CMyImageList::~CMyImageList( ) { }

void CMyImageList::Initialize( ) {
	if ( m_hImageList == NULL ) {
#ifdef DRAW_ICONS
		CString s;
		GetSystemDirectory( s.GetBuffer( _MAX_PATH ), _MAX_PATH );
		s.ReleaseBuffer( );
		SHFILEINFO sfi;
		sfi.dwAttributes = NULL;
		sfi.hIcon = NULL;
		sfi.iIcon = NULL;

		auto hil = ( HIMAGELIST ) SHGetFileInfo( s, 0, &sfi, sizeof( sfi ), SHGFI_SYSICONINDEX | SHGFI_SMALLICON );//TODO

		Attach( ImageList_Duplicate( hil ) );
		auto imageCount = GetImageCount( );
		for ( INT i = 0; i < imageCount; i++ ) {
			m_indexMap.SetAt( i, i );
			}
		AddCustomImages( );
#endif
		}
	}

#ifndef DRAW_ICONS
inline
#endif
INT CMyImageList::CacheIcon( _In_z_ PCTSTR path, _In_ UINT flags, _Inout_opt_ CString *psTypeName ) {
#ifndef DRAW_ICONS
	//------------------------------------------------------temp hack!!
	UNREFERENCED_PARAMETER( path );
	UNREFERENCED_PARAMETER( flags );
	UNREFERENCED_PARAMETER( psTypeName );
	return GetEmptyImage( );
	//------------------------------------------------------temp hack!!
#else
	ASSERT(m_hImageList != NULL); // should have been Initialize()ed.
	flags|= SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
	if ( psTypeName != NULL ) {
		flags |= SHGFI_TYPENAME;
		}
	SHFILEINFO sfi;
	sfi.dwAttributes = NULL;
	sfi.hIcon = NULL;
	sfi.iIcon = NULL;
	auto hil = ( HIMAGELIST ) SHGetFileInfo( path, 0, &sfi, sizeof( sfi ), flags );
	if (hil == NULL) {
		TRACE(_T("SHGetFileInfo() failed\n"));
		return GetEmptyImage();
		}


	if ( psTypeName != NULL ) {
		*psTypeName = sfi.szTypeName;
		}
	INT i = 0;
	if (!m_indexMap.Lookup(sfi.iIcon, i)) {//CMap::lookup - "Nonzero if the element was found; otherwise 0."
		CImageList *sil = CImageList::FromHandle( hil );
	
		/*
			This doesn't work:
			IMAGEINFO ii;	
			VERIFY(sil->GetImageInfo(sfi.iIcon, &ii));

			i= Add(CBitmap::FromHandle(ii.hbmImage), CBitmap::FromHandle(ii.hbmMask));

			So we use this method:
		*/
		TRACE( _T( "Caching icon for path %s, flags %u. System image list index for icon: %i\r\n" ), path, flags, sfi.iIcon );
		i = Add( sil->ExtractIcon( sfi.iIcon ) ); // ExtractIcon - "Retrieves a handle to an icon from the specified executable file, DLL, or icon file."

		m_indexMap.SetAt(sfi.iIcon, i);//iIcon - "The index of the icon image within the system image list."
		}
	else {
		//TRACE( _T( "Icon for path %s found in cache!\r\n" ), path );
		}
	return i;
#endif
	}

INT CMyImageList::GetMyComputerImage( ) {
	LPITEMIDLIST pidl = NULL;
	HRESULT hr = SHGetSpecialFolderLocation( NULL, CSIDL_DRIVES, &pidl );
	if (FAILED(hr)) {
		TRACE(_T("SHGetSpecialFolderLocation(CSIDL_DRIVES) failed!\n"));
		return 0;
		}

	auto i = CacheIcon( ( PCTSTR ) pidl, SHGFI_PIDL );

	CoTaskMemFree( pidl );

	return i;
	}

INT CMyImageList::GetMountPointImage( ) {
	return CacheIcon(GetADriveSpec(), 0); // The flag SHGFI_USEFILEATTRIBUTES doesn't work on W95.
	}

INT CMyImageList::GetJunctionImage( ) {
	// Intermediate solution until we find a nice icon for junction points
	return m_junctionImage;
	}

INT CMyImageList::GetFolderImage( ) {
	CString s;
	GetSystemDirectory(s.GetBuffer(_MAX_PATH), _MAX_PATH);
	s.ReleaseBuffer();

	return CacheIcon(s, 0);
	}

INT CMyImageList::GetFileImage( _In_z_ PCTSTR path ) {
	return CacheIcon(path, 0);
	}

INT CMyImageList::GetExtImageAndDescription( _In_z_ PCTSTR ext, _Inout_ CString& description ) {
	return CacheIcon(ext, SHGFI_USEFILEATTRIBUTES, &description);
	}

INT CMyImageList::GetFilesFolderImage( ) {
#ifdef DRAW_ICONS
	ASSERT(m_hImageList != NULL); // should have been Initialize()ed.
#endif
	return m_filesFolderImage;
	}

INT CMyImageList::GetFreeSpaceImage( ) {
#ifdef DRAW_ICONS
	ASSERT(m_hImageList != NULL); // should have been Initialize()ed.
#endif
	return m_freeSpaceImage;
	}

INT CMyImageList::GetUnknownImage( ) {
#ifdef DRAW_ICONS
	ASSERT(m_hImageList != NULL); // should have been Initialize()ed.
#endif
	return m_unknownImage;
	}

INT CMyImageList::GetEmptyImage( ) {
#ifdef DRAW_ICONS
	ASSERT(m_hImageList != NULL);
#endif
	return m_emptyImage;
	}


// Returns an arbitrary present drive
CString CMyImageList::GetADriveSpec( ) {
	CString s;
	auto u = GetWindowsDirectory( s.GetBuffer( _MAX_PATH ), _MAX_PATH );
	s.ReleaseBuffer( );
	if ( u == 0 || s.GetLength( ) < 3 || s[ 1 ] != _T( ':' ) || s[ 2 ] != _T( '\\' ) ) {
		return _T( "C:\\" );
		}
	return s.Left( 3 );
	}

void CMyImageList::AddCustomImages( ) {
	const INT CUSTOM_IMAGE_COUNT = 5;
	const COLORREF bgcolor = RGB( 255, 255, 255 );

	auto folderImage = GetFolderImage( );
	auto driveImage = GetMountPointImage( );

	IMAGEINFO ii;
	ZeroMemory( &ii, sizeof( ii ) );
	VERIFY( GetImageInfo( folderImage, &ii ) );
	CRect rc = ii.rcImage;

	CClientDC dcClient( CWnd::GetDesktopWindow( ) );

	CDC dcmem;
	dcmem.CreateCompatibleDC( &dcClient );
	CBitmap target;
	target.CreateCompatibleBitmap( &dcClient, rc.Width( ) * CUSTOM_IMAGE_COUNT, rc.Height( ) );

	// Junction point
	CBitmap junc;
	junc.LoadBitmap( IDB_JUNCTIONPOINT );
	BITMAP bmjunc;
	junc.GetBitmap( &bmjunc );
	CDC dcjunc;
	dcjunc.CreateCompatibleDC( &dcClient );

	{
		CSelectObject sotarget( &dcmem, &target );
		CSelectObject sojunc( &dcjunc, &junc );

		dcmem.FillSolidRect( 0, 0, rc.Width( ) * CUSTOM_IMAGE_COUNT, rc.Height( ), bgcolor );
		CPoint pt( 0, 0 );
		auto safe = SetBkColor( CLR_NONE );
		VERIFY( Draw( &dcmem, folderImage, pt, ILD_NORMAL ) );
		pt.x += rc.Width( );
		VERIFY( Draw( &dcmem, driveImage, pt, ILD_NORMAL ) );
		pt.x += rc.Width( );
		VERIFY( Draw( &dcmem, driveImage, pt, ILD_NORMAL ) );
		pt.x += rc.Width( );
		VERIFY( Draw( &dcmem, folderImage, pt, ILD_NORMAL ) );
		SetBkColor( safe );

		// Now we re-color the images
		for ( INT i = 0; i < ( rc.Width( ) ); i++ ) {
			for ( INT j = 0; j < ( rc.Height( ) ); j++ ) {
				INT idx = 0;

				// We "blueify" the folder image ("<Files>")
				auto c = dcmem.GetPixel( idx * rc.Width( ) + i, j );
				dcmem.SetPixel( idx * rc.Width( ) + i, j, Blueify( c ) );
				idx++;

				// ... "greenify" the drive image ("<Free Space>")
				c = dcmem.GetPixel( idx * rc.Width( ) + i, j );
				dcmem.SetPixel( idx * rc.Width( ) + i, j, Greenify( c ) );
				idx++;

				// ...and "yellowify" the drive image ("<Unknown>")
				c = dcmem.GetPixel( idx * rc.Width( ) + i, j );
				dcmem.SetPixel( idx * rc.Width( ) + i, j, Yellowify( c ) );
				idx++;

				// ...and overlay the junction point image with the link symbol.
				auto jjunc = j - ( rc.Height( ) - bmjunc.bmHeight );

				c = dcmem.GetPixel( idx * rc.Width( ) + i, j );
				dcmem.SetPixel( idx * rc.Width( ) + i, j, c ); // I don't know why this statement is required.
				if ( i < bmjunc.bmWidth && jjunc >= 0 ) {
					auto cjunc = dcjunc.GetPixel( i, jjunc );
					if ( cjunc != RGB( 255, 0, 255 ) ) {
						dcmem.SetPixel( idx * rc.Width( ) + i, j, cjunc );
						}
					}
				}
			}
	}
	auto k = Add( &target, bgcolor );
	m_filesFolderImage = k++;
	m_freeSpaceImage = k++;
	m_unknownImage = k++;
	m_junctionImage = k++;
	m_emptyImage = k++;
	}

// $Log$
// Revision 1.9  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.8  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.7  2004/12/12 13:40:51  bseifert
// Improved image coloring. Junction point image now with awxlink overlay.
//
// Revision 1.6  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
