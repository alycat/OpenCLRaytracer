// OpenCLRaytracer.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

#include <CL\cl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include "ContextManager.h"
#include "OpenCLRaytracer.h"
#include "INIReader.h"
#include "Parser.h"
#include "Scene.h"
#include <vector>
#include "ply.h"

HBITMAP bitmap = NULL;
HANDLE buffer = NULL;
float* texture = NULL;

#include <direct.h>
#define GetCurrentDir _getcwd

const char* currentDirectory()
{
	char currentPath[255];
	memset(currentPath, 0x00, 255);
	if (!GetCurrentDir(currentPath, 255))
	{
		return "";
	}
	return currentPath;
}



char *elem_names[] = {
	"vertex", "face"
};

typedef struct Vertex{
	float x, y, z, confidence, intensity;
};

typedef struct Face{
	unsigned char nverts;
	int *verts;
};

PlyProperty vert_props[] = {
	{ "x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, x), 0, 0, 0, 0 },
	{ "y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, y), 0, 0, 0, 0 },
	{ "z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, z), 0, 0, 0, 0 },
	{ "confidence", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, confidence), 0, 0, 0, 0 },
	{ "intensity", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, intensity), 0, 0, 0, 0 }
};

const PlyProperty face_props[] = {
	{ "vertex_indices", PLY_INT, PLY_INT, offsetof(Face, verts),
	1, PLY_UCHAR, PLY_UCHAR, offsetof(Face, nverts) },
};

static void ReadPlyFile(char* filepath, Scene& scene)
{
	int i, j, k;
	PlyFile *ply = NULL;
	int nelems;
	char **elist = NULL;
	int file_type;
	float version;
	int nprops;
	int num_elems;
	PlyProperty **plist = NULL;
	Vertex **vlist = NULL;
	Face **flist = NULL;
	char * elem_name = NULL;
	int num_obj_info;

	ply = ply_open_for_reading(filepath, &nelems, &elist, &file_type, &version);
	for (int i = 0; i < nelems; ++i)
	{
		elem_name = elist[i];
		plist = ply_get_element_description(ply, elem_name, &num_elems, &nprops);
		if (equal_strings("vertex", elem_name)){
			vlist = (Vertex **)malloc(sizeof(Vertex *) * num_elems);
			for (int i = 0; i < 3; ++i){
				ply_get_property(ply, elem_name, &vert_props[i]);
			}

			for (j = 0; j < num_elems; ++j){
				vlist[j] = (Vertex*)malloc(sizeof(Vertex));
				ply_get_element(ply, (void*)vlist[j]);
			}
		}
		else if (equal_strings("face", elem_name)){
			flist = (Face**)malloc(sizeof(Face*)*num_elems);
			ply_get_property(ply, elem_name, const_cast<PlyProperty*>(&face_props[0]));

			for (k = 0; k < num_elems; ++k){
				flist[k] = (Face*)malloc(sizeof(Face));
				ply_get_element(ply, (void*)flist[k]);
				cl_float3 points[3];
				for (int p = 0; p < 3; ++p){
					points[p].x = vlist[flist[k]->verts[p]]->x;// -1.0f;
					points[p].y = vlist[flist[k]->verts[p]]->y;// -0.8f;
					points[p].z = vlist[flist[k]->verts[p]]->z;// +4.0f;
				}
				static cl_float3 black = { 0, 0, 0 };
				static int count = 0;
				scene.addTriangle(Triangle(points[0], points[1], points[2], black, 0.0, 0.0, 0.3, false));
				free(flist[k]);
			}
			free(flist);
		}
	}
	if (ply)
		ply_close(ply);
	free(vlist);
}

//#include <png.h>


ContextManager* ctxMgr;


#define MAX_LOADSTRING 100

cl_float3 *color = NULL;
cl_int W = GetSystemMetrics(SM_CXSCREEN);// 400;
cl_int H = GetSystemMetrics(SM_CYSCREEN);//300;
float factor = 9000000.0f;
cl_float w = W * factor;
cl_float h = H * factor;
cl_float f = sqrt(pow(W, 2) + pow(H, 2)) *factor;

/*
void setRGB(png_byte *ptr, float val)
{
	int v = (int)(val * 767);
	if (v < 0) v = 0;
	if (v > 767) v = 767;
	int offset = v % 256;
	if (v < 256){
		ptr[0] = 0; ptr[1] = 0; ptr[2] = offset;
	}
	else if (v < 512){
		ptr[0] = 0; ptr[1] = offset; ptr[2] = 255-offset;
	}
	else{
		ptr[0] = offset; ptr[1] =255-offset; ptr[2]=0;
	}
	
}*/
/*
int writeToImage(const char* filename, int width, int height, float* buffer, char* title = "")
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	fp = fopen(filename, "wb");
	if (fp == NULL){
		code = 1;
		goto finalise;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL){
		code = 1;
		goto finalise;
	}
	/*
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL){
		code = 1;
		goto finalise;
	}

	if (setjmp(png_jmpbuf(png_ptr))){
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if (title != NULL)
	{
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}
	png_write_info(png_ptr, info_ptr);
	
	row = (png_bytep)malloc(3 * width * sizeof(png_byte));

	int _x, _y;
	for (_y = 0; _y < height; ++_y){
		for (_x = 0; _x < width; ++_x){
			setRGB(&(row[_x * 3]), buffer[_y * width + _x]);
		}
		png_write_row(png_ptr, row);
	}
	png_write_end(png_ptr, NULL);

finalise:
	if (fp != NULL) fclose(fp);
	/*if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);
	
	return code;
}
*/



// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	//W = 400;// GetSystemMetrics(SM_CXSCREEN);
	//H = 300;// GetSystemMetrics(SM_CYSCREEN);
	//f = 50000
	ctxMgr = ContextManager::instance();
	Scene scene;

	char path[255];
	strncpy(path, currentDirectory(), sizeof(path));
	strncat(path, "\\scene.ini", sizeof(path));
	INIReader reader(path);
	
	std::vector<char*> sections = reader.getSectionData();
	for each(char* section in sections)
	{
		char* c_type = reader.readString(section, "type");
		std::string type(c_type);
		if (type == "light")
		{
			cl_float3 l_position = reader.readFloat3(section, "position");
			cl_float3 l_color = reader.readFloat3(section, "color");
			int l_numPhotons = reader.readInt(section, "numPhotons");
			scene.addLight(Light(l_position, l_color, l_numPhotons));
		}
		else if (type == "sphere")
		{
			cl_float3 s_center = reader.readFloat3(section, "center");
			float s_r = reader.readFloat(section, "r");
			cl_float3 s_color = reader.readFloat3(section, "color");
			float s_kr = reader.readFloat(section, "kr");
			float s_kt = reader.readFloat(section, "kt");
			float s_roughness = reader.readFloat(section, "roughness");
			bool s_checker = reader.readBool(section, "isChecker");
			scene.addSphere(Sphere(s_center, s_r, s_color, s_kr, s_kt, s_roughness, s_checker));
		}
		else if (type == "triangle")
		{
			cl_float3 t_a = reader.readFloat3(section, "a");
			cl_float3 t_b = reader.readFloat3(section, "b");
			cl_float3 t_c = reader.readFloat3(section, "c");
			cl_float3 t_color = reader.readFloat3(section, "color");
			float t_kr = reader.readFloat(section, "kr");
			float t_kt = reader.readFloat(section, "kt");
			float t_roughness = reader.readFloat(section, "roughness");
			bool t_checker = reader.readBool(section, "isChecker");
			scene.addTriangle(Triangle(t_a, t_b, t_c, t_color, t_kr, t_kt, t_roughness, t_checker));
		}
		else if (type == "camera")
		{
			cl_float3 c_pos = reader.readFloat3(section, "position");
			cl_float3 c_view = reader.readFloat3(section, "view");
			cl_float3 c_up = reader.readFloat3(section, "up");
			scene.setCamera(c_pos, c_view, c_up, W, H, w, h, f);
		}
		else if (type == "model")
		{
			char * filename = reader.readString(section, "file");
			ReadPlyFile(filename, scene);
		}
	}


	int outputsize = sizeof(cl_float3) * W * H;
	color = (cl_float3*)malloc(outputsize);

	scene.createBuffers();
	scene.setParamaters();

	color = scene.color();
	

	/*
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(bufferTa);
	clReleaseMemObject(bufferTb);
	clReleaseMemObject(bufferTc);
	clReleaseMemObject(bufferCenter);
	clReleaseMemObject(bufferR);
	clReleaseMemObject(bufferColor);
	clReleaseContext(context);
	free(ta);
	free(tb);
	free(tb);
	free(center);
	free(r);
	//free(color);
	free(platforms);
	free(devices);
	*/

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_OPENCLRAYTRACER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OPENCLRAYTRACER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OPENCLRAYTRACER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_OPENCLRAYTRACER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		//SetConsoleActiveScreenBuffer(buffer);
		for (int i = 0; i < W * H; ++i)
		{
			int x = i % W;
			int y = i / W;
			cl_float3 c = color[i];
			SetPixel(hdc, x, y, RGB(c.x*255, c.y*255, c.z*255));
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
