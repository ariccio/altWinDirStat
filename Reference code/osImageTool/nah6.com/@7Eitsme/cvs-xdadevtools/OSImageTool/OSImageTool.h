/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "ComboBoxControl.h"
#include "OSImageToolDlg.h"

#include "OsImageReader.h"
#include "OsImageWriter.h"

#include "SdCardDevice.h"

#include <string>
#include <vector>

#include "stringutils.h"

class COSImageToolApp : public CWinApp
{
public:
	COSImageToolApp();
	virtual ~COSImageToolApp();

	public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

private:
	std::vector<SdCardDevice*> m_sdcards;

	std::string m_cmdsourcename;
	std::string m_cmdtargetname;
	bool m_bDoRegisterSelf;
	bool m_bAuto;
	bool m_bChecksize;

	bool ParseCommandLine(const std::string& cmdline);
	void usage();

	bool FindListIndex(const string& str, int& index);
	bool GetFriendlyNameList(StringList& list);

	OsImageWriter* GetWriter(const ComboBoxControl& cb);
	OsImageReader* GetReader(const ComboBoxControl& cb);

	bool DoCopy(COSImageToolDlg& dlg);
};

extern COSImageToolApp theApp;
