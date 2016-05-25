#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
	:m_origin({ 0, 0, 0 }), m_W(0), m_H(0), m_w(0), m_h(0), m_f(0)
{}

Camera::Camera(cl_float3 origin, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f)
	:m_origin(origin), 
	m_W(W), m_H(H), m_w(w), m_h(h), m_f(f)
{}

Camera::Camera(cl_float3 origin, cl_float3 view, cl_float3 up, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f)
	: m_origin(origin), m_view(view), m_up(up), 
	m_W(W), m_H(H), m_w(w), m_h(h), m_f(f)
{}

Camera::Camera(const Camera& copy)
	: m_origin(copy.m_origin), m_view(copy.m_view), m_up(copy.m_up), 
	m_W(copy.m_W), m_H(copy.m_H), m_w(copy.m_w), m_h(copy.m_h), m_f(copy.m_f)
{}

Camera::~Camera(){}

Camera& Camera::operator=(const Camera& copy)
{
	m_origin = copy.m_origin;
	m_view = copy.m_view;
	m_up = copy.m_up;
	m_W = copy.m_W;
	m_H = copy.m_H;
	m_w = copy.m_w;
	m_h = copy.m_h;
	m_f = copy.m_f;
	return *this;
}

cl_float3 Camera::getOrigin(){ return m_origin; }
void Camera::setOrigin(cl_float3 origin){ m_origin = origin; }

cl_float3 Camera::getView(){ return m_view; }
void Camera::setView(cl_float3 view){ m_view = view; }

cl_float3 Camera::getUp(){ return m_up; }
void Camera::setUp(cl_float3 up){ m_up = up; }

cl_int Camera::getW(){ return m_W; }
void Camera::setW(cl_int W){ m_W = W; }

cl_int Camera::getH(){ return m_H; }
void Camera::setH(cl_int H){ m_H = H; }

cl_float Camera::getWidth(){ return m_w; }
void Camera::setWidth(cl_float w){ m_w = w; }

cl_float Camera::getHeight(){ return m_h; }
void Camera::setHeight(cl_float h){ m_h = h; }

cl_float Camera::getF(){ return m_f; }
void Camera::setF(cl_float f){ m_f = f; }