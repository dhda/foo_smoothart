#include "stdafx.h"
#include "RendererGL.h"
#include <cmath>

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF


RendererGL::RendererGL(HDC hdc, RECT& rc) : Renderer(hdc, rc)
{
	Recreate(hdc, rc);

	image_texture = 0;
	prev_image_texture = 0;
}

RendererGL::~RendererGL()
{
	Destroy();
}


bool RendererGL::glew = false;
bool RendererGL::initialized = false;
bool RendererGL::anisotropy = true;
void RendererGL::Initialize()
{
	// Create temporary window and context for GLEW initialization, then destroy it
	HWND dummyWindow = CreateWindowEx(0, L"Static", L"", 0, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	ShowWindow(dummyWindow, SW_HIDE);
	HDC dummyDC = GetDC(dummyWindow);

	if (!SetupPixelFormatFallback(dummyDC))
	{
		console::error("Smooth Album Art: Total failure");
		return;
	}

	HGLRC tempRC;

	tempRC = wglCreateContext(dummyDC);
	wglMakeCurrent(dummyDC, tempRC);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		//console::formatter() << "Smooth Album Art: GLEW Error: " << (char *)glewGetErrorString(err);
		console::formatter() << "Smooth Album Art: Using OpenGL without GLEW";
		RendererGL::glew = false;
	}
	else
	{
		console::formatter() << "Smooth Album Art: Using OpenGL with GLEW " << (char *)glewGetString(GLEW_VERSION);
		RendererGL::glew = true;
	}

	wglMakeCurrent(dummyDC, NULL);
	wglDeleteContext(tempRC);
	tempRC = NULL;
	DestroyWindow(dummyWindow);

	ILuint devilError;
	ilInit();
	iluInit();
	devilError = ilGetError();

	if (devilError != IL_NO_ERROR)
		console::print("Smooth Album Art: DevIL error");

	RendererGL::initialized = true;
}


bool RendererGL::SetupPixelFormat()
{
	int multisampling = GL_TRUE;
	int samples = 8; //TODO: detect max

	//TODO: Move checks to Initialize()
	if (!WGLEW_ARB_multisample)
	{
		console::print("Smooth Album Art: Multisampling is not supported");
		multisampling = GL_FALSE;
		samples = 0;
	}

	if (!GLEW_EXT_texture_filter_anisotropic)
	{
		console::print("Smooth Album Art: Anisotropic filtering is not supported");
		RendererGL::anisotropy = false;
	}
	

	const int attribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, multisampling,
		WGL_SAMPLES_ARB, samples,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_ACCUM_BITS_ARB, 64,
		WGL_STENCIL_BITS_ARB, 0,
		0, //End
	};

	int pixelFormat;
	UINT numFormats;
	if (!wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixelFormat, &numFormats))
		return false;

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd);
	if (!SetPixelFormat(hdc, pixelFormat, &pfd))
		return false;

	return true;
}
bool RendererGL::SetupPixelFormatFallback(HDC dc)
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,                           // version number
		PFD_DRAW_TO_WINDOW |         // support window
		PFD_SUPPORT_OPENGL |         // support OpenGL
		PFD_DOUBLEBUFFER,            // double buffered
		PFD_TYPE_RGBA,               // RGBA type
		24,                          // 24-bit color depth
		0, 0, 0, 0, 0, 0,            // color bits ignored
		8,                           // no alpha buffer
		0,                           // shift bit ignored
		64,                          // no accumulation buffer
		16, 16, 16, 16,              // accum bits ignored
		32,                          // 32-bit z-buffer
		0,                           // no stencil buffer
		0,                           // no auxiliary buffer
		PFD_MAIN_PLANE,              // main layer
		0,                           // reserved
		0, 0, 0                      // layer masks ignored
	};

	int pixelformat;

	if ((pixelformat = ChoosePixelFormat(dc, &pfd)) == 0)
		return false;

	if (SetPixelFormat(dc, pixelformat, &pfd) == FALSE)
		return false;

	return true;
}

void RendererGL::CreateContext()
{
	bool setup;
	if (RendererGL::glew)
		setup = SetupPixelFormat();
	else
		setup = SetupPixelFormatFallback(hdc);
	
	if (!setup)
	{
		console::error("Smooth Album Art: Couldn't set pixel format");
		return;
	}
	
	hRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hRC);

	maxAnisotropy = 1.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	console::formatter() << "Smooth Album Art: Using anisotropy level " << maxAnisotropy;

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glEnable(GL_MULTISAMPLE_ARB);

	glClearColor(0.0f, 0.0f, 0.0f, 10.0f);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	Resize(width, height);
}

void RendererGL::Destroy()
{
	wglMakeCurrent(hdc, NULL);
	if (hRC)
	{
		wglDeleteContext(hRC);
		hRC = NULL;
	}
}

