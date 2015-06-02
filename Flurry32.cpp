/*

Copyright (c) 2002-2003, Calum Robinson, Tobias Sargeant, Phuong LeCong
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the author nor the names of its contributors may be used
  to endorse or promote products derived from this software without specific
  prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* flurry */

#include "Flurry32.h"

global_info_t *flurry_info = NULL;

static char preset_str[256];
static double gTimeCounter = 0.0;

char achTemp[256];       // temporary buffer 

#define VendorName "Phuong LeCong"		// predefined stuff
#define AppName "Flurry32"							// for registry access
#define RegKeyName "Software\\" VendorName "\\" AppName

// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
							   WPARAM wParam, LPARAM lParam)
{

	switch ( message ) 
	{
	case WM_CREATE: 
		struct _timeb tb;
		_ftime( &tb );

		// initialize random number generator
		srand((999*tb.time) + (1001*tb.millitm) + (1003 * _getpid()));

		if (gTimeCounter == 0.0) {
			gTimeCounter = currentTime();
		}

		if (flurry_info == NULL) {
			flurry_info = (global_info_t*)malloc(sizeof(global_info_t));
		}

		// enable OpenGL for the window
		ReadPresetFromRegistry();

		EnableOpenGL( hWnd, &flurry_info->window, &flurry_info->glx_context);
		init_flurry( hWnd, flurry_info );

		/* 
		* this clamps the speed at below 60fps and, here
		* at least, produces a reasonably accurate 50fps.
		* (probably part CPU speed and part scheduler).
		*
		* Flurry is designed to run at this speed; much higher
		* than that and the blending causes the display to
		* saturate, which looks really ugly.
		*/

		//create a timer that ticks every 10 milliseconds
		// 40ms delay = 25fps
		// 30ms delay ~ 30fps
		// 20ms delay = 50fps

		SetTimer( hWnd, TIMER, 20, NULL ); 
		return 0;
	case WM_DESTROY:
		KillTimer( hWnd, TIMER );

		//delete any objects created during animation
		//and close down OpenGL nicely
		DisableOpenGL( hWnd, flurry_info->window, flurry_info->glx_context );
		release_flurry();

		return 0;
	case WM_TIMER:
		// call some function to advance your animation	
		draw_flurry( &flurry_info->window, &flurry_info->glx_context );
		return 0;				
	}

	//let the screensaver library take care of any
	//other messages
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

BOOL WINAPI ReadPresetFromRegistry()
{
	HKEY hFlurry;
	DWORD dummy;			// don't really care if the key exists or not, just open it
	
	if (RegCreateKeyEx(HKEY_CURRENT_USER, RegKeyName, 0, "", 0, KEY_READ | KEY_WRITE,
		NULL, &hFlurry, &dummy))
	{
		MessageBox(NULL, "Error opening/creating registry key.  Check that you "
			"are not out of hard disk space and have permission to write to the "
			"HKEY_CURRENT_USER key (if running Windows NT).",
			"Flurry32", MB_OK | MB_ICONSTOP);

		return FALSE;		// still can't recover...
	}

	LONG err;
	DWORD type;
	char szValue[255];
	DWORD dwSize;

	dwSize = sizeof(szValue);
	err = RegQueryValueEx(hFlurry, "preset", NULL, &type, (LPBYTE)szValue, &dwSize);
	if (type == REG_SZ) {		
		strcpy(preset_str, szValue);
	}

	RegCloseKey(hFlurry);	// done with the registry
	return TRUE;			// and we're outta here
}


BOOL WINAPI
ScreenSaverConfigureDialog(
	HWND hDlg, 
	UINT message,	
	WPARAM wParam, 
	LPARAM lParam)
{
	static HWND hPreset;

	switch(message)
	{
	case WM_INITDIALOG:
		InitCommonControls();
		ReadPresetFromRegistry();

        if (!InitPresetList(hDlg)) 
        { 
            EndDialog(hDlg, -1); 
            break; 
        } 

		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:				// save settings
		case IDCANCEL:			// ending the dialog either way so just fall through
			EndDialog(hDlg, LOWORD(wParam) == IDOK);
			return TRUE;
		}
		break;
	}

	return FALSE; 
}

