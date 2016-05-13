// OpenCLRaytracer.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

#include <CL\cl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include "ContextManager.h"
#include "KernelArguments.h"

HBITMAP bitmap = NULL;
HANDLE buffer = NULL;
float* texture = NULL;
//#include <png.h>


ContextManager* ctxMgr;

#include "OpenCLRaytracer.h"

#define MAX_LOADSTRING 100

cl_float3 *color = NULL;
cl_int W = GetSystemMetrics(SM_CXSCREEN);// 400;
cl_int H = GetSystemMetrics(SM_CYSCREEN);//300;
float factor = 9000000.0f;
cl_float w = W * factor;
cl_float h = H * factor;
cl_float f = sqrt(pow(W, 2) + pow(H, 2)) *factor;

const char* readFile(const char* filename)
{
	std::ifstream sourceFilename(filename);
	std::string sourceFile(std::istreambuf_iterator<char>(sourceFilename),
		(std::istreambuf_iterator<char>()));

	int len = sourceFile.size();
	char* charArr = new char[len + 1];
	std::memcpy(charArr, sourceFile.c_str(), sizeof(char) * len);
	charArr[len] = '\0';
	return charArr;
}
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
	cl_image_format imgFmt;
	imgFmt.image_channel_data_type = CL_FLOAT;
	imgFmt.image_channel_order = CL_RGB;

	cl_image_desc imgDesc;
	imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
	imgDesc.image_width = W;
	imgDesc.image_height = H;
	imgDesc.buffer = NULL;
	imgDesc.image_array_size = 1;
	imgDesc.image_depth = 0;
	imgDesc.image_row_pitch = 0;
	imgDesc.image_slice_pitch = 0;
	imgDesc.num_samples = 0;
	imgDesc.num_mip_levels = 0;

	cl_float3 origin;
	origin.x = -1.0f, origin.y = 0.0f, origin.z = -10.0f;
	cl_float3 view;
	view.x = 5.0f, view.y = 0.0f, view.z = 1.0f;
	cl_float3 up;
	up.x = -0.25f, up.y = -1.0f, up.z = 0.0f;
	cl_float3 *ta = NULL;
	cl_float3 *tb = NULL;
	cl_float3 *tc = NULL;
	cl_float3 *triColor = NULL;
	cl_float *tri_kr = NULL;
	cl_float *tri_kt = NULL;
	cl_int triCount = 2;

	int trilistsize = sizeof(cl_float3) * triCount;
	ta = (cl_float3*)malloc(trilistsize);
	tb = (cl_float3*)malloc(trilistsize);
	tc = (cl_float3*)malloc(trilistsize);
	triColor = (cl_float3*)malloc(trilistsize);
	tri_kr = (cl_float*)malloc(sizeof(cl_float) * triCount);
	tri_kt = (cl_float*)malloc(sizeof(cl_float) * triCount);

	cl_float3 *center = NULL;
	cl_float *r = NULL;
	cl_float3 *sprColor = NULL;
	cl_float *spr_kr = NULL;
	cl_float *spr_kt = NULL;
	cl_int sphereCount = 2;

	center = (cl_float3*)malloc(sizeof(cl_float3) * sphereCount);
	r = (cl_float*)malloc(sizeof(cl_float) * sphereCount);
	sprColor = (cl_float3*)malloc(sizeof(cl_float3) * sphereCount);
	spr_kr = (cl_float*)malloc(sizeof(cl_float) * sphereCount);
	spr_kt = (cl_float*)malloc(sizeof(cl_float) * sphereCount);

	cl_float3 *light = NULL;
	cl_float3 *light_color = NULL;
	cl_int *num_photons = NULL;
	cl_int lightCount = 4;

	light = (cl_float3*)malloc(sizeof(cl_float3) * lightCount);
	light_color = (cl_float3*)malloc(sizeof(cl_float3) * lightCount);
	num_photons = (cl_int*)malloc(sizeof(cl_int) * lightCount);

	light[0].x = -1.0, light[0].y = 12.0, light[0].z = -5.0;
	light_color[0].x = 2.0, light_color[0].y = 2.0, light_color[0].y = 2.0;
	num_photons[0] = 4000;


	light[1].x = -1.7, light[1].y = 1.0, light[1].z = 5.0;
	light_color[1].x = 0.1, light_color[1].y = 0.1, light_color[1].y = 0.1;
	num_photons[1] = 1000;


	light[2].x = 0.0, light[2].y = 2.6, light[2].z = 6.0;
	light_color[2].x = 0.1, light_color[2].y = 0.1, light_color[2].y = 0.1;
	num_photons[2] = 1000;


	light[3].x = 0.0, light[3].y = -5.0, light[3].z = -2.0;
	light_color[3].x = 0.1, light_color[3].y = 0.1, light_color[3].y = 0.1;
	num_photons[3] = 1000;


	cl_float filmplane = 10.0f;

	const int pixels = W*H;

	cl_float3 a;
	a.x = 4.0, a.y = -3, a.z = 0;
	ta[0] = a;
	ta[1] = a;

	cl_float3 b;
	b.x = -4.0, b.y = 1.5, b.z = 22;
	tb[0] = b;
	tb[1] = b;
	a.x = -4;
	tc[1] = a;

	cl_float3 c;
	c.x = 4.0, c.y = 1.5, c.z = 22;
	tc[0] = c;
	triColor[0].x = 0, triColor[0].y = 0, triColor[0].z = 0;
	tri_kr[0] = 0.25f;
	tri_kt[0] = 0.0f;
	triColor[1].x = 0, triColor[1].y = 0, triColor[1].z = 0;
	tri_kr[1] = 0.25f;
	tri_kt[1] = 0.0f;


	cl_float3 cen;
	cen.x = 0.0f, cen.y = 0.2f, cen.z = 8;

	center[0] = cen;
	r[0] = 0.85f;
	sprColor[0].x = 0, sprColor[0].y = 0, sprColor[0].z = 0;
	spr_kr[0] = 0.5f;
	spr_kt[0] = 0.0f;


	cen.x = 1.3, cen.y = 0.7, cen.z = 6;
	center[1] = cen;
	r[1] = 1.0f;
	sprColor[1].x = 0, sprColor[1].y = 0, sprColor[1].z = 0;
	spr_kr[1] = 0.0f;
	spr_kt[1] = 0.5f;


	int outputsize = sizeof(cl_float3) * pixels;
	color = (cl_float3*)malloc(outputsize);
	texture = (float*)malloc(sizeof(float)*pixels);

	cl_int status;
	
	cl_mem bufferTa;
	cl_mem bufferTb;
	cl_mem bufferTc;
	cl_mem bufferTriColor;
	cl_mem bufferTriRef;
	cl_mem bufferTriTrans;

	cl_mem bufferCenter;
	cl_mem bufferR;
	cl_mem bufferSprColor;
	cl_mem bufferSprRef;
	cl_mem bufferSprTrans;

	cl_mem bufferLight;
	cl_mem bufferLightColor;
	cl_mem bufferNumPhotons;

	cl_mem bufferColor;
	cl_mem bufferTexture;
	cl_mem image;

	std::cout << "Hello World\n";
	cl_context context = ctxMgr->context();
	
	
	bufferTa = clCreateBuffer(context, CL_MEM_READ_ONLY, trilistsize, NULL, &status);
	bufferTb = clCreateBuffer(context, CL_MEM_READ_ONLY, trilistsize, NULL, &status);
	bufferTc = clCreateBuffer(context, CL_MEM_READ_ONLY, trilistsize, NULL, &status);
	bufferTriColor = clCreateBuffer(context, CL_MEM_READ_ONLY, trilistsize, NULL, &status);
	bufferTriRef = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * triCount, NULL, &status);
	bufferTriTrans = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * triCount, NULL, &status);

	bufferCenter = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * sphereCount, NULL, &status);
	bufferR = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * sphereCount, NULL, &status);
	bufferSprColor = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * sphereCount, NULL, &status);
	bufferSprRef = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * sphereCount, NULL, &status);
	bufferSprTrans = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float) * sphereCount, NULL, &status);

	bufferLight = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * lightCount, NULL, &status);
	bufferLightColor = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3) * lightCount, NULL, &status);
	bufferNumPhotons = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_int) * lightCount, NULL, &status);

	bufferColor = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputsize, NULL, &status);
	bufferTexture = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * pixels, NULL, &status);
	//image = clCreateImage(context, CL_MEM_WRITE_ONLY, &imgFmt, &imgDesc, NULL, &status);

	cl_command_queue cmdQueue = ctxMgr->cmdQueue();

	status = clEnqueueWriteBuffer(cmdQueue, bufferTa, CL_FALSE, 0, trilistsize, ta, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferTb, CL_FALSE, 0, trilistsize, tb, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferTc, CL_FALSE, 0, trilistsize, tc, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferTriColor, CL_FALSE, 0, trilistsize, triColor, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferTriRef, CL_FALSE, 0, sizeof(cl_float) * triCount, tri_kr, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferTriTrans, CL_FALSE, 0, sizeof(cl_float) * triCount, tri_kt, 0, NULL, NULL);

	status = clEnqueueWriteBuffer(cmdQueue, bufferCenter, CL_FALSE, 0, sizeof(cl_float3) * sphereCount, center, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferR, CL_FALSE, 0, sizeof(cl_float) * sphereCount, r, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferSprColor, CL_FALSE, 0, sizeof(cl_float3) * sphereCount, sprColor, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferSprRef, CL_FALSE, 0, sizeof(cl_float) * sphereCount, spr_kr, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferSprTrans, CL_FALSE, 0, sizeof(cl_float) * sphereCount, spr_kt, 0, NULL, NULL);

	status = clEnqueueWriteBuffer(cmdQueue, bufferLight, CL_FALSE, 0, sizeof(cl_float3) * lightCount, light, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferLightColor, CL_FALSE, 0, sizeof(cl_float3) * lightCount, light_color, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bufferNumPhotons, CL_FALSE, 0, sizeof(cl_int) * lightCount, num_photons, 0, NULL, NULL);
	
	const char* programSource = readFile("clraytracer.cl");


	cl_program program = clCreateProgramWithSource(ctxMgr->context(), 1, (const char**)&programSource, NULL, &status);
	status = clBuildProgram(program, ctxMgr->numDevices(), ctxMgr->devices(), NULL, NULL, NULL);
	
	cl_kernel kernel = NULL;

	kernel = clCreateKernel(program, "scan", &status);
	
	status = clSetKernelArg(kernel, 0, sizeof(cl_float3), &origin);
	status |= clSetKernelArg(kernel, 1, sizeof(cl_float3), &view);
	status |= clSetKernelArg(kernel, 2, sizeof(cl_float3), &up);
	status |= clSetKernelArg(kernel, 3, sizeof(cl_int), &W);
	status |= clSetKernelArg(kernel, 4, sizeof(cl_int), &H);
	status |= clSetKernelArg(kernel, 5, sizeof(cl_float), &w);
	status |= clSetKernelArg(kernel, 6, sizeof(cl_float), &h);
	status |= clSetKernelArg(kernel, 7, sizeof(cl_float), &f);
	status |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &bufferTa);
	status |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &bufferTb);
	status |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &bufferTc);
	status |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &bufferTriColor);
	status |= clSetKernelArg(kernel, 12, sizeof(cl_mem), &bufferTriRef);
	status |= clSetKernelArg(kernel, 13, sizeof(cl_mem), &bufferTriTrans);
	status |= clSetKernelArg(kernel, 14, sizeof(cl_int), &triCount);
	status |= clSetKernelArg(kernel, 15, sizeof(cl_mem), &bufferCenter);
	status |= clSetKernelArg(kernel, 16, sizeof(cl_mem), &bufferR);
	status |= clSetKernelArg(kernel, 17, sizeof(cl_mem), &bufferSprColor);
	status |= clSetKernelArg(kernel, 18, sizeof(cl_mem), &bufferSprRef);
	status |= clSetKernelArg(kernel, 19, sizeof(cl_mem), &bufferSprTrans);
	status |= clSetKernelArg(kernel, 20, sizeof(cl_int), &sphereCount);
	status |= clSetKernelArg(kernel, 21, sizeof(cl_mem), &bufferLight);
	status |= clSetKernelArg(kernel, 22, sizeof(cl_mem), &bufferLightColor);
	status |= clSetKernelArg(kernel, 23, sizeof(cl_mem), &bufferNumPhotons);
	status |= clSetKernelArg(kernel, 24, sizeof(cl_int), &lightCount);
	//status |= clSetKernelArg(kernel, 25, sizeof(cl_mem), &image);
	status |= clSetKernelArg(kernel, 25, sizeof(cl_mem), &bufferColor);
	
	size_t globalWorkSize[1];


	globalWorkSize[0] = pixels;

	status = clEnqueueNDRangeKernel(ctxMgr->cmdQueue(), kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);

	//clEnqueueReadBuffer(cmdQueue, bufferTexture, CL_TRUE, 0, sizeof(float) * pixels, texture, 0, NULL, NULL);
	clEnqueueReadBuffer(ctxMgr->cmdQueue(), bufferColor, CL_TRUE, 0, outputsize, color, 0, NULL, NULL);

	//bitmap = CreateBitmap(W, H, 1, 64, texture);
	

	//buffer = CreateConsoleScreenBuffer(GENERIC_READ, FILE_SHARE_READ, 0, CONSOLE_TEXTMODE_BUFFER, texture);
	//writeToImage("scene.png", W, H, texture);
	

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
