#include "stdafx.h"
#include "Renderer.h"


class RendererGL : public Renderer
{
public:
	RendererGL(HDC hdc, RECT& rc);
	~RendererGL();

	void Render(CDC dc);
	void Resize(int w, int h);
	void Destroy();
	void Recreate(HDC n_hdc, RECT& n_rc);

	double getFrameTime();
	
private:
	HGLRC hRC;

	BOOL SetupPixelFormat();
	BOOL SetupPixelFormatFallback(HDC dc);
	void CreateContext();

	LARGE_INTEGER prevTime, frameTime;
};