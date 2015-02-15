// layout.h	- Declaration of CLayout
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_LAYOUT_H
#define WDS_LAYOUT_H

// CLayout. A poor men's dialog layout mechanism.
// Simple, flat, and sufficient for our purposes.
class CLayout {
	struct SControlInfo {
		SControlInfo( CWnd* control_in, DOUBLE movex_in, DOUBLE movey_in, DOUBLE stretchx_in, DOUBLE stretchy_in, RECT originalRectangle_in ) : control( control_in ), movex( std::move( movex_in ) ), movey( std::move( movey_in ) ), stretchx ( std::move( stretchx_in ) ), stretchy( std::move( stretchy_in ) ), originalRectangle( std::move( originalRectangle_in ) ) { }

		SControlInfo& operator=( const SControlInfo& in ) = delete;
		CWnd*  control;
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



	class CSizeGripper final : public CWnd {
	public:
		static const INT _width;

		CSizeGripper( ) { }
		CSizeGripper& operator=( const CSizeGripper& in ) = delete;
		CSizeGripper( const CSizeGripper& in ) = delete;

		//CSizeGripper( );
#pragma warning( suppress: 4263 )
		void Create( _Inout_ CWnd* parent, _In_ const RECT rc ) {
			VERIFY( CWnd::Create( AfxRegisterWndClass( 0, AfxGetApp( )->LoadStandardCursor( IDC_ARROW ), ( HBRUSH ) ( COLOR_BTNFACE + 1 ), 0 ), _T( "" ), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, rc, parent, IDC_SIZEGRIPPER ) );
			}

	private:
		void DrawShadowLine( _In_ CDC& pdc, _In_ WTL::CPoint start, _In_ WTL::CPoint end );

		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint();
		afx_msg LRESULT OnNcHitTest( CPoint point );
		};



	//size_t  AddControl  ( _In_       CWnd*       control,  _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy );
	void AddControl     ( _In_ const UINT        id,       _In_ const DOUBLE movex, _In_ const DOUBLE movey, _In_ const DOUBLE stretchx, _In_ const DOUBLE stretchy );
	void OnInitDialog   ( _In_ const bool        centerWindow                                                                                                       );
	
	void OnGetMinMaxInfo( _Out_    MINMAXINFO* mmi ) const {
		mmi->ptMinTrackSize.x = m_originalDialogSize.cx;
		mmi->ptMinTrackSize.y = m_originalDialogSize.cy;
		}
	
	void OnDestroy ( );
	void OnSize    ( );
protected:
	CWnd*                               m_dialog;
	PCWSTR const                        m_name;
	WTL::CSize                          m_originalDialogSize;
	std::vector<SControlInfo>           m_control;
	CSizeGripper                        m_sizeGripper;
	};


#else


#endif
