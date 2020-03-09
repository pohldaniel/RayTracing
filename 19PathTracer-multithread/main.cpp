#include <atomic>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <future>

#include "TVector3.h"
#include "SMaterial.h"
#include "SAABB.h"
#include "SOBB.h"
#include "STriangle.h"
#include "SSphere.h"
#include "SQuad.h"
#include "SRayHitInfo.h"
#include "STimer.h"

#include <windows.h> // for bitmap headers
#include <iostream>
#include <mutex>


#define FORCE_SINGLE_THREAD() 0

#define COSINE_WEIGHTED_HEMISPHERE_SAMPLES() 1
#define JITTER_AA() 1
#define RENDER_SCENE() 3

//=================================================================================
// User tweakable parameters - Scenes	Globals
//=================================================================================
const UINT WM_APP_MY_THREAD_UPDATE = WM_APP + 0;
// image size
const size_t c_imageWidth = 512;
const size_t c_imageHeight = 512;

// sampling parameters
const size_t c_samplesPerPixel = 100;
const size_t c_numBounces = 5;
const float c_rayBounceEpsilon = 0.001f;

// multithreaded rendering
std::vector<TPixelRGBF32> g_pixels;
std::vector<std::thread> threads;
size_t numThreads;
STimer timer;

HBITMAP hbitmap;
unsigned char *data;
void restartTask(HWND hWnd);
std::mutex mutex;
bool waitAllThreads = false;
bool finishedAllThreads = false;
std::condition_variable m_condVar;
void RenderPixel(float u, float v, TPixelRGBF32& pixel);

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = { 278.0f, 273.0f, -800.0f };
const TVector3 c_cameraLookAt = { 278.0f, 273.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

class Stoppable {

	std::promise<void> exitSignal;
	std::future<void> futureObj;

public:
	Stoppable() : futureObj(exitSignal.get_future()) {

	}

	Stoppable(Stoppable && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj)) {

		std::cout << "Move Constructor is called" << std::endl;
	}

	Stoppable & operator=(Stoppable && obj) {
		std::cout << "Move Assignment is called" << std::endl;
		exitSignal = std::move(obj.exitSignal);
		futureObj = std::move(obj.futureObj);
		return *this;
	}

	// Task need to provide defination  for this function
	// It will be called by thread function
	virtual void run(STimer& timer, std::vector<TPixelRGBF32> & pixels) = 0;

	// Thread function to be executed by thread
	void operator()(STimer& timer, std::vector<TPixelRGBF32> & pixels) {
		run(timer, pixels);
	}

	//Checks if thread is requested to stop
	bool stopRequested() {

		// checks if value in future object is available
		if (futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
			return false;
		return true;
	}
	// Request the thread to stop by setting value in promise object
	void stop() {
		exitSignal.set_value();
	}

};

class RenderTask : public Stoppable {

	int currentRowIndex = 0;
	bool m_wait = false;
	bool m_finished = false;

public:

	int getCurrentRowIndex() { return currentRowIndex; }
	void reset() { currentRowIndex = 0; m_finished = false;  m_wait = false; }
	void setWaiting() { m_wait = true; }
	void setFinished() { m_finished = true; }
	bool isFinished() { return m_finished; }

	// Function to be executed by thread function
	void run(STimer& timer, std::vector<TPixelRGBF32> & pixels) {

		// Check if thread is requested to stop ?
		while (stopRequested() == false) {

			mutex.lock();
			waitAllThreads = m_wait;
			mutex.unlock();

			// each thread grabs a pixel at a time and renders it
			currentRowIndex = min(currentRowIndex, c_imageHeight);
			size_t rowIndex = currentRowIndex++;

			bool firstThread = rowIndex == 0;
			int lastPercent = -1;

			while ((rowIndex < c_imageHeight && !m_wait && !m_finished)) {
				for (size_t x = 0; x < c_imageWidth; ++x) {
					// render the pixel by taking multiple samples and incrementally averaging them
					for (size_t i = 0; i < c_samplesPerPixel; ++i) {

						float jitterX = JITTER_AA() ? RandomFloat() : 0.5f;
						float jitterY = JITTER_AA() ? RandomFloat() : 0.5f;
						float u = ((float)x + jitterX) / (float)c_imageWidth;
						float v = ((float)rowIndex + jitterY) / (float)c_imageHeight;

						TPixelRGBF32 sample;
						RenderPixel(u, v, sample);
						pixels[rowIndex * c_imageWidth + x] += (sample - pixels[rowIndex * c_imageWidth + x]) / float(i + 1.0f);
					}
				}

				// move to next row
				rowIndex = currentRowIndex++;

				// report our progress (from a single thread only)
				if (firstThread) {
					timer.ReportProgress(rowIndex, c_imageHeight);
				}

			}// rendering

			mutex.lock();
			finishedAllThreads = (currentRowIndex - 1) == c_imageHeight;
			mutex.unlock();
		}// stop requested

		 //std::cout << "Task End" << std::endl;
	}// end run

};
RenderTask task;


// the scene
// I slightly modified the cornell box from http://www.graphics.cornell.edu/online/box/data.html
const std::vector<SSphere> c_spheres = { };

const std::vector<STriangle> c_triangles = { };

const std::vector<SQuad> c_quads = {
    // floor diffuse
    SQuad({ 552.8f, 0.0f, 0.0f }, { 0.0f, 0.0f,   0.0f }, {   0.0f, 0.0f, 559.2f },{ 549.6f, 0.0f, 559.2f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f })),

    // Cieling emissive
    SQuad({ 556.0f, 548.8f,   0.0f },{ 556.0f, 548.8f, 559.2f },{ 0.0f, 548.8f, 559.2f },{ 0.0f, 548.8f,   0.0f }, SMaterial({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f })),

    // back wall diffuse
    SQuad({549.6f,   0.0f, 559.2f},{  0.0f,   0.0f, 559.2f},{  0.0f, 548.8f, 559.2f},{556.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),

    // left wall diffuse
    SQuad({0.0f,   0.0f, 559.2f},{0.0f,   0.0f,   0.0f},{0.0f, 548.8f,   0.0f},{0.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f })),

    // right wall diffuse
    SQuad({552.8f,   0.0f,   0.0f},{549.6f,   0.0f, 559.2f},{556.0f, 548.8f, 559.2f},{556.0f, 548.8f,   0.0f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f })),
};

