// includes
#include <windows.h>			// standard Windows app include
#include <stdio.h>
#include <vector> 
#include <iostream>
#include <stdlib.h>

#include "Bitmap.h"
#include "Camera.h"
#include "Sphere.h"
#include "Ray.h"

// globals

int height = 480;
int width = 640;

Bitmap *bitmap;
POINT g_OldCursorPos;
Camera *camera;
ProjectionMap *projectionMap;

//prototype funktions
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);
void ComposeFrame();
void ProcessInput(HWND hWnd);
void setCursortoMiddle(HWND hwnd);

enum DIRECTION {
	DIR_FORWARD = 1,
	DIR_BACKWARD = 2,
	DIR_LEFT = 4,
	DIR_RIGHT = 8,
	DIR_UP = 16,
	DIR_DOWN = 32,

	DIR_FORCE_32BIT = 0x7FFFFFFF
};
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

	try {

		bitmap = new Bitmap(height, width, 24);
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

	RECT rect;
	GetClientRect(hwnd, &rect);
	g_OldCursorPos.x = rect.right / 2;
	g_OldCursorPos.y = rect.bottom / 2;
	
	// main message loop
	while (true)
	{
		// Did we recieve a message, or are we idling ?
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// test if this is a quit
			if (msg.message == WM_QUIT) break;
			// translate and dispatch message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			ProcessInput(hwnd);
			
			//bitmap->writeBitmap24(RGBMatrix, 480, 640);
			//Dispatch WM_PAINT message
			ComposeFrame();
			InvalidateRect(hwnd, 0, true);


			hdc = GetDC(hwnd);
			SwapBuffers(hdc);			// bring backbuffer to foreground
			ReleaseDC(hwnd, hdc);

		} // end If messages waiting
	} // end while

	//avoid memory leaks
	delete bitmap;
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


					  // set the cursor to the middle of the window and capture the window via "SendMessage"
					  GetClientRect(hWnd, &rect);
					  pt.x = rect.right / 2;
					  pt.y = rect.bottom / 2;
					  SetCursorPos(pt.x , pt.y);
					  SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));


					  Vector3f camPos{ 0, 0, 5 };
					  Vector3f xAxis{ 1, 0, 0 };
					  Vector3f yAxis{ 0, 1, 0 };
					  Vector3f zAxis{ 0, 0, 1 };

					  camera = new Camera(camPos, xAxis, yAxis, zAxis);
					  projectionMap = new ProjectionMap(height, width, 45);


					  return 0;
	}
	case WM_PAINT:
	{

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
		delete camera;
		delete projectionMap;

		PostQuitMessage(0);
		return 0;


	case WM_LBUTTONDOWN:
	{	// Capture the mouse
		setCursortoMiddle(hWnd);
		SetCapture(hWnd);

	} break;
	case WM_KEYDOWN:
	{
					   switch (wParam)
					   {

					   case VK_ESCAPE:
					   {
										 PostQuitMessage(0);
										 return 0;
					   }break;
					   case VK_SPACE:
					   {
										ReleaseCapture();
										return 0;
					   }break;
					   return 0;
					   }break;

					   return 0;
	}break;

	case WM_ERASEBKGND:
		return true;


	}



	return (DefWindowProc(hWnd, message, wParam, lParam));
}




void ProcessInput(HWND hWnd)
{
	static UCHAR pKeyBuffer[256];
	ULONG        Direction = 0;
	POINT        CursorPos;
	float        X = 0.0f, Y = 0.0f;

	// Retrieve keyboard state
	if (!GetKeyboardState(pKeyBuffer)) return;

	// Check the relevant keys
	if (pKeyBuffer['W'] & 0xF0) Direction |= DIR_FORWARD;
	if (pKeyBuffer['S'] & 0xF0) Direction |= DIR_BACKWARD;
	if (pKeyBuffer['A'] & 0xF0) Direction |= DIR_LEFT;
	if (pKeyBuffer['D'] & 0xF0) Direction |= DIR_RIGHT;
	if (pKeyBuffer['E'] & 0xF0) Direction |= DIR_UP;
	if (pKeyBuffer['Q'] & 0xF0) Direction |= DIR_DOWN;

	// Now process the mouse (if the button is pressed)
	if (GetCapture() == hWnd)
	{
		// Hide the mouse pointer
		SetCursor(NULL);

		// Retrieve the cursor position
		GetCursorPos(&CursorPos);

		// Calculate mouse rotational values
		X = (float)(g_OldCursorPos.x - CursorPos.x) * 0.1;
		Y = (float)(g_OldCursorPos.y - CursorPos.y) * 0.1;

		// Reset our cursor position so we can keep going forever :)
		SetCursorPos(g_OldCursorPos.x, g_OldCursorPos.y);



		if (Direction > 0 || X != 0.0f || Y != 0.0f)
		{
			// Rotate camera
			if (X || Y)
			{
				camera->rotate(X, Y);
				//

			} // End if any rotation


			if (Direction)
			{
				float dx = 0, dy = 0, dz = 0;

				if (Direction & DIR_FORWARD) dz = +0.2f;
				if (Direction & DIR_BACKWARD) dz = -0.2f;
				if (Direction & DIR_LEFT) dx = -0.2f;
				if (Direction & DIR_RIGHT) dx = +0.2f;
				if (Direction & DIR_UP) dy = +0.2f;
				if (Direction & DIR_DOWN) dy = -0.2f;
				
				camera->move(dx, dy, dz);

			}

		}// End if any movement
	} // End if Captured
}


void ComposeFrame(){

	//scene objects
	Sphere sphere(Vector3f(0, 0, 0), 1);

	int n = 1;

	float vps = 1.0;

	float pxamnt;
	float pyamnt;

	//Torus torus(0.5, 0.25);
	double tmp; 
	for (int y = 0; y < 480; y++){
		for (int x = 0; x < 640; x++){
			
			Color color = Color(0, 0, 0);
			float r = 0;

			for (int p = 0; p < n; p++)	{		// up pixel
				for (int q = 0; q < n; q++) {

					//mapping from Raster Space to Camera Space
					Vector3f rayDirection = projectionMap->map2(x, y, q, p, n, camera->getViewDirection(), camera->getCamX(), camera->getCamY());

					//Vector3f rayDirection = projectionMap->map(x, y, camera->getViewDirection(), camera->getCamX(), camera->getCamY());
					Ray ray(camera->getPosition(), rayDirection);

					if (sphere.RaySphereIntersection(ray)){
					//	color = color + Color(255, 0, 0);
						
						r = r + 255.0;
					}
					else{
						r = r + 0;
					//	color = color + Color(0, 0, 0);
						//bitmap->setPixel24(x, y, color);
					}

				}

			}	

			//std::cout << r << std::endl;
			r = r / (n*n);
			//std::cout << r << std::endl;
			color = Color(r, 0, 0);

			bitmap->setPixel24(x, y, color);
		}
	}


}

void setCursortoMiddle(HWND hwnd){

	RECT rect;

	GetClientRect(hwnd, &rect);
	SetCursorPos(rect.right / 2, rect.bottom / 2);

}