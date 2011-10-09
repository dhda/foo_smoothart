#include "stdafx.h"

#pragma once


class Renderer
{
public:
	Renderer(HDC hdc, RECT& rc) : hdc(hdc), rc(rc) {};
	virtual ~Renderer() {}

	virtual void Render(CDC dc) = 0;
	virtual void Resize(int w, int h) = 0;
	virtual void Destroy() = 0;
	virtual void Recreate(HDC n_hdc, RECT& n_rc) = 0;

protected:
	HDC hdc;
	RECT& rc;

	static_api_ptr_t<playlist_manager> playlist;
	album_art_manager_instance_ptr art_loader;
	album_art_data_ptr art;
};