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
	}

	wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes); 

	TimerQueue = NULL;
	RepaintTimer = NULL;
}

ArtWindow::~ArtWindow() {}


LRESULT ArtWindow::OnCreate(LPCREATESTRUCT)
{
	console::print("Smooth Album Art: OnCreate");

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
	CreateTimerQueueTimer(&RepaintTimer, TimerQueue, (WAITORTIMERCALLBACK)TimerCallback, this, 10, 1, WT_EXECUTEDEFAULT);

	return 0;
}

void ArtWindow::OnDestroy()
{
	console::print("Smooth Album Art: OnDestroy");

	delete renderer;
	//KillTimer(TIMER_REPAINT);

	if (RepaintTimer)
		DeleteTimerQueueTimer(TimerQueue, RepaintTimer, INVALID_HANDLE_VALUE);
	if (TimerQueue)
		DeleteTimerQueue(TimerQueue);
}

void ArtWindow::OnPaint(HDC dc)
{
	CPaintDC hdc(*this);
	renderer->Render(hdc); // Using dc here doesn't work for some reason
}

void ArtWindow::OnSize(UINT nType, CSize size)
{
	//console::formatter() << "OnSize: (" << size.cx << "," << size.cy << ")";

	renderer->Destroy();
	RECT rc;
	GetClientRect(&rc);
	renderer->Recreate(GetDC(), rc);

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