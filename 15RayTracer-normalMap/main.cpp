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
					  Vector3f camPos(-3.4, 12.0, 20.0);
					  Vector3f xAxis(1, 0, 0);
					  Vector3f yAxis(0, 1, 0);
					  Vector3f zAxis(0, 0, 1);
					  Vector3f target(0.0, 6.0, -0.1);

					  Vector3f up(0, 1.0, 0.0);

					  Projection *camera = new Projection(camPos, xAxis, yAxis, zAxis, target, up, 45, new Regular(16, 1));

					  scene = new Scene(ViewPlane(width, height, 1.0), Color(0.2, 0.2, 0.2));

					  Color color = Color(0.4, 0.4, 0.4);

					  scene->addLight(new Light(Vector3f(-60, 60, 60), Color(0.1, 0.1, 0.1), Color(0.4, 0.4, 0.4), Color(0.8, 0.8, 0.8)));
					  scene->addLight(new Light(Vector3f(60, 60, 60), Color(0.1, 0.1, 0.1), color, color));

					  Model* model = new Model(Color(1.0, 1.0, 1.0));

					  //filename, cull backface, smooth shading
					  model->loadObject("objs/Altair/altair.obj", Vector3f(1.0,0.0,0.0), 0.0, Vector3f(0.0, 0.0, 0.0),4, false, true);
					  model->buildKDTree();
					  model->rotate(Vector3f(1.0, 0.0, 0.0), -90.0);
					  model->rotate(Vector3f(0.0, 1.0, 0.0), 30.0);
					  model->translate(-6.0, 0.0, 0.0);
					  model->generateTangents();
					  model->setTexture(NULL);
					  scene->addPrimitive(model);


					  model = new Model(Color(1.0, 1.0, 1.0));

					  //filename, cull backface, smooth shading
					  model->loadObject("objs/Altair/altair.obj", Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 4, false, true);
					  model->buildKDTree();
					  model->rotate(Vector3f(1.0, 0.0, 0.0), -90.0);
					  model->rotate(Vector3f(0.0, 1.0, 0.0), 20.0);
					  model->translate(-2.0, 0.0, 0.0);
					  model->generateTangents();
					  model->setTexture(NULL);
					  model->setMaterial(new Phong(model->getMaterialMesh()));
					  scene->addPrimitive(model);

					  model = new Model();

					  //filename, cull backface, smooth shading
					  model->loadObject("objs/Altair/altair.obj", Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 4, false, true);
					  model->buildKDTree();
					  model->rotate(Vector3f(1.0, 0.0, 0.0), -90.0);
					  model->rotate(Vector3f(0.0, 1.0, 0.0), 20.0);
					  model->translate(2.0, 0.0, 0.0);
					  model->generateTangents();
					  scene->addPrimitive(model);

					  model = new Model();

					  //filename, cull backface, smooth shading
					  model->loadObject("objs/Altair/altair.obj", Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 4, false, true);
					  model->buildKDTree();
					  model->rotate(Vector3f(1.0, 0.0, 0.0), -90.0);
					  model->rotate(Vector3f(0.0, 1.0, 0.0), 20.0);
					  model->translate(6.0, 0.0, 0.0);
					  model->generateTangents();
					  model->setMaterial(new Phong(model->getMaterialMesh()));
					  scene->addPrimitive(model);

					  camera->renderScene(*scene);

					  InvalidateRect(hWnd, 0, true);



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