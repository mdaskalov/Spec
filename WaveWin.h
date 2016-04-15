// WaveWin.h : header file
//

#include "Settings.h"

/////////////////////////////////////////////////////////////////////////////
// CWaveWin window

class CWaveWin : public CStatic
{
	friend class CSpecDlg;
// Construction
public:
	CWaveWin();

// Attributes
protected:
	CDC*    m_WaveWinDC;
	int     m_oWave[MAXWAVE+1];
	int     m_cWaveY[256];
// Operations
public:
	void InitWave();
	void DrawWave();
	void ClearWave();
	void GenerateWave(double x);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveWin)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWaveWin();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWaveWin)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
