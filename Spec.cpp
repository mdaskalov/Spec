// Spec.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Spec.h"
#include "SpecDlg.h"
#include "WaveWin.h"
#include "SpecWin.h"
#include <mmsystem.h>
#include <dsound.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSpecApp

BEGIN_MESSAGE_MAP(CSpecApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

int       gSource;  // 0=Audio, 1=Test
BOOL      gPlaying; // 0=Stop, 1=Play
BOOL      gQuit;    // Signal sampling thread to exit
PBYTE     gWave;    // Pointer to the current sampled buffer
BYTE      gStaticWave[MAXWAVE + 1]; // Static wave buffer

CSpecApp  theApp;
CWaveWin  gWaveWin;
CSpecWin  gSpecWin;

// Direct Sound stuff

LPDIRECTSOUNDCAPTUREBUFFER pDSCB;
LPDIRECTSOUNDCAPTURE       pDSC;
LPDIRECTSOUNDNOTIFY        pDSNotify;
DSBPOSITIONNOTIFY          aPosNotify[NUM_BUFFERS + 1];
HANDLE                     hNotificationEvent;


// -------------------------------------------------------------------------

VOID ThreadFunction(LPVOID lpParam)
{
	HRESULT hr;
	DWORD dwResult, dwOffs;
	VOID* pbData1;
	VOID* pbData2;
	DWORD dwLen1;
	DWORD dwLen2;

	dwOffs = 0;
	while (!gQuit) {
		dwResult = MsgWaitForMultipleObjects(1, &hNotificationEvent, FALSE, INFINITE, QS_ALLEVENTS);

		switch (dwResult) {
		case WAIT_OBJECT_0:
			hr = pDSCB->Lock(dwOffs, MAXWAVE, &pbData1, &dwLen1, &pbData2, &dwLen2, 0L);

			if (gSource == 1) {
				// gWaveWin.GenerateWave(m_Progress.GetPos()/10.0);
			}
			else
				gWave = (gPlaying == 1) ? (PBYTE)pbData1 : gStaticWave;
			gWaveWin.DrawWave();
			gSpecWin.DrawSpec();

			hr = pDSCB->Unlock(pbData1, dwLen1, pbData2, dwLen2);
			dwOffs += (MAXWAVE + 1);
			dwOffs %= ((MAXWAVE + 1)*NUM_BUFFERS); // Circular buffer

			break;
		}
	}
}

void InitDirectSound()
{
	DWORD IDThread;
	HRESULT hr;
	DSCBUFFERDESC dscbd;
	WAVEFORMATEX  wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	// Create direct sound capture interface
	pDSC = NULL;
	if ((hr = DirectSoundCaptureCreate(&DSDEVID_DefaultCapture, &pDSC, NULL)) != S_OK) {
		switch (hr) {
		case DSERR_BADFORMAT:
		case DSERR_INVALIDPARAM:
			AfxMessageBox(IDS_BADFORMAT);
			break;
		case DSERR_NODRIVER:
			AfxMessageBox(IDS_BADDEVICE);
			break;
		case DSERR_ALLOCATED:
			AfxMessageBox(IDS_DEVINUSE);
			break;
		case DSERR_OUTOFMEMORY:
			AfxMessageBox(IDS_NOMEMORY);
			break;
		default:
			AfxMessageBox(IDS_OPENDEVERR);
		}
	}

	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = 0;
	dscbd.dwBufferBytes = (MAXWAVE + 1)*NUM_BUFFERS;
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &wfx;
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;

	// pDSC is a valid IDirectSoundCapture interface pointer. Create sound capture buffer.
	pDSCB = NULL;
	if ((hr = pDSC->CreateCaptureBuffer(&dscbd, &pDSCB, NULL)) != S_OK) {
		switch (hr) {
		case DSERR_NOAGGREGATION:
			AfxMessageBox(IDS_BADDEVICE);
			break;
		case DSERR_ALLOCATED:
			AfxMessageBox(IDS_DEVINUSE);
			break;
		case DSERR_OUTOFMEMORY:
			AfxMessageBox(IDS_NOMEMORY);
			break;
		case DSERR_INVALIDPARAM:
			AfxMessageBox(IDS_BADFORMAT);
			break;
		default:
			AfxMessageBox(IDS_OPENDEVERR);
		}
	}

	// Create Notification Event to signal when a buffer is ready
	hNotificationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Setup the notification positions at each MAXWAVE position
	ZeroMemory(&aPosNotify, sizeof(DSBPOSITIONNOTIFY)*(NUM_BUFFERS + 1));
	for (int i = 0; i < NUM_BUFFERS; i++) {
		aPosNotify[i].dwOffset = (MAXWAVE + 1)*(i + 1) - 1;
		aPosNotify[i].hEventNotify = hNotificationEvent;
	}

	// Get direct sound notify interface
	if (pDSCB->QueryInterface(IID_IDirectSoundNotify, (VOID**)&pDSNotify) != DS_OK)
		AfxMessageBox(IDS_OPENDEVERR);

	// Set the notification positions (at each MAXWAVE)
	if (pDSNotify->SetNotificationPositions(NUM_BUFFERS, aPosNotify) != DS_OK)
		AfxMessageBox(IDS_OPENDEVERR);

	// Create thread to process the notifications
	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunction, &hNotificationEvent, 0, &IDThread) == NULL)
		AfxMessageBox(IDS_THREADERR);

	// And start sampling
	if (pDSCB->Start(DSCBSTART_LOOPING) != DS_OK)
		AfxMessageBox(IDS_OPENDEVERR);
}

void StartSampling()
{
	if (pDSCB->Start(DSCBSTART_LOOPING) != DS_OK)
		AfxMessageBox(IDS_OPENDEVERR);
}

void StopSampling()
{
	if (pDSCB->Stop() != DS_OK)
		AfxMessageBox(IDS_OPENDEVERR);
}

void FreeDirectSound()
{
	// Free DSC Buffer
	if (pDSCB) {
		pDSCB->Release();
		pDSCB = NULL;
	}

	// And then the DSC 
	if (pDSC) {
		pDSC->Release();
		pDSC = NULL;
	}

	CloseHandle(hNotificationEvent);
}

// CSpecApp construction

CSpecApp::CSpecApp()
{
	gSource = 0;
	gPlaying = -2;
	gWave = gStaticWave;
	pDSC = NULL;
	pDSCB = NULL;
}

// CSpecApp initialization

BOOL CSpecApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();


	CSpecDlg dlg;
	m_pMainWnd = &dlg;

	InitDirectSound();
	INT_PTR nResponse = dlg.DoModal();
	gQuit = 0;
	FreeDirectSound();

	return FALSE;
}