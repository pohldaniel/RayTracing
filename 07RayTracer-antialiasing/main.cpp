// includes
#include <windows.h>			// standard Windows app include
#include <stdio.h>
#include <vector> 
#include <iostream>
#include <stdlib.h>

#include "Scene.h"
#include "Sampler.h"

#include "Camera.h"
#include "Sphere.h"


// globals

int height = 480;
int width = 640;
Scene *scene;


//prototype funktions
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);


// the main windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
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

	// class registered, so now create our window
	hwnd = CreateWindowEx(NULL,						// extended style
		"MyClass",									// class name
		"RayTracer",							// app name
		WS_OVERLAPPEDWINDOW,
		0, 0,										// x,y coordinate
		800,
		600,										// width, height
		NULL,										// handle to parent
		NULL,										// handle to menu
		hInstance,									// application instance
		NULL);										// no extra params

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			// display the window
	UpdateWindow(hwnd);					// update the window

	

	

	
	

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	
	return msg.wParam;
}




// the Windows Procedure event handler
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
					  Vector3f camPos{ 0, 0, 0 };
					  Vector3f xAxis{ 1, 0, 0 };
					  Vector3f yAxis{ 0, 1, 0 };
					  Vector3f zAxis{ 0, 0, 1 };

					  Regular *regular = new Regular();
					 
					
					  Pinhole *pinhole = new Pinhole(camPos, xAxis, yAxis, zAxis, 1.0, 500, regular);

					  scene = new Scene();
					  scene->addSphere(&Sphere(Vector3f(1.0, 0, -5), 1, Color(0, 1.0, 0)));
					  scene->addSphere(&Sphere(Vector3f(0, 0, -7), 1, Color(1.0, 0, 0)));
					 
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
					   delete scene;
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