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

	void LoadArt(album_art_data_ptr art);

	double getFrameTime();

	static bool isInitialized() { return RendererGL::initialized; }
	
private:
	HGLRC hRC;

	GLuint image_texture, prev_image_texture;
	GLfloat image_aspect;

	// OpenGL settings
	static bool glew;
	static bool initialized;
	static bool anisotropy;
	GLfloat maxAnisotropy;

	// Display settings
	GLfloat fov;
	GLfloat aspect;

	bool SetupPixelFormat();
	static bool SetupPixelFormatFallback(HDC dc);
	void CreateContext();

	LARGE_INTEGER prevTime, frameTime;
};