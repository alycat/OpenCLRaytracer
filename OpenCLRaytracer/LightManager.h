#ifndef _LIGHTMANAGER_H
#define _LIGHTMANAGER_H

#include "Light.h"
#include <vector>

class BufferManager;

class LightManager{
public:
	LightManager(BufferManager& bufferMgr);
	LightManager(const LightManager& copy);
	~LightManager();
	LightManager& operator=(const LightManager& copy);

	int lightCount();
	int totalPhotons();
	void addLight(Light light);
	void createLightBuffers();
protected:
	std::vector<Light> m_light_list;
	BufferManager& m_bufferMgr;
	cl_float3 * lightPos;
	cl_float3 * lightColor;
	cl_int * numPhotons;
	cl_float3 * photon_pow;
	cl_float3 * photon_pos;
private:
};
#endif