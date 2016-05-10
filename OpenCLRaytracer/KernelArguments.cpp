#include "stdafx.h"
#include "KernelArguments.h"

KernelArguments::KernelArguments()
{
	ctxMgr = ContextManager::instance();
}

void KernelArguments::addArgument(void* value, ArgumentType dataType, bool readOnly)
{
	int size = 0; 
	switch (dataType)
	{
	case FLOAT: size = sizeof(cl_float);
		break;
	case FLOAT2: size = sizeof(cl_float2);
		break;
	case FLOAT3: size = sizeof(cl_float3);
		break;
	case FLOAT4: size = sizeof(cl_float4);
		break;
	case INT: size = sizeof(cl_int);
		break;
	case INT2: size = sizeof(cl_int2);
		break;
	case INT3: size = sizeof(cl_int3);
		break;
	case UINT: size = sizeof(cl_uint);
		break;
	}
	m_args.push_back({ value, 0, size, 0, readOnly ? CL_FALSE : CL_TRUE, false });
}

void KernelArguments::addBuffer(void* value, ArgumentType dataType, int elements, cl_map_flags mem_flags)
{
	cl_context context = ctxMgr->context();
	cl_int status = 0;
	int size = 0;
	switch (dataType)
	{
	case FLOAT: size = sizeof(cl_float);
		break;
	case FLOAT2: size = sizeof(cl_float2);
		break;
	case FLOAT3: size = sizeof(cl_float3);
		break;
	case FLOAT4: size = sizeof(cl_float4);
		break;
	case INT: size = sizeof(cl_int);
		break;
	case INT2: size = sizeof(cl_int2);
		break;
	case INT3: size = sizeof(cl_int3);
		break;
	case UINT: size = sizeof(cl_uint);
		break;
	}
	cl_mem buffer = clCreateBuffer(context, mem_flags, size * elements, NULL, &status);
	cl_bool canWrite = (mem_flags == CL_MEM_WRITE_ONLY || mem_flags == CL_MEM_READ_WRITE);
	m_args.push_back({ value, buffer, sizeof(cl_mem), size * elements, canWrite, true});
}

void KernelArguments::enqueueWriteBuffer()
{
	cl_command_queue cmdQueue = ctxMgr->cmdQueue();
	cl_int status = 0;
	for (int i = 0; i < m_args.size(); ++i)
	{
		if (m_args[i].isBuffer)
		{
			if (!m_args[i].canWrite)
				status = clEnqueueWriteBuffer(cmdQueue, m_args[i].buffer, CL_FALSE, 0, m_args[i].buffersize, m_args[i].data, 0, NULL, NULL);
		}
	}
}

void KernelArguments::enqueueReadBuffer()
{
	cl_command_queue cmdQueue = ctxMgr->cmdQueue();
	cl_int status = 0;
	for (int i = 0; i < m_args.size(); ++i)
	{
		if (m_args[i].isBuffer)
		{
			if (m_args[i].canWrite)
			{
				clEnqueueReadBuffer(cmdQueue, m_args[i].buffer, CL_TRUE, 0, m_args[i].buffersize, m_args[i].data, 0, NULL, NULL);
			}
		}
	}
}

void KernelArguments::setKernelArguments(cl_kernel kernel)
{
	cl_int status = 0;
	for (int i = 0; i < m_args.size(); ++i)
	{
		status |= clSetKernelArg(kernel, i, m_args[i].size, m_args[i].isBuffer ? &m_args[i].buffer : m_args[i].data);
	}
}