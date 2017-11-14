// specwin.cpp : implementation file
//

#include "stdafx.h"
#include "specwin.h"
#include "resource.h"
#include <math.h>


extern PBYTE gWave;
extern int   gPlaying; // 0=Stop, 1=Play

CBrush     g_br(_SPECBAR);
CBrush     g_bk(_SPECBKG);
CPen       g_pk(PS_SOLID, 0, _SPECPCK);
CPen       g_pb(PS_SOLID, 0, _SPECBKG);
CPen       g_blk(PS_SOLID, 0, _SPECBLK);
CPen       g_pbr(PS_SOLID, 0, _SPECBAR);
CPen*      g_pOldPen;


/////////////////////////////////////////////////////////////////////////////
// CSpecWin

CSpecWin::CSpecWin()
{
	m_SpecWinDC = NULL;
	m_BarDly = 55;
	m_PckDly = 60;
	m_3dView = FALSE;
	BARWITH = 8;
	BARSTART = 1;
}

CSpecWin::~CSpecWin()
{
	if (m_hSTable != NULL) FreeResource(m_hSTable);
	if (m_hSQTable != NULL) FreeResource(m_hSQTable);
	gPlaying = 0;
}

BEGIN_MESSAGE_MAP(CSpecWin, CStatic)
	//{{AFX_MSG_MAP(CSpecWin)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSpecWin::InitSpec()
{
	HRSRC hRes; // resource handle to Sine tables

	short int *t;

	hRes = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_STABLE), RT_RCDATA);
	m_hSTable = ::LoadResource(AfxGetResourceHandle(), hRes);
	DWORD size = ::SizeofResource(AfxGetResourceHandle(), hRes);
	t = (short int*)::LockResource(m_hSTable);

	if (t == NULL) AfxMessageBox(IDS_NO_SINE_TABLE);

	short int TAB[(MAXSPEC + 1)*MAXWAVE];

	for (long i = 0; i < (MAXSPEC + 1)*MAXWAVE; i++)
		m_STAB[i] = t[i];

	float a, k1, k2;
	float n;
	long k;
	int s, c;
	long cnt;

	cnt = 0;
	k1 = 1;
	k2 = 3;

	bool ok = true;

	/*
	for (k = 0; k < MAXSPEC; k++) {
		for (n = 0; n < (float)MAXWAVE-1; n++) {
			a = 2 * PI * (k1 * n / MAXWAVE);
			s = round(127 * sin(a)*sin(PI*n / (MAXWAVE-1)));
			c = round(127 * cos(a)*sin(PI*n / (MAXWAVE-1)));
			m_STAB[cnt] = ((s & 0xff) << 8) + (c & 0xff);

			if (TAB[cnt] != m_STAB[cnt])
				ok = false;
			cnt++;
		}
		k1 = k1 + 1;
		k2 = k2 * 1.05946309436;
	}
	*/

	hRes = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_SQRTABLE), RT_RCDATA);
	m_hSQTable = ::LoadResource(AfxGetResourceHandle(), hRes);
	m_SQRTAB = (unsigned char *)::LockResource(m_hSQTable);

	if (m_SQRTAB == NULL) AfxMessageBox(IDS_NO_SQRT_TABLE);

	if (m_SpecWinDC != NULL) {
		delete m_SpecWinDC;
	}

	m_SpecWinDC = new CWindowDC(this);
	g_pOldPen = m_SpecWinDC->SelectObject(&g_pb);

	CRect rect;
	GetClientRect(&rect);

	double waveyl = rect.Height();

	for (int i = 0;i <= 255;i++) {
		m_cSpecY[i] = (int)(waveyl - (i / (256.0 / (double)(waveyl - 3)))) - 1;
		m_cSpecSY[i] = (int)ceil((float)i*0.894f + 0.9f);
	}

	ClearSpec();
}

#define ADJUST(x,k) (int)ceil((float)(x)*(1.0f-(float)k/(float)(MAXSPEC*BARWITH/2.0f))+k)

