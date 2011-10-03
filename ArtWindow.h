#include "stdafx.h"
#include "Renderer.h"
#include "RendererGL.h"


enum {TIMER_REPAINT};

struct ArtWindow : public CWindowImpl<ArtWindow> {
public:
	//ArtWindow(ui_element_config::ptr,ui_element_instance_callback_ptr p_callback);
	ArtWindow();
	~ArtWindow();

	void repaint();

	DECLARE_WND_CLASS_EX(L"SmoothArt", CS_VREDRAW | CS_HREDRAW, -1);

	BEGIN_MSG_MAP(ArtWindow)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

private:
	LRESULT OnCreate(LPCREATESTRUCT);
	void OnDestroy();
	void OnPaint(HDC dc);
	void OnSize(UINT nType, CSize size);
	BOOL OnEraseBkgnd(HDC dc);
	void OnTimer(WPARAM wParam);

	HANDLE TimerQueue;
	HANDLE RepaintTimer;

	Renderer * renderer;
};