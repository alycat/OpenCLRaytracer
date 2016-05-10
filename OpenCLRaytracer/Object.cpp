
#include "stdafx.h"
#include "Object.h"

Object::Object()
	:m_kt(0), m_kr(0), m_roughness(0.3), m_isChecker(false)
{

}

Object::Object(cl_float kt, cl_float kr, cl_float roughness, cl_bool isChecker)
	: m_kt(kt), m_kr(kr), m_roughness(roughness), m_isChecker(isChecker)
{

}

Object::Object(const Object& copy)
	: m_kt(copy.m_kt), m_kr(copy.m_kr), m_roughness(copy.m_roughness), m_isChecker(copy.m_isChecker)
{

}

Object& Object::operator=(const Object& copy)
{
	m_kt = copy.m_kt;
	m_kr = copy.m_kr;
	m_roughness = copy.m_roughness;
	m_isChecker = copy.m_isChecker;
	return *this;
}

Object::~Object()
{

}

cl_float Object::getKt(){ return m_kt; }
void Object::setKt(cl_float kt){ m_kt = kr; }

cl_float Object::getKr(){ return m_kr; }
void Object::setKr(cl_float kr){ m_kr = kr; }

cl_float Object::getRoughness(){ return m_roughness; }
void Object::setRoughness(cl_float roughness){ m_roughness = roughness; }

cl_bool Object::getIsChecker(){ return m_isChecker; }
void Object::setIsChecker(cl_bool isChecker){ m_isChecker = isChecker; }