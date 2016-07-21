#ifndef _SPHERE_H
#define _SPHERE_H

#include "Object.h"

class Sphere : public Object
{
public:
	Sphere();
	Sphere(cl_float3 center, cl_float radius, cl_float3 color, cl_float kr, cl_float kt, cl_float roughness, cl_bool isChecker);
	Sphere(const Sphere& copy);
	Sphere& operator=(const Sphere& copy);
	~Sphere();

	bool operator==(const Sphere& other) const;

	cl_float3 getCenter();
	void setCenter(cl_float3 center);
	__declspec(property(get = getCenter, put = setCenter)) cl_float3 center;

	cl_float getRadius();
	void setRadius(cl_float radius);
	__declspec(property(get = getRadius, put = setRadius)) cl_float radius;

	BoundingBox boundingBox() override;
protected:
	cl_float3 m_center;
	cl_float m_radius;
private:
};
#endif