#include <atomic>
#include <vector>
#include <thread>
#include <windows.h> // for bitmap headers
#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include <mutex>


#include "TVector3.h"
#include "RenderTask.h"

#include "Primitive.h"
#include "Camera.h"
#include "Scene.h"

POINT g_OldCursorPos;
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
size_t c_samplesPerPixel = 10000;
const size_t c_numBounces = 5;
const float c_rayBounceEpsilon = 0.001f;

// multithreaded rendering
std::vector<TPixelRGBF32> g_pixels;
unsigned char *g_pixels2 = NULL;

std::vector<std::thread> threads;
size_t numThreads;
STimer timer;

HBITMAP hbitmap;
Projection *camera;
Scene *scene;

void restartTask(HWND hWnd);
std::mutex mutex;
bool waitAllThreads = false;
bool finishedAllThreads = false;

Color RenderPixel(float u, float v, Color& color);

RenderTask *renderTask;

float RandomFloat2() {

	/*
	// Xorshift random number algorithm invented by George Marsaglia
	static uint32_t rng_state = 0xf2eec0de;
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return float(rng_state) * (1.0f / 4294967296.0f);
	*/

	// alternately, using a standard c++ prng
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(mt);
}

class Stoppable {

	std::promise<void> exitSignal;
	std::future<void> futureObj;

public:
	Stoppable() : futureObj(exitSignal.get_future()) {

	}

	Stoppable(Stoppable && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj)) {

		//std::cout << "Move Constructor is called" << std::endl;
	}

	Stoppable & operator=(Stoppable && obj) {
		//std::cout << "Move Assignment is called" << std::endl;
		exitSignal = std::move(obj.exitSignal);
		futureObj = std::move(obj.futureObj);
		return *this;
	}

	// Task need to provide defination  for this function
	// It will be called by thread function
	virtual void run(STimer& timer, std::vector<TPixelRGBF32> & pixels, unsigned char*&data) = 0;

	// Thread function to be executed by thread
	void operator()(STimer& timer, std::vector<TPixelRGBF32> & pixels, unsigned char*&data) {
		run(timer, pixels, data);
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

class Render : public Stoppable {

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
	void run(STimer& timer, std::vector<TPixelRGBF32> & pixels, unsigned char* &pixels2) {

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

			while (((rowIndex < c_imageHeight && !m_finished && !m_wait))) {

				for (size_t x = 0, k = 0; x < c_imageWidth; ++x, k = k + 3) {

					if (m_wait) {
						break;
					}

					// render the pixel by taking multiple samples and incrementally averaging them
					for (size_t i = 0; i < c_samplesPerPixel; ++i) {
						float jitterX = JITTER_AA() ? RandomFloat2() : 0.5f;
						float jitterY = JITTER_AA() ? RandomFloat2() : 0.5f;
						float u = ((float)x + jitterX);
						float v = ((float)rowIndex + jitterY);
						Color color;
						
						RenderPixel(u, v, color);
						TPixelRGBF32 sample;
						sample[0] = color.r; sample[1] = color.g; sample[2] = color.b;

						pixels[rowIndex * c_imageWidth + x] += sample;
						//pixels[rowIndex * c_imageWidth + x] += (sample - pixels[rowIndex * c_imageWidth + x]) / float(i + 1.0f);
					}

					for (size_t j = 0; j < 3; j++) {
						pixels2[rowIndex * c_imageWidth * 3 + k + j] = uint8(Clamp((pixels[rowIndex * c_imageWidth + x][2 - j] / (c_samplesPerPixel)), 0.0f, 1.0f)* 255.0f);
						//pixels2[rowIndex * c_imageWidth * 3 + k + j] = uint8(Clamp(powf(pixels[rowIndex * c_imageWidth + x][2 - j], 1.0f / 2.2f)* 255.0f, 0.0f, 255.0f));
					}
				}
				// move to next row
				rowIndex = currentRowIndex++;
			}// rendering

			mutex.lock();
			finishedAllThreads = (currentRowIndex - 1) == c_imageHeight;
			mutex.unlock();
		}// stop requested
	}// end run
};
Render task;
//=================================================================================
Color L_out(const Vector3f& outDir, size_t bouncesLeft, const Hit& hit) {

	// if no bounces left, return the ray miss color
	if (bouncesLeft == 0)
		return Color(0.0, 0.0, 0.0);

	Material* material = hit.material;

	Color ret = dynamic_cast<Emissive*>(material) ? Color(1.0, 1.0, 1.0) : Color(0.0, 0.0, 0.0);

	Vector3f normal = hit.normal;
	Vector3f intersectionPoint = hit.hitPoint;
	Color diffuse = hit.color;
	 
	// add in random recursive samples for global illumination
	{
#if COSINE_WEIGHTED_HEMISPHERE_SAMPLES()
		Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
		Vector3f newRayDir = CosineSampleHemisphere(normal);
		Vector3f newRayOrigin = transformedhitPoint + newRayDir * c_rayBounceEpsilon;

		Hit hit = scene->hitObjects2(Ray(newRayOrigin, newRayDir));
		if (hit.hitObject) {
			ret = ret + L_out(-newRayDir, bouncesLeft - 1, hit) * diffuse;
		}else {
			ret = ret + Color(0.0, 0.0, 0.0) * diffuse;
		}		
#else
		// this point is in  eyespace 
		Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
		Vector3f newRayDir = UniformSampleHemisphere(normal);
		Vector3f newRayOrigin = transformedhitPoint + newRayDir * c_rayBounceEpsilon;

		Hit hit = scene->hitObjects2(Ray(newRayOrigin, newRayDir));

		if (hit.hitObject) {
			ret = ret + L_out(-newRayDir, bouncesLeft - 1, hit) * diffuse * Vector3f::dot(newRayDir, normal) * 2.0f ;
		}else {
			ret = ret + Color(0.0, 0.0, 0.0) * diffuse * Vector3f::dot(newRayDir, normal) * 2.0f;
		}		
#endif
	}
	return ret;
}


