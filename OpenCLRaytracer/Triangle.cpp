
#include "stdafx.h"
#include "Triangle.h"
#include <iostream>

Triangle::Triangle() : Object()
{
	cl_float3 zero;
	zero.x = zero.y = zero.z = 0;

	for (int i = 0; i < 3; ++i)
		m_points[i] = zero;
}

Triangle::Triangle(cl_float3 A, cl_float3 B, cl_float3 C, cl_float3 color, cl_float kr, cl_float kt, cl_float roughness, cl_bool isChecker)
	:Object(color, kr, kt, roughness, isChecker)
{
	m_points[0] = A;
	m_points[1] = B;
	m_points[2] = C;
}

Triangle::Triangle(const Triangle& copy)
{
	std::memcpy(m_points, copy.m_points, sizeof(cl_float3) * 3);
	m_color = copy.m_color;
	m_kr = copy.m_kr;
	m_kt = copy.m_kt;
	m_roughness = copy.m_roughness;
	m_isChecker = copy.m_isChecker;
}

Triangle& Triangle::operator=(const Triangle& copy)
{
	std::memcpy(m_points, copy.m_points, sizeof(cl_float3) * 3);
	m_color = copy.m_color;
	m_kr = copy.m_kr;
	m_kt = copy.m_kt;
	m_roughness = copy.m_roughness;
	m_isChecker = copy.m_isChecker;
	return *this;
}

Triangle::~Triangle()
{

}

cl_float3 Triangle::getA(){ return m_points[0]; }
void Triangle::setA(cl_float3 A){ m_points[0] = A; }


cl_float3 Triangle::getB(){ return m_points[1]; }
void Triangle::setB(cl_float3 B){ m_points[1] = B; }


cl_float3 Triangle::getC(){ return m_points[2]; }
void Triangle::setC(cl_float3 A){ m_points[2] = C; }