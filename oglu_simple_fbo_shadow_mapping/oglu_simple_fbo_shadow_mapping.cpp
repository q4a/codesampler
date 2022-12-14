//------------------------------------------------------------------------------
//           Name: oglu_simple_fbo_shadow_mapping.cpp
//         Author: Tristan Dean
//  Last Modified: 07/05/06
//    Description: This sample modifies the 'frame_buffer_object' sample
//                 to handle simple, compatible shadow mapping. The main 2 differences
//                 between this sample and Kevin's 'Shadow_mapping_nv' sample is
//                 firstly using FBO's over pBuffers, But more importantly storing the
//                 depth of the fragment in RGBA format instead of DEPTH_COMPONENT.
//                 This is for both compatability (non-nv extensions) and so we can store 
//                 the depth in a linear format, not the non-linear 'DEPTH_COMPONENT'.
//                 
//                 This sample is done this way as it is a prequal to a cube shadow
//                 mapping sample i plan to make, but more about that will be in the
//                 next sample, along with an explanation into why i have done things this
//                 way.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//           Name: ogl_frame_buffer_object.cpp
//         Author: Kevin Harris 
//  Last Modified: 07/06/05
//    Description: This sample demonstrates how to create dynamic textures 
//                 through off-screen rendering. The off-screen rendering step 
//                 is accomplished using a frame-buffer and render-buffer 
//                 object, which is created using OpenGL's 
//                 EXT_framebuffer_object extension.
//
//                 As a demonstration, a spinning textured cube is rendered 
//                 to a frame-buffer object, which is in turn, used to create a 
//                 dynamic texture. The dynamic texture is then used to texture 
//                 a second spinning cube, which will be rendered to the 
//                 application's window.
//
//   Control Keys: Left Mouse Button  - Spin the large, black cube.
//                 Right Mouse Button - Spin the textured cube being rendered 
//                                      into the p-buffer.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Note: The EXT_framebuffer_object extension is an excellent replacement for 
//       the WGL_ARB_pbuffer and WGL_ARB_render_texture combo which is normally 
//       used to create dynamic textures. An example of this older technique 
//       can be found here:
//
//       http://www.codesampler.com/oglsrc/oglsrc_7.htm#ogl_offscreen_rendering
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <sys/stat.h>
#include <stdio.h>
#include "resource.h"
#include <math.h>

//------------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//------------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn?t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

// EXT_framebuffer_object - http://oss.sgi.com/projects/ogl-sample/registry/EXT/framebuffer_object.txt
extern PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = NULL;
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = NULL;
extern PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = NULL;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = NULL;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = NULL;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = NULL;
extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = NULL;


///////////////////////////////////////////////////////////
// NEW - We will require shading, and by overwhelming request
// the shading will be done via GLSL.
///////////////////////////////////////////////////////////

// GL_ARB_shader_objects
PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgramObjectARB  = NULL;
PFNGLDELETEOBJECTARBPROC         glDeleteObjectARB         = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgramObjectARB     = NULL;
PFNGLCREATESHADEROBJECTARBPROC   glCreateShaderObjectARB   = NULL;
PFNGLSHADERSOURCEARBPROC         glShaderSourceARB         = NULL;
PFNGLCOMPILESHADERARBPROC        glCompileShaderARB        = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC         glAttachObjectARB         = NULL;
PFNGLGETINFOLOGARBPROC           glGetInfoLogARB           = NULL;
PFNGLLINKPROGRAMARBPROC          glLinkProgramARB          = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocationARB   = NULL;
PFNGLUNIFORM4FARBPROC            glUniform4fARB            = NULL;
PFNGLUNIFORM1IARBPROC            glUniform1iARB            = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC	 glUniformMatrix4fvARB	   = NULL;

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------
HWND   g_hWnd             = NULL;
HDC	   g_hDC              = NULL;
HGLRC  g_hRC              = NULL;

GLuint g_frameBuffer;
GLuint g_depthRenderBuffer;

int g_nWindowWidth  = 640;
int g_nWindowHeight = 480;

const int RENDERBUFFER_WIDTH  = 512;
const int RENDERBUFFER_HEIGHT = 512;

