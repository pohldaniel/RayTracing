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
					   int numSample = 100;

					   Sampler* sampler = new Regular(numSample, 83);
					   Sampler* samplerMatte = new MultiJittered(numSample, 83);
					   samplerMatte->mapSamplesToHemisphere(1.0);

					   Vector3f camPosPinhole(27.6, 27.4, -80.0);
					   Vector3f targetPinhole(27.6, 27.4, 0.0);
					   Vector3f upPinhole(0, 1.0, 0.0);
					   Pinhole *pinhole = new Pinhole(camPosPinhole, targetPinhole, upPinhole, sampler);
					   pinhole->setViewPlaneDistance(400);
					   //////////////////set up scene and light/////////////////////////
					   scene = new Scene(ViewPlane(300, 300, 1.0), Color(0.0, 0.0, 0.0));
					   scene->setTracer(Scene::Tracer::PathTracer);
					   scene->setDepth(10);
			

					   Emissive* emissiveMat = new Emissive();
					   emissiveMat->setScaleRadiance(100.0);
					   emissiveMat->setColor(Color(1.0, 1.0, 1.0));

					   // define a rectangle for the rectangular light
					   Vector3f p0, a, b;
					   // box dimensions
					   double width = 55.28;   	// x direction
					   double height = 54.88;  	// y direction
					   double depth = 55.92;	// z direction


					   primitive::Rectangle* light = new primitive::Rectangle(Vector3f(21.3, height - 0.005, 22.7), Vector3f(0.0, 0.0, 10.5), Vector3f(13.0, 0.0, 0.0));

					   light->setMaterial(emissiveMat);
					   light->setSampler(new MultiJittered(1, 83));
					   light->setColor(Color(1.0, 1.0, 1.0));
					   light->flipNormal();
					   scene->addPrimitive(light);

					   AreaLight* areaLight = new AreaLight;
					   areaLight->setObject(light);
					   areaLight->setShadows(true);
					   scene->addLight(areaLight);

					   // left wall
					   Matte* matte1 = new Matte();
					   matte1->setKa(0.25);
					   matte1->setKd(0.6);
					   matte1->setSampler(samplerMatte);

					   p0 = Vector3f(width, 0.0, 0.0); a = Vector3f(0.0, 0.0, depth); b = Vector3f(0.0, height, 0.0);
					   primitive::Rectangle* leftWall = new primitive::Rectangle(p0, a, b);
					   leftWall->setMaterial(matte1);
					   leftWall->setColor(Color(0.57, 0.025, 0.025));  // red
					   //leftWall->flipNormal();
					   scene->addPrimitive(leftWall);
					  
					   // right wall
					   Matte* matte2 = new Matte();
					   matte2->setKa(0.25);
					   matte2->setKd(0.6);
					   matte2->setSampler(samplerMatte);

					   p0 = Vector3f(0.0, 0.0, 0.0); a = Vector3f(0.0, 0.0, depth); b = Vector3f(0.0, height, 0.0);
					   primitive::Rectangle* rightWall = new primitive::Rectangle(p0, a, b);
					   rightWall->setMaterial(matte2);
					   rightWall->setColor(Color(0.37, 0.59, 0.2));  // green
					   rightWall->flipNormal();
					   scene->addPrimitive(rightWall);

					   // back wall
					   Matte* matte3 = new Matte();
					   matte3->setKa(0.25);
					   matte3->setKd(0.6);
					   matte3->setSampler(samplerMatte);

					   p0 = Vector3f(0.0, 0.0, depth); a = Vector3f(width, 0.0, 0.0); b = Vector3f(0.0, height, 0.0);
					   primitive::Rectangle* backWall = new primitive::Rectangle(p0, a, b);
					   backWall->setMaterial(matte3);
					   backWall->setColor(Color(1.0f, 1.0f, 1.0f));
					   backWall->flipNormal();
					   scene->addPrimitive(backWall);

					   // floor
					   p0 = Vector3f(0.0, 0.0, 0.0); a = Vector3f(0.0, 0.0, depth); b = Vector3f(width, 0.0, 0.0);
					   primitive::Rectangle* floor = new primitive::Rectangle(p0, a, b);
					   floor->setMaterial(matte3);
					   floor->setColor(Color(1.0f, 1.0f, 1.0f));
					   scene->addPrimitive(floor);

					   // ceiling
					   p0 = Vector3f(0.0, height, 0.0); a = Vector3f(0.0, 0.0, depth); b = Vector3f(width, 0.0, 0.0);
					   primitive::Rectangle* ceiling = new primitive::Rectangle(p0, a, b);
					   ceiling->setMaterial(matte3);
					   ceiling->setColor(Color(1.0f, 1.0f, 1.0f));
					   ceiling->flipNormal();
					   scene->addPrimitive(ceiling);

					   // short box
					   // top
					   p0 = Vector3f(13.0, 16.5, 6.5); a = Vector3f(-4.8, 0.0, 16.0); b = Vector3f(16.0, 0.0, 4.9);
					   primitive::Rectangle* shortTop = new primitive::Rectangle(p0, a, b);
					   shortTop->setMaterial(matte3);
					   shortTop->setColor(Color(1.0f, 1.0f, 1.0f));
					   scene->addPrimitive(shortTop);

					   // side 1 right
					   p0 = Vector3f(13.0, 0.0, 6.5); a = Vector3f(-4.8, 0.0, 16.0); b = Vector3f(0.0, 16.5, 0.0);
					   primitive::Rectangle* shortSide1 = new primitive::Rectangle(p0, a, b);
					   shortSide1->setMaterial(matte3);
					   shortSide1->setColor(Color(1.0f, 1.0f, 1.0f));
					   //shortSide1->flipNormal();
					   scene->addPrimitive(shortSide1);

					   // side 2
					   p0 = Vector3f(8.2, 0.0, 22.5); a = Vector3f(15.8, 0.0, 4.7);
					   primitive::Rectangle* shortSide2 = new primitive::Rectangle(p0, a, b);
					   shortSide2->setMaterial(matte3);
					   shortSide2->setColor(Color(1.0f, 1.0f, 1.0f));
					   //shortSide2->flipNormal();
					   scene->addPrimitive(shortSide2);

					   // side 3
					   p0 = Vector3f(24.2, 0.0, 27.4); a = Vector3f(4.8, 0.0, -16.0);
					   primitive::Rectangle* shortSide3 = new primitive::Rectangle(p0, a, b);
					   shortSide3->setMaterial(matte3);
					   shortSide3->setColor(Color(1.0f, 1.0f, 1.0f));
					   //shortSide3->flipNormal();
					   scene->addPrimitive(shortSide3);

					   // side 4 front
					   p0 = Vector3f(29.0, 0.0, 11.4); a = Vector3f(-16.0, 0.0, -4.9);
					   primitive::Rectangle* shortSide4 = new primitive::Rectangle(p0, a, b);
					   shortSide4->setMaterial(matte3);
					   shortSide4->setColor(Color(1.0f, 1.0f, 1.0f));
					   //shortSide4->flipNormal();
					   scene->addPrimitive(shortSide4);

					   // tall box
					   // top
					   p0 = Vector3f(42.3, 33.0, 24.7); a = Vector3f(-15.8, 0.0, 4.9); b = Vector3f(4.9, 0.0, 15.9);
					   primitive::Rectangle* tallTop = new primitive::Rectangle(p0, a, b);
					   tallTop->setMaterial(matte3);
					   tallTop->setColor(Color(1.0f, 1.0f, 1.0f));
					   tallTop->flipNormal();
					   scene->addPrimitive(tallTop);

					   // side 1 front
					   p0 = Vector3f(42.3, 0.0, 24.7); a = Vector3f(-15.8, 0.0, 4.9); b = Vector3f(0.0, 33.0, 0.0);
					   primitive::Rectangle* tallSide1 = new primitive::Rectangle(p0, a, b);
					   tallSide1->setMaterial(matte3);
					   tallSide1->setColor(Color(1.0f, 1.0f, 1.0f));
					   //tallSide1->flipNormal();
					   scene->addPrimitive(tallSide1);

					   // side 2
					   p0 = Vector3f(26.5, 0.0, 29.6); a = Vector3f(4.9, 0.0, 15.9);
					   primitive::Rectangle* tallSide2 = new primitive::Rectangle(p0, a, b);
					   tallSide2->setMaterial(matte3);
					   tallSide2->setColor(Color(1.0f, 1.0f, 1.0f));
					   //tallSide2->flipNormal();
					   scene->addPrimitive(tallSide2);

					   // side 3
					   p0 = Vector3f(31.4, 0.0, 45.5); a = Vector3f(15.8, 0.0, -4.9);
					   primitive::Rectangle* tallSide3 = new primitive::Rectangle(p0, a, b);
					   tallSide3->setMaterial(matte3);
					   tallSide3->setColor(Color(1.0f, 1.0f, 1.0f));
					   tallSide3->flipNormal();
					   scene->addPrimitive(tallSide3);

					   // side 4 right
					   p0 = Vector3f(47.2, 0.0, 40.6); a = Vector3f(-4.9, 0.0, -15.9);
					   primitive::Rectangle* tallSide4 = new primitive::Rectangle(p0, a, b);
					   tallSide4->setMaterial(matte3);
					   tallSide4->setColor(Color(1.0f, 1.0f, 1.0f));
					   //tallSide4->flipNormal();
					   scene->addPrimitive(tallSide4);

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