void CSpecWin::ClearSpec()
{
	int i, x, k;

	int ys[LDEPTH];

	ys[0] = m_cSpecY[0];
	for (k = 1;k < LDEPTH;k++)
		ys[k] = m_cSpecSY[ys[k - 1]];

	x = 2;

	for (k = 0;k < LDEPTH;k++) {
		m_SpecLine[k][0].x = ADJUST(x, k);
		m_SpecLine[k][0].y = ys[k];
	}

	x = 1 + BARWITH / 2;

	for (i = 0;i <= MAXSPEC;i++) {

		m_Spec[i] = m_PckT[i] = 0;
		m_oSpec[i] = m_PckY[i] = ys[0];

		for (k = 0;k < LDEPTH;k++) {
			m_SpecLine[k][i + 1].x = ADJUST(x, k);
			m_SpecLine[k][i + 1].y = ys[k];
		}
		x += BARWITH;
	}

	x -= BARWITH / 2;

	for (k = 0;k < LDEPTH;k++) {
		m_SpecLine[k][i + 1].x = ADJUST(x, k);
		m_SpecLine[k][i + 1].y = ys[k];
	}
}

inline void CSpecWin::DrawSpec3d()
{
	register unsigned char k, n, r;
	register int a, b, w;
	register int Cnt, Sum;

	Cnt = 0;
	GetClientRect(&m_rect);

	for (k = 0;k <= MAXSPEC;k++) { 							// For each bar
		a = b = 0;
		for (n = 0;n <= MAXWAVE - 1;n++) { 					// Calculate the power
			w = gWave[n] - 0x7f;
			a += (int)(m_STAB[Cnt] >> 8)*w;
			b += (int)((char)(m_STAB[Cnt] & 0xff))*w;
			Cnt++;
		}
		a /= GAIN;
		b /= GAIN;
		Sum = (a*a + b*b);

		if (Sum > MAXSQRT)
			r = 255;
		else
			r = (BYTE)m_SQRTAB[Sum];

		for (n = LDEPTH;n > 0;n--) {
			m_SpecLine[n][k + 1].y = m_cSpecSY[m_SpecLine[n - 1][k + 1].y];
		}
		m_SpecLine[0][k + 1].y = m_cSpecY[r];
	}
	m_SpecWinDC->BitBlt(m_rect.left + 1, m_rect.top + 1, m_rect.right, m_rect.bottom, NULL, 0, 0, BLACKNESS);
	m_SpecWinDC->SelectObject(&g_pbr);
	for (k = 0;k < LDEPTH;k++) m_SpecWinDC->Polyline(m_SpecLine[k], MAXLINE);
}

void CSpecWin::DrawSpec()
{
	register unsigned char k, n, r;
	register int w, Cnt, Sum;
	register int a, b, x, ys, ye, yp;
	int *bar;

	if (!m_SpecWinDC) return;

	if (m_3dView) {
		DrawSpec3d();
	}
	else {

		Cnt = 0;
		x = 1;

		ys = m_cSpecY[0];

		for (k = 0;k <= MAXSPEC;k++) { 							// For each bar
			a = b = 0;
			for (n = 0;n <= MAXWAVE - 1;n++) { 					// Calculate the power
				w = gWave[n] - 0x7f;
				a += (int)(m_STAB[Cnt] >> 8)*w;
				b += (int)((char)(m_STAB[Cnt] & 0xff))*w;
				Cnt++;
			}
			a /= GAIN;
			b /= GAIN;
			Sum = (a*a + b*b);
			if (Sum > MAXSQRT)
				r = 0xff;
			else
				r = (unsigned char)m_SQRTAB[Sum];

			if (r > *(bar = &m_Spec[k]) - m_BarDly)				// And the bar length
				*bar = r;
			else
				*bar -= m_BarDly;

			m_PckT[k]++;									// Then draw the peak & bar

			yp = m_oSpec[k];
			ye = m_cSpecY[*bar];

			if (m_PckT[k] >= m_PckDly) {
				m_PckT[k] = 0;
				if (ye > m_PckY[k]) {
					m_SpecWinDC->SelectObject(&g_pb);
					m_SpecWinDC->MoveTo(x + 1, m_PckY[k]);
					m_SpecWinDC->LineTo(x + BARWITH, m_PckY[k]);
				}
				m_PckY[k] = ys;
			}

			if (ye < m_PckY[k]) {
				m_PckT[k] = 0;
				m_PckY[k] = ye;
			}

			if (m_PckY[k] < ys) {
				m_SpecWinDC->SelectObject(&g_pk);
				m_SpecWinDC->MoveTo(x + 1, m_PckY[k]);
				m_SpecWinDC->LineTo(x + BARWITH, m_PckY[k]);
			}

			if (ye != m_oSpec[k]) {
				if (yp < ye) {
					m_rect.SetRect(x + 1, ye, x + BARWITH, yp);
					m_SpecWinDC->FillRect(m_rect, &g_bk);
				}
				else {
					m_rect.SetRect(x + 1, yp + 1, x + BARWITH, ye);
					m_SpecWinDC->FillRect(m_rect, &g_br);
				}
				m_oSpec[k] = ye;
			}
			x += BARWITH;
		}
	}
}

