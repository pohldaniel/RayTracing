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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

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


	while (GetMessage(&msg, 0, 0, 0)){

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

	case WM_CREATE:{
					   //////////////////set up camera/////////////////////////
					   Vector3f camPos(6.0, 5.0, 15.0);
					   Vector3f target(3.0, 5.0, -1.0);
					   Vector3f up(0, 1.0, 0.0);
					   Projection *camera = new Projection(camPos, target, up, new Regular(16, 1));
					   //////////////////set up scene/////////////////////////
					   scene = new Scene(ViewPlane(width, height, 1.0), Color(0.5, 0.5, 0.5));
					   scene->setDepth(1.0);
					   //////////////////set up light/////////////////////////
					   scene->addLight(new Light(Vector3f(0, 8, -100), Color(0.1, 0.1, 0.1), Color(1.8, 1.8, 1.8), Color(0.8, 0.8, 0.8)));
					   scene->addLight(new Light(Vector3f(0, 8, 100), Color(0.1, 0.1, 0.1), Color(0.8, 0.8, 0.8), Color(0.8, 0.8, 0.8)));
					   scene->addLight(new Light(Vector3f(60, 100, 100), Color(0.1, 0.1, 0.1), Color(0.8, 0.8, 0.8), Color(0.8, 0.8, 0.8)));
					   //////////////////set up objects/////////////////////////
					   Model* model = new Model();
					   //filename,rotation, translation, cull backface, smooth shading
					   model->loadObject("objs/statue/statue.obj", Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 2.0, true, true);
					   model->generateTangents();
					   model->buildKDTree();

					   Reflective *reflective = new Reflective();
					   reflective->setReflectionColor(1.0);
					   reflective->setFrensel(0.4);

					   primitive::Rectangle* mirror = new primitive::Rectangle(Vector3f(-12.0, 0.0, -5.0), Vector3f(30.0, 0.0, 0.0), Vector3f(0.0, 20.0, 0.0));
					   mirror->setColor(Color(0.4, 0.4, 0.4));
					   mirror->setMaterial(reflective);

					   RectangleChecker* rectangleChecker = new RectangleChecker();
					   rectangleChecker->setNumXCheckers(4);
					   rectangleChecker->setNumZCheckers(4);
					   rectangleChecker->setXLineWidth(0.0);
					   rectangleChecker->setZLineWidth(0.0);
					   rectangleChecker->setColor1(Color(1.0, 1.0, 1.0));
					   rectangleChecker->setColor2(Color(0.0, 0.0, 0.0));
					   rectangleChecker->setLineColor(Color(1.0, 1.0, 0.0));
					   rectangleChecker->setAttributes(Vector3f(-12.0, 0.0, -5.0), Vector3f(30.0, 0.0, 0.0), Vector3f(0.0, 0.0, 30.0));

					   primitive::Rectangle* bottom = new primitive::Rectangle(Vector3f(-12.0, 0.0, -5.0), Vector3f(30.0, 0.0, 0.0), Vector3f(0.0, 0.0, 30.0));
					   bottom->setColor(Color(0.4, 0.4, 0.4));
					   bottom->setMaterial(new Phong(Color(0.5, 0.5, 0.5), Color(0.8, 0.8, 0.8), Color(1.0, 1.0, 1.0), 50));
					   bottom->setTexture(rectangleChecker);


					   scene->addPrimitive(mirror);
					   scene->addPrimitive(model);
					   scene->addPrimitive(bottom);

					   camera->renderScene(*scene);

					   InvalidateRect(hWnd, 0, true);

					   return 0;

	}case WM_PAINT:{				
					std::shared_ptr<Bitmap> bitmap = scene->getBitmap();
					
					 hdc = BeginPaint(hWnd, &ps);

					 hmemdc = CreateCompatibleDC(NULL);
					 HGDIOBJ m_old = SelectObject(hmemdc, bitmap->hbitmap);

					 BitBlt(hdc, bitmap->width / 12, bitmap->height / 12, bitmap->width, bitmap->height, hmemdc, 0, 0, SRCCOPY);

					 SelectObject(hmemdc, m_old);
					 DeleteDC(hmemdc);

					 EndPaint(hWnd, &ps);

					 return 0;

	}case WM_DESTROY:{				
					  
					 PostQuitMessage(0);
					 return 0;

	}case WM_KEYDOWN:{

					   switch (wParam){

							case VK_ESCAPE:{

								PostQuitMessage(0);
								return 0;
							}break;

						    return 0;

					   }
	break;
	return 0;
	}

	break;
	}//end switch


	return (DefWindowProc(hWnd, message, wParam, lParam));
}