BOOL InitPresetList(HWND hwndDlg) 
{ 
    HWND hwndCombo = GetDlgItem(hwndDlg, IDC_PRESET); 
	HINSTANCE hinst = GetModuleHandle(NULL);
    DWORD dwIndex; 
	DWORD preset_sel = 0;
 
    LoadString(hinst, ID_CLASSIC, achTemp, sizeof(achTemp)/sizeof(TCHAR)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_CLASSIC); 
	if (!strcmp(preset_str, "classic")) {
		preset_sel = dwIndex;
	}
 
    LoadString(hinst, ID_WATER, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_WATER); 
	if (!strcmp(preset_str, "water")) {
		preset_sel = dwIndex;
	}

	LoadString(hinst, ID_FIRE, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_FIRE); 
	if (!strcmp(preset_str, "fire")) {
		preset_sel = dwIndex;
	}

	LoadString(hinst, ID_PSYCHEDELIC, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_PSYCHEDELIC); 
 	if (!strcmp(preset_str, "psychedelic")) {
		preset_sel = dwIndex;
	}

    LoadString(hinst, ID_RGB, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_RGB); 
 	if (!strcmp(preset_str, "rgb")) {
		preset_sel = dwIndex;
	}

    LoadString(hinst, ID_BINARY, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_BINARY); 
	if (!strcmp(preset_str, "binary")) {
		preset_sel = dwIndex;
	}

    LoadString(hinst, ID_INSANE, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_INSANE); 
	if (!strcmp(preset_str, "insane")) {
		preset_sel = dwIndex;
	}

    LoadString(hinst, ID_RANDOM, achTemp, sizeof(achTemp)); 
    dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
        (LPARAM) (LPCSTR) achTemp); 
    SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, ID_RANDOM); 
	if (!strcmp(preset_str, "random")) {
		preset_sel = dwIndex;
	}

	SendMessage(hwndCombo, CB_SETCURSEL, preset_sel, 0);

	return TRUE; 
} 
 


// needed for SCRNSAVE.LIB
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
	return TRUE;
}


// Enable OpenGL

void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	*hDC = GetDC( hWnd );
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( *hDC, &pfd );
	SetPixelFormat( *hDC, format, &pfd );
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext( *hDC );
	wglMakeCurrent( *hDC, *hRC );
	
}

// Disable OpenGL

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}

// Functions From Flurry
double currentTime(void) {
	struct _timeb tb;
	_ftime( &tb );
	return (double)tb.time + (double)tb.millitm / 1000;
}

double TimeInSecondsSinceStart(void) {
    return currentTime() - gTimeCounter;
}

void delete_flurry_info(flurry_info_t *flurry)
{
    int i;

    free(flurry->s);
    free(flurry->star);    
	for (i = 0; i < MAX_SPARKS; i++)
    {
		free(flurry->spark[i]);
    }
	
}

