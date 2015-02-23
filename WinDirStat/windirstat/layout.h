// layout.h	- Declaration of CLayout
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"
#include "globalhelpers.h" //?

#ifndef WDS_LAYOUT_H
#define WDS_LAYOUT_H

#define WDS_CLAYOUT_CSIZEGRIPPER_WIDTH INT( 14 )//temp hack. TODO: fix!

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

	CLayout( _In_ CWnd* dialog, _In_z_ PCWSTR const name ) : m_dialog( dialog ), m_name( name ) {
		ASSERT( m_dialog != NULL );
	
		// This is necessary because OnGetMinMaxInfo() will be called before OnInitDialog!
		m_originalDialogSize.cx = 0;
		m_originalDialogSize.cy = 0;
		}

	//TODO: convert to ATL
	class CSizeGripper final : public ATL::CWindowImpl<CSizeGripper> {
	public:
		//static const INT _width;

		CSizeGripper( ) { }
		CSizeGripper& operator=( const CSizeGripper& in ) = delete;
		CSizeGripper( const CSizeGripper& in ) = delete;

		//CSizeGripper( );
//#pragma warning( suppress: 4263 )
//		void Create( _Inout_ CWnd* parent, _In_ const RECT rc ) {
//			/*
//	HWND CWindowImpl::Create(
//		_In_opt_ HWND hWndParent,
//		_In_ _U_RECT rect = NULL,
//		_In_opt_z_ LPCTSTR szWindowName = NULL,
//		_In_ DWORD dwStyle = 0,
//		_In_ DWORD dwExStyle = 0,
//		_In_ _U_MENUorID MenuOrID = 0U,
//		_In_opt_ LPVOID lpCreateParam = NULL)
//
//			*/
//			// _In_ const HWND parent_hWnd, rc, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDC_SIZEGRIPPER, NULL
//			
//			VERIFY( CWnd::Create( AfxRegisterWndClass( 0, AfxGetApp( )->LoadStandardCursor( IDC_ARROW ), ( HBRUSH ) ( COLOR_BTNFACE + 1 ), 0 ), _T( "" ), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, rc, parent, IDC_SIZEGRIPPER ) );
//			}

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

	void AddControl( _In_ const UINT id, _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy ) {
		//AddControl( m_dialog->GetDlgItem( INT( id ) ), movex, movey, stretchx, stretchy );
		m_control.emplace_back( SControlInfo { m_dialog->GetDlgItem( static_cast<INT>( id ) )->m_hWnd, movex, movey, stretchx, stretchy, CRect( ) } );
		}

	void OnInitDialog( _In_ const bool centerWindow ) {
		//m_dialog->SetIcon( GetApp( )->LoadIcon( IDR_MAINFRAME ), false );

		RECT rcDialog;
		m_dialog->GetWindowRect( &rcDialog );
		m_originalDialogSize = CRect( rcDialog ).Size( );

		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( auto& aControl : m_control ) {
			RECT rc;
		
			//If [GetWindowRect] succeeds, the return value is nonzero.
			VERIFY( ::GetWindowRect( aControl.control, &rc ) );
			//aControl.control->GetWindowRect( &rc );

			/*
	_AFXWIN_INLINE void CWnd::GetWindowRect(LPRECT lpRect) const
		{ ASSERT(::IsWindow(m_hWnd)); ::GetWindowRect(m_hWnd, lpRect); }
			*/

			m_dialog->ScreenToClient( &rc );
			aControl.originalRectangle = rc;
			}

		RECT sg;
		m_dialog->GetClientRect( &sg );
		sg.left = sg.right  - WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
		sg.top  = sg.bottom - WDS_CLAYOUT_CSIZEGRIPPER_WIDTH;
		//m_dialog->m_hWnd
		// _In_ const HWND parent_hWnd, rc, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDC_SIZEGRIPPER, NULL
		m_sizeGripper.Create( m_dialog->m_hWnd, sg, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, IDC_SIZEGRIPPER, NULL );
		//m_sizeGripper.Create( m_dialog, sg );

		m_control.emplace_back( SControlInfo { m_sizeGripper.m_hWnd, 1, 1, 0, 0, sg } );

		CPersistence::GetDialogRectangle( m_name, rcDialog );
	
		ASSERT( ::IsWindow( m_dialog->m_hWnd ) );
	
		//If [MoveWindow] succeeds, the return value is nonzero.
		VERIFY( ::MoveWindow( m_dialog->m_hWnd, rcDialog.left, rcDialog.top, ( rcDialog.right - rcDialog.left ), ( rcDialog.bottom - rcDialog.top ), TRUE ) );
		if ( centerWindow ) {
			m_dialog->CenterWindow( );
			}
		}
	
	void OnGetMinMaxInfo( _Out_    MINMAXINFO* mmi ) const {
		mmi->ptMinTrackSize.x = m_originalDialogSize.cx;
		mmi->ptMinTrackSize.y = m_originalDialogSize.cy;
		}
	
	void OnDestroy( ) {
		RECT rc;
		m_dialog->GetWindowRect( &rc );
		CPersistence::SetDialogRectangle( m_name, rc );
		}

	void OnSize( ) {
		RECT rc_outer_temp;
		m_dialog->GetWindowRect( &rc_outer_temp );
		const RECT rc_outer = rc_outer_temp;
		const auto newDialogSize = CRect( rc_outer ).Size( );
		const auto diff = newDialogSize - m_originalDialogSize;
	
		// The DeferWindowPos-stuff prevents the controls from overwriting each other.
		auto hdwp = BeginDeferWindowPos( static_cast<int>( m_control.size( ) ) );

		//Not vectorized: 1304, loop includes assignments of different sizes
		for ( auto& aControl : m_control ) {
			auto rc = CRect( aControl.originalRectangle );
			const auto movex = static_cast<int>( aControl.movex );
			const auto movey = static_cast<int>( aControl.movey );
			WTL::CSize move( diff.cx * movex, diff.cy * movey );
			CRect stretch( 0, 0, diff.cx * static_cast<int>( aControl.stretchx ), diff.cy * static_cast<int>( aControl.stretchy ) );
			rc += move;
			rc += stretch;
			hdwp = DeferWindowPos( hdwp, ( aControl.control ), NULL, rc.left, rc.top, rc.Width( ), rc.Height( ), SWP_NOOWNERZORDER | SWP_NOZORDER );
			}
	
		VERIFY( EndDeferWindowPos( hdwp ) );
		}



protected:
	CWnd*                               m_dialog;
	PCWSTR const                        m_name;
	WTL::CSize                          m_originalDialogSize;
	std::vector<SControlInfo>           m_control;
	CSizeGripper                        m_sizeGripper;
	};


#else


#endif