void CSpecWin::Toggle3dView()
{
	int k, x, ys, ye;

	ClearSpec();
	m_3dView = !m_3dView;

	GetClientRect(&m_rect);
	m_SpecWinDC->BitBlt(m_rect.left + 1, m_rect.top + 1, m_rect.right, m_rect.bottom, NULL, 0, 0, BLACKNESS);

	if (m_3dView) {
		m_SpecWinDC->SelectObject(&g_pbr);
		for (k = 0;k < LDEPTH;k++)
			m_SpecWinDC->Polyline(m_SpecLine[k], MAXLINE);
	}
	else {
		m_SpecWinDC->SelectObject(&g_pk);
		ys = m_cSpecY[0];
		x = 1;
		for (k = 0;k <= MAXSPEC;k++) {
			m_rect.SetRect(x + 1, ys + 1, x + BARWITH, m_cSpecY[0xff]);
			m_SpecWinDC->FillRect(m_rect, &g_bk);

			m_SpecWinDC->MoveTo(x + 1, m_PckY[k]);
			m_SpecWinDC->LineTo(x + BARWITH, m_PckY[k]);

			ye = m_oSpec[k];
			m_rect.SetRect(x + 1, ys + 1, x + BARWITH, ye);
			m_SpecWinDC->FillRect(m_rect, &g_br);
			x += BARWITH;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpecWin message handlers

void CSpecWin::OnPaint()
{
	// Signal that Spec is ready
	if (gPlaying == -1) gPlaying = 1;

	int    k, x, ys, ye;
	CPen   pbr(PS_SOLID, 0, _SPECBAR);
	CPen   blk(PS_SOLID, 0, _SPECBLK);
	CPen   pk(PS_SOLID, 0, _SPECPCK);
	CPen*  pOldPen;
	CBrush bk(_SPECBKG);
	CBrush br(_SPECBAR);
	CRect  rect;

	CPaintDC dc(this); // device context for painting

	GetClientRect(&rect);

	BARWITH = rect.Width() / MAXSPEC;
	BARSTART = (rect.Width() - MAXSPEC*BARWITH) / 2;

	dc.BitBlt(rect.left, rect.top, rect.right, rect.bottom, NULL, 0, 0, BLACKNESS);

	if (m_3dView) {
		pOldPen = dc.SelectObject(&pbr);
		for (k = 0;k < LDEPTH;k++)
			dc.Polyline(m_SpecLine[k], MAXLINE);
	}
	else {
		ys = m_cSpecY[0];
		x = 0;
		pOldPen = dc.SelectObject(&pk);
		for (k = 0;k <= MAXSPEC;k++) {
			rect.SetRect(x + 1, ys, x + BARWITH, m_cSpecY[0xff] - 1);
			dc.FillRect(rect, &bk);

			dc.MoveTo(x + 1, m_PckY[k] - 1);
			dc.LineTo(x + BARWITH, m_PckY[k] - 1);

			ye = m_oSpec[k];
			rect.SetRect(x + 1, ys, x + BARWITH, ye - 1);
			dc.FillRect(rect, &br);

			x += BARWITH;
		}
	}
	dc.SelectObject(pOldPen);
}

void CSpecWin::OnDestroy()
{
	CStatic::OnDestroy();

	m_SpecWinDC->SelectObject(g_pOldPen);

	if (m_SpecWinDC != NULL) {
		delete m_SpecWinDC;
		m_SpecWinDC = NULL;
	}
}
