#include "stdafx.h"
#include "ProgramManager.h"
#include <iostream>
#include <fstream>
#include "Parser.h"

ProgramManager::ProgramManager()
{
	m_ctxtMgr = ContextManager::instance();
}

ProgramManager::~ProgramManager(){}
ProgramManager::ProgramManager(const ProgramManager& copy){}
ProgramManager& ProgramManager::operator=(const ProgramManager& copy){ return *this; }

cl_program ProgramManager::program(std::string program_name)
{
	auto it = m_programs.find(program_name);
	if (it != m_programs.end())
		return it->second;
	return NULL;
}

cl_int ProgramManager::createProgram(std::string program_name, const char* filename)
{
	const char* programSource = Parser::readFile(filename);
	cl_int status;
	cl_program new_program = clCreateProgramWithSource(m_ctxtMgr->context(), 1, (const char**)&programSource, NULL, &status);
	status = clBuildProgram(new_program, m_ctxtMgr->numDevices(), m_ctxtMgr->devices(), NULL, NULL, NULL);
	m_programs[program_name] = new_program;
	return status;
}