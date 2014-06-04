/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */

// OSImageToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OSImageTool.h"
#include "OSImageToolDlg.h"

#include "debug.h"

#include "osimagetooldlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//----------------------------------------------------------------------------

COSImageToolDlg::COSImageToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COSImageToolDlg::IDD, pParent),
	m_bCancelled(false), m_bDoQuit(false), m_bDoCopy(false),
	m_src(&m_cSource), m_dst(&m_cDestination)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
COSImageToolDlg::~COSImageToolDlg()
{

}

void COSImageToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_cProgress);
	DDX_Control(pDX, IDC_STATIC_TIMELEFT, m_cTimeLeft);
	DDX_Control(pDX, IDC_STATIC_SPEED, m_cCopyingSpeed);
	DDX_Control(pDX, IDC_COMBO_DESTINATION, m_cDestination);
	DDX_Control(pDX, IDC_COMBO_SOURCE, m_cSource);
	DDX_Control(pDX, IDOK, m_cButtonCopy);
	DDX_Control(pDX, IDC_BROWSE_DESTINATION_FILE, m_cBrowseDestination);
	DDX_Control(pDX, IDC_BROWSE_SOURCE_FILE, m_cBrowseSource);
	DDX_Control(pDX, IDC_STATIC_SPEED_UNIT, m_cSpeedUnit);
	DDX_Control(pDX, IDC_DESCRIPTION_TEXT, m_cHelpText);
}

BEGIN_MESSAGE_MAP(COSImageToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BROWSE_SOURCE_FILE, OnBnClickedBrowseSourceFile)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BROWSE_DESTINATION_FILE, OnBnClickedBrowseDestinationFile)
END_MESSAGE_MAP()



// COSImageToolDlg message handlers

BOOL COSImageToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	debug("oninitdialog\n");
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_bCancelled= false;
	m_bDoCopy= false;
	m_bDoQuit= false;

	m_cProgress.SetRange(0, 100);

	m_cHelpText.SetWindowText(
"Using this tool, you can copy the Operating System to and from files, "
"device memory, or bootloader-format SD-cards "
"(either through the device or using an attached SD-card reader/writer).\n"
"\n"
"Enter a source and destination by typing file names, selecting from the dropdown list, "
"or by using the browse button to select\n"
"a file.\n"
);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COSImageToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COSImageToolDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COSImageToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void RunFileDialog(CComboBox& cCombo, bool bMustExist)
{
	CFileDialog cd( true, NULL, NULL, (bMustExist?OFN_FILEMUSTEXIST:0)|OFN_ENABLESIZING, 
		"Windows CE (*.nb1)|*.nb1|"
		"ROM update files (*.nbf)|*.nbf|"
		"BIN Flash files (*.bin)|*.bin|"
		"SD card images (*.img)|*.img|"
		"All files (*.*)|*.*||" );

	TCHAR szCurDir[1024]; szCurDir[0]=0;

	CString curfile;
	cCombo.GetWindowText(curfile);

	if (curfile.IsEmpty())
	{
		if (GetCurrentDirectory(1024, szCurDir)) {
			cd.m_pOFN->lpstrInitialDir= szCurDir;
		}
	}
	else {
		cd.m_pOFN->lpstrInitialDir= curfile;
	}
	if( cd.DoModal()==IDOK )
	{
		cCombo.SetCurSel(CB_ERR);
		cCombo.SetWindowText(cd.GetPathName());
	}
}
void COSImageToolDlg::OnBnClickedBrowseSourceFile()
{
	RunFileDialog(m_cSource, true);
}
void COSImageToolDlg::OnBnClickedBrowseDestinationFile()
{
	RunFileDialog(m_cDestination, false);
}

// from afxpriv.h
#define WM_KICKIDLE         0x036A
bool COSImageToolDlg::PumpMessages(bool bIdle)
{
	MSG msg;
	int idlecount= 0;
	// first try to be idle
	while ( !PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
	{ 
		if (!bIdle)
			return false;

		if (!this->SendMessage(WM_KICKIDLE, MSGF_DIALOGBOX, idlecount++))
			break;
	}
	
	// then process messages
	do
	{ 
		if (!AfxPumpMessage())
		{
			AfxPostQuitMessage(0);
			return true;
		}
	} while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) ;


	return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void COSImageToolDlg::SetPercentage(double percentage)
{
	m_cProgress.SetPos((int)percentage);
}

void COSImageToolDlg::SetSpeed(double bps)
{
	CString str;
	CString unit;
	if (bps<1000)
	{
		str.Format("%7.2f", bps);
		unit= "B/s";
	}
	else if (bps<1000000)
	{
		str.Format("%7.2f", bps/1000);
		unit= "kB/s";
	}
	else if (bps<1000000000)
	{
		str.Format("%7.2f", bps/1000000);
		unit= "MB/s";
	}
	else
	{
		str.Format("%7.2f", bps/1000000000);
		unit= "GB/s";
	}
	m_cCopyingSpeed.SetWindowText(str);
	m_cSpeedUnit.SetWindowText(unit);
}

void COSImageToolDlg::SetRemainingTime(long time)
{
	CString str;
	str.Format("%2d:%02d", time/60, time%60);
	m_cTimeLeft.SetWindowText(str);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------


void COSImageToolDlg::EnableInterface(bool bEnabled)
{
	m_cDestination.EnableWindow(bEnabled);
	m_cSource.EnableWindow(bEnabled);
	m_cButtonCopy.EnableWindow(bEnabled);
	m_cBrowseSource.EnableWindow(bEnabled);
	m_cBrowseDestination.EnableWindow(bEnabled);
}

void COSImageToolDlg::OnBnClickedOk()
{
	m_bCancelled= false;
	m_bDoQuit= false;
	m_bDoCopy= true;
}


void COSImageToolDlg::OnBnClickedCancel()
{
	m_bCancelled= true;

	if (m_cButtonCopy.IsWindowEnabled())
		m_bDoQuit= true;
}
