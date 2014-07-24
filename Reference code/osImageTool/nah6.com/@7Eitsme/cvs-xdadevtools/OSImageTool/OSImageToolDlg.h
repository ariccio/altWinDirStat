/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */


#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "resource.h"
#include "ComboBoxControl.h"

class COSImageToolDlg : public CDialog
{

public:
	COSImageToolDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~COSImageToolDlg();

	// interface from app to dialog
	void EnableInterface(bool bEnabled);

	// interface from copier to dialog
	void SetPercentage(double percentage);
	void SetSpeed(double bps);
	void SetRemainingTime(long time);

    bool PumpMessages(bool bIdle);

	bool m_bCancelled;
	bool m_bDoCopy;
	bool m_bDoQuit;

	ComboBoxControl m_src;
	ComboBoxControl m_dst;

// Dialog Data
	enum { IDD = IDD_OSIMAGETOOL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


private:
	CProgressCtrl m_cProgress;
	afx_msg void OnBnClickedBrowseSourceFile();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CStatic m_cTimeLeft;
	CStatic m_cCopyingSpeed;
	CComboBox m_cDestination;
	CComboBox m_cSource;
	afx_msg void OnBnClickedBrowseDestinationFile();
	CButton m_cButtonCopy;
	CButton m_cBrowseDestination;
	CButton m_cBrowseSource;


public:
	CStatic m_cSpeedUnit;
	CStatic m_cHelpText;
};
