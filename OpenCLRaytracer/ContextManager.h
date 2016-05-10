#ifndef _CONTEXTMANAGER_H
#define _CONTEXTMANAGER_H

#include <CL\cl.h>

class ContextManager
{
public:
	static ContextManager* instance();
	cl_context context();
	cl_command_queue cmdQueue();
	cl_uint numDevices();
	cl_device_id* devices();
protected:
	ContextManager();
	static ContextManager* m_instance;
	static cl_context m_context;
	static cl_command_queue m_cmdQueue;
	static cl_uint m_num_devices;
	static cl_device_id *m_devices;
private:
};
#endif