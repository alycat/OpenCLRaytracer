#ifndef _OBJECTMANAGER_H
#define _OBJECTMANAGER_H

#include "Triangle.h"
#include "Sphere.h"
#include <vector>

class BufferManager;

class ObjectManager
{
public:
	ObjectManager(BufferManager& bufferMgr);
	ObjectManager(const ObjectManager& copy);
	~ObjectManager();
	ObjectManager& operator=(const ObjectManager& copy);
	void addTriangle(Triangle triangle);
	int triCount();
	void addSphere(Sphere sphere);
	int sprCount();
	void createObjectBuffers();

	std::vector<Triangle> triangles();
	std::vector<Sphere> spheres();

	int triangleIndex(Triangle triangle);
	int sphereIndex(Sphere sphere);
protected:
	std::vector<Triangle> m_triangles;
	std::vector<Sphere> m_spheres;
	BufferManager& m_bufferMgr;
	cl_float3 *ta;
	cl_float3 *tb;
	cl_float3 *tc;
	cl_float3 *tri_color;
	cl_float *t_kr;
	cl_float *t_kt;
	cl_float *t_roughness;
	cl_bool *t_isChecker;
	
	cl_float3 *center;
	cl_float *r;
	cl_float3 *sprColor;
	cl_float *s_kr;
	cl_float *s_kt;
	cl_float *s_roughness;
	cl_bool *s_isChecker;
private:
};
#endif