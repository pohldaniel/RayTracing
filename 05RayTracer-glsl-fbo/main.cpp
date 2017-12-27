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

int usedQuadProgram = 1;
int usedProgram = 2;
int resolutionx = 800;
int resolutiony = resolutionx*0.75;

bool g_enableVerticalSync;

GLuint              program;
GLuint				quadProgram;
GLuint				tex, vbo, fbo, depth, fboMsaa, rboColor, rbo;

Camera *camera;


//prototype funktions
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParma, LPARAM lParam);

void ReadTextFile(const char *pszFilename, std::string &buffer);
void InitApp(HWND hWnd);
void    Cleanup();
void    EnableVerticalSync(bool enableVerticalSync);

GLuint  CompileShader(GLenum type, const char *pszSource);
GLuint  LinkShaders(GLuint vertShader, GLuint fragShader);
GLuint  LoadShaderProgram(GLenum type, const char *pszFilename);


GLuint quadFullScreenVbo();
GLuint createFramebufferTexture();
GLuint createQuadProgram();
GLuint createProgram();


GLuint CreateNullTexture();
GLuint createFrameBufferObject(GLuint tex);

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
		"RayTraycer GLSL",							// app name
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

	InitApp(hwnd);

	GLuint uniform_offset = glGetUniformLocation(quadProgram, "offset");
	GLuint uniform_texture = glGetUniformLocation(quadProgram, "fbo_texture");
	GLuint uniform_time = glGetUniformLocation(program, "u_time");
	
	float offset = 0;
	float time = 0;

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
			
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboMsaa);
			time += 0.03f;
			glClearColor(0.3, 0.3, 0.3, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(program);
			glUniform1f(uniform_time, time);
			glBegin(GL_QUADS);
			glVertex3f(-1.0f, -1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, 0.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
			glEnd();
			glUseProgram(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			// copy rendered image from MSAA (multi-sample) to normal (single-sample) FBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMsaa);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			glBlitFramebuffer(0, 0, resolutionx, resolutiony,  // src rect
				0, 0, resolutionx, resolutiony,  // dst rect
				GL_COLOR_BUFFER_BIT, // buffer mask
				GL_LINEAR);                           // scale filter

			// trigger mipmaps generation explicitly
			// NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
			// triggers mipmap generation automatically. However, the texture attached
			// onto a FBO should generate mipmaps manually via glGenerateMipmap().
			glBindTexture(GL_TEXTURE_2D, tex);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//////////////////////////////////////////////////////////////////////////////////

			//Post-processing 
			offset += 0.1f;
			glClearColor(0.7, 0.7, 0.7, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(quadProgram);
			glUniform1f(uniform_offset, offset);

		
			glBindTexture(GL_TEXTURE_2D, tex);
			glUniform1i(uniform_texture, 0);
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

	switch (message)
	{
	
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

	
	

	tex = CreateNullTexture();
	fboMsaa = createFrameBufferObject(tex);

	quadProgram =  createQuadProgram();
	program = createProgram();
	vbo = quadFullScreenVbo();
	
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "u_resx"), resolutionx/2.0);
	glUniform1i(glGetUniformLocation(program, "u_resy"), resolutiony/2.0);
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
	if (quadProgram)
	{
		glDeleteProgram(quadProgram);
		quadProgram = 0;
	}

	if (program)
	{
		glDeleteProgram(program);
		program = 0;
	}

}


GLuint createQuadProgram() {
	GLuint quadProgram = glCreateProgram();

	std::string tmp = "quad" + std::to_string(usedQuadProgram) + ".vert";
	GLuint vshader = LoadShaderProgram(GL_VERTEX_SHADER, tmp.c_str());

	tmp = "quad" + std::to_string(usedQuadProgram) + ".fraq";
	GLuint fshader = LoadShaderProgram(GL_FRAGMENT_SHADER, tmp.c_str());

	return LinkShaders(vshader, fshader);

}


GLuint createProgram() {
	GLuint quadProgram = glCreateProgram();

	std::string tmp = "program" + std::to_string(usedProgram) + ".vert";
	GLuint vshader = LoadShaderProgram(GL_VERTEX_SHADER, tmp.c_str());
	
	tmp = "program" + std::to_string(usedProgram) + ".fraq";
	GLuint fshader = LoadShaderProgram(GL_FRAGMENT_SHADER, tmp.c_str());

	return LinkShaders(vshader, fshader);

}


GLuint quadFullScreenVbo() {
	
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindVertexArray(vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat data[] = {

		-0.8, -0.8,   // bottom left corner
		0.8, -0.8,   // bottom right corner
		0.8, 0.8,    // top right corner
		0.8, 0.8,	  // top right corner
		-0.8, 0.8,   // top left corner
		-0.8, -0.8,   // bottom left corner
	};

	//varying effects the maximum resolution
	/*GLfloat data[] = {

	-1.0, -1.0,   // bottom left corner
	1.0, -1.0,   // bottom right corner
	1.0, 1.0,    // top right corner
	1.0, 1.0,	  // top right corner
	-1.0, 1.0,   // top left corner
	-1.0, -1.0,   // bottom left corner
	};*/

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* 12, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	return vbo;

}


GLuint CreateNullTexture(){

	GLuint texture = 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionx, resolutiony, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}


GLuint createFrameBufferObject(GLuint tex) {

	GLuint      fboMsaa = 0;

	glGenFramebuffers(1, &fboMsaa);
	glBindFramebuffer(GL_FRAMEBUFFER, fboMsaa);

	// create a MSAA renderbuffer object to store color info
	glGenRenderbuffers(1, &rboColor);
	glBindRenderbuffer(GL_RENDERBUFFER, rboColor);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, resolutionx, resolutiony);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenRenderbuffers(1, &depth);
	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, resolutionx, resolutiony);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach msaa RBOs to FBO attachment points
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColor);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// create a normal (no MSAA) FBO to hold a render-to-texture
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolutionx, resolutiony);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	// attach a rbo to FBO depth attachement point
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fboMsaa;
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

