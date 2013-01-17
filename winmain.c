#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "bool.h"

#include "GfxOpenGL.h"

const int windowWidth = 800;
const int windowHeight = 600;
const int windowBits = 32;

bool fullscreen = false;

GfxOpenGL g_glRender;

void SetupPixelFormat(HDC hDC)
{
	int pixelFormat;

	PIXELFORMATDESCRIPTOR pfd =
	{	
		sizeof(PIXELFORMATDESCRIPTOR),	/* size */
		1,							/* version */
		PFD_SUPPORT_OPENGL |		/* OpenGL window */
		PFD_DRAW_TO_WINDOW |		/* render to window */
		PFD_DOUBLEBUFFER,			/* support double-buffering */
		PFD_TYPE_RGBA,				/* color type */
		32,							/* prefered color depth */
		0, 0, 0, 0, 0, 0,			/* color bits (ignored) */
		0,							/* no alpha buffer */
		0,							/* alpha bits (ignored) */
		0,							/* accumulation buffer */
		0, 0, 0, 0,					/* accum bits (ignored) */
		24,							/* depth buffer */
		0,							/* stencil buffer */
		0,							/* no auxiliary buffers */
		PFD_MAIN_PLANE,				/* main layer */
		0,							/* reserved */
		0, 0, 0,					/* no layer, visible, damage masks */
	};

	pixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pixelFormat, &pfd);
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
								LPARAM lParam)
{
	static HDC hDC;
	static HGLRC hRC;
	int height, width;

	switch (uMsg)
	{	
	case WM_CREATE:
		hDC = GetDC(hWnd);
		SetupPixelFormat(hDC);
		/* SetupPalette(); */
		hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);
		PostMessage(hWnd, WM_PAINT, NULL, NULL);
		break;

	case WM_DESTROY:
	case WM_QUIT:
	case WM_CLOSE:

		/* Deselect rendering context and delete it.  */
		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);

		/* Send WM_QUIT to message queue.  */
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		height = HIWORD(lParam);
		width = LOWORD(lParam);

		gfxSetupProjection(&g_glRender, width, height);
		PostMessage(hWnd, WM_PAINT, NULL, NULL);
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
			PostMessage(hWnd, WM_PAINT, NULL, NULL);

		break;

	case WM_ACTIVATEAPP:
		break;

	case WM_PAINT:
		gfxRender(&g_glRender);
		SwapBuffers(hDC);
		break;

	case WM_LBUTTONDOWN:
		gfxMousePick(&g_glRender, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, NULL, NULL);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;

		default:
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX windowClass;
	HWND hwnd;
	MSG msg;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT windowRect;

	g_glRender.ready = false;

	windowRect.left=(long)0;
	windowRect.right=(long)windowWidth;
	windowRect.top=(long)0;
	windowRect.bottom=(long)windowHeight;

	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= MainWindowProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(hInstance, (LPCTSTR)101);
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground	= NULL;
	windowClass.lpszMenuName	= NULL;
	windowClass.lpszClassName	= "GLClass";
	windowClass.hIconSm			= NULL;

	if (!RegisterClassEx(&windowClass))
		return 0;

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	
		dmScreenSettings.dmPelsWidth = windowWidth;
		dmScreenSettings.dmPelsHeight = windowHeight;
		dmScreenSettings.dmBitsPerPel = windowBits; /* Bits per pixel */
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) !=
			DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, "Display mode change failed", NULL, MB_OK);
			fullscreen = FALSE;	
		}
	}

	if (fullscreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;
		ShowCursor(FALSE);
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	/* Adjust the window so that the window's client area is the size
	   specified by `windowRect'.  */
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	hwnd = CreateWindowEx(NULL,
		"GLClass",
		"Tic Tac Toe",
		dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	if (!gfxInit(&g_glRender))
		return 0;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	gfxShutdown(&g_glRender);

	if (fullscreen)
	{
		/* Switch back to the desktop resolution.  */
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}

	return (int)msg.wParam;
}
