#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector> 
#include <iostream>
#include "Primitive.h"
#include "Bitmap.h"

#include "scene.h"
#include "Camera.h"


#define SCRWIDTH	800
#define SCRHEIGHT	600



Bitmap *bitmap;
Scene *scene;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle("Debug console");
	MoveWindow(GetConsoleWindow(), 790, 0, 500, 200, true);

	WNDCLASSEX windowClass;		// window class
	HWND	   hwnd;			// window handle
	MSG		   msg;				// message
	HDC		   hdc;				// device context handle


	try {

		bitmap = new Bitmap(SCRHEIGHT, SCRWIDTH, 24);
	}
	catch (const char* e) {
		MessageBox(NULL, e, "Error", MB_OK);
		return 0;
	}

	// fill out the window class structure
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);		// default icon
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);			// default arrow
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// white background
	windowClass.lpszMenuName = NULL;									// no menu
	windowClass.lpszClassName = "MyClass";
	windowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);			// windows logo small icon


	// register the windows class
	if (!RegisterClassEx(&windowClass))
		return 0;

	hwnd = CreateWindowEx(NULL,						// extended style
		"MyClass",									// class name
		"RayTracer",							// app name
		WS_OVERLAPPEDWINDOW,
		0, 0,										// x,y coordinate
		1000,
		800,										// width, height
		NULL,										// handle to parent
		NULL,										// handle to menu
		hInstance,									// application instance
		NULL);										// no extra params

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);


	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete bitmap;
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, hmemdc;
	PAINTSTRUCT ps;
	POINT pt;
	RECT rect;

	switch (message)
	{

	case WM_CREATE:
	{				 
					  Mesh* mesh = new Mesh();
					  mesh->loadObject("face.obj", 2.0);
					


					  Torus *torus = new Torus(1.0, 0.5, Color(0.5, 0.5, 0.5));

					  torus->rotate(Vector3f(0.0, 1.0, 0.0), 77);
					  torus->rotate(Vector3f(1.0, 0.0, 0.0), 20);
					  torus->translate(0.4, 0.0, 2.0);

					  mesh->translate(0.0 , 0.2 , 0.0);
					  Vector3f camPos( 0.0f, 0.0f, 20.0f );
					  Vector3f xAxis( 1, 0, 0 );
					  Vector3f yAxis( 0, 1, 0 );
					  Vector3f zAxis( 0, 0, 1 );
					

					  Regular *regular = new Regular(4, 1);

					  Pinhole *pinhole = new Pinhole(camPos, xAxis, yAxis, zAxis, 500.0, 1.0, regular);

					
					  scene = new Scene();

					  scene->addPrimitive(new Plane(Vector3f(0.0, 1.0, 0.0), -6.0, Color(0, 0, 1.0)));
					  scene->addPrimitive(new Sphere(Vector3f(-6.0, -4.0, 0), 4, Color(1.0, 0, 0)));
					  scene->addPrimitive(mesh);	
					  scene->addPrimitive(torus);
					 
					  pinhole->renderScene(*scene);
				
					  InvalidateRect(hWnd, 0, true);
					  return 0;
	}
	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);

					 hmemdc = CreateCompatibleDC(NULL);
					 HGDIOBJ m_old = SelectObject(hmemdc, scene->bitmap->hbitmap);

					 BitBlt(hdc, scene->bitmap->width / 12, scene->bitmap->height / 12, scene->bitmap->width, scene->bitmap->height, hmemdc, 0, 0, SRCCOPY);

					 SelectObject(hmemdc, m_old);
					 DeleteDC(hmemdc);

					 EndPaint(hWnd, &ps);

					 return 0;
	}
	case WM_DESTROY:
	{
					   PostQuitMessage(0);
					   return 0;
	}
	case WM_KEYDOWN:
	{
					   switch (wParam)
					   {

					   case VK_ESCAPE:
					   {
										 PostQuitMessage(0);
										 return 0;
					   }break;

					   return 0;
					   }break;

					   return 0;
	}break;


	}//end switch



	return (DefWindowProc(hWnd, message, wParam, lParam));
}