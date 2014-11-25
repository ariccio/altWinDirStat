
#ifndef ABOUTDLG_H
#define ABOUTDLG_H
#else
#error ass
#endif

#pragma once
#include "stdafx.h"



void StartAboutDialog();

class CAboutThread : public CWinThread {
	DECLARE_DYNCREATE(CAboutThread);
protected:
	virtual BOOL InitInstance( ) override;
	};