//=================================================================================
Color L_out2(size_t bouncesLeft, const Hit& hit) {

	// if no bounces left, return the ray miss color
	if (bouncesLeft == 0)
		return Color(0.0, 0.0, 0.0);

	Material* material = hit.material;
	
	if (dynamic_cast<Emissive*>(material)) {
		for (unsigned int i = 0; i < hit.scene->m_lights.size(); i++) {
			AreaLight* light = static_cast<AreaLight*>(hit.scene->m_lights[i].get());
			if (light->m_primitive.get() == hit.primitive) {
				return hit.color *(1.0 / light->pdf(hit));
			}
		}
	}

	Vector3f normal = hit.normal;
	Vector3f intersectionPoint = hit.hitPoint;
	Color diffuse = hit.color;
	

	// add in random recursive samples for global illumination
	{
#if COSINE_WEIGHTED_HEMISPHERE_SAMPLES()
		// this point is in  eyespace 
		Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
		Vector3f newRayOrigin;
		Vector3f newRayDir;

		if (dynamic_cast<Reflective*>(material)) {
			float dot = Vector3f::dot(hit.originalRay.direction, normal);
			newRayDir = hit.originalRay.direction - (normal  * 2.0f * dot);
			newRayOrigin = dot < 0 ? transformedhitPoint + normal * c_rayBounceEpsilon : transformedhitPoint - normal * c_rayBounceEpsilon;
		}else {
			newRayDir = CosineSampleHemisphere(normal);
			newRayOrigin = transformedhitPoint + newRayDir * c_rayBounceEpsilon;			
		}

		Hit hit = scene->hitObjects2(Ray(newRayOrigin, newRayDir));
		return hit.hitObject ? L_out2(bouncesLeft - 1, hit) * diffuse : Color(0.0, 0.0, 0.0);

#else
		// this point is in  eyespace 
		Vector3f transformedhitPoint = hit.originalRay.origin + hit.originalRay.direction * hit.t;
		Vector3f newRayOrigin;
		Vector3f newRayDir;

		if(dynamic_cast<Reflective*>(material)){
		
			float dot = Vector3f::dot(hit.originalRay.direction, normal);
			newRayDir = hit.originalRay.direction - (normal  * 2.0f * dot);
			newRayOrigin  = dot < 0 ? transformedhitPoint + normal * 0.01 : transformedhitPoint - normal * 0.01;			
		}else{

			newRayDir = UniformSampleHemisphere(normal);
			newRayOrigin = transformedhitPoint + newRayDir * c_rayBounceEpsilon;

			float lambert = Vector3f::dot(normal, newRayDir);
			float pdf = max(1e-6f, max(0, Vector3f::dot(normal, newRayDir)));
			diffuse = diffuse * lambert * 2.0;			
		}
			
		Hit hit = scene->hitObjects2(Ray(newRayOrigin, newRayDir));
		return hit.hitObject ? L_out2(bouncesLeft - 1, hit) * diffuse : Color(0.0, 0.0, 0.0);	
#endif
	}
}

