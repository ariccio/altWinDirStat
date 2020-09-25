// colorbutton.h - Declaration of CColorButton
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"
#include "macros_that_scare_small_children.h"
#include "hwnd_funcs.h"

#ifndef WDS_COLORBUTTON_H
#define WDS_COLORBUTTON_H

WDS_FILE_INCLUDE_MESSAGE


#define COLBN_CHANGED	0x87	// this is a value, I hope, that is nowhere used as notification code.


// The color preview is an own little child window of the button.
class CPreview final : public ATL::CWindowImpl<CPreview> {
	DISALLOW_COPY_AND_ASSIGN(CPreview);
public:
	COLORREF m_color = 0u;
	CPreview() {

	}

	void SetColor(_In_ const COLORREF color) noexcept;


	// ya know, I bet there's a way to do message maps with variadic templates.
	// http://imadiversion.co.uk/2016/12/03/window-message-dispatch-c-17-refactor/ is one such neat example, but I'm not sure it's the cleanest or fastest way to do it.

#pragma warning( push )
#pragma warning( disable: 4365 )
#pragma warning( disable: 6103 )
	BEGIN_MSG_MAP(CPreview)
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_PAINT ), ( CPreview::OnPaint ) )
		MESSAGE_HANDLER( static_cast<unsigned int>( WM_LBUTTONDOWN ), ( CPreview::OnLButtonDown ) )
		DEFAULT_REFLECTION_HANDLER( )
	END_MSG_MAP()