flurry_info_t *new_flurry_info(global_info_t *global, int streams, ColorModes colour, float thickness, float speed, double bf)
{
	int i,k;
	flurry_info_t *flurry = (flurry_info_t *)malloc(sizeof(flurry_info_t));

	if (!flurry) return NULL;

	flurry->flurryRandomSeed = RandFlt(0.0, 300.0);

	flurry->dframe = 0;
	flurry->fOldTime = 0;
	flurry->fTime = TimeInSecondsSinceStart() + flurry->flurryRandomSeed;
	flurry->fDeltaTime = flurry->fTime - flurry->fOldTime;

	flurry->numStreams = streams;
	flurry->streamExpansion = thickness;
	flurry->currentColorMode = colour;
	flurry->briteFactor = bf;

	flurry->s = (SmokeV*)malloc(sizeof(SmokeV));
	InitSmoke(flurry->s);

	flurry->star = (Star*)malloc(sizeof(Star));
	InitStar(flurry->star);

	flurry->star->rotSpeed = speed;

	for (i = 0; i < MAX_SPARKS; i++)
	{
		flurry->spark[i] = (Spark*)malloc(sizeof(Spark));
		InitSpark(flurry->spark[i]);
		flurry->spark[i]->mystery = 1800 * (i + 1) / 13; /* 100 * (i + 1) / (flurry->numStreams + 1); */
		UpdateSpark(global, flurry, flurry->spark[i]);
	}

	for (i = 0; i < NUMSMOKEPARTICLES / 4; i++) {
		for (k = 0; k < 4; k++) {
			flurry->s->p[i].dead.i[k] = 1;
		}
	}

	flurry->next = NULL;

	return flurry;
}

void GLSetupRC(global_info_t *global)
{
    /* setup the defaults for OpenGL */
    glDisable(GL_DEPTH_TEST);
    glAlphaFunc(GL_GREATER,0.0f);
    glEnable(GL_ALPHA_TEST);
    glShadeModel(GL_FLAT);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glViewport(0,0,(int) global->sys_glWidth,(int) global->sys_glHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,global->sys_glWidth,0,global->sys_glHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_COLOR_ARRAY);	
    glEnableClientState(GL_VERTEX_ARRAY);	
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void GLRenderScene(global_info_t *global, flurry_info_t *flurry, double b)
{
    int i;

    flurry->dframe++;

    flurry->fOldTime = flurry->fTime;
    flurry->fTime = TimeInSecondsSinceStart() + flurry->flurryRandomSeed;
    flurry->fDeltaTime = flurry->fTime - flurry->fOldTime;
    flurry->drag = (float)pow(0.9965, flurry->fDeltaTime * 85.0);

    UpdateStar(global, flurry, flurry->star);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	for (i = 0; i < flurry->numStreams; i++) {
		flurry->spark[i]->color[0]=1.0;
		flurry->spark[i]->color[1]=1.0;
		flurry->spark[i]->color[2]=1.0;
		flurry->spark[i]->color[2]=1.0;

		UpdateSpark(global, flurry, flurry->spark[i]);
		// DrawSpark(global, flurry, flurry->spark[i]);
    }

    UpdateSmoke_ScalarBase(global, flurry, flurry->s);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	DrawSmoke_Scalar(global, flurry, flurry->s, b);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

/* new window size or exposure */
void reshape_flurry(int width, int height)
{
    global_info_t *global = flurry_info;
    wglMakeCurrent(global->window, global->glx_context);

    glViewport(0.0, 0.0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glFlush();

	global->sys_glWidth = width;
    global->sys_glHeight = height;
}

void
init_flurry(HWND hWnd, global_info_t *global)
{
	int preset_num;
	int i;

	RECT rect;

	enum presets{
		PRESET_INSANE = -1,
		PRESET_WATER = 0,
		PRESET_FIRE,
		PRESET_PSYCHEDELIC,
		PRESET_RGB,
		PRESET_BINARY,
		PRESET_CLASSIC,
		PRESET_MAX
	};

	global->flurry = NULL;

	char* preset;

	if (!preset_str || !*preset_str) 
		preset = DEF_PRESET;
	else
		preset = preset_str;

	if (!strcmp(preset, "random")) {
		preset_num = (random() % PRESET_MAX);
	} else if (!strcmp(preset, "water")) {
		preset_num = PRESET_WATER;
	} else if (!strcmp(preset, "fire")) {
		preset_num = PRESET_FIRE;
	} else if (!strcmp(preset, "psychedelic")) {
		preset_num = PRESET_PSYCHEDELIC;
	} else if (!strcmp(preset, "rgb")) {
		preset_num = PRESET_RGB;
	} else if (!strcmp(preset, "binary")) {
		preset_num = PRESET_BINARY;
	} else if (!strcmp(preset, "insane")) {
		preset_num = PRESET_INSANE;
	} else if (!strcmp(preset, "classic")) {
		preset_num = PRESET_CLASSIC;
	}

	switch (preset_num) {
		case PRESET_WATER: 
		{
			for (i = 0; i < 9; i++) {
				flurry_info_t *flurry;

				flurry = new_flurry_info(global, 1, blueColorMode, 100.0, 2.0, 2.0);
				flurry->next = global->flurry;
				global->flurry = flurry;
			}
			break;
		}
		case PRESET_FIRE: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 12, slowCyclicColorMode, 10000.0, 0.0, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;	
		
			break;
		}
		case PRESET_PSYCHEDELIC: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 10, rainbowColorMode, 200.0, 2.0, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;	
		
			break;
		}
		case PRESET_RGB: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 3, redColorMode, 100.0, 0.8, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;	

			flurry = new_flurry_info(global, 3, greenColorMode, 100.0, 0.8, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;	

			flurry = new_flurry_info(global, 3, blueColorMode, 100.0, 0.8, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;	
		
			break;
		}
		case PRESET_BINARY: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 16, tiedyeColorMode, 1000.0, 0.5, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;

			flurry = new_flurry_info(global, 16, tiedyeColorMode, 1000.0, 1.5, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;

			break;
		}
		case PRESET_INSANE: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 64, tiedyeColorMode, 1000.0, 0.5, 0.5);
			flurry->next = global->flurry;
			global->flurry = flurry;

			break;
		}
		case PRESET_CLASSIC:
		default: 
		{
			flurry_info_t *flurry;

			flurry = new_flurry_info(global, 5, tiedyeColorMode, 10000.0, 1.0, 1.0);
			flurry->next = global->flurry;
			global->flurry = flurry;

			break;
		}
	};

	GetClientRect( hWnd, &rect );
	reshape_flurry(rect.right, rect.bottom);
	GLSetupRC(global);
}

