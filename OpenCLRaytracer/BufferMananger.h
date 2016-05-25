#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H

#include <map>
#include <CL\cl.h>
#include "ContextManager.h"

class BufferManager{
public:
	BufferManager();
	BufferManager(const BufferManager& copy);
	~BufferManager();
	BufferManager& operator=(const BufferManager& copy);
	cl_int createBuffer(std::string buffer_name, size_t size, const void* data, cl_mem_flags mem_flags = CL_MEM_READ_ONLY);
	cl_mem buffer(std::string buffer_name);
	const void* data(std::string buffer_name);
	cl_int enqueueWriteBuffer();
protected:
	struct Buffer{
		cl_mem buffer;
		size_t size;
		const void* data;
		cl_mem_flags mem_flags;
	};
	std::map<std::string, Buffer> m_buffers;
	ContextManager* m_ctxtMgr;
private:
};
#endif