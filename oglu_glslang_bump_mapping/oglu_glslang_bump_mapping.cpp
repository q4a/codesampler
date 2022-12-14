//-----------------------------------------------------------------------------
//           Name: oglu_glslang_bump_mapping.cpp
//         Author: Anudhyan Boral
//  Last Modified: 02/27/06
//    Description: This sample demonstrates how to perform bump-mapping with 
//                 shaders using OpenGL's new high-level shading language 
//                 GLslang.
//
//   Control Keys: F1 - Toggle usage of vertex and fragment shaders.
//				   ARROW KEYS - To move the light
//
// Note: The idea is to have a normal map and get the normal's xyz values from 
//       the texture's rgb values. This is done in the fragment shader.
//		 The textures are from CodeSampler's Sample "Dot3 Bump Mapping"
//		 This sample was coded on top of CodeSampler's Sample 
//       "ogl_glslang_simple_vs2ps".
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn?t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

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
PFNGLUNIFORM1IARBPROC			 glUniform1iARB			   = NULL; 
PFNGLUNIFORM4FARBPROC			 glUniform4fARB			   = NULL;
PFNGLACTIVETEXTUREARBPROC		 glActiveTextureARB		   = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID  = 0;
GLuint    g_normalMapID = 1;


GLhandleARB g_programObj;
GLhandleARB g_vertexShader;
GLhandleARB g_fragmentShader;

GLUquadric *g_sphere;        // quadric surface for drawing the spherical light

bool g_bUseShaders = true;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

// initial position of our light
float g_lightpos[] = { 0.0f, 0.0f, -3.5f, 0.0f };

// GL_T2F_C3F_V3F
struct Vertex
{
	float tu, tv;
	float r, g, b;
	float x, y, z;
};

