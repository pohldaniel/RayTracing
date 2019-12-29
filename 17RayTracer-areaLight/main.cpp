#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector> 
#include <iostream>

#include "Primitive.h"
#include "Model.h"
#include "ModelIndexed.h"
#include "MeshTorus.h"
#include "MeshSpiral.h"
#include "MeshSphere.h"
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
					   Sampler* sampler = new MultiJittered(1, 83);
					   //Sampler* sampler = new MultiJittered(100, 83);

					   Vector3f camPosPinhole(-20.0, 10.0, 20.0);
					   Vector3f targetPinhole(0.0, 2.0, 0.0);
					   Vector3f upPinhole(0, 1.0, 0.0);
					   Pinhole *pinhole = new Pinhole(camPosPinhole, targetPinhole, upPinhole, sampler);
					   pinhole->setViewPlaneDistance(1080);
					   //////////////////set up scene and light/////////////////////////
					   scene = new Scene(ViewPlane(600, 600, 1.0), Color(0.5, 0.5, 0.5));
					   scene->setTracer(Scene::Tracer::AreaLighting);

					   Emissive* emissiveMat = new Emissive();
					   emissiveMat->setScaleRadiance(40.0);
					   emissiveMat->setAmbient(Color(1.0, 1.0, 1.0));

					   // define a rectangle for the rectangular light
					   float width = 4.0;				
					   float height = 4.0;
					 
					   Vector3f center(0.0, 7.0, -7.0);	
					   Vector3f p0(-0.5 * width, center[1] - 0.5 * height, center[2]);
					   Vector3f a(width, 0.0, 0.0);
					   Vector3f b(0.0, height, 0.0);
					   //Vector3f normal(0, 0, 1);

					   primitive::Rectangle* rectangle = new primitive::Rectangle(p0, a, b);
					   rectangle->setMaterial(emissiveMat);
					   rectangle->setSampler(new MultiJittered(1, 83));
					   rectangle->setColor(Color(1.0, 1.0, 1.0));

					   AreaLight* areaLight = new AreaLight;
					   areaLight->setObject(rectangle);
					   areaLight->setShadows(true);
					   scene->addLight(areaLight);
					   
					   // Four axis aligned boxes
					   float boxWidth = 1.0; 		// x dimension
					   float boxDepth = 1.0; 		// z dimension
					   float boxHeight = 4.5; 		// y dimension
					   float gap = 3.0;

					   Matte* matte1 = new Matte();
					   matte1->setKa(0.25);
					   matte1->setKd(0.75);
					   matte1->setDiffuse(Color(0.4, 0.7, 0.4));     // green

					   Box* box0 = new Box(Vector3f(-1.5 * gap - 2.0 * boxWidth, 0.0, -0.5 * boxDepth), Vector3f(1.0 * boxWidth, boxHeight, boxDepth));
					   box0->setMaterial(matte1);
					   box0->setColor(Color(0.4, 0.7, 0.4));
					   box0->flipNormals();

					   Box* box1 = new Box(Vector3f(-0.5 * gap - boxWidth, 0.0, -0.5 * boxDepth), Vector3f(-0.5 * gap - (-0.5 * gap - boxWidth), boxHeight, 0.5 * boxDepth - (-0.5 * boxDepth)));
					   box1->setMaterial(matte1);
					   box1->setColor(Color(0.4, 0.7, 0.4));
					   box1->flipNormals();

					   Box* box2 = new Box(Vector3f(0.5 * gap, 0.0, -0.5 * boxDepth), Vector3f(0.5 * gap + boxWidth - (0.5 * gap), boxHeight, 0.5 * boxDepth - (-0.5 * boxDepth)));
					   box2->setMaterial(matte1);
					   box2->setColor(Color(0.4, 0.7, 0.4));
					   box2->flipNormals();

					   Instance* _box2 = new Instance(box2);
					   _box2->rotate(Vector3f(1.0, 0.0, 0.0), 90.0);
					   _box2->rotate(Vector3f(0.0, 1.0, 0.0), 70.0);
					   _box2->translate(0.0, 2.0, 3.0);

					   Box* box3 = new Box(Vector3f(1.5 * gap + boxWidth, 0.0, -0.5 * boxDepth), Vector3f(1.5 * gap + 2.0 * boxWidth - (1.5 * gap + boxWidth), boxHeight, 0.5 * boxDepth - (-0.5 * boxDepth)));
					   box3->setMaterial(matte1);
					   box3->setColor(Color(0.4, 0.7, 0.4));
					   box3->flipNormals();

					   // ground plane
					   Matte* matte2 = new Matte();
					   matte2->setKa(0.1);
					   matte2->setKd(0.90);

					   Plane* plane = new Plane(Vector3f(0.0, 1.0, 0.0), 0.0);
					   plane->setMaterial(matte2);
					   plane->setColor(Color(1.0, 1.0, 1.0));

					   scene->addPrimitive(rectangle);
					   scene->addPrimitive(box0);
					   scene->addPrimitive(box1);
					   scene->addPrimitive(_box2);
					   scene->addPrimitive(box3);
					   scene->addPrimitive(plane);

					   pinhole->renderScene(*scene);


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