//=================================================================================
Color L_in(const Vector3f& rayPos, const Vector3f& rayDir) {

	Hit hit = scene->hitObjects2(Ray(rayPos, rayDir));

	if (!hit.hitObject)
		return Color(0.0, 0.0, 0.0);
	
	return L_out2(c_numBounces, hit) * 0.003;
}


//=================================================================================
Color RenderPixel(float u, float v, Color& color) {
	 color = L_in(camera->getPosition(), camera->rasterToCamera(u, v));
	 return color;
}
//=================================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	SetConsoleTitle(L"Debug console");
	MoveWindow(GetConsoleWindow(), 790, 0, 500, 200, true);

	std::cout << "w, a, s, d, mouse : move camera" << std::endl;
	std::cout << "space				: release capture" << std::endl;
	std::cout << "+, -              : increase, decrease samples" << std::endl;
	std::cout << "k                 : send WM_PAINT" << std::endl;
	std::cout << "m                 : restart rendering" << std::endl;

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


	int numSample = 100;

	Vector3f camPos(278.0f, 273.0f, -800.0f);
	Vector3f target(278.0f, 273.0f, 0.0f);
	Vector3f up(0, 1.0, 0.0);
	camera = new Projection(camPos, target, up);

	Sampler* samplerMatte = new MultiJittered(numSample, 83);
	samplerMatte->mapSamplesToHemisphere(1.0);


	Vector3f p0, a, b;
	// box dimensions
	double width = 552.8f;   	// x direction
	double height = 548.8f;  	// y direction
	double depth = 559.2f;	// z direction

	scene = new Scene(ViewPlane(300, 300, 1.0), Color(0.0, 0.0, 0.0));

	Emissive* emissiveMat = new Emissive();
	emissiveMat->setScaleRadiance(100.0);
	emissiveMat->setColor(Color(1.0, 1.0, 1.0));

	Matte* matte1 = new Matte();
	matte1->setKa(0.25);
	matte1->setKd(0.6);
	matte1->setSampler(samplerMatte);
	
	Reflective *reflective = new Reflective();
	reflective->setReflectionColor(1.0);
	reflective->setFrensel(0.4);

	QuadCC* light = new QuadCC(Vector3f(343.0f, 548.6f, 227.0f + 45.0f), Vector3f(343.0f, 548.6f, 332.0f + 45.0f), Vector3f(213.0f, 548.6f, 332.0f + 45.0f), Vector3f(213.0f, 548.6f, 227.0f + 45.0f));
	light->setMaterial(emissiveMat);
	light->setColor(Color(1.0, 1.0, 1.0));
	scene->addPrimitive(light);

	p0 = Vector3f(width, height, 0.0); a = Vector3f(0.0, 0.0, depth); b = Vector3f(-width, 0.0, 0.0);
	QuadCC* ceiling = new QuadCC(Vector3f(556.0f, 548.8f, 0.0f), Vector3f(556.0f, 548.8f, 559.2f), Vector3f(0.0f, 548.8f, 559.2f), Vector3f(0.0f, 548.8f, 0.0f));
	ceiling->setMaterial(emissiveMat);
	ceiling->setColor(Color(0.7f, 0.7f, 0.7f));
	scene->addPrimitive(ceiling);

	AreaLight* areaLight = new AreaLight;
	areaLight->setObject(light);
	areaLight->setShadows(true);
	scene->addLight(areaLight);

	QuadCC* leftWall = new QuadCC(Vector3f(0.0f, 0.0f, 559.2f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 548.8f, 0.0f), Vector3f(0.0f, 548.8f, 559.2f));
	leftWall->setMaterial(matte1);
	leftWall->setColor(Color(0.2f, 0.7f, 0.2f));  // green
	scene->addPrimitive(leftWall);

	QuadCC* floor = new QuadCC(Vector3f(552.8f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 559.2f), Vector3f(549.6f, 0.0f, 559.2f));
	floor->setMaterial(matte1);
	floor->setColor(Color(0.7f, 0.7f, 0.7f));
	scene->addPrimitive(floor);

	QuadCC* rightWall = new QuadCC(Vector3f(552.8f, 0.0f, 0.0f), Vector3f(549.6f, 0.0f, 559.2f), Vector3f(556.0f, 548.8f, 559.2f), Vector3f(556.0f, 548.8f, 0.0f));
	rightWall->setMaterial(matte1);
	rightWall->setColor(Color(0.7f, 0.2f, 0.2f));  // red
	scene->addPrimitive(rightWall);
	
	QuadCC* backWall = new QuadCC(Vector3f(549.6f, 0.0f, 559.2f), Vector3f(0.0f, 0.0f, 559.2f), Vector3f(0.0f, 548.8f, 559.2f), Vector3f(556.0f, 548.8f, 559.2f));
	backWall->setMaterial(matte1);
	backWall->setColor(Color(0.7f, 0.7f, 0.7f));
	scene->addPrimitive(backWall);
	
	Sphere* sphere = new Sphere(Vector3f(185.5f, 225.5f, 169.0f), 60);
	sphere->setMaterial(reflective);
	sphere->setColor(Color(1.0f, 1.0f, 1.0f));
	scene->addPrimitive(sphere);

	AABB* smallBox = new AABB(Vector3f(0.0, 0.0, 0.0), Vector3f(82.5f, 82.5f, 82.5f));
	smallBox->setMaterial(matte1);
	smallBox->setColor(Color(0.7f, 0.7f, 0.7f));

	Instance* _smallBox = new Instance(smallBox);
	_smallBox->rotate(Vector3f(0.0, 1.0, 0.0), -17.0f);
	_smallBox->translate(185.5f, 82.5f, 169.0f);
	scene->addPrimitive(_smallBox);

	AABB* largeBox = new AABB(Vector3f(0.0, 0.0, 0.0), Vector3f(82.5f, 165.0f, 82.5f));
	largeBox->setMaterial(matte1);
	largeBox->setColor(Color(0.7f, 0.7f, 0.7f));

	Instance* _largeBox = new Instance(largeBox);
	_largeBox->rotate(Vector3f(0.0, 1.0, 0.0), 107.0f );
	_largeBox->translate(368.5f, 165.0f, 351.25);
	scene->addPrimitive(_largeBox);

	// report the params
	numThreads = FORCE_SINGLE_THREAD() ? 1 : std::thread::hardware_concurrency();
	std::cout << std::string("Using ") + std::to_string(numThreads) + std::string(" threads.") << std::endl;

	threads.resize(numThreads);
	for (std::thread& t : threads) {
		t = std::thread([&]() { task.run(std::ref(timer), std::ref(g_pixels), std::ref(g_pixels2)); });
	}

	renderTask = new RenderTask();

	while (true) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {

				task.setWaiting();
				while (!waitAllThreads) {
					std::this_thread::yield();
				}

				task.stop();
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}else {
			
			ProcessInput(hwnd);
			if (finishedAllThreads) {

				if (!task.isFinished()) {
					task.setFinished();
					timer.Report();
					PostMessage(hwnd, WM_APP_MY_THREAD_UPDATE, NULL, 0);
				}

			}else {
				

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	HDC hdc, hmemdc;
	PAINTSTRUCT ps;
	POINT pt;
	RECT rect;

	switch (message) {

	case WM_CREATE: {

		GetClientRect(hWnd, &rect);
		g_OldCursorPos.x = rect.right / 2;
		g_OldCursorPos.y = rect.bottom / 2;
		pt.x = rect.right / 2;
		pt.y = rect.bottom / 2;
		SetCursorPos(pt.x, pt.y);
		// set the cursor to the middle of the window and capture the window via "SendMessage"
		SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));

		g_pixels.resize(c_imageWidth * c_imageHeight);
		g_pixels2 = (unsigned char*)LocalAlloc(LPTR, c_imageHeight *c_imageWidth * 3 * sizeof(unsigned char));
		memset(g_pixels2, 0, c_imageHeight *c_imageWidth * 3 * sizeof(unsigned char));

		BITMAPINFO* bmi = (BITMAPINFO*)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
		bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi->bmiHeader.biWidth = (LONG)c_imageWidth;
		bmi->bmiHeader.biHeight = (LONG)c_imageHeight;
		bmi->bmiHeader.biPlanes = 1;
		bmi->bmiHeader.biBitCount = 24;
		bmi->bmiHeader.biCompression = 0;
		bmi->bmiHeader.biSizeImage = (DWORD)c_imageWidth * c_imageHeight * 3;
		bmi->bmiHeader.biXPelsPerMeter = 0;
		bmi->bmiHeader.biYPelsPerMeter = 0;
		bmi->bmiHeader.biClrUsed = 0;
		bmi->bmiHeader.biClrImportant = 0;

		hbitmap = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (VOID**)&g_pixels2, NULL, 0);

		return 0;
	}case WM_PAINT: {

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
	case WM_LBUTTONDOWN: {
		// Capture the mouse
		setCursortoMiddle(hWnd);
		SetCapture(hWnd);

	} break;
	}case WM_KEYDOWN: {

		switch (wParam) {

		case VK_ESCAPE: {

			PostQuitMessage(0);
			return 0;
		} case 'M': {
			restartTask(hWnd);
			break;
		} case 'K': {
			PostMessage(hWnd, WM_APP_MY_THREAD_UPDATE, NULL, 0);
			break;
		}case VK_ADD: {
			c_samplesPerPixel = c_samplesPerPixel + 1000;
			std::cout << c_samplesPerPixel << std::endl;
			break;
		}case VK_SUBTRACT: {
			c_samplesPerPixel = c_samplesPerPixel - 1000;
			std::cout << c_samplesPerPixel << std::endl;
			break;
		}
		
		case VK_RETURN: {

			break;
		}case VK_SPACE: {

			ReleaseCapture();
			return 0;
		}break;


		return 0;

		}
		break;
		return 0;
	}case WM_APP_MY_THREAD_UPDATE: {

		RECT rect;
		ZeroMemory(&rect, sizeof(RECT));
		GetClientRect(hWnd, &rect);

		rect.top = rect.top + c_imageHeight / 12;
		rect.bottom = rect.top + c_imageHeight;

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

	while (!waitAllThreads) {
		std::this_thread::yield();
	}


	std::fill(g_pixels.begin(), g_pixels.end(), TPixelRGBF32{0.0, 0.0, 0.0});
	memset(g_pixels2, 0, 512 *512 * 3 * sizeof(unsigned char));

	InvalidateRect(hWnd, NULL, true);
	task.reset();
	waitAllThreads = false;
	finishedAllThreads = false;
	timer.Restart();

}

