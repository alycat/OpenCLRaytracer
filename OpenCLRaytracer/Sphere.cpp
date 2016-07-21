
#include "stdafx.h"
#include "Sphere.h"
#include "Global.h"

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

bool Sphere::operator==(const Sphere& other) const
{
	if (m_color != other.m_color)
		return false;
	if (m_center != other.m_center)
		return false;
	if (m_radius != other.m_radius)
		return false;
	if (m_kr != other.m_kr)
		return false;
	if (m_kt != other.m_kt)
		return false;
	if (m_roughness != other.m_roughness)
		return false;
	if (m_isChecker != other.m_isChecker)
		return false;
	return true;
}

cl_float3 Sphere::getCenter(){ return m_center; }
void Sphere::setCenter(cl_float3 center){ m_center = center; }

cl_float Sphere::getRadius(){ return m_radius; }
void Sphere::setRadius(cl_float radius){ m_radius = radius; }

BoundingBox Sphere::boundingBox(){
	cl_float3 dimensions = { m_radius * 2.0f, m_radius * 2.0f, m_radius * 2.0f };
	return BoundingBox{ m_center, dimensions };
}