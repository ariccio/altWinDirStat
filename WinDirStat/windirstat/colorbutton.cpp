// colorbutton.cpp	- Skeleton implementation of CColorButton

#include "stdafx.h"

#ifndef COLORBUTTON_H
#include "colorbutton.h"
#else
#error ass!
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CColorButton::CPreview, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( CColorButton, CButton )
	ON_WM_PAINT( )
	ON_WM_DESTROY( )
	ON_CONTROL_REFLECT( BN_CLICKED, OnBnClicked )
	ON_WM_ENABLE( )
END_MESSAGE_MAP( )
