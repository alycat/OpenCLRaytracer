#include "stdafx.h"
#include "Light.h"

Light::Light()
	:m_numPhotons(0)
{}

Light::Light(cl_float3 position, cl_float3 color, cl_int numPhotons)
	: m_position(position), m_color(color), m_numPhotons(numPhotons)
{}

Light::Light(const Light& copy)
	:m_position(copy.m_position), m_color(copy.m_color), m_numPhotons(copy.m_numPhotons)
{}

Light::~Light(){}

Light& Light::operator=(const Light& copy)
{
	m_position = copy.m_position;
	m_color = copy.m_color;
	m_numPhotons = copy.m_numPhotons;
	return *this;
}

cl_float3 Light::getPosition(){ return m_position; }
void Light::setPosition(cl_float3 position){ m_position = position; }

cl_float3 Light::getColor(){ return m_color; }
void Light::setColor(cl_float3 color){ m_color = color; }

cl_int Light::getNumPhotons(){ return m_numPhotons; }
void Light::setNumPhotons(cl_int numPhotons){ m_numPhotons = numPhotons; }