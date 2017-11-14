// SpecDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Spec.h"
#include "SpecDlg.h"
#include "WaveWin.h"
#include "SpecWin.h"
#include <mmsystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int      gSource;   // 0=Audio, 1=Test
extern int      gPlaying;  // 0=Stop, 1=Play, -1=Wait SpecWin, -2=Wait both Wave & Spec Wins
extern BOOL     gQuit;     // Signal sampling thread to exit
extern PBYTE    gWave;     // Pointer to the current sampled buffer
extern BYTE     gStaticWave[MAXWAVE + 1];  // Static wave buffer

extern CWaveWin gWaveWin;
extern CSpecWin gSpecWin;

extern void StartSampling();
extern void StopSampling();

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CSpecDlg dialog

CSpecDlg::CSpecDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpecDlg::IDD, pParent)
{
	gSource = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SPECICO);
}

void CSpecDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_PEAK, m_PeakTime);
	DDX_Control(pDX, IDC_BAR, m_BarTime);
	DDX_Control(pDX, IDC_WAVE, gWaveWin);
	DDX_Control(pDX, IDC_SPEC, gSpecWin);
	DDX_Radio(pDX, IDC_RB_RECORD, gSource);
}

BEGIN_MESSAGE_MAP(CSpecDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_VSCROLL()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_3D, OnBnClicked3d)
	ON_BN_CLICKED(IDC_RB_RECORD, OnBnClickedRbRecord)
	ON_BN_CLICKED(IDC_RB_TEST, OnBnClickedRbTest)
	ON_WM_MOVE()
	ON_BN_CLICKED(IDC_STARTSTOP, OnBnClickedStartstop)
END_MESSAGE_MAP()


// CSpecDlg message handlers

BOOL CSpecDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

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

	m_Progress.SetRange(0, 7200);
	m_Progress.SetTicFreq(800);
	m_Progress.SetPos(0);

	m_BarTime.SetRange(1, 128);
	m_BarTime.SetPos(gSpecWin.m_BarDly);

	m_PeakTime.SetRange(0, 300);
	m_PeakTime.SetPos(300 - gSpecWin.m_PckDly);

	gWaveWin.InitWave();
	gSpecWin.InitSpec();
	StartSampling();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSpecDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSpecDlg::OnPaint()
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
HCURSOR CSpecDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSpecDlg::OnBnClickedStartstop()
{
	if (gPlaying == 0)
		gPlaying = 1;
	else if (gPlaying == 1)
		gPlaying = 0;
}

void CSpecDlg::OnBnClicked3d()
{
	gSpecWin.Toggle3dView();
	if (gSpecWin.m_3dView) {
		m_PeakTime.EnableWindow(FALSE);
		m_BarTime.EnableWindow(FALSE);
	}
	else {
		m_PeakTime.EnableWindow();
		m_BarTime.EnableWindow();
	}
}

void CSpecDlg::OnBnClickedRbRecord()
{
	gSource = 0;
	gPlaying = 1;
	gWave = gStaticWave;
	gWaveWin.GenerateWave(0);
	m_Progress.SetPos(0);
	m_Progress.EnableWindow(FALSE);
}

void CSpecDlg::OnBnClickedRbTest()
{
	gSource = 1;
	gPlaying = 0;
	gWave = gStaticWave;
	gWaveWin.GenerateWave(0);
	m_Progress.EnableWindow();
}

void CSpecDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	gPlaying = 0;
	gWaveWin.GenerateWave(m_Progress.GetPos() / 10.0);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSpecDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	gSpecWin.m_PckDly = 300 - m_PeakTime.GetPos();
	gSpecWin.m_BarDly = m_BarTime.GetPos();

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CSpecDlg::OnClose()
{
	gQuit = TRUE;
	gPlaying = 0;

	CDialog::OnClose();
}

void CSpecDlg::OnDestroy()
{
	CDialog::OnDestroy();

	gQuit = TRUE;
	gPlaying = 0;
}

BOOL CSpecDlg::DestroyWindow()
{
	gQuit = TRUE;
	StopSampling();
	return CDialog::DestroyWindow();
}

void CSpecDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	if (gPlaying == 1) gWaveWin.OnPaint();
}