void
draw_flurry(HDC *hDC, HGLRC *hRC)
{
    static int first = 1;
    static double oldFrameTime = -1;
    double newFrameTime;
    double deltaFrameTime = 0;
    double brite;
    GLfloat alpha;

    global_info_t *global = flurry_info;
    flurry_info_t *flurry;

	newFrameTime = currentTime();

    if (oldFrameTime == -1) {
		/* special case the first frame -- clear to black */
		alpha = 1.0;
	} else {
		/* framerate clamping taken care of by timer */
		deltaFrameTime = newFrameTime - oldFrameTime;
		alpha = 5.0 * deltaFrameTime;
	}
	
	oldFrameTime = newFrameTime;

    if (alpha > 0.2) alpha = 0.2;
    if (!global->glx_context) return;

    if (first) {
		MakeTexture();
		first = 0;
    }
    glDrawBuffer(GL_BACK);
	wglMakeCurrent(*hDC, *hRC);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0, 0.0, 0.0, alpha);
    glRectd(0, 0, global->sys_glWidth, global->sys_glHeight);

    brite = pow(deltaFrameTime, 0.75) * 10;
    for (flurry = global->flurry; flurry; flurry = flurry->next) {
		GLRenderScene(global, flurry, brite * flurry->briteFactor);
    }

    glFinish();
    SwapBuffers(*hDC);
}

void
release_flurry()
{
	if (flurry_info != NULL) {
		global_info_t *global = flurry_info;
		flurry_info_t *flurry;
		flurry_info_t *flurry_old;

		wglMakeCurrent(global->window, global->glx_context);

		flurry = global->flurry; 

		while (flurry) {
			delete_flurry_info(flurry);
			flurry_old = flurry;
			flurry = flurry->next;
			free(flurry_old);
		}
	
		(void)free((void*)flurry_info);
		flurry_info = NULL;
	}
}