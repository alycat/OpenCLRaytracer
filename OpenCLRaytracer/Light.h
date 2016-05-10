#ifndef _LIGHT_H
#define _LIGHT_H

#include <CL\cl.h>

class Light{
public:
	Light();
protected:
	cl_float3 position;
	cl_float3 color;
private:
};
#endif