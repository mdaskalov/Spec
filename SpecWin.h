// SpecWin.h : header file
//

#include "Settings.h"

/////////////////////////////////////////////////////////////////////////////
// CSpecWin window

class CSpecWin : public CStatic
{
	friend class CSpecDlg;
public:
	int 			 m_Spec[MAXSPEC+1];
	int 			 m_BarDly;
	int 			 m_PckDly;
	BOOL       m_3dView;
protected:
	CDC*       m_SpecWinDC;
  CRect      m_rect;

	short int m_STAB[(MAXSPEC + 1)*MAXWAVE];
	PBYTE	     m_SQRTAB;
	HGLOBAL		 m_hSTable;
	HGLOBAL 	 m_hSQTable;
	CPoint		 m_SpecLine[LDEPTH][MAXLINE];
	int 			 m_oSpec[MAXSPEC+1];
	int 			 m_PckY[MAXSPEC+1];
	int 			 m_PckT[MAXSPEC+1];
	int 			 m_cSpecY[256];
	int				 m_cSpecSY[256];
  int        BARWITH;
  int        BARSTART;

public:
	CSpecWin();
	void CalcSpec();
	void InitSpec();
	void DrawSpec();
	void ClearSpec();
	void Toggle3dView();
	void DrawSpec3d();

// Implementation
public:
	virtual ~CSpecWin();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpecWin)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////