/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
// OSImageTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OSImageTool.h"
#include "OSImageToolDlg.h"

#include "ErrorHandler.h"
#include "debug.h"
#include "stringutils.h"
#include "Configuration.h"

#include "OsImageCopier.h"
#include <boost/smart_ptr.hpp>
// for 'GetReader'
#include "OsImageReader.h"
#include "FileOsImageReader.h"
#include "SdOsImageReader.h"
#include "XDAOsImageReader.h"

// for 'GetWriter'
#include "OsImageWriter.h"
#include "FileOsImageWriter.h"
#include "SdOsImageWriter.h"
#include "XDAOsImageWriter.h"

// for creating m_sdcards list
#include "RemoteSdCard.h"
#include "LocalSdCard.h"
#include "DiskInfo.h"

#include "stringutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COSImageToolApp theApp;

BEGIN_MESSAGE_MAP(COSImageToolApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

COSImageToolApp::COSImageToolApp()
	: m_bDoRegisterSelf(false), m_bAuto(false)
{
}

COSImageToolApp::~COSImageToolApp()
{
	for(size_t i=0 ; i<m_sdcards.size() ; i++)
		delete m_sdcards[i];
	m_sdcards.clear();
}


void COSImageToolApp::usage()
{
	MessageBox(0,
		"Usage: osimagetool [-r source] [-w destination] [-a]\n"
		"    source and destination can be one of the following:\n"
		"       - XdaMemory ( can only be source )\n"
		"       - XdaSdSlot\n"
		"       - LocalSdCard\n"
		"       - a filename ( nbf, nb1, bin, img )\n"
		"    destination file can only be nb1\n"
		"    -a  : be as automatic as safely possible\n"
		"    -f  : do not check filesize\n", "", 0);
}

#define HANDLESTROPTION(var) (arg[2] ? !(var= arg.substr(2)).empty() : i+1 != args.end() ? !(var= *(++i)).empty() : 0)

bool COSImageToolApp::ParseCommandLine(const string& cmdline)
{
	StringList args;
	if (!SplitString(cmdline, args, false))
	{
		g_err.Set("Error in commandline");
		return false;
	}

	int argsfound=0;
	for (StringList::iterator i= args.begin() ; i!=args.end() ; ++i)
	{
		string& arg= *i;
		if (arg[0]=='-') switch(arg[1])
		{
		case 'r':
			if (!HANDLESTROPTION(m_cmdsourcename))
			{
				g_err.Set("Error in commandline, '-r' needs a parameter");
				return false;
			}
			break;
		case 'w':
			if (!HANDLESTROPTION(m_cmdtargetname))
			{
				g_err.Set("Error in commandline, '-w' needs a parameter");
				return false;
			}
			break;
		case 'a':
			m_bAuto= true;
			break;
		case 'f':
			g_cfg.bCheckFilesize= false;
			break;
		case '-':
			if (arg=="--register")
				m_bDoRegisterSelf= true;
			else {
				g_err.Set(stringformat("unknown option: %s", arg.c_str()));
				return false;
			}
			break;
		default:
			g_err.Set(stringformat("unknown option: %s", arg.c_str()));
			return false;
		}
		else switch(argsfound)
		{
		case 0:
		default:
			g_err.Set(stringformat("unknown argument: %s", arg.c_str()));
			return false;
		}
	}
	return true;
}


bool COSImageToolApp::GetFriendlyNameList(StringList& list)
{
	// add names to source listbox
	for (size_t i=0 ; i<m_sdcards.size() ; i++)
	{
        std::string friendlyname;
		if (!m_sdcards[i]->GetFriendlyName(friendlyname))
			friendlyname= "error";
			
		list.push_back(friendlyname);
	}
	return true;
}
bool COSImageToolApp::FindListIndex(const string& str, int& index)
{
	if (stringicompare(str, "xdamemory")==0)
		index= (int)m_sdcards.size();
	else if (stringicompare(str, "xdasdslot")==0)
		index= (int)m_sdcards.size()-1;
	else if (stringicompare(str, "localsdcard")==0)
		index= 0;
	else
		return false;

	return true;
}
BOOL COSImageToolApp::InitInstance()
{
	DebugOutputDebugString();

	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	SetRegistryKey("XDA Developers\\OSImageTool");

	if (!ParseCommandLine(string(this->m_lpCmdLine)))
	{
		g_err.Display();
		return false;
	}

	if (m_bDoRegisterSelf)
		return g_cfg.RegisterSelf();


	DiskList list;
	DiskInfo::GetDiskList(list);
	for (DiskList::iterator i= list.begin() ; i!=list.end() ; ++i)
		m_sdcards.push_back(new LocalSdCard(*i));

	m_sdcards.push_back(new RemoteSdCard());

	debug("before create\n");
	COSImageToolDlg dlg;
	dlg.Create(dlg.IDD, NULL);
	m_pMainWnd = &dlg;

	StringList namelist;
	GetFriendlyNameList(namelist);
	namelist.push_back("Device Memory (current rom)");
	dlg.m_src.Initialize(namelist);

	namelist.pop_back();

	namelist.push_back("Device Memory (via Programme A)");
	dlg.m_dst.Initialize(namelist);

	int index;
	if (FindListIndex(m_cmdsourcename, index))
		dlg.m_src.SetSelection(index);
	else
		dlg.m_src.SetText(m_cmdsourcename);
	if (FindListIndex(m_cmdtargetname, index))
		dlg.m_dst.SetSelection(index);
	else
		dlg.m_dst.SetText(m_cmdtargetname);
	
	// if only 1 local sdcard and target is localsdcard
	m_bAuto = m_bAuto && (
		(m_sdcards.size()==2 && m_cmdtargetname=="localsdcard")
		|| m_cmdtargetname=="xdasdslot"
		|| m_cmdtargetname=="xdamemory");

	dlg.ShowWindow(SW_SHOW);

	while (!dlg.PumpMessages(true))
	{
		if (dlg.m_bDoCopy || m_bAuto)
		{
			dlg.EnableInterface(false);
			if (!DoCopy(dlg))
				g_err.Display();
			dlg.EnableInterface(true);
			m_bAuto= false;
		}
		else if (dlg.m_bDoQuit)
			break;
		dlg.m_bDoCopy= false;
		dlg.m_bCancelled= false;
	}

	return FALSE;
}

OsImageWriter* COSImageToolApp::GetWriter(const ComboBoxControl& cb)
{
	string str;
	int sel;
	if (cb.GetSelection(sel))
	{
		debug("dstsel= %d\n", sel);
		if (sel==m_sdcards.size())
			return new XDAOsImageWriter();
		else
			return new SdOsImageWriter(m_sdcards[sel]);
	}

	if (cb.GetText(str))
	{
		OsImageWriter* pwriter= FileOsImageWriter::GetFileWriter(str);
		if (pwriter==NULL)
			g_err.Set(stringformat("don't know how to write destinationfile %s", str.c_str()));
		return pwriter;
	}
	g_err.Set("no target selected");
	return NULL;
}

OsImageReader* COSImageToolApp::GetReader(const ComboBoxControl& cb)
{
	string str;
	int sel;
	if (cb.GetSelection(sel))
	{
		debug("srcsel= %d\n", sel);
		if (sel==m_sdcards.size())
			return new XDAOsImageReader();
		else
			return new SdOsImageReader(m_sdcards[sel]);
	}

	if (cb.GetText(str))
	{
		OsImageReader* preader= FileOsImageReader::GetFileReader(str);
		if (preader==NULL)
			g_err.Set(stringformat("don't know how to read sourcefile %s", str.c_str()));
		return preader;
	}
	g_err.Set("no source selected");
	return NULL;
}

bool COSImageToolApp::DoCopy(COSImageToolDlg& dlg)
{
	boost::scoped_ptr<OsImageWriter> writer(GetWriter(dlg.m_dst));
	boost::scoped_ptr<OsImageReader> reader(GetReader(dlg.m_src));

	if (writer==NULL || reader==NULL)
		return false;

	if (!writer.get()->CheckSpace())
		return false;

	OsImageCopier copier(&dlg);

	return copier.copy(reader.get(), writer.get());
}