void ProcessInput(HWND hWnd) {

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
	if (GetCapture() == hWnd) {

		// Hide the mouse pointer
		SetCursor(NULL);

		// Retrieve the cursor position
		GetCursorPos(&CursorPos);

		// Calculate mouse rotational values
		X = (float)(g_OldCursorPos.x - CursorPos.x) * 0.1;
		Y = (float)(g_OldCursorPos.y - CursorPos.y) * 0.1;

		// Reset our cursor position so we can keep going forever :)
		SetCursorPos(g_OldCursorPos.x, g_OldCursorPos.y);

		if (Direction > 0 || X != 0.0f || Y != 0.0f) {

			// Rotate camera
			if (X || Y) {
				camera->rotate(X, Y);
				//
			} // End if any rotation

			if (Direction) {
				float dx = 0, dy = 0, dz = 0;

				if (Direction & DIR_FORWARD) dz = +20.2f;
				if (Direction & DIR_BACKWARD) dz = -20.2f;
				if (Direction & DIR_LEFT) dx = -20.2f;
				if (Direction & DIR_RIGHT) dx = +20.2f;
				if (Direction & DIR_UP) dy = +20.2f;
				if (Direction & DIR_DOWN) dy = -20.2f;

				camera->move(dx, dy, dz);
			}
			restartTask(hWnd);
		}// End if any movement
	} // End if Captured
}

void setCursortoMiddle(HWND hwnd) {

	RECT rect;

	GetClientRect(hwnd, &rect);
	SetCursorPos(rect.right / 2, rect.bottom / 2);
}