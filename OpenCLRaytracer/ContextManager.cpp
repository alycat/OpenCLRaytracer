#include "stdafx.h"
#include "ContextManager.h"

ContextManager* ContextManager::m_instance = nullptr;
cl_context ContextManager::m_context = NULL;
cl_command_queue ContextManager::m_cmdQueue = NULL;
cl_uint ContextManager::m_num_devices = 0;
cl_device_id* ContextManager::m_devices = NULL;

ContextManager::ContextManager()
{
	cl_int status;
	cl_uint num_platforms = 0;
	cl_platform_id *platforms = NULL;

	status = clGetPlatformIDs(0, NULL, &num_platforms);
	platforms = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id));
	status = clGetPlatformIDs(num_platforms, platforms, NULL);

	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &m_num_devices);
	m_devices = (cl_device_id*)malloc(m_num_devices * sizeof(cl_device_id));
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, m_num_devices, m_devices, NULL);

	m_context = clCreateContext(NULL, m_num_devices, m_devices, NULL, NULL, &status);

	m_cmdQueue = clCreateCommandQueueWithProperties(m_context, m_devices[1], nullptr, &status);
}

ContextManager* ContextManager::instance()
{
	if (m_instance == nullptr)
	{
		m_instance = new ContextManager();
	}
	return m_instance;
}

cl_context ContextManager::context(){ return m_context; }

cl_command_queue ContextManager::cmdQueue(){ return m_cmdQueue; }

cl_uint ContextManager::numDevices(){ return m_num_devices; }

cl_device_id* ContextManager::devices(){ return m_devices; }