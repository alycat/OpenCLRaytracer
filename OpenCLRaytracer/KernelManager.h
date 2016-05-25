#ifndef _KERNELMANAGER_H
#define _KERNELMANAGER_H

#include <CL\cl.h>
#include <map>
#include <string>

class KernelManager{
public:
	KernelManager();
	KernelManager(const KernelManager &copy);
	~KernelManager();
	KernelManager& operator=(const KernelManager &copy);

	cl_int createKernel(cl_program program, const char* function, std::string kernel);
	cl_kernel kernel(std::string kernel);
	cl_int addArgument(std::string kernel, size_t size, const void* data);
protected:
	std::map<std::string, cl_kernel> m_kernels;
	std::map<std::string, cl_uint> m_args;
private:
};
#endif