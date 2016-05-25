#ifndef _CAMERA_H
#define _CAMERA_H

#include "CL\cl.h"

class Camera
{
public:
	Camera();
	Camera(cl_float3 origin, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f);
	Camera(cl_float3 origin, cl_float3 view, cl_float3 up, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f);
	Camera(const Camera& copy);
	~Camera();
	Camera& operator=(const Camera& copy);

	cl_float3 getOrigin();
	void setOrigin(cl_float3 origin);

	cl_float3 getView();
	void setView(cl_float3 view);

	cl_float3 getUp();
	void setUp(cl_float3 up);

	cl_int getW();
	void setW(cl_int W);

	cl_int getH();
	void setH(cl_int H);

	cl_float getWidth();
	void setWidth(cl_float w);

	cl_float getHeight();
	void setHeight(cl_float h);

	cl_float getF();
	void setF(cl_float f);

	__declspec(property(get = getOrigin, put = setOrigin)) cl_float3 origin;
	__declspec(property(get = getView, put = setView)) cl_float3 view;
	__declspec(property(get = getUp, put = setUp)) cl_float3 up;
	__declspec(property(get = getW, put = setW)) cl_int W;
	__declspec(property(get = getH, put = setH)) cl_int H;
	__declspec(property(get = getWidth, put = setWidth)) cl_float w;
	__declspec(property(get = getHeight, put = setHeight)) cl_float h;
	__declspec(property(get = getF, put = setF)) cl_float f;
protected:
	cl_float3 m_origin;
	cl_float3 m_view;
	cl_float3 m_up;
	cl_int m_W;
	cl_int m_H;
	cl_float m_w;
	cl_float m_h;
	cl_float m_f;
private:
};
#endif