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
};