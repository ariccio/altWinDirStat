#pragma once



#ifndef ABOUTDLG_H
#define ABOUTDLG_H


#include "stdafx.h"

//#include "layout.h"


void StartAboutDialog();

class CAboutThread : public CWinThread {
	DECLARE_DYNCREATE(CAboutThread);
protected:
	virtual BOOL InitInstance( ) override;
	};

#else
#error ass
#endif
