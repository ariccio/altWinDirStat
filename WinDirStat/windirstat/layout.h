// layout.h	- Declaration of CLayout
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"
#include "globalhelpers.h" //?


#ifndef WDS_LAYOUT_H
#define WDS_LAYOUT_H

#define WDS_CLAYOUT_CSIZEGRIPPER_WIDTH INT( 14 )//temp hack. TODO: fix!

class CSelectDrivesDlg;

// CLayout. A poor men's dialog layout mechanism.
// Simple, flat, and sufficient for our purposes.
// Used for SelectDrivesDlg
class CLayout {
	struct SControlInfo {
		SControlInfo( HWND control_in, DOUBLE movex_in, DOUBLE movey_in, DOUBLE stretchx_in, DOUBLE stretchy_in, RECT originalRectangle_in ) : control( control_in ), movex( std::move( movex_in ) ), movey( std::move( movey_in ) ), stretchx ( std::move( stretchx_in ) ), stretchy( std::move( stretchy_in ) ), originalRectangle( std::move( originalRectangle_in ) ) { }

		SControlInfo& operator=( const SControlInfo& in ) = delete;
		HWND control;
		const DOUBLE movex;
		const DOUBLE movey;
		const DOUBLE stretchx;
		const DOUBLE stretchy;
		RECT   originalRectangle;
		};

public:

	CLayout& operator=( const CLayout& in ) = delete;
	CLayout( const CLayout& in ) = delete;

	CLayout( _In_ CSelectDrivesDlg* dialog, _In_z_ PCWSTR const name );

	//TODO: convert to ATL
	class CSizeGripper final : public ATL::CWindowImpl<CSizeGripper> {
	public:
		//static const INT _width;

		CSizeGripper( ) { }
		CSizeGripper& operator=( const CSizeGripper& in ) = delete;
		CSizeGripper( const CSizeGripper& in ) = delete;

	private:
		void DrawShadowLine( _In_ HDC& hDC, _In_ WTL::CPoint start, _In_ WTL::CPoint end );

		//DECLARE_MESSAGE_MAP()
		LRESULT OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ ) {
			//CPaintDC dc( this );
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint( &ps );

			RECT rc;

			ASSERT( ::IsWindow( m_hWnd ) );
			VERIFY( ::GetClientRect( m_hWnd, &rc ) );
			//GetClientRect( &rc );

			ASSERT( CRect( rc ).Width( ) == WDS_CLAYOUT_CSIZEGRIPPER_WIDTH );
			ASSERT( CRect( rc ).Height( ) == WDS_CLAYOUT_CSIZEGRIPPER_WIDTH );

			WTL::CPoint start;
			WTL::CPoint end;

			start.x = 1;
			start.y = WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
			end.x   = WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
			end.y   = 1;

			//DrawShadowLine( dc, start, end );
			DrawShadowLine( hDC, start, end );

			start.x += 4;
			end.y   += 4;

			//DrawShadowLine( dc, start, end );
			DrawShadowLine( hDC, start, end );
	
			start.x += 4;
			end.y   += 4;

			//DrawShadowLine( dc, start, end );
			DrawShadowLine( hDC, start, end );

			// Do not call CWnd::OnPaint() for painting messages
			return 0;
			}

		LRESULT OnNcHitTest( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ ) {
			POINT point_temp;
			point_temp.x = GET_X_LPARAM( lParam );
			point_temp.y = GET_Y_LPARAM( lParam );
			POINT point = point_temp;

			//If [ScreenToClient] succeeds, the return value is nonzero.
			VERIFY( ScreenToClient( &point ) );
			if ( point.x + point.y >= WDS_CLAYOUT_CSIZEGRIPPER_WIDTH ) {
				return HTBOTTOMRIGHT;
				}
			return 0;
			}

	public:
		BEGIN_MSG_MAP( CSizeGripper )
			MESSAGE_HANDLER( WM_PAINT, ( CSizeGripper::OnPaint ) )
			MESSAGE_HANDLER( WM_NCHITTEST, ( CSizeGripper::OnNcHitTest ) )
#pragma warning( suppress : 4365 )//C4365: 'argument' : conversion from 'unsigned int' to 'int', signed/unsigned mismatch
		END_MSG_MAP()

		};

	void AddControl( _In_ const UINT id, _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy );


	void OnInitDialog( _In_ const bool centerWindow );
	
	void OnGetMinMaxInfo( _Out_    MINMAXINFO* mmi ) const {
		mmi->ptMinTrackSize.x = m_originalDialogSize.cx;
		mmi->ptMinTrackSize.y = m_originalDialogSize.cy;
		}
	
	void OnDestroy( );

	void OnSize( );



protected:
	CSelectDrivesDlg*                   m_dialog;
	PCWSTR const                        m_name;
	WTL::CSize                          m_originalDialogSize;
	std::vector<SControlInfo>           m_control;
	CSizeGripper                        m_sizeGripper;
	};


#else


#endif