#pragma warning( pop )


	//Keeping OnPaint in the implementation file means that we don't need to include globalhelpers.h in the header.
	LRESULT OnPaint( UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ );

	//UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/
	LRESULT OnLButtonDown( UINT /*nMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ ) {
		const UINT nFlags = static_cast< UINT >( wParam );

		POINT point = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };

		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
		//If the window handle identifies an existing window, the return value is nonzero.
		//If the window handle does not identify an existing window, the return value is zero.
		ASSERT( ::IsWindow( m_hWnd ) );

		//ClientToScreen function: https://msdn.microsoft.com/en-us/library/dd183434.aspx
		//If the function succeeds, the return value is nonzero.
		//If the function fails, the return value is zero.
		VERIFY( ::ClientToScreen( m_hWnd, &point ) );

		const HWND this_parent = ::GetParent( m_hWnd );

		//ScreenToClient function: https://msdn.microsoft.com/en-us/library/dd162952.aspx
		//If the function succeeds, the return value is nonzero.
		//If the function fails, the return value is zero.
		//VERIFY( ::ScreenToClient( this_parent, &point ) );
		hwnd::ScreenToClient(this_parent, &point);

		TRACE( _T( "User clicked x:%ld, y:%ld! Sending WM_LBUTTONDOWN!\r\n" ), point.x, point.y );

		const HWND parent = ::GetParent( m_hWnd );

		//WM_LBUTTONDOWN message: https://msdn.microsoft.com/en-us/library/windows/desktop/ms645607.aspx
		//If an application processes this message, it should return zero.
		::SendMessageW( parent, WM_LBUTTONDOWN, nFlags, MAKELPARAM( point.x, point.y ) );
		return 0;
		}
	};


//
// CColorButton. A Pushbutton which allows to choose a color and shows this color on its surface.
// In the resource editor, the button should be set to "right align text", as the color will be shown in the left third.
// When the user chose a color, the parent is notified via WM_NOTIFY and the notification code COLBN_CHANGED.
class CColorButton final : public CButton {
	DISALLOW_COPY_AND_ASSIGN(CColorButton);
public:

	CColorButton( ) = default;

	//C4820: 'CColorButton::CPreview' : '4' bytes padding added after data member 'CColorButton::CPreview::m_color'

	CPreview m_preview;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint( ) {
		if ( m_preview.m_hWnd == nullptr) {
			RECT rc = { 0 };
			//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
			//Return value: If the function succeeds, the return value is nonzero.
			//If the function fails, the return value is zero.
			//To get extended error information, call GetLastError.
			VERIFY( ::GetClientRect( m_hWnd, &rc ) );

			rc.right = rc.left + ( rc.right - rc.left ) / 3;

			//InflateRect function: https://msdn.microsoft.com/en-us/library/dd144994.aspx
			//If the function succeeds, the return value is nonzero.
			//If the function fails, the return value is zero.
			VERIFY( ::InflateRect( &rc, -4, -4 ) );


			/*
			// from: C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\atlmfc\include\atlwin.h:3602
			template <class TBase, class TWinTraits>
			HWND CWindowImplBaseT< TBase, TWinTraits >::Create(
				_In_opt_ HWND hWndParent,
				_In_ _U_RECT rect,
				_In_z_ LPCTSTR szWindowName,
				_In_ DWORD dwStyle,
				_In_ DWORD dwExStyle,
				_In_ _U_MENUorID MenuOrID,
				_In_ ATOM atom,
				_In_opt_ LPVOID lpCreateParam)
			{
				ATLASSUME(this->m_hWnd == NULL);

				// Allocate the thunk structure here, where we can fail gracefully.
				BOOL result = this->m_thunk.Init(NULL,NULL);
				if (result == FALSE) {
					SetLastError(ERROR_OUTOFMEMORY);
					return NULL;
				}

				if(atom == 0)
					return NULL;

				_AtlWinModule.AddCreateWndData(&this->m_thunk.cd, this);

				if(MenuOrID.m_hMenu == NULL && (dwStyle & WS_CHILD))
					MenuOrID.m_hMenu = (HMENU)(UINT_PTR)this;
				if(rect.m_lpRect == NULL)
					rect.m_lpRect = &TBase::rcDefault;

				HWND hWnd = ::CreateWindowEx(dwExStyle, MAKEINTATOM(atom), szWindowName,
					dwStyle, rect.m_lpRect->left, rect.m_lpRect->top, rect.m_lpRect->right - rect.m_lpRect->left,
					rect.m_lpRect->bottom - rect.m_lpRect->top, hWndParent, MenuOrID.m_hMenu,
					_AtlBaseModule.GetModuleInstance(), lpCreateParam);

				ATLASSUME(this->m_hWnd == hWnd);

				return hWnd;
			}

			*/

			// TODO: 4711 is an ATOM here, used for ID elsewhere.
			VERIFY( m_preview.Create( m_hWnd, rc, _T( "" ), WS_CHILD | WS_VISIBLE, 0, 4711, nullptr) );

			VERIFY( CWnd::ModifyStyle( 0, WS_CLIPCHILDREN ) );
			}
		CButton::OnPaint( );
		}

	afx_msg void OnDestroy( ) {
		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
		//If the window handle identifies an existing window, the return value is nonzero.
		//If the window handle does not identify an existing window, the return value is zero.
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
			NMHDR hdr = { m_hWnd, static_cast< UINT_PTR >( dialog_ctrl_id ), COLBN_CHANGED };
			TRACE( _T( "Color button clicked! Sending WM_NOTIFY to Dialog with Ctrl ID: %llu\r\n" ), static_cast<ULONGLONG>( hdr.idFrom ) );

			//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
			//If the window handle identifies an existing window, the return value is nonzero.
			//If the window handle does not identify an existing window, the return value is zero.
			ASSERT( ::IsWindow( m_hWnd ) );

			//WM_NOTIFY message: https://msdn.microsoft.com/en-us/library/windows/desktop/bb775583.aspx
			//The return value is ignored except for notification messages that specify otherwise.
			::SendMessageW( m_hWnd, WM_NOTIFY, static_cast< WPARAM >( dialog_ctrl_id ), reinterpret_cast<LPARAM>( &hdr ) );
			}
		}

	afx_msg void OnEnable(const BOOL bEnable) noexcept;

	};
#endif
