// colorbutton.h - Declaration of CColorButton
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#ifndef WDS_COLORBUTTON_H
#define WDS_COLORBUTTON_H


#include "stdafx.h"


#define COLBN_CHANGED	0x87	// this is a value, I hope, that is nowhere used as notification code.


// The color preview is an own little child window of the button.
class CPreview final : public ATL::CWindowImpl<CPreview> {
public:
	COLORREF m_color;

	CPreview& operator=( const CPreview& in ) = delete;
	CPreview( const CPreview& in ) = delete;

	CPreview( ) : m_color{ 0u } { }
	void SetColor( _In_ const COLORREF color ) {
		m_color = color;
		if ( ::IsWindow( m_hWnd ) ) {
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_hWnd, NULL, TRUE ) );
			//InvalidateRect( NULL );
			}
		}

#pragma warning( push )
#pragma warning( disable: 4365 )
	BEGIN_MSG_MAP(CPreview)
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_PAINT ), ( CPreview::OnPaint ) )
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_LBUTTONDOWN ), ( CPreview::OnLButtonDown ) )
		DEFAULT_REFLECTION_HANDLER( )
	END_MSG_MAP()
#pragma warning( pop )


	//Keeping OnPaint in the implementation file means that we don't need to include globalhelpers.h in the header.
	LRESULT OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );


	//UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/
	//UINT nFlags, CPoint point
	LRESULT OnLButtonDown( UINT /*nMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ ) {
		const UINT nFlags = static_cast< UINT >( wParam );
		//const POINT point = static_cast< POINT >( lParam );
		POINT point_temp;
		point_temp.x = GET_X_LPARAM( lParam );
		point_temp.y = GET_Y_LPARAM( lParam );
		POINT point = point_temp;
		ASSERT( ::IsWindow( m_hWnd ) );
		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::ClientToScreen( m_hWnd, &point ) );
		//ClientToScreen( &point );

		const HWND this_parent = ::GetParent( m_hWnd );

		//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
		VERIFY( ::ScreenToClient( this_parent, &point ) );
			
		//this_parent->ScreenToClient( &point );

		TRACE( _T( "User clicked x:%ld, y:%ld! Sending WM_LBUTTONDOWN!\r\n" ), point.x, point.y );
			
		/*
		_AFXWIN_INLINE CWnd* CWnd::GetParent() const
		{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::GetParent(m_hWnd)); }	
		*/
		/*
_AFXWIN_INLINE LRESULT CWnd::_AFX_FUNCNAME(SendMessage)(UINT message, WPARAM wParam, LPARAM lParam) const
{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, message, wParam, lParam); }
#pragma push_macro("SendMessage")
#undef SendMessage
_AFXWIN_INLINE LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{ return _AFX_FUNCNAME(SendMessage)(message, wParam, lParam); }
#pragma pop_macro("SendMessage")

		*/
		//this_parent->SendMessageW( WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
			
		const HWND parent = ::GetParent( m_hWnd );
		::SendMessageW( parent, WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
		return 0;
	}
	};


//
// CColorButton. A Pushbutton which allows to choose a color and shows this color on its surface.
// In the resource editor, the button should be set to "right align text", as the color will be shown in the left third.
// When the user chose a color, the parent is notified via WM_NOTIFY and the notification code COLBN_CHANGED.
//
class CColorButton final : public CButton {
public:

	CColorButton( ) = default;
	CColorButton& operator=( const CColorButton& in ) = delete;
	CColorButton( const CColorButton& in ) = delete;
	


	//C4820: 'CColorButton::CPreview' : '4' bytes padding added after data member 'CColorButton::CPreview::m_color'

	CPreview m_preview;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint( ) {
		if ( m_preview.m_hWnd == NULL ) {
			RECT rc;
			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero. To get extended error information, call GetLastError."
			VERIFY( ::GetClientRect( m_hWnd, &rc ) );
			//GetClientRect( &rc );

			rc.right = rc.left + ( rc.right - rc.left ) / 3;
			//rc.DeflateRect( 4, 4 );
			VERIFY( ::InflateRect( &rc, -4, -4 ) );

			VERIFY( m_preview.Create( m_hWnd, rc, _T( "" ), WS_CHILD | WS_VISIBLE, 0, 4711, NULL ) );

			VERIFY( CWnd::ModifyStyle( 0, WS_CLIPCHILDREN ) );
			/*
BOOL CWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ModifyStyle(m_hWnd, dwRemove, dwAdd, nFlags);
	else
		return m_pCtrlSite->ModifyStyle(dwRemove, dwAdd, nFlags);
}
			*/
			}
		/*
void CWnd::OnPaint()
{
	if (m_pCtrlCont != NULL)
	{
		// Paint windowless controls
		CPaintDC dc(this);
		m_pCtrlCont->OnPaint(&dc);
	}

	Default();
}
		*/
		CButton::OnPaint( );
		}

	afx_msg void OnDestroy( ) {
		if ( ::IsWindow( m_preview.m_hWnd ) ) {
			VERIFY( m_preview.DestroyWindow( ) );
			}
		CButton::OnDestroy( );
		}

	afx_msg void OnBnClicked( ) {
		WTL::CColorDialog dlg( m_preview.m_color );
		if ( IDOK == dlg.DoModal( ) ) {
			const auto dialog_ctrl_id = ::GetDlgCtrlID( m_hWnd );
			m_preview.SetColor( dlg.GetColor( ) );
			NMHDR hdr;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = static_cast<UINT_PTR>( dialog_ctrl_id );
			hdr.code = COLBN_CHANGED;
			TRACE( _T( "Color button clicked! Sending WM_NOTIFY to Dialog with Ctrl ID: %llu\r\n" ), static_cast<ULONGLONG>( hdr.idFrom ) );
			/*
_AFXWIN_INLINE CWnd* CWnd::GetParent() const
	{ ASSERT(::IsWindow(m_hWnd)); return CWnd::FromHandle(::GetParent(m_hWnd)); }
			*/

			/*
int CWnd::GetDlgCtrlID() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::GetDlgCtrlID(m_hWnd);//<this is the branch that we take.
	else
		return m_pCtrlSite->GetDlgCtrlID();
}
			*/
			ASSERT( ::IsWindow( m_hWnd ) );
			//const auto parent_wnd = CWnd::FromHandle( ::GetParent( m_hWnd ) );
			

			/*
_AFXWIN_INLINE LRESULT CWnd::_AFX_FUNCNAME(SendMessage)(UINT message, WPARAM wParam, LPARAM lParam) const
	{ ASSERT(::IsWindow(m_hWnd)); return ::SendMessage(m_hWnd, message, wParam, lParam); }
			*/
			ASSERT( ::IsWindow( m_hWnd ) );

			::SendMessageW( m_hWnd, WM_NOTIFY, static_cast< WPARAM >( dialog_ctrl_id ), ( LPARAM ) &hdr );
			//parent_wnd->SendMessageW( WM_NOTIFY, static_cast<WPARAM>( dialog_ctrl_id ), ( LPARAM ) &hdr );
			}
		}

	afx_msg void OnEnable( const BOOL bEnable ) {
		if ( ::IsWindow( m_preview.m_hWnd ) ) {

			//"Return value: If the function succeeds, the return value is nonzero. If the function fails, the return value is zero."
			VERIFY( ::InvalidateRect( m_preview.m_hWnd, NULL, TRUE ) );
			//m_preview.InvalidateRect( NULL );
			
			}
		/*
_AFXWIN_INLINE void CWnd::OnEnable(BOOL)
	{ Default(); }
		*/

		CWnd::OnEnable( bEnable );
		}
	};

#else


#endif
