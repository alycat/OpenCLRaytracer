#include "stdafx.h"
#include "ObjectManager.h"
#include "BufferMananger.h"
#include <CL\cl.h>

ObjectManager::ObjectManager(BufferManager& bufferMgr)
	:m_bufferMgr(bufferMgr),
	ta(NULL), tb(NULL), tc(NULL), tri_color(NULL), t_kr(NULL), t_kt(NULL), t_roughness(NULL), t_isChecker(NULL), 
	center(NULL), r(NULL), sprColor(NULL), s_kr(NULL), s_kt(NULL), s_roughness(NULL), s_isChecker(NULL)
{
}
ObjectManager::ObjectManager(const ObjectManager& copy)
	: m_bufferMgr(copy.m_bufferMgr),
	ta(copy.ta), tb(copy.tb), tc(copy.tc), tri_color(copy.tri_color), 
	t_kr(copy.t_kr), t_kt(copy.t_kt), t_roughness(copy.t_roughness), t_isChecker(copy.t_isChecker),
	center(copy.center), r(copy.r), sprColor(copy.sprColor), 
	s_kr(copy.s_kr), s_kt(copy.s_kt), s_roughness(copy.s_roughness), s_isChecker(copy.s_isChecker)
{}

ObjectManager::~ObjectManager(){}

ObjectManager& ObjectManager::operator=(const ObjectManager& copy)
{ 
	return *this; 
}


void ObjectManager::setTriangles(std::vector<Triangle> triangles)
{
	m_triangles = triangles;
}


void ObjectManager::addTriangle(Triangle triangle)
{
	m_triangles.push_back(triangle);
	m_positions.push_back(triangle.boundingBox().position);
}

int ObjectManager::triCount(){ return m_triangles.size(); }

void ObjectManager::addSphere(Sphere sphere)
{
	m_spheres.push_back(sphere);
	m_positions.push_back(sphere.center);
}

int ObjectManager::sprCount(){ return m_spheres.size(); }

std::vector<Triangle> ObjectManager::triangles(){ return m_triangles; }
std::vector<Sphere> ObjectManager::spheres(){ return m_spheres; }
std::vector<cl_float3> ObjectManager::positions(){ return m_positions; }

int ObjectManager::triangleIndex(Triangle triangle){
	return std::find(m_triangles.begin(), m_triangles.end(), triangle) - m_triangles.begin();
}

int ObjectManager::sphereIndex(Sphere sphere){
	return std::find(m_spheres.begin(), m_spheres.end(), sphere) - m_spheres.begin();
}

void ObjectManager::createObjectBuffers()
{
	int triCount = m_triangles.size();
	ta = (cl_float3*)malloc(sizeof(cl_float3) * triCount);
	tb = (cl_float3*)malloc(sizeof(cl_float3) * triCount);
	tc = (cl_float3*)malloc(sizeof(cl_float3) * triCount);
	tri_color = (cl_float3*)malloc(sizeof(cl_float3) * triCount);
	t_kr = (cl_float*)malloc(sizeof(cl_float) * triCount);
	t_kt = (cl_float*)malloc(sizeof(cl_float) * triCount);
	t_roughness = (cl_float*)malloc(sizeof(cl_float) * triCount);
	t_isChecker = (cl_bool*)malloc(sizeof(cl_bool) * triCount);

	for (int t = 0; t < triCount; ++t)
	{
		ta[t] = m_triangles[t].A;
		tb[t] = m_triangles[t].B;
		tc[t] = m_triangles[t].C;
		tri_color[t] = m_triangles[t].color;
		t_kr[t] = m_triangles[t].kr;
		t_kt[t] = m_triangles[t].kt;
		t_roughness[t] = m_triangles[t].roughness;
		t_isChecker[t] = m_triangles[t].isChecker;
	}

	int sprCount = m_spheres.size();
	center = (cl_float3*)malloc(sizeof(cl_float3) * sprCount);
	r = (cl_float*)malloc(sizeof(cl_float) * sprCount);
	sprColor = (cl_float3*)malloc(sizeof(cl_float3) * sprCount);
	s_kr = (cl_float*)malloc(sizeof(cl_float) * sprCount);
	s_kt = (cl_float*)malloc(sizeof(cl_float) * sprCount);
	s_roughness = (cl_float*)malloc(sizeof(cl_float) * sprCount);
	s_isChecker = (cl_bool*)malloc(sizeof(cl_bool) * sprCount);

	for (int s = 0; s < sprCount; ++s)
	{
		center[s] = m_spheres[s].center;
		r[s] = m_spheres[s].radius;
		sprColor[s] = m_spheres[s].color;
		s_kr[s] = m_spheres[s].kr;
		s_kt[s] = m_spheres[s].kt;
		s_roughness[s] = m_spheres[s].roughness;
		s_isChecker[s] = m_spheres[s].isChecker;
	}

	m_bufferMgr.createBuffer("ta", sizeof(cl_float3) * triCount, ta);
	m_bufferMgr.createBuffer("tb", sizeof(cl_float3) * triCount, tb);
	m_bufferMgr.createBuffer("tc", sizeof(cl_float3) * triCount, tc);
	m_bufferMgr.createBuffer("triColor", sizeof(cl_float3) * triCount, tri_color);
	m_bufferMgr.createBuffer("t_kr", sizeof(cl_float) * triCount, t_kr);
	m_bufferMgr.createBuffer("t_kt", sizeof(cl_float) * triCount, t_kt);

	m_bufferMgr.createBuffer("center", sizeof(cl_float3) * sprCount, center);
	m_bufferMgr.createBuffer("r", sizeof(cl_float) * sprCount, r);
	m_bufferMgr.createBuffer("sprColor", sizeof(cl_float3) * sprCount, sprColor);
	m_bufferMgr.createBuffer("s_kr", sizeof(cl_float) * sprCount, s_kr);
	m_bufferMgr.createBuffer("s_kt", sizeof(cl_float) * sprCount, s_kt);

}