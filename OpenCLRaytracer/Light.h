#ifndef _LIGHT_H
#define _LIGHT_H

#include <CL\cl.h>

class Light{
public:
	Light();
	Light(cl_float3 position, cl_float3 color, cl_int numPhotons);
	Light(const Light& copy);
	~Light();
	Light& operator=(const Light& copy);

	__declspec(property(get = getPosition, put = setPosition)) cl_float3 position;
	__declspec(property(get = getColor, put = setColor)) cl_float3 color;
	__declspec(property(get = getNumPhotons, put = setNumPhotons)) cl_int numPhotons;

	cl_float3 getPosition();
	void setPosition(cl_float3 position);

	cl_float3 getColor();
	void setColor(cl_float3 color);

	cl_int getNumPhotons();
	void setNumPhotons(cl_int numPhotons);
protected:
	cl_float3 m_position;
	cl_float3 m_color;
	cl_int m_numPhotons;
private:
};
#endif