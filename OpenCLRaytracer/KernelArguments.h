#ifndef _KERNELARGUMENTS_H
#define _KERNELARGUMENTS_H

#include <CL\cl.h>
#include <vector>
#include "ContextManager.h"

#include <Windows.h>

class KernelArguments
{
public:
	enum ArgumentType{
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		INT,
		INT2,
		INT3,
		UINT
	};

	KernelArguments();
	void addArgument(void* value, ArgumentType dataType, bool readOnly = true);
	void addBuffer(void* value, ArgumentType dataType, int elements, cl_mem_flags mem_flags);
	void enqueueWriteBuffer();
	void enqueueReadBuffer();
	void setKernelArguments(cl_kernel kernel);
protected:
	ContextManager* ctxMgr;
	struct Argument{
		void* data;
		cl_mem buffer;
		int size;
		int buffersize;
		cl_bool canWrite;
		bool isBuffer;
	};

	std::vector<Argument> m_args;
private:
};

#endif