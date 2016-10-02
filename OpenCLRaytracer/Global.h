#ifndef GLOBAL_H
#define GLOBAL_H

#include "CL\cl.h"
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include <memory>
//#define OutputDebugNumber(x) char buffer[60]; sprintf(buffer, "%d\n", x); OutputDebugString(buffer)

static void OutputDebugString(const std::string fmt, ...){
	va_list arg_list;
	va_start(arg_list, fmt);

	char buffer[256];
	const size_t needed = vsnprintf(buffer, sizeof buffer, fmt.c_str(), arg_list) + 1;
	if (needed <= sizeof buffer)
	{
		OutputDebugString(buffer);
		return;
	}

	char * p = static_cast<char*>(alloca(needed));
	vsnprintf(p, needed, fmt.c_str(), arg_list);
	OutputDebugString(p);
}

static void OutputDebugNumber(int number){
	char buffer[60];
	sprintf(buffer, "%d\n", number);
	OutputDebugString(buffer);
}

static void OutputDebugNumber(float number){
	char buffer[60];
	sprintf(buffer, "%.1f\n", number);
	OutputDebugString(buffer);
}

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

static cl_float3 operator/(const cl_float3 &vec, const float & denom){
	cl_float3 result = { vec.x / denom, vec.y / denom, vec.z / denom };
	return result;
}

namespace Global{
	static const cl_float3 zero3 = {0,0,0};
};
#endif