float  g_fSpinX_L = 0.0f;
float  g_fSpinY_L = 0.0f;
float  g_fSpinX_R = -75.0f;
float  g_fSpinY_R = 0.0f;

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 1.0f },
   
    { 1.0f,0.0f, -1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f, -1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f,-1.0f },
   
    { 0.0f,1.0f, -1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f, -1.0f, 1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f, 1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
   
    { 1.0f,1.0f, -1.0f,-1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,-1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
   
    { 1.0f,0.0f,  1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f, 1.0f, 1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
   
    { 0.0f,0.0f, -1.0f,-1.0f,-1.0f },
    { 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f, -1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f, -1.0f, 1.0f,-1.0f },
	
   
    { 0.0f,1.0f, -3.0f, -1.0f,-3.0f },
    { 0.0f,0.0f, -3.0f, -1.0f, 3.0f },
    { 1.0f,0.0f,  3.0f, -1.0f, 3.0f },
    { 1.0f,1.0f,  3.0f, -1.0f,-3.0f }
};


///////////////////////////////////////////////////////////
// NEW - We will need 2 shader objects.
// One for generating the shadow map, and one for using it.
///////////////////////////////////////////////////////////

GLuint g_shad_shadowmap;
GLuint g_shad_lighting;

// Now, we need a vector for the light position.
// and a temporary vector to store the light's
// projection matrix.
float g_light_pos[] = {-6,6,-6};
float g_lightproj_matrix[16];

// And most importantly, the shadow map texture.
GLuint g_shadowMapID = -1;

//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);

//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//------------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
		                     "OpenGL - Simple Shadow Mapping Using Frame Buffer Objects",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//------------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit_L;
	static POINT ptCurrentMousePosit_L;
	static bool  bMousing_L;
	
	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;

    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
			}
		}
        break;

		case WM_LBUTTONDOWN:
		{
			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y = HIWORD (lParam);
			bMousing_L = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing_L = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y = HIWORD (lParam);
			bMousing_R = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			bMousing_R = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit_L.x = LOWORD (lParam);
			ptCurrentMousePosit_L.y = HIWORD (lParam);
			ptCurrentMousePosit_R.x = LOWORD (lParam);
			ptCurrentMousePosit_R.y = HIWORD (lParam);

			if( bMousing_L )
			{
				g_fSpinX_L -= (ptCurrentMousePosit_L.x - ptLastMousePosit_L.x);
				g_fSpinY_L -= (ptCurrentMousePosit_L.y - ptLastMousePosit_L.y);
			}
			
			if( bMousing_R )
			{
				g_fSpinX_R -= (ptCurrentMousePosit_R.x - ptLastMousePosit_R.x);
				g_fSpinY_R -= (ptCurrentMousePosit_R.y - ptLastMousePosit_R.y);
			}

			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x;
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y;
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x;
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y;
		}
		break;
		
		case WM_SIZE:
		{
			g_nWindowWidth  = LOWORD(lParam); 
			g_nWindowHeight = HIWORD(lParam);
			glViewport(0, 0, g_nWindowWidth, g_nWindowHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)g_nWindowWidth / (GLdouble)g_nWindowHeight, 0.1, 100.0);
		}
		break;
		
		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		break;

        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;
		
		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: readShaderFile()
// Desc: 
//-----------------------------------------------------------------------------
unsigned char *readShaderFile( const char *fileName )
{
    FILE *file = fopen( fileName, "r" );

    if( file == NULL )
    {
        MessageBox( NULL, "Cannot open shader file!", "ERROR",
            MB_OK | MB_ICONEXCLAMATION );
		return 0;
    }

    struct _stat fileStats;

    if( _stat( fileName, &fileStats ) != 0 )
    {
        MessageBox( NULL, "Cannot get file stats for shader file!", "ERROR",
                    MB_OK | MB_ICONEXCLAMATION );
        return 0;
    }

    unsigned char *buffer = new unsigned char[fileStats.st_size];

	int bytes = fread( buffer, 1, fileStats.st_size, file );

    buffer[bytes] = 0;

	fclose( file );

	return buffer;
}



