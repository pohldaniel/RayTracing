// includes
#include <windows.h>			// standard Windows app include
#include <GL.h>	


#include <stdio.h>
#include <vector> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "Extension.h"
#include "Camera.h"
//program 1: Sphere
//program 2: Torus
//program 3: Plane
//program 4: Disk
//program 5: Rectangle
//program 6: Triangle
//program 7: Cylinder
//program 8: Ellipsoid

int usedProgram = 2;
int height = 600;
int width =800;

Camera *camera;
GLuint u_campos, u_camright, u_camup, u_viewdir, u_projection, u_resx, u_resy, u_aspectRatio, u_scale, u_time;
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

bool g_enableVerticalSync;
GLuint vbo;
GLuint program;
GLuint uniform_rot, uniform_pos, uniform_eyedist, uniform_stepsize, uniform_accuracy, uniform_user_params0, uniform_user_params1;

//prototype funktions
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);

void ReadTextFile(const char *pszFilename, std::string &buffer);
void InitApp(HWND hWnd);
void Cleanup();
void EnableVerticalSync(bool enableVerticalSync);

GLuint CompileShader(GLenum type, const char *pszSource);
GLuint LinkShaders(GLuint vertShader, GLuint fragShader);
GLuint LoadShaderProgram(GLenum type, const char *pszFilename);
GLuint quadFullScreenVbo();
GLuint createProgram();


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
		"Render to Texture",							// app name
		WS_OVERLAPPEDWINDOW,
		0, 0,										// x,y coordinate
		width,
		height,										// width, height
		NULL,										// handle to parent
		NULL,										// handle to menu
		hInstance,									// application instance
		NULL);										// no extra params

	// check if window creation failed (hwnd would equal NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			// display the window
	UpdateWindow(hwnd);					// update the window

	InitApp(hwnd);



	GLuint uniform_time = glGetUniformLocation(program, "u_time");
	float offset = 0;
	float time = 45;

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
			
			time = time + 0.5;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.2, 0.2, 0.2, 1.0);
			glUseProgram(program);
			glUniform1f(u_time, time);
			glUniform3fv(u_campos, 1, camera->getPosition()[0]);
			glUniform3fv(u_camright, 1, camera->getCamX()[0]);
			glUniform3fv(u_camup, 1, camera->getCamY()[0]);
			glUniform3fv(u_viewdir, 1, camera->getViewDirection()[0]);
			
			
			// Draw one quad so that we get one fragment covering the whole
			// screen.
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexAttribPointer(
				0,					// attribute
				2,                  // number of elements per vertex, here (x,y)
				GL_FLOAT,           // the type of each element
				GL_FALSE,           // take our values as-is
				0,                  // no extra data between each position
				0                   // offset of first element
				);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
			glUseProgram(0);

			hdc = GetDC(hwnd);
			SwapBuffers(hdc);			// bring backbuffer to foreground
			ReleaseDC(hwnd, hdc);

		} // end If messages waiting
	} // end while

	//avoid memory leaks
	Cleanup();
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	RECT rect;
	switch (message)
	{
	
	case WM_DESTROY:
	{

		PostQuitMessage(0);
		return 0;
	}

	case WM_CREATE:
	{
		// set the cursor to the middle of the window and capture the window via "SendMessage"
		GetClientRect(hWnd, &rect);
		pt.x = rect.right / 2;
		pt.y = rect.bottom / 2;
		SetCursorPos(pt.x, pt.y);
		SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
		return 0;
	}
	case WM_LBUTTONDOWN:
	{ // Capture the mouse
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
						  {case 'v':
						   case 'V':
							  EnableVerticalSync(!g_enableVerticalSync);
							  break;
						  }

					   }break;
					   return 0;
	}break;

	}

	return (DefWindowProc(hWnd, message, wParam, lParam));
}



void InitApp(HWND hWnd)
{
	static HGLRC hRC;					// rendering context
	static HDC hDC;						// device context

	hDC = GetDC(hWnd);
	int nPixelFormat;					// our pixel format index

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of structure
		1,								// default version
		PFD_DRAW_TO_WINDOW |			// window drawing support
		PFD_SUPPORT_OPENGL |			// OpenGL support
		PFD_DOUBLEBUFFER,				// double buffering support
		PFD_TYPE_RGBA,					// RGBA color mode
		32,								// 32 bit color mode
		0, 0, 0, 0, 0, 0,				// ignore color bits, non-palettized mode
		0,								// no alpha buffer
		0,								// ignore shift bit
		0,								// no accumulation buffer
		0, 0, 0, 0,						// ignore accumulation bits
		16,								// 16 bit z-buffer size
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main drawing plane
		0,								// reserved
		0, 0, 0 };						// layer masks ignored

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// choose best matching pixel format
	SetPixelFormat(hDC, nPixelFormat, &pfd);		// set pixel format to device context
	
	
	// create rendering context and make it current
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
	EnableVerticalSync(true);

	RECT rect;
	GetClientRect(hWnd, &rect);
	g_OldCursorPos.x = rect.right / 2;
	g_OldCursorPos.y = rect.bottom / 2;

	Vector3f camPos{ 0, 0, 5.0f };
	Vector3f xAxis{ 1, 0, 0 };
	Vector3f yAxis{ 0, 1, 0 };
	Vector3f zAxis{ 0, 0, 1 };

	camera = new Camera(camPos, xAxis, yAxis, zAxis);
	camera->orthographic(-0.8, 0.8, -0.6, 0.6, -1.0, 1.0);
	

	program = createProgram();
	vbo = quadFullScreenVbo();
	
	u_campos = glGetUniformLocation(program, "u_campos");
	u_camright =  glGetUniformLocation(program, "u_camright");
	u_camup = glGetUniformLocation(program, "u_camup");
	u_viewdir = glGetUniformLocation(program, "u_viewdir");
	u_projection = glGetUniformLocation(program, "u_projection");
	u_resx = glGetUniformLocation(program, "u_resx");
	u_resy = glGetUniformLocation(program, "u_resy");
	u_aspectRatio = glGetUniformLocation(program, "u_aspectRatio");
	u_scale = glGetUniformLocation(program, "u_scale");
	u_time = glGetUniformLocation(program, "u_time");

	glUseProgram(program);  
	glUniformMatrix4fv(u_projection, 1, true, &camera->getOrthographicMatrix()[0][0]);
	glUniform1f(u_resx, width);
	glUniform1f(u_resy, height);
	glUniform1f(u_aspectRatio, (float)width/height);
	glUniform1f(u_scale, tan(PI / 180 * 45));
	/*glUniform1f(uniform_eyedist, 1 / scale);
	glUniform1f(uniform_stepsize, raymarching_stepsize);
	glUniform1f(uniform_accuracy, raymarching_accuracy);
	glUniform4fv(uniform_user_params0, 1, user_params[0]);
	glUniform4fv(uniform_user_params1, 1, user_params[1]);*/
	glUseProgram(0);
}