Vertex g_quadVertices[] =
{
	// tu,  tv     r    g    b       x     y     z 
	{ 0.0f,0.0f,  1.0f,1.0f,0.0f, -1.0f,-1.0f, 0.0f },
	{ 1.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,-1.0f, 0.0f },
	{ 1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f, 1.0f, 0.0f },
	{ 0.0f,1.0f,  0.0f,0.0f,1.0f, -1.0f, 1.0f, 0.0f },
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
unsigned char *readShaderFile(const char *fileName);
void initShader(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
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

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Simple Bump Mapping Using GLslang",
							WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initShader();

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

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
    static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
    
    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_F1:
					g_bUseShaders = !g_bUseShaders;
					break;

				// Control of light's position

				case VK_LEFT:
					g_lightpos[0]-=(float)0.01;
					break;

				case VK_RIGHT:
					g_lightpos[0]+=(float)0.01;
					break;

				case VK_UP:
					g_lightpos[1]+=(float)0.01;
					break;

				case VK_DOWN:
					g_lightpos[1]-=(float)0.01;
					break;
				
			}
		}
        break;

        case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bMousing )
			{
				g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
			}
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;
		
		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}

        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;
		
		default:
		{
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	// Setting up the attenuation factors for LIGHT0	
    glLightf (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, (float)2.50);
    glLightf (GL_LIGHT0, GL_LINEAR_ATTENUATION, (float)0.25);
    glLightf (GL_LIGHT0, GL_CONSTANT_ATTENUATION,(float)0.1);

	// Set up the light's initial position
	glLightfv (GL_LIGHT0,GL_POSITION,g_lightpos);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Set up our quadratic object
	g_sphere=gluNewQuadric();			
	gluQuadricNormals(g_sphere, GLU_SMOOTH);
	gluQuadricTexture(g_sphere, GL_TRUE);	

	//
	// Create the texture
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\stone_wall.bmp" );

	if( pTextureImage != NULL )
	{
		glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0,
			GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
	}

	if( pTextureImage )
	{
		if( pTextureImage->data )
			free( pTextureImage->data );

		free( pTextureImage );
	}

	//
	// Create the normal map texture
	//

	
	AUX_RGBImageRec *pTextureImage2 = auxDIBImageLoad( ".\\stone_wall_normal_map.bmp" );

	if( pTextureImage2 != NULL )
	{
		glGenTextures( 1, &g_normalMapID );

		glBindTexture( GL_TEXTURE_2D, g_normalMapID );

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST );

		glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage2->sizeX, pTextureImage2->sizeY, 0,
			GL_RGB, GL_UNSIGNED_BYTE, pTextureImage2->data );
	}

	if( pTextureImage2 )
	{
		if( pTextureImage2->data )
			free( pTextureImage2->data );

		free( pTextureImage2 );
	}
	glBindTexture( GL_TEXTURE_2D, g_textureID );

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

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Assemble the shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
	//
	// If the required extension is present, get the addresses of its 
	// functions that we wish to use...
	//

	char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
    {
        // This extension string indicates that the OpenGL Shading Language,
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
		glUniform1iARB			  = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
		glUniform4fARB			  = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
		glActiveTextureARB        = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");

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

    const char *vertexShaderStrings[1];
    const char *fragmentShaderStrings[1];
    GLint bVertCompiled;
    GLint bFragCompiled;
    GLint bLinked;
    char str[4096];

	//
	// Create the vertex shader...
	//

    g_vertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );

	unsigned char *vertexShaderAssembly = readShaderFile( "vertex_shader.vert" );
    vertexShaderStrings[0] = (char*)vertexShaderAssembly;
    glShaderSourceARB( g_vertexShader, 1, vertexShaderStrings, NULL );
    glCompileShaderARB( g_vertexShader);
    delete vertexShaderAssembly;

    glGetObjectParameterivARB( g_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, 
                               &bVertCompiled );
    if( bVertCompiled  == false )
	{
		glGetInfoLogARB(g_vertexShader, sizeof(str), NULL, str);
		MessageBox( NULL, str, "Vertex Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

	//
	// Create the fragment shader...
	//

    g_fragmentShader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );

    unsigned char *fragmentShaderAssembly = readShaderFile( "fragment_shader.frag" );
    fragmentShaderStrings[0] = (char*)fragmentShaderAssembly;
    glShaderSourceARB( g_fragmentShader, 1, fragmentShaderStrings, NULL );
    glCompileShaderARB( g_fragmentShader );
    delete fragmentShaderAssembly;

    glGetObjectParameterivARB( g_fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, 
                               &bFragCompiled );
    if( bFragCompiled == false )
	{
		glGetInfoLogARB( g_fragmentShader, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Fragment Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

    //
    // Create a program object and attach the two compiled shaders...
    //

    g_programObj = glCreateProgramObjectARB();
    glAttachObjectARB( g_programObj, g_vertexShader );
    glAttachObjectARB( g_programObj, g_fragmentShader );

    //
    // Link the program object and print out the info log...
    //

    glLinkProgramARB( g_programObj );
    glGetObjectParameterivARB( g_programObj, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

    if( bLinked == false )
	{
		glGetInfoLogARB( g_programObj, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Linking Error", MB_OK|MB_ICONEXCLAMATION );
	}
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    glDeleteTextures( 1, &g_textureID );
    glDeleteTextures( 1, &g_normalMapID );

    glDeleteObjectARB( g_vertexShader );
    glDeleteObjectARB( g_fragmentShader );
    glDeleteObjectARB( g_programObj );

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

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    glMatrixMode( GL_MODELVIEW );
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glLightfv (GL_LIGHT0,GL_POSITION,g_lightpos); //update light's position each frame
	glTranslatef(g_lightpos[0],g_lightpos[1],g_lightpos[2]);
	gluSphere(g_sphere,0.05,16,16);  //Draw our light	
	glEnable(GL_LIGHTING);

    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	if( g_bUseShaders == true )
	{
		//
		// Use vertex and fragment shaders...
		//
        glUseProgramObjectARB( g_programObj );

		int texture2ID;
		// Get the variable ID of Texture2
		texture2ID=glGetUniformLocationARB(g_programObj,"NormalMap");
		// Set "Texture2" to 1. So we use glActiveTextureARB(GL_TEXTURE1_ARB)
		glUniform1iARB(texture2ID,1);
		
		//Set normalmap texture as GL_TEXTURE1_ARB (NormalMap)
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,g_normalMapID);

		//Set display texture as the stone texture
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D,g_textureID);

		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );
		glUseProgramObjectARB( NULL );
	}
	else
	{
		//
		// Render the normal way...
		//
		glBindTexture( GL_TEXTURE_2D, g_textureID );
		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );
	}

	SwapBuffers( g_hDC );
}
