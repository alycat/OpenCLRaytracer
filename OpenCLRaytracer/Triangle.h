#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "Object.h"

class Triangle : public Object
{
public:
	Triangle();
	Triangle(cl_float3 A, cl_float3 B, cl_float3 C, cl_float3 color, cl_float kr, cl_float kt, cl_float roughness, cl_bool isChecker);
	Triangle(const Triangle& copy);
	Triangle& operator=(const Triangle& copy);

	~Triangle();

	cl_float3 getA();
	void setA(cl_float3 A);
	__declspec(property(get = getA, put = setA)) cl_float3 A;

	cl_float3 getB();
	void setB(cl_float3 B);
	__declspec(property(get = getB, put = setB)) cl_float3 B;

	cl_float3 getC();
	void setC(cl_float3 C);
	__declspec(property(get = getC, put = setC)) cl_float3 C;
protected:
	cl_float3 m_points[3];
private:
};
#endif