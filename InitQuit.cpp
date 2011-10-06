#include "stdafx.h"
#include "RendererGL.h"


class SmoothArt_InitQuit : public initquit {
public:
	void on_init() 
	{
		console::print("Smooth Album Art: on_init()");
		RendererGL::Initialize();
	}

	void on_quit() {}
};

static initquit_factory_t<SmoothArt_InitQuit> g_smoothart_initquit_factory;
