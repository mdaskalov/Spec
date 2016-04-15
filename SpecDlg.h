// SpecDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

// CSpecDlg dialog
class CSpecDlg : public CDialog
{
// Construction
public:
	CSpecDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SPEC_DIALOG };

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
public:
  virtual BOOL DestroyWindow();
  CSliderCtrl m_BarTime;
  CSliderCtrl m_PeakTime;
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnClose();
  afx_msg void OnDestroy();
  CSliderCtrl m_Progress;
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnBnClickedStart();
  afx_msg void OnBnClickedStop();
  afx_msg void OnBnClicked3d();
  afx_msg void OnBnClickedRbRecord();
  afx_msg void OnBnClickedRbTest();
  afx_msg void OnMove(int x, int y);
  afx_msg void OnBnClickedStartstop();
};
