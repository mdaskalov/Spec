// wavewin.cpp : implementation file
//

#include "stdafx.h"
#include "wavewin.h"
#include <math.h>

extern PBYTE gWave;
extern int   gPlaying; // 0=Stop, 1=Record, 2=Test
extern BYTE  gStaticWave[MAXWAVE+1];  // Static wave buffer

/////////////////////////////////////////////////////////////////////////////
// CWaveWin

CWaveWin::CWaveWin()
{
	m_WaveWinDC = NULL;
}

CWaveWin::~CWaveWin()
{
  gPlaying=0;
}


BEGIN_MESSAGE_MAP(CWaveWin, CStatic)
	//{{AFX_MSG_MAP(CWaveWin)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CWaveWin::InitWave() 
{
	if (m_WaveWinDC!=NULL) {
		delete m_WaveWinDC;
	}

	m_WaveWinDC = new CWindowDC(this);

	CRect rect;
	GetClientRect(&rect);
	
	double waveyl = rect.Height();

	for(int i=0;i<=255;i++) 
		m_cWaveY[i]=(int)(waveyl-(i/(256.0/(double)(waveyl-2))));

	ClearWave();
}

void CWaveWin::ClearWave() 
{
	for(int i=0;i<MAXWAVE+1;i++) {
		gWave[i]=0x7f;
		m_oWave[i]=0x80;
	}
}

void CWaveWin::GenerateWave(double x)
{
	double Step=MAXWAVE/2.0*3.1415926/x;

	for(int i=0;i<MAXWAVE+1;i++) {
		gWave[i]=(BYTE)(128*sin(i/Step))-0x80;

		if (gWave[i]>0xFF) gWave[i]=0xFF;
	}
}

void CWaveWin::DrawWave()
{
	register unsigned char x,y,o,m;

  if (!m_WaveWinDC) return;

	m=m_cWaveY[0x7F];

	for(x=0;x<MAXWAVE+1;x++) {
    gStaticWave[x]=gWave[x];
		y=m_cWaveY[gWave[x]];
		o=m_oWave[x];
		if (y != o)	{
			m_WaveWinDC->SetPixelV(x+2,o,_WAVEBKG);
			m_WaveWinDC->SetPixelV(x+2,y,_WAVEDAT);
		}
		m_oWave[x] = y;
	
		if (y != m)
			m_WaveWinDC->SetPixelV(x+2,m,_WAVENUL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWaveWin message handlers

void CWaveWin::OnPaint() 
{
  // Signal that Wave is ready
  if (gPlaying==-2) gPlaying=-1;

  int x,y,m;
  CBrush bk(_WAVEBKG);

	CPaintDC dc(this); // device context for painting

  CRect rect;
	GetClientRect(&rect);


	CBrush* pOldBrush=dc.SelectObject(&bk);
	dc.Rectangle(rect);
  dc.SelectObject(pOldBrush);

	m=m_cWaveY[0x7F]-1;

	for(x=0;x<MAXWAVE+1;x++) {
		y=m_oWave[x]-1;
	  dc.SetPixelV(x+1,y,_WAVEDAT);
		if (y != m)
			dc.SetPixelV(x+1,m,_WAVENUL);
	}

	// Do not call CStatic::OnPaint() for painting messages
}

void CWaveWin::OnDestroy() 
{
	CStatic::OnDestroy();
	
	if (m_WaveWinDC!=NULL) {
		delete m_WaveWinDC;
		m_WaveWinDC = NULL;
	}
}