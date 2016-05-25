#include "stdafx.h"
#include "BufferMananger.h"

BufferManager::BufferManager()
{
	m_ctxtMgr = ContextManager::instance();
}
BufferManager::~BufferManager(){}
BufferManager::BufferManager(const BufferManager& copy){}
BufferManager& BufferManager::operator=(const BufferManager& copy){ return *this; }


cl_int BufferManager::createBuffer(std::string buffer_name, size_t size, const void* data, cl_mem_flags mem_flags)
{
	cl_int status;
	cl_mem buffer = clCreateBuffer(m_ctxtMgr->context(), mem_flags, size, NULL, &status);
	m_buffers[buffer_name] = { buffer, size, data, mem_flags };
	return status;
}

cl_mem BufferManager::buffer(std::string buffer_name)
{
	auto it = m_buffers.find(buffer_name);
	if (it != m_buffers.end())
		return it->second.buffer;
	return NULL;
}

const void* BufferManager::data(std::string buffer_name)
{
	auto it = m_buffers.find(buffer_name);
	if (it != m_buffers.end())
		return it->second.data;
	return NULL;
}

cl_int BufferManager::enqueueWriteBuffer()
{
	cl_int status;
	for (const auto& pair : m_buffers)
	{
		if (pair.second.mem_flags == CL_MEM_READ_ONLY || pair.second.mem_flags == CL_MEM_READ_WRITE)
		{
			status = clEnqueueWriteBuffer(
				m_ctxtMgr->cmdQueue(),
				pair.second.buffer,
				CL_FALSE,
				0,
				pair.second.size,
				pair.second.data,
				0,
				NULL,
				NULL);
		}
	}
	return status;
}