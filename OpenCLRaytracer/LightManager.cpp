#include "stdafx.h"
#include "BufferMananger.h"
#include "LightManager.h"

LightManager::LightManager(BufferManager& bufferMgr)
	:m_bufferMgr(bufferMgr), lightPos(NULL), lightColor(NULL), numPhotons(NULL), photon_pos(NULL), photon_pow(NULL)
{}

LightManager::LightManager(const LightManager& copy)
	: m_bufferMgr(copy.m_bufferMgr), lightPos(copy.lightPos), lightColor(copy.lightColor), numPhotons(copy.numPhotons),
	m_light_list(copy.m_light_list), photon_pos(copy.photon_pos), photon_pow(copy.photon_pow)
{}

LightManager::~LightManager(){}

LightManager& LightManager::operator=(const LightManager& copy)
{
	m_bufferMgr = copy.m_bufferMgr;
	lightPos = copy.lightPos;
	lightColor = copy.lightColor;
	numPhotons = copy.numPhotons;
	m_light_list = copy.m_light_list;
	photon_pos = copy.photon_pos;
	photon_pow = copy.photon_pow;
	return *this;
}

void LightManager::addLight(Light light)
{
	m_light_list.push_back(light);
}

int LightManager::lightCount()
{
	return m_light_list.size();
}

int LightManager::totalPhotons()
{
	int total = 0;
	for each(Light light in m_light_list)
		total += light.numPhotons;
	return total;
}

void LightManager::createLightBuffers()
{
	int lightCount = m_light_list.size();
	lightPos = (cl_float3*)malloc(sizeof(cl_float3) * lightCount);
	lightColor = (cl_float3*)malloc(sizeof(cl_float3) * lightCount);
	numPhotons = (cl_int*)malloc(sizeof(cl_int) * lightCount);

	int total_photons = 0;
	for (int l = 0; l < lightCount; ++l)
	{
		lightPos[l] = m_light_list[l].position;
		lightColor[l] = m_light_list[l].color;
		numPhotons[l] = m_light_list[l].numPhotons;
		total_photons += m_light_list[l].numPhotons;
	}
	photon_pos = (cl_float3*)malloc(sizeof(cl_float3) * total_photons);
	photon_pow = (cl_float3*)malloc(sizeof(cl_float3) * total_photons);

	m_bufferMgr.createBuffer("lightPos", sizeof(cl_float3) * lightCount, lightPos);
	m_bufferMgr.createBuffer("lightColor", sizeof(cl_float3) * lightCount, lightColor);
	m_bufferMgr.createBuffer("numPhotons", sizeof(cl_int) * lightCount, numPhotons);

	m_bufferMgr.createBuffer("photonPos", sizeof(cl_float3) * total_photons, photon_pos, CL_MEM_READ_WRITE);
	m_bufferMgr.createBuffer("photonPow", sizeof(cl_float3) * total_photons, photon_pow, CL_MEM_READ_WRITE);
}