//------------------------------------------------------------------------------
// Name: init()
// Desc: 
//------------------------------------------------------------------------------
void init( void )
{
	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );

	//
	// If the required extensions are present, get the addresses for the
	// functions that we wish to use...
	//

	//
	// EXT_framebuffer_object
	//

	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( ext, "EXT_framebuffer_object" ) == NULL )
	{
		MessageBox(NULL,"EXT_framebuffer_object extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}
	else
	{
		glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)wglGetProcAddress("glIsRenderbufferEXT");
		glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
		glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
		glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
		glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
		glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
		glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)wglGetProcAddress("glIsFramebufferEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
		glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
		glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)wglGetProcAddress("glFramebufferTexture1DEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
		glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)wglGetProcAddress("glFramebufferTexture3DEXT");
		glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
		glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
		glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");

		if( !glIsRenderbufferEXT || !glBindRenderbufferEXT || !glDeleteRenderbuffersEXT || 
			!glGenRenderbuffersEXT || !glRenderbufferStorageEXT || !glGetRenderbufferParameterivEXT || 
			!glIsFramebufferEXT || !glBindFramebufferEXT || !glDeleteFramebuffersEXT || 
			!glGenFramebuffersEXT || !glCheckFramebufferStatusEXT || !glFramebufferTexture1DEXT || 
			!glFramebufferTexture2DEXT || !glFramebufferTexture3DEXT || !glFramebufferRenderbufferEXT||  
			!glGetFramebufferAttachmentParameterivEXT || !glGenerateMipmapEXT )
		{
			MessageBox(NULL,"One or more EXT_framebuffer_object functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
	}

	
///////////////////////////////////////////////////////////
// NEW - We will also need to initialize GLSL.
///////////////////////////////////////////////////////////

	if( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
    {
        //This extension string indicates that the OpenGL Shading Language,
        // version 1.00, is supported.
        MessageBox(NULL,"GL_ARB_shading_language_100 extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    if( strstr( ext, "GL_ARB_shader_objects" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_shader_objects extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
        glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform4fARB            = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
		glUniform1iARB            = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
		glUniformMatrix4fvARB	  = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");

        if( !glCreateProgramObjectARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
            !glCreateShaderObjectARB || !glCreateShaderObjectARB || !glCompileShaderARB || 
            !glGetObjectParameterivARB || !glAttachObjectARB || !glGetInfoLogARB || 
            !glLinkProgramARB || !glGetUniformLocationARB || !glUniform4fARB ||
			!glUniform1iARB )
        {
            MessageBox(NULL,"One or more GL_ARB_shader_objects functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

	//
	// Create a frame-buffer object and a render-buffer object...
	//

	glGenFramebuffersEXT( 1, &g_frameBuffer );
	glGenRenderbuffersEXT( 1, &g_depthRenderBuffer );

	// Initialize the render-buffer for usage as a depth buffer.
	// We don't really need this to render things into the frame-buffer object,
	// but without it the geometry will not be sorted properly.
	glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, g_depthRenderBuffer );
	glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, RENDERBUFFER_WIDTH, RENDERBUFFER_HEIGHT );

	//
	// Check for errors...
	//

	GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );

	switch( status )
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			//MessageBox(NULL,"GL_FRAMEBUFFER_COMPLETE_EXT!","SUCCESS",MB_OK|MB_ICONEXCLAMATION);
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			MessageBox(NULL,"GL_FRAMEBUFFER_UNSUPPORTED_EXT!","ERROR",MB_OK|MB_ICONEXCLAMATION);
			exit(0);
			break;

		default:
			exit(0);
	}

	//
	// Now, create our dynamic texture. It doesn't actually get loaded with any 
	// pixel data, but its texture ID becomes associated with the pixel data
	// contained in the frame-buffer object. This allows us to bind to this data
	// like we would any regular texture.
	//

	glGenTextures( 1, &g_shadowMapID );

	glBindTexture( GL_TEXTURE_2D, g_shadowMapID );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 
		          RENDERBUFFER_WIDTH, RENDERBUFFER_HEIGHT, 
		          0, GL_RGB, GL_UNSIGNED_BYTE, 0 );


///////////////////////////////////////////////////////////
// NEW - Shadow map filtering is set to 'Nearest', This
// method of shadow mapping has artifacts if linear
// blending is used.
///////////////////////////////////////////////////////////

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );


///////////////////////////////////////////////////////////
// NEW - We will need to load the shaders from file.
// NOTE: The shader code has been taken and modified from
// the simple glslang sample.
// This sample does not concentrate on learning GLSL.
///////////////////////////////////////////////////////////

	
	// Load the shadow map creation shader
    const char *shaderStrings[1];
    int nResult;
    char str[4096];

	GLuint shad_vert, shad_frag;

	g_shad_shadowmap = glCreateProgramObjectARB();

	// Load the vertex shader from a file.
	unsigned char *shader_assembly = readShaderFile( "shadow.vert" );

    shad_vert = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
    shaderStrings[0] = (char*)shader_assembly;
    glShaderSourceARB( shad_vert, 1, shaderStrings, NULL );
    glCompileShaderARB( shad_vert);
    glGetObjectParameterivARB( shad_vert, GL_OBJECT_COMPILE_STATUS_ARB, &nResult );

    if( nResult )
        glAttachObjectARB( g_shad_shadowmap, shad_vert );
	else
	{
		glGetInfoLogARB(shad_vert, sizeof(str), NULL, str);
		MessageBox( NULL, str, "Vertex Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}


	// Load the fragment shader from a file.
	delete shader_assembly;
	shader_assembly = readShaderFile( "shadow.frag" );

    shad_frag = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
    shaderStrings[0] = (char*)shader_assembly;
    glShaderSourceARB( shad_frag, 1, shaderStrings, NULL );
    glCompileShaderARB( shad_frag );
    glGetObjectParameterivARB( shad_frag, GL_OBJECT_COMPILE_STATUS_ARB, &nResult );

    if( nResult )
        glAttachObjectARB( g_shad_shadowmap, shad_frag );
	else
	{
		glGetInfoLogARB( shad_frag, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Fragment Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

	// Link and then clean up the shader.
    glLinkProgramARB( g_shad_shadowmap );
    glGetObjectParameterivARB( g_shad_shadowmap, GL_OBJECT_LINK_STATUS_ARB, &nResult );

    if( !nResult )
	{
		glGetInfoLogARB( g_shad_shadowmap, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Linking Error", MB_OK|MB_ICONEXCLAMATION );
	}

	glDeleteObjectARB(shad_vert);
	glDeleteObjectARB(shad_frag);
	delete shader_assembly;



	// Create the lighting shader.
	g_shad_lighting = glCreateProgramObjectARB();

	// Load the vertex shader from disk.
	shader_assembly = readShaderFile( "lighting.vert" );

    shad_vert = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
    shaderStrings[0] = (char*)shader_assembly;
    glShaderSourceARB( shad_vert, 1, shaderStrings, NULL );
    glCompileShaderARB( shad_vert);
    glGetObjectParameterivARB( shad_vert, GL_OBJECT_COMPILE_STATUS_ARB, &nResult );

    if( nResult )
        glAttachObjectARB( g_shad_lighting, shad_vert );
	else
	{
		glGetInfoLogARB(shad_vert, sizeof(str), NULL, str);
		MessageBox( NULL, str, "Vertex Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}


	// Load the fragment shader from disk.
	delete shader_assembly;
	shader_assembly = readShaderFile( "lighting.frag" );

    shad_frag = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
    shaderStrings[0] = (char*)shader_assembly;
    glShaderSourceARB( shad_frag, 1, shaderStrings, NULL );
    glCompileShaderARB( shad_frag );
    glGetObjectParameterivARB( shad_frag, GL_OBJECT_COMPILE_STATUS_ARB, &nResult );

    if( nResult )
        glAttachObjectARB( g_shad_lighting, shad_frag );
	else
	{
		glGetInfoLogARB( shad_frag, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Fragment Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

	// Link and then clean up.
    glLinkProgramARB( g_shad_lighting );
    glGetObjectParameterivARB( g_shad_lighting, GL_OBJECT_LINK_STATUS_ARB, &nResult );

    if( !nResult )
	{
		glGetInfoLogARB( g_shad_lighting, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Linking Error", MB_OK|MB_ICONEXCLAMATION );
	}

	glDeleteObjectARB(shad_vert);
	glDeleteObjectARB(shad_frag);
	delete shader_assembly;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    glDeleteTextures( 1, &g_shadowMapID );

	glDeleteFramebuffersEXT( 1, &g_frameBuffer );
	glDeleteRenderbuffersEXT( 1, &g_depthRenderBuffer );

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
}

//------------------------------------------------------------------------------
// Name: render()
// Desc: 
//------------------------------------------------------------------------------
void render( void )
{
	//
	// Bind the frame-buffer object and attach to it a render-buffer object 
	// set up as a depth-buffer.
	//

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, g_frameBuffer );
	//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, g_depthRenderBuffer );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_shadowMapID, 0 );
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_depthRenderBuffer );

	//
	// Set up the frame-buffer object just like you would set up a window.
	//

	glViewport( 0, 0, RENDERBUFFER_WIDTH, RENDERBUFFER_HEIGHT );
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//
	// Let the user spin the cube about with the right mouse button, so our 
	// dynamic texture will show motion.
	//


///////////////////////////////////////////////////////////
// NEW - Firstly, we need to set up the light's position 
// and save the light's matrix for later use.
///////////////////////////////////////////////////////////

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();


	// Position the light vector.
	g_light_pos[0]=10*cos(g_fSpinX_R/100);
	g_light_pos[1]=10;
	g_light_pos[2]=10*sin(g_fSpinX_R/100);

	// Set up the light's camera.
	gluLookAt( g_light_pos[0],g_light_pos[1],g_light_pos[2], // Look from the light's position
               0.0f, -0.0f, 0.0f,   // Towards the teapot's position
               0.0f, 1.0f, 0.0f ); 


	// Set up the projection matrix for the light, and store it
	// the the first texture matrix to be used later.
	glGetFloatv(GL_MODELVIEW_MATRIX, g_lightproj_matrix);	
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    glTranslatef( 0.5f, 0.5f, 0.5f );                      // Offset
    glScalef( 0.5f, 0.5f, 0.5f );                          // Bias
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );
    glMultMatrixf( g_lightproj_matrix ); 
	glMatrixMode( GL_MODELVIEW );

	//
	// Now, render the cube to the frame-buffer object just like you we would
	// have done with a regular window.
	//


///////////////////////////////////////////////////////////
// NEW - The rendering is now shaded by the shadow map 
// creation shader.
///////////////////////////////////////////////////////////
	glUseProgramObjectARB( g_shad_shadowmap );

	glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
	glDrawArrays( GL_QUADS, 0, 24 +4);

	glUseProgramObjectARB( NULL );

	//
	// Unbind the frame-buffer and render-buffer objects.
	//

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );

	//--------------------------------------------------------------------------
	// Now, set up the regular window for rendering...
	//--------------------------------------------------------------------------

	glViewport( 0, 0, g_nWindowWidth, g_nWindowHeight );
	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//
	// Let the user spin the cube about with the left mouse button.
	//

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();


///////////////////////////////////////////////////////////
// NEW - Set up the light's position again, as well as 
// the camera.
///////////////////////////////////////////////////////////

	// Set the lights position
    glLightfv( GL_LIGHT0, GL_POSITION, g_light_pos );

	
	// Position the light vector.
	float camera[3];
	camera[0]=10*cos(-g_fSpinX_L/100);
	camera[1]=5;
	camera[2]=10*sin(-g_fSpinX_L/100);

	// Set up the view camera.
	gluLookAt( camera[0],camera[1],camera[2], // Look from the light's position
               0.0f, 0.0f, 0.0f,   // Towards the teapot's position
               0.0f, 1.0f, 0.0f ); 


    //glRotatef( -g_fSpinY_L*0-g_fSpinY_R, 1.0f, 0.0f, 0.0f );
    //glRotatef( -g_fSpinX_L*0-g_fSpinX_R, 0.0f, 1.0f, 0.0f );

    //
    // Finally, we'll use the dynamic texture like a regular static texture.
    //


///////////////////////////////////////////////////////////
// NEW - render using the shadow mapping shader.
// There is one very important thing to note;
// there will be artifacts when the face is at a steep
// angle from the light. This, however, will not be visible
// when normal lighting is used as any face at such a steep 
// angle will be almost too dark to notice.
///////////////////////////////////////////////////////////

	glUseProgramObjectARB( g_shad_lighting );

	glBindTexture( GL_TEXTURE_2D, g_shadowMapID );
    glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
    glDrawArrays( GL_QUADS, 0, 24 +4);
	glUseProgramObjectARB( NULL );

	SwapBuffers( g_hDC );
}
