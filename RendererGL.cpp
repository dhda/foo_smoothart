#include "stdafx.h"
#include "RendererGL.h"
#include <cmath>

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

double pi = atan((double)1)*4;

RendererGL::RendererGL(HDC hdc, RECT& rc) : Renderer(hdc, rc)
{
	Recreate(hdc, rc);

	image_texture = 0;
	prev_image_texture = 0;

	fov = 40.0f;

	image_aspect = 1.0f;
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

	max_anisotropy = 1.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
	console::formatter() << "Smooth Album Art: Using anisotropy level " << max_anisotropy;

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_TEXTURE_2D);

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

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//if (w > h)
	//{
	//	GLfloat half = w*0.5f;
	//	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	//	GLfloat v = (1.0f + half*aspect) / half;
	//	glFrustum(-v, v, -1.0f, 1.0f, 12.0f, 12.0f/tan(pi*40.0f/180.0f));
	//}
	//else
	//{
	//	GLfloat half = h*0.5f;
	//	GLfloat aspect = (GLfloat)h / (GLfloat)w;
	//	GLfloat v = (1.0f + half*aspect) / half;
	//	glFrustum(-1.0f, 1.0f, -v, v, 12.0f, 12.0f/tan(pi*40.0f/180.0f));
	//}

	aspect = (GLfloat)w / (GLfloat)h;

	if (w > h)
	{
		gluPerspective(fov, aspect, 0.1f, 20.0f);
	}
	else
	{
		gluPerspective(360.0f/pi*atan(tan(fov*pi/360.0f)/aspect), aspect, 0.1f, 20.0f);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RendererGL::LoadArt(album_art_data_ptr art)
{
	ILuint devilImg, devilError = IL_NO_ERROR;

	ilGenImages(1, &devilImg);
	ilBindImage(devilImg);

	ilLoadL(IL_TYPE_UNKNOWN, art->get_ptr(), art->get_size());
	if (devilError != IL_NO_ERROR)
		console::print("Smooth Album Art: DevIL error");

	GLsizei img_width = ilGetInteger(IL_IMAGE_WIDTH);
	GLsizei img_height = ilGetInteger(IL_IMAGE_HEIGHT);
	
	image_aspect = (GLfloat)img_width / (GLfloat)img_height;

	console::formatter() << "Smooth Album Art: New album art (" << img_width << "x" << img_height << ")";

	ilutRenderer(ILUT_OPENGL);
	//image_texture = ilutGLBindTexImage();

	GLuint tex_size;
	// Make a square texture
	if (img_width > img_height)
		tex_size = img_width;
	else
		tex_size = img_height;

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), NULL);

	GLint img_xoffset = (tex_size - img_width) / 2;
	GLint img_yoffset = (tex_size - img_height) / 2;
	glTexSubImage2D(GL_TEXTURE_2D, 0, img_xoffset, img_yoffset, img_width, img_height, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glGenerateMipmap(GL_TEXTURE_2D);

	ilDeleteImage(devilImg);
	glDeleteTextures(1, &prev_image_texture);
	glDeleteBuffers(1, &tex);
	glDeleteBuffers(1, &prev_image_texture);
	glDeleteBuffers(1, &image_texture);

	prev_image_texture = image_texture;
	image_texture = tex;
}

void RendererGL::Render(CDC dc)
{
	wglMakeCurrent(dc, hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//GLfloat back = 12.0f + 0.5f*(12.0f/tan(pi*40.0f/180.0f) - 12.0f);

	//glTranslatef(0.0f, 0.0f, -back);

	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	//glRotatef((GLfloat)tick.QuadPart / 20000.f, 1.f, 0.f, 0.f);
	//glTranslatef(0.0f, 0.0f, 5.0f);
	//glRotatef(sin((float)tick.QuadPart / 5000000.f) * 130.f, 0.f, 1.f, 0.f);
	//glRotatef((GLfloat)tick.QuadPart / 50000.f, 0.f, 0.f, 1.f);

	//GLdouble rot = 120.0f*pi/180.0f * sin((float)tick.QuadPart / 2000000.f);
	GLdouble rot = pi/2.0f;
	GLdouble dist = 1.0f/tan(fov*pi/360.0f);
	//gluLookAt(dist*cos((float)tick.QuadPart / 3000000.f), 0.0f, -5.0f-dist + dist*sin((float)tick.QuadPart / 3000000.f), 0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(dist*cos(45.0f*pi/180.0f), 0.0f, -5.0f+dist + dist*sin(45.0f*pi/180.0f), 0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(0.0f, 0.0f, 0.0f,
	//          0.0f, 0.0f, -5.0f,
	//		    0.0f, 1.0f, 0.0f);
	//glRotatef(sin((float)tick.QuadPart / 5000000.f) * 130.f, 0.f, 1.f, 0.f);;

	//gluLookAt(dist*cos((float)tick.QuadPart / 3000000.f), 0.0f, dist*sin((float)tick.QuadPart / 3000000.f), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	gluLookAt(dist*cos(rot), 0.0f, dist*sin(rot), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	GLfloat quad_size = 1.0f;
	if (aspect > 1.0f && image_aspect > 1.0f)
	{
		if (aspect > image_aspect)
			quad_size = image_aspect;
		else
			quad_size = aspect;
	}
	else if (aspect < 1.0f && image_aspect < 1.0f)
	{
		if (aspect < image_aspect)
			quad_size = 1.0f/image_aspect;
		else
			quad_size = 1.0f/aspect;
	}

	glBindTexture(GL_TEXTURE_2D, image_texture);
	glBegin(GL_QUADS);
		// Front
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2d(1.0f, 1.0f);
		glVertex3f( quad_size, -quad_size, 0.0f);
		glTexCoord2d(1.0f, 0.0f);
		glVertex3f( quad_size,  quad_size, 0.0f);
		glTexCoord2d(0.0f, 0.0f);
		glVertex3f(-quad_size,  quad_size, 0.0f);
		glTexCoord2d(0.0f, 1.0f);
		glVertex3f(-quad_size, -quad_size, 0.0f);

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
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2d(1.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2d(1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
		glTexCoord2d(0.0f, 0.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
		glTexCoord2d(0.0f, 1.0f);
		glVertex3f( 1.0f, -1.0f, 0.0f);
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