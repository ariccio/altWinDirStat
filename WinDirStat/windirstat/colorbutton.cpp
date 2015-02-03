// colorbutton.cpp	- Skeleton implementation of CColorButton

#pragma once

#include "stdafx.h"
#include "colorbutton.h"

#ifndef WDS_COLORBUTTON_CPP
#define WDS_COLORBUTTON_CPP

BEGIN_MESSAGE_MAP(CColorButton::CPreview, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP( CColorButton, CButton )
	ON_WM_PAINT( )
	ON_WM_DESTROY( )
	ON_CONTROL_REFLECT( BN_CLICKED, &( CColorButton::OnBnClicked ) )
	ON_WM_ENABLE( )
END_MESSAGE_MAP( )

#else

#endif