void RendererGL::Recreate(HDC n_hdc, RECT& n_rc)
{
	Destroy();
	
	hdc = n_hdc;
	rc = n_rc;

	CreateContext();
}

void RendererGL::Resize(int w, int h)
{
	if (h <= 0)
		h = 1;

	GLfloat aspect = (GLfloat)w / (GLfloat)h;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0f, aspect, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RendererGL::LoadArt(album_art_data_ptr art)
{
	ILuint devilID, devilError = IL_NO_ERROR;

	ilGenImages(1, &devilID);
	ilBindImage(devilID);

	ilLoadL(IL_TYPE_UNKNOWN, art->get_ptr(), art->get_size());
	if (devilError != IL_NO_ERROR)
		console::print("Smooth Album Art: DevIL error");

	console::formatter() << "Smooth Album Art: New album art (" << ilGetInteger(IL_IMAGE_WIDTH) << "x" << ilGetInteger(IL_IMAGE_HEIGHT) << ")";

	ilutRenderer(ILUT_OPENGL);
	prev_image_texture = image_texture;
	image_texture = ilutGLBindTexImage();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.5f);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void RendererGL::Render(CDC dc)
{
	wglMakeCurrent(dc, hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glEnable(GL_TEXTURE_2D);

	glTranslatef(0.f, 0.f, -3.f);

	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	//glRotatef((GLfloat)tick.QuadPart / 20000.f, 1.f, 0.f, 0.f);
	//glRotatef(sin((float)tick.QuadPart / 5000000.f) * 130.f, 0.f, 1.f, 0.f);
	//glRotatef((GLfloat)tick.QuadPart / 50000.f, 0.f, 0.f, 1.f);

	glBindTexture(GL_TEXTURE_2D, image_texture);
	glBegin(GL_QUADS);
		// Front
		glColor3f(1.f, 1.f, 1.f);
		glTexCoord2d(1.0,0.0);
		glVertex3f( 1.f, -1.f, 0.f);
		glTexCoord2d(1.0,1.0);
		glVertex3f( 1.f,  1.f, 0.f);
		glTexCoord2d(0.0,1.0);
		glVertex3f(-1.f,  1.f, 0.f);
		glTexCoord2d(0.0,0.0);
		glVertex3f(-1.f, -1.f, 0.f);

		//// Left
		//glColor3f(0.f, 1.f, 0.f);
		//glVertex3f(-50.f, -50.f,  50.f);
		//glVertex3f(-50.f,  50.f,  50.f);
		//glVertex3f(-50.f,  50.f, -50.f);
		//glVertex3f(-50.f, -50.f, -50.f);

		//// Right
		//glColor3f(0.f, 1.f, 0.f);
		//glVertex3f(50.f, -50.f, -50.f);
		//glVertex3f(50.f,  50.f, -50.f);
		//glVertex3f(50.f,  50.f,  50.f);
		//glVertex3f(50.f, -50.f,  50.f);

		//// Top
		//glColor3f(0.f, 0.f, 1.f);
		//glVertex3f(-50.f, -50.f,  50.f);
		//glVertex3f(-50.f, -50.f, -50.f);
		//glVertex3f( 50.f, -50.f, -50.f);
		//glVertex3f( 50.f, -50.f,  50.f);

		//// Bottom
		//glColor3f(0.f, 0.f, 1.f);
		//glVertex3f(-50.f, 50.f,  50.f);
		//glVertex3f( 50.f, 50.f,  50.f);
		//glVertex3f( 50.f, 50.f, -50.f);
		//glVertex3f(-50.f, 50.f, -50.f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, prev_image_texture);
	glBegin(GL_QUADS);
		// Back
		glColor3f(1.f, 1.f, 1.f);
		glTexCoord2d(1.0,0.0);
		glVertex3f(-1.f, -1.f, 0.f);
		glTexCoord2d(1.0,1.0);
		glVertex3f(-1.f,  1.f, 0.f);
		glTexCoord2d(0.0,1.0);
		glVertex3f( 1.f,  1.f, 0.f);
		glTexCoord2d(0.0,0.0);
		glVertex3f( 1.f, -1.f, 0.f);
	glEnd();

	glPopMatrix();

	glFinish();

	//glAccum(GL_ACCUM, 1.f);
	//glAccum(GL_RETURN, 1.f);
	//glAccum(GL_MULT, 0.95f);

	SwapBuffers(wglGetCurrentDC());

	/*QueryPerformanceCounter(&tick);
	frameTime.QuadPart = tick.QuadPart - prevTime.QuadPart;
	prevTime = tick;

	console::formatter() << "Frame Time = " << getFrameTime();*/
}

double RendererGL::getFrameTime()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)frameTime.QuadPart / (double)freq.QuadPart;
}