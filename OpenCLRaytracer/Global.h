#ifndef GLOBAL_H
#define GLOBAL_H

#include "CL\cl.h"
#include <stdio.h>

#define OutputDebugNumber(x) char buffer[60]; sprintf(buffer, "%d\n", x); OutputDebugString(buffer)

static void OutputDebugFloat3(cl_float3 vector)
{
	char buffer[60];
	sprintf(buffer, "{%d,%d,%d}\n", vector.x, vector.y, vector.z);
	OutputDebugString(buffer);
}

static cl_float3 operator+(const cl_float3& a, const cl_float3 & b){
	cl_float3 c = { a.x + b.x, a.y + b.y, a.z + b.z };
	return c;
}

static cl_float3 operator-(const cl_float3& a, const cl_float3 & b){
	cl_float3 c = { a.x - b.x, a.y - b.y, a.z - b.z };
	return c;
}

static bool operator!=(const cl_float3& a, const cl_float3& b)
{
	if (a.x != b.x)
		return true;
	if (a.y != b.y)
		return true;
	if (a.z != b.z)
		return true;
	return false;
}

namespace Global{
	static const cl_float3 zero3 = {0,0,0};
};
#endif