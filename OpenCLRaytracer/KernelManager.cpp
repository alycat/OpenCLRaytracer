#include "stdafx.h"
#include "KernelManager.h"

using namespace std;

KernelManager::KernelManager(){}

KernelManager::KernelManager(const KernelManager& copy)
{}

KernelManager::~KernelManager(){}

KernelManager& KernelManager::operator=(const KernelManager& copy)
{
	return *this;
}

cl_int KernelManager::createKernel(cl_program program, const char* function, std::string kernel)
{
	cl_int status;
	cl_kernel new_kernel = clCreateKernel(program, function, &status);
	m_kernels[kernel] = new_kernel;
	m_args[kernel] = 0;
	return status;
}

cl_kernel KernelManager::kernel(std::string kernel)
{
	auto it = m_kernels.find(kernel);
	if (it != m_kernels.end())
		return it->second;
	return NULL;
}


cl_int KernelManager::addArgument(std::string kernel, size_t size, const void* data)
{
	cl_kernel curr_kernel = this->kernel(kernel);
	cl_uint argument = 0;

	auto it = m_args.find(kernel);
	if (it != m_args.end())
		argument = it->second;

	m_args[kernel] = argument + 1;
	return clSetKernelArg(curr_kernel, argument, size, data);

}