GLuint LinkShaders(GLuint vertShader, GLuint fragShader)
{
	GLuint program = glCreateProgram();

	if (program)
	{
		GLint linked = 0;

		if (vertShader)
			glAttachShader(program, vertShader);

		if (fragShader)
			glAttachShader(program, fragShader);

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			GLsizei infoLogSize = 0;
			std::string infoLog;

			glGetShaderiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
			infoLog.resize(infoLogSize);
			glGetShaderInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);
			std::cout << "Compile status: \n" << &infoLog << std::endl;
		}

		// Mark the two attached shaders for deletion. These two shaders aren't
		// deleted right now because both are already attached to a shader
		// program. When the shader program is deleted these two shaders will
		// be automatically detached and deleted.

		if (vertShader)
			glDeleteShader(vertShader);

		if (fragShader)
			glDeleteShader(fragShader);

	}

	return program;
}

GLuint LoadShaderProgram(GLenum type, const char *pszFilename )
{
	GLuint shader = 0;
	std::string buffer;
	ReadTextFile(pszFilename, buffer);
	
	if (buffer.length() > 0)
	{

		
		shader = CompileShader(type, reinterpret_cast<const char *>(&buffer[0]));
	}

	return shader;
}

GLuint CompileShader(GLenum type, const char *pszSource)
{
	GLuint shader = glCreateShader(type);

	//std::cout << pszSource << std::endl;

	if (shader)
	{
		GLint compiled = 0;
	
		glShaderSource(shader, 1, &pszSource, NULL);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			GLsizei infoLogSize = 0;
			std::string infoLog;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
			infoLog.resize(infoLogSize);
			glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);
			std::cout << "Compile status: \n" << &infoLog << std::endl;
			
		}

	}
	return shader;
}

void ReadTextFile(const char *pszFilename, std::string &buffer)
{
	std::ifstream file(pszFilename, std::ios::binary);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);

		std::ifstream::pos_type fileSize = file.tellg();

		buffer.resize(static_cast<unsigned int>(fileSize));
		file.seekg(0, std::ios::beg);
		file.read(&buffer[0], fileSize);
	}
}


void Cleanup()
{
	
	if (program)
	{
		glDeleteProgram(program);
		program = 0;
	}

}



GLuint createProgram() {
	GLuint quadProgram = glCreateProgram();

	GLuint vshader = LoadShaderProgram(GL_VERTEX_SHADER, "vertex.vert");
	
	std::string tmp = "program" + std::to_string(usedProgram) + ".fraq";
	GLuint fshader = LoadShaderProgram(GL_FRAGMENT_SHADER, tmp.c_str());

	return LinkShaders(vshader, fshader);

}


GLuint quadFullScreenVbo() {

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindVertexArray(vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);


	GLfloat data[] = {

		-0.6, -0.6,   // bottom left corner
		0.6, -0.6,   // bottom right corner
		0.6, 0.6,    // top right corner
		0.6, 0.6,	  // top right corner
		-0.6, 0.6,   // top left corner
		-0.6, -0.6,   // bottom left corner
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 12, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return vbo;

}


void EnableVerticalSync(bool enableVerticalSync)
{
	// WGL_EXT_swap_control.

	typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC)(GLint);

	static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
		reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
		wglGetProcAddress("wglSwapIntervalEXT"));

	if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(enableVerticalSync ? 1 : 0);
		g_enableVerticalSync = enableVerticalSync;
	}
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
				camera->rotate(X, Y, 0.0f);
				//

			} // End if any rotation


			if (Direction)
			{
				
				float dx = 0, dy = 0, dz = 0;

				if (Direction & DIR_FORWARD) dz = +0.1f;
				if (Direction & DIR_BACKWARD) dz = -0.1f;
				if (Direction & DIR_LEFT) dx = -0.1f;
				if (Direction & DIR_RIGHT) dx = +0.1f;
				if (Direction & DIR_UP) dy = +0.1f;
				if (Direction & DIR_DOWN) dy = -0.1f;

				camera->move(dx, dy, dz);
		

			}

		}// End if any movement
	} // End if Captured
}

void setCursortoMiddle(HWND hwnd){

	RECT rect;

	GetClientRect(hwnd, &rect);
	SetCursorPos(rect.right / 2, rect.bottom / 2);

}