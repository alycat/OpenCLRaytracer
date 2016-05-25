
#include "stdafx.h"
#include "Sphere.h"

Sphere::Sphere() : Object()
{

}

Sphere::Sphere(cl_float3 center, cl_float radius, cl_float3 color, cl_float kr, cl_float kt, cl_float roughness, cl_bool isChecker)
	: m_center(center), m_radius(radius), Object(color, kr, kt, roughness, isChecker)
{

}

Sphere::Sphere(const Sphere& copy)
	: m_center(copy.m_center), m_radius(copy.m_radius)//, Object(copy)
{
	m_color = copy.m_color;
	m_kr = copy.m_kr;
	m_kt = copy.m_kt;
	m_roughness = copy.m_roughness;
	m_isChecker = copy.m_isChecker;
}

Sphere& Sphere::operator=(const Sphere& copy)
{
	m_center = copy.m_center;
	m_radius = copy.m_radius;
	m_color = copy.m_color;
	m_kr = copy.m_kr;
	m_kt = copy.m_kt;
	m_roughness = copy.m_roughness;
	m_isChecker = copy.m_isChecker;
	return *this;
}

Sphere::~Sphere()
{

}


cl_float3 Sphere::getCenter(){ return m_center; }
void Sphere::setCenter(cl_float3 center){ m_center = center; }

cl_float Sphere::getRadius(){ return m_radius; }
void Sphere::setRadius(cl_float radius){ m_radius = radius; }