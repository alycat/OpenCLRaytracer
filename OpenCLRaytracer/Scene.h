#ifndef _SCENE_H
#define _SCENE_H

#include <CL\cl.h>
#include "LightManager.h"
#include "ObjectManager.h"
#include "Camera.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Light.h"
#include "BufferMananger.h"
#include "KernelManager.h"
#include "ContextManager.h"
#include "KDNode.h"

class Scene{
public:
	Scene();
	Scene(const Scene& copy);
	~Scene();
	Scene& operator=(const Scene& copy);
	void setCamera(cl_float3 origin, cl_float3 view, cl_float3 up, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f);
	void setCamera(cl_float3 origin, cl_float3 view, cl_float3 up);
	void addLight(Light light);
	void addTriangle(Triangle triangle);
	void addSphere(Sphere sphere);
	void setOrigin(cl_float3 origin);
	void setView(cl_float3 view);
	void setUp(cl_float3 up);
	void setW(cl_int W);
	void setH(cl_int H);
	void setWidth(cl_float w);
	void setHeight(cl_float h);

	void createBuffers();
	void setParamaters();

	cl_float3 color(int x, int y);
	cl_float3* color();
protected:
	LightManager* m_lgtMgr;
	ObjectManager* m_objMgr;
	BufferManager m_bufferMgr;
	KernelManager m_kerMgr;
	Camera m_cam;
	ContextManager* m_ctxMgr;
	cl_float3 * m_color;
	KDTree* m_tree;
private:
};
#endif