const std::vector<SAABB> c_aabbs = { };

const std::vector<SOBB> c_obbs = {

	// left box diffuse
    SOBB( SAABB({ 185.5f, 82.5f, 169.0f },{ 82.5f, 82.5f, 82.5f },SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, -17.0f * c_pi / 180.0f),
	// rigth box diffuse
    SOBB(SAABB({ 368.5f, 165.0f, 351.25}, {82.5f, 165.0f, 82.5f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, 107.0f * c_pi / 180.0f),
};

const TVector3 c_rayMissColor = { 0.0f, 0.0f, 0.0f };
//=================================================================================
// Derived values
const size_t c_numPixels = c_imageWidth * c_imageHeight;
const float c_aspectRatio = float(c_imageWidth) / float(c_imageHeight);
const float c_cameraHorizFOV = c_cameraVerticalFOV * c_aspectRatio;
const float c_windowTop = tan(c_cameraVerticalFOV / 2.0f) * c_nearPlaneDistance;
const float c_windowRight = tan(c_cameraHorizFOV / 2.0f) * c_nearPlaneDistance;
const TVector3 c_cameraFwd = Normalize(c_cameraLookAt - c_cameraPos);
const TVector3 c_cameraRight = Cross({ 0.0f, 1.0f, 0.0f }, c_cameraFwd);
const TVector3 c_cameraUp = Cross(c_cameraFwd, c_cameraRight);

//=================================================================================


//=================================================================================
bool ClosestIntersection (const TVector3& rayPos, const TVector3& rayDir, SRayHitInfo& info){

    bool ret = false;
    for (const SSphere& s : c_spheres)
        ret |= RayIntersects(rayPos, rayDir, s, info);
    for (const STriangle& t : c_triangles)
        ret |= RayIntersects(rayPos, rayDir, t, info);
    for (const SQuad& q : c_quads)
        ret |= RayIntersects(rayPos, rayDir, q, info);
    for (const SAABB& a : c_aabbs)
        ret |= RayIntersects(rayPos, rayDir, a, info);
    for (const SOBB& o : c_obbs)
        ret |= RayIntersects(rayPos, rayDir, o, info);
    return ret;
}

//=================================================================================
TVector3 L_out (const SRayHitInfo& X, const TVector3& outDir, size_t bouncesLeft){

    // if no bounces left, return the ray miss color
    if (bouncesLeft == 0)
        return c_rayMissColor;

    // start with emissive lighting
    const SMaterial* material = X.m_material;

    TVector3 ret = material->m_emissive;

    // add in random recursive samples for global illumination
    {
#if COSINE_WEIGHTED_HEMISPHERE_SAMPLES()
        TVector3 newRayDir = CosineSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += L_out(info, -newRayDir, bouncesLeft - 1) * material->m_diffuse;
        else
            ret += c_rayMissColor * material->m_diffuse;
#else
        TVector3 newRayDir = UniformSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * L_out(info, -newRayDir, bouncesLeft - 1) * material->m_diffuse;
        else
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * c_rayMissColor * material->m_diffuse;
#endif
    }

    return ret;
}

//=================================================================================
TPixelRGBF32 L_in (const TPixelRGBF32& rayPos, const TPixelRGBF32& rayDir){

    // if this ray doesn't hit anything, return black / darkness
    SRayHitInfo info;
    if (!ClosestIntersection(rayPos, rayDir, info))
        return c_rayMissColor;

    // else, return the amount of light coming towards us from the object we hit
    return L_out(info, -rayDir, c_numBounces);
}

//=================================================================================
void RenderPixel (float u, float v, TPixelRGBF32& pixel){

    // make (u,v) go from [-1,1] instead of [0,1]
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    // find where the ray hits the near plane, and normalize that vector to get the ray direction.
    TVector3 rayStart = c_cameraPos + c_cameraFwd * c_nearPlaneDistance;
    rayStart += c_cameraRight * c_windowRight * u;
    rayStart += c_cameraUp * c_windowTop * v;
    TVector3 rayDir = Normalize(rayStart - c_cameraPos);

    // our pixel is the amount of light coming in to the position on our near plane from the ray direction
    pixel = L_in(rayStart, rayDir);
}

//=================================================================================

template <typename T>
T Lerp(const T& A, const T& B, float t){

    return A * (1.0f - t) + B * t;
}

//=================================================================================





//=================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle(L"Debug console");
	MoveWindow(GetConsoleWindow(), 790, 0, 500, 200, true);

	WNDCLASSEX windowClass;		// window class
	HWND	   hwnd;	// window handle
	MSG		   msg;				// message
	HDC		   hdc;

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
	windowClass.lpszClassName = L"MyClass";
	windowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);			// windows logo small icon
															// register the windows class
	if (!RegisterClassEx(&windowClass))
		return 0;

	hwnd = CreateWindowEx(NULL,						// extended style
		L"MyClass",									// class name
		L"RayTracer",							// app name
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

	// report the params
	numThreads = FORCE_SINGLE_THREAD() ? 1 : std::thread::hardware_concurrency();
	printf("Rendering a %zux%zu image with %zu samples per pixel and %zu ray bounces.\n", c_imageWidth, c_imageHeight, c_samplesPerPixel, c_numBounces);
	printf("Using %zu threads.\n", numThreads);
	threads.resize(numThreads);

	for (std::thread& t : threads){

		t = std::thread([&]() { task.run(std::ref(timer), std::ref(g_pixels));});
	}

	while (true) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				
				task.setWaiting();
				std::unique_lock<std::mutex> mlock(mutex);
				m_condVar.wait(mlock, [] {return  !waitAllThreads; });
				task.stop();
				break;				
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}else {
						
			if (finishedAllThreads) {
			
				if (!task.isFinished()) {
					task.setFinished();
					timer.Report();
					PostMessage(hwnd, WM_APP_MY_THREAD_UPDATE, NULL, 0);
				}

			}else {
				//std::this_thread::sleep_for(std::chrono::seconds(5));

				PostMessage(hwnd, WM_APP_MY_THREAD_UPDATE, NULL, 0);
				hdc = GetDC(hwnd);
				SwapBuffers(hdc);
				ReleaseDC(hwnd, hdc);
			}
		} // end If messages waiting
	} // end while

	for (std::thread& t : threads) {
		t.join();		
	}

	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){

	HDC hdc, hmemdc;
	PAINTSTRUCT ps;


	switch (message){

	case WM_CREATE: {
		
		BITMAPINFO* bmi = (BITMAPINFO*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
		bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi->bmiHeader.biWidth = (LONG)c_imageWidth;
		bmi->bmiHeader.biHeight = (LONG)c_imageHeight;
		bmi->bmiHeader.biPlanes = 1;
		bmi->bmiHeader.biBitCount = 24;
		bmi->bmiHeader.biCompression = 0;
		bmi->bmiHeader.biSizeImage = (DWORD)c_numPixels * 3;
		bmi->bmiHeader.biXPelsPerMeter = 0;
		bmi->bmiHeader.biYPelsPerMeter = 0;
		bmi->bmiHeader.biClrUsed = 0;
		bmi->bmiHeader.biClrImportant = 0;

		data = (unsigned char*)LocalAlloc(LPTR, bmi->bmiHeader.biSizeImage);
		hbitmap = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (VOID**)&data, NULL, 0);

		// allocate memory for our rendered image
		g_pixels.resize(c_numPixels);

		for (size_t i = 0; i < c_numPixels; ++i) {
			g_pixels[i][0] = 0.0;
			g_pixels[i][1] = 0.0;
			g_pixels[i][2] = 0.0;
		}

		return 0;
	}case WM_PAINT: {
		
		// convert from RGB F32 to BGR U8
		for (size_t i = 0, j = 0; i < c_numPixels; ++i, j = j + 3) {
			const TPixelRGBF32& src = g_pixels[i];


			// apply gamma correction
			TPixelRGBF32 correctedPixel;
			correctedPixel[0] = powf(src[0], 1.0f / 2.2f);
			correctedPixel[1] = powf(src[1], 1.0f / 2.2f);
			correctedPixel[2] = powf(src[2], 1.0f / 2.2f);

			// clamp and convert
			data[j] = uint8(Clamp(correctedPixel[2] * 255.0f, 0.0f, 255.0f));
			data[j + 1] = uint8(Clamp(correctedPixel[1] * 255.0f, 0.0f, 255.0f));
			data[j + 2] = uint8(Clamp(correctedPixel[0] * 255.0f, 0.0f, 255.0f));
		}

		hdc = BeginPaint(hWnd, &ps);

		hmemdc = CreateCompatibleDC(NULL);
		HGDIOBJ m_old = SelectObject(hmemdc, hbitmap);

		BitBlt(hdc, c_imageWidth / 12, c_imageHeight / 12, c_imageWidth, c_imageHeight, hmemdc, 0, 0, SRCCOPY);

		SelectObject(hmemdc, m_old);
		DeleteDC(hmemdc);

		EndPaint(hWnd, &ps);

		return 0;

	}case WM_DESTROY: {

		PostQuitMessage(0);
		return 0;

	}case WM_KEYDOWN: {

		switch (wParam) {

		case VK_ESCAPE: {

			PostQuitMessage(0);
			return 0;
		} case 'M': {
			
			restartTask(hWnd);
			break;
		}
						
						

			return 0;

		}
		break;
		return 0;
	}case WM_APP_MY_THREAD_UPDATE: {

		RECT rect;
		ZeroMemory(&rect, sizeof(RECT));
		GetClientRect(hWnd, &rect);

		rect.top = rect.top + c_imageHeight / 12;
		rect.bottom = rect.top + c_imageHeight ;

		rect.left = rect.left + c_imageWidth / 12;
		rect.right = rect.left + c_imageWidth;
		

		InvalidateRect(hWnd, &rect, true);
		//UpdateWindow(hWnd);
	}

	break;
	}//end switch


	return (DefWindowProc(hWnd, message, wParam, lParam));
}


void restartTask(HWND hWnd) {

	task.setWaiting();
	std::unique_lock<std::mutex> mlock(mutex);
	m_condVar.wait(mlock, [] {return  !waitAllThreads; });

	for (size_t i = 0; i < c_numPixels; ++i) {
		g_pixels[i][0] = 0.0;
		g_pixels[i][1] = 0.0;
		g_pixels[i][2] = 0.0;
	}

	InvalidateRect(hWnd, NULL, true);
	task.reset();
	waitAllThreads = false;
	finishedAllThreads = false;
	timer.Restart();
}

