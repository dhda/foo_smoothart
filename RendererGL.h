#include "stdafx.h"
#include "Renderer.h"


class RendererGL : public Renderer
{
public:
	RendererGL(HDC hdc, RECT& rc);
	~RendererGL();

	static void Initialize();

	void Render(CDC dc);
	void Resize(int w, int h);
	void Destroy();
	void Recreate(HDC n_hdc, RECT& n_rc);

	double getFrameTime();

	static bool isInitialized() { return RendererGL::initialized; }
	
private:
	HGLRC hRC;

	GLuint image_texture, prev_image_texture;

	static bool glew;
	static bool initialized;
	static bool anisotropy;
	GLfloat maxAnisotropy;

	bool SetupPixelFormat();
	static bool SetupPixelFormatFallback(HDC dc);
	void CreateContext();

	LARGE_INTEGER prevTime, frameTime;
};