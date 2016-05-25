#ifndef _PROGRAMMANAGER_H
#define _PROGRAMMANAGER_H

#include <map>
#include <CL\cl.h>
#include <string>
#include "ContextManager.h"

class ProgramManager{
public:
	ProgramManager();
	ProgramManager(const ProgramManager& copy);
	~ProgramManager();
	ProgramManager& operator=(const ProgramManager& copy);

	cl_program program(std::string program_name);
	cl_int createProgram(std::string program_name, const char* filename);
protected:
	std::map<std::string, cl_program> m_programs;
	ContextManager* m_ctxtMgr;
private:
};

#endif