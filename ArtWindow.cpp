#include "stdafx.h"
#include "ArtWindow.h"


VOID CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	((ArtWindow*) lpParam)->repaint();
}


ArtWindow::ArtWindow()
{
	TIMECAPS tc;
	UINT wTimerRes;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
	{
		console::error("Smooth Album Art: Timer error");
		// TODO: handle error
	}

	wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes); 

	TimerQueue = NULL;
	RepaintTimer = NULL;
}

ArtWindow::~ArtWindow() {}


LRESULT ArtWindow::OnCreate(LPCREATESTRUCT)
{
	RECT rc;
	GetClientRect(&rc);
	renderer = new RendererGL(GetDC(), rc);

	//KillTimer(TIMER_REPAINT);
	//SetTimer(TIMER_REPAINT, 15);

	if (RepaintTimer)
		DeleteTimerQueueTimer(TimerQueue, RepaintTimer, INVALID_HANDLE_VALUE);
	if (TimerQueue)
		DeleteTimerQueue(TimerQueue);

	TimerQueue = CreateTimerQueue();
	CreateTimerQueueTimer(&RepaintTimer, TimerQueue, (WAITORTIMERCALLBACK)TimerCallback, this, 10, 5, WT_EXECUTEDEFAULT);

	return 0;
}

void ArtWindow::OnDestroy()
{
	delete renderer;
	//KillTimer(TIMER_REPAINT);

	if (RepaintTimer)
		DeleteTimerQueueTimer(TimerQueue, RepaintTimer, INVALID_HANDLE_VALUE);
	if (TimerQueue)
		DeleteTimerQueue(TimerQueue);

	// Does this do anything?
	RepaintTimer = NULL;
	TimerQueue = NULL;
}

void ArtWindow::OnPaint(HDC dc)
{
	CPaintDC cdc(*this);
	renderer->Render(cdc);
}

void ArtWindow::OnSize(UINT nType, CSize size)
{
	renderer->Resize(size.cx, size.cy);

	repaint();
}

BOOL ArtWindow::OnEraseBkgnd(HDC dc)
{
	return TRUE;
}

void ArtWindow::OnTimer(WPARAM wParam)
{
	if (wParam == TIMER_REPAINT && core_api::are_services_available())
		repaint();
}

void ArtWindow::repaint()
{
	if ((HWND)*this)
		Invalidate();
}