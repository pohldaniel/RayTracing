#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector> 
#include <iostream>

#include "Primitive.h"
#include "Model.h"
#include "Bitmap.h"

#include "scene.h"
#include "Camera.h"


int height = 480;
int width = 640;
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
		800,
		600,										// width, height
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


	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, hmemdc;
	PAINTSTRUCT ps;
	

	switch (message)
	{

	case WM_CREATE:
	{
					  Vector3f camPos(-2.0, 0.0, 4.0);
					  Vector3f xAxis(1, 0, 0);
					  Vector3f yAxis(0, 1, 0);
					  Vector3f zAxis(0, 0, 1);
					  Vector3f target(1.0, 0.0, -2.0);

					  Vector3f up(0, 1.0, 0.0);


					  Camera *camera = new Projection(camPos, xAxis, yAxis, zAxis, target, up, 45, new Regular(16, 1));

					  scene = new Scene(ViewPlane(width, height, 1.0), Color(0.2, 0.2, 0.2));

					  Color color = Color(0.7, 0.7, 0.7);

					  scene->addLight(new Light(Vector3f(0, 200, 600), color, color, color));
					  scene->addLight(new Light(Vector3f(40, 0, 0), color, color, color));


					  Torus *torus1 = new Torus(1.0, 0.3, Color(0.4, 0.4, 0.4));
					  torus1->rotate(Vector3f(0.0, 0.0, 1.0), 90);
					  torus1->rotate(Vector3f(1.0, 0.0, 0.0), 30);
					  torus1->setMaterial(new Phong(Color(0.1, 0.1, 0.1), Color(0.8, 0.8, 0.8), Color(0.6, 0.6, 0.6), 50));

					  Torus *torus2 = new Torus(1.0, 0.3, Color(1.0, 0.4, 0.4));
					  torus2->rotate(Vector3f(0.0, 0.0, 1.0), 90);
					  torus2->rotate(Vector3f(1.0, 0.0, 0.0), 85);
					  torus2->translate(1.0, 0.0, 0.0);
					  torus2->setMaterial(new Phong(Color(0.1, 0.1, 0.1), Color(0.8, 0.8, 0.8), Color(1.0, 1.0, 1.0), 50));


					  Model* model = new Model(Color(0.1, 0.7, 0.1));
					  //filename, cull backface, smooth shading
					  model->loadObject("objs/face.obj", false, true);
					  model->buildKDTree();
					  model->rotate(Vector3f(0.0, 1.0, 0.0), 50.0);
					  model->scale(2.0, 2.0, 2.0);
					  model->translate(-5.0, 2.0, -30.0);
					  model->setMaterial(new Phong(Color(0.1, 0.1, 0.1), Color(0.8, 0.8, 0.8), Color(0.6, 0.6, 0.6), 50));
					  model->setTexture(new Texture("textures/pinkwater.bmp"));

					  scene->addPrimitive(torus1);
					  scene->addPrimitive(torus2);
					  scene->addPrimitive(model);

					  camera->renderScene(*scene);

					  InvalidateRect(hWnd, 0, true);

					  delete camera;

					  return 0;

	}
	case WM_PAINT:{				
					std::unique_ptr<Bitmap> bitmap = scene->getBitmap();
					
					 hdc = BeginPaint(hWnd, &ps);

					 hmemdc = CreateCompatibleDC(NULL);
					 HGDIOBJ m_old = SelectObject(hmemdc, bitmap->hbitmap);

					 BitBlt(hdc, bitmap->width / 12, bitmap->height / 12, bitmap->width, bitmap->height, hmemdc, 0, 0, SRCCOPY);

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