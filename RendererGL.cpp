#include "stdafx.h"
#include "RendererGL.h"


RendererGL::RendererGL(HDC hdc, RECT& rc) : Renderer(hdc, rc)
{
	Recreate(hdc, rc);
}

RendererGL::~RendererGL()
{
	Destroy();
}

BOOL RendererGL::SetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
		1,                           // version number
		PFD_DRAW_TO_WINDOW |         // support window
		PFD_SUPPORT_OPENGL |         // support OpenGL
		PFD_DOUBLEBUFFER,            // double buffered
		PFD_TYPE_RGBA,               // RGBA type
		24,                          // 24-bit color depth
		0, 0, 0, 0, 0, 0,            // color bits ignored
		0,                           // no alpha buffer
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

	if ((pixelformat = ChoosePixelFormat(hdc, &pfd)) == 0)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

void RendererGL::CreateContext()
{
	PIXELFORMATDESCRIPTOR pfd;
	if (!SetupPixelFormat())
		return;

	int n = GetPixelFormat(hdc);
	DescribePixelFormat(hdc, n, sizeof(pfd), &pfd);
	hRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hRC);


	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glClearColor(0.0f, 0.0f, 0.0f, 10.0f);

	Resize(width, height);

/*
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.f, 1.f, 1.f, 500.f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
}

void RendererGL::Destroy()
{
	wglMakeCurrent(NULL, NULL);
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
	gluPerspective(80.0f, aspect, 0.1f, 500.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RendererGL::Render(HDC dc)
{
	wglMakeCurrent(dc, hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0.f, 0.f, -200.f);

	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	glRotatef((GLfloat)tick.QuadPart / 20000.f, 1.f, 0.f, 0.f);
	glRotatef((GLfloat)tick.QuadPart / 30000.f, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)tick.QuadPart / 40000.f, 0.f, 0.f, 1.f);

	glBegin(GL_QUADS);
		// Back
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(-50.f, -50.f, -50.f);
		glVertex3f(-50.f,  50.f, -50.f);
		glVertex3f( 50.f,  50.f, -50.f);
		glVertex3f( 50.f, -50.f, -50.f);

		// Front
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f( 50.f, -50.f, 50.f);
		glVertex3f( 50.f,  50.f, 50.f);
		glVertex3f(-50.f,  50.f, 50.f);
		glVertex3f(-50.f, -50.f, 50.f);

		// Left
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(-50.f, -50.f,  50.f);
		glVertex3f(-50.f,  50.f,  50.f);
		glVertex3f(-50.f,  50.f, -50.f);
		glVertex3f(-50.f, -50.f, -50.f);

		// Right
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(50.f, -50.f, -50.f);
		glVertex3f(50.f,  50.f, -50.f);
		glVertex3f(50.f,  50.f,  50.f);
		glVertex3f(50.f, -50.f,  50.f);

		// Top
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(-50.f, -50.f,  50.f);
		glVertex3f(-50.f, -50.f, -50.f);
		glVertex3f( 50.f, -50.f, -50.f);
		glVertex3f( 50.f, -50.f,  50.f);

		// Bottom
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(-50.f, 50.f,  50.f);
		glVertex3f( 50.f, 50.f,  50.f);
		glVertex3f( 50.f, 50.f, -50.f);
		glVertex3f(-50.f, 50.f, -50.f);
	glEnd();

	glPopMatrix();

	glFinish();

	glAccum(GL_ACCUM, 1.f);
	glAccum(GL_RETURN, 1.f);
	glAccum(GL_MULT, 0.95f);

	SwapBuffers(wglGetCurrentDC());

	QueryPerformanceCounter(&tick);
	frameTime.QuadPart = tick.QuadPart - prevTime.QuadPart;
	prevTime = tick;

	console::formatter() << "Frame Time = " << getFrameTime();
}

double RendererGL::getFrameTime()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (double)frameTime.QuadPart / (double)freq.QuadPart;
}