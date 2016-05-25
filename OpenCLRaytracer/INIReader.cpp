#include "stdafx.h"
#include "INIReader.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include "Shlwapi.h"

INIReader::INIReader()
	:m_filename("")
{}

INIReader::INIReader(char* filename)
	: m_filename(filename)
{}

INIReader::INIReader(const INIReader& copy)
	: m_filename(copy.m_filename)
{}

INIReader::~INIReader()
{}

INIReader& INIReader::operator=(const INIReader& copy)
{
	m_filename = copy.m_filename;
	return *this;
}
LPCTSTR getCurrentDirectory() {
	DWORD len = 64;
	for (;;) {
		LPTSTR fileName = new TCHAR[len];
		if (len == ::GetCurrentDirectory(NULL, fileName)) {
			delete fileName;
			len *= 2;
		}
		else {
			return fileName;
		}
	}
}




char* INIReader::readString(char* section, char* key)
{

	char* szResult = new char[255];
	memset(szResult, 0x00, 255);
	if (PathFileExists(m_filename)){
		GetPrivateProfileString(
			section,
			key,
			TEXT("Error"),
			szResult,
			255,
			m_filename);
	}
	return szResult;
}


int INIReader::readInt(char* section, char* key)
{
	int result = -1;
	if (PathFileExists(m_filename)){
		result = GetPrivateProfileInt(
			section,
			key,
			-1,
			m_filename);
	}
	return result;
}
float INIReader::readFloat(char* section, char* key)
{
	char* szResult = new char[255];
	memset(szResult, 0x00, 255);
	float result = 0.0f;
	if (PathFileExists(m_filename)){
		GetPrivateProfileString(
			section,
			key,
			"0.0",
			szResult,
			255,
			m_filename);
		result = atof(szResult);
	}
	return result;
}
bool INIReader::readBool(char* section, char* key)
{
	bool result = false;
	char* szResult = new char[255];
	memset(szResult, 0x00, 255);
	if (PathFileExists(m_filename)){
		GetPrivateProfileString(
			section,
			key,
			"false",
			szResult,
			255,
			m_filename);
		result = strcmp(szResult, "True") < strcmp(szResult, "False") ? true : false;
	}
	return result;
}


cl_float3 INIReader::readFloat3(char* section, char* key)
{
	cl_float3 result = { 0, 0, 0 };
	char* szResult = new char[255];
	memset(szResult, 0x00, 255);
	if (PathFileExists(m_filename)){
		GetPrivateProfileString(
			section,
			key,
			"0,0,0",
			szResult,
			255,
			m_filename);
		std::string str(szResult);
		std::stringstream ss(str);
		std::string item;
		cl_float vec[3] = { 0, 0, 0 };
		int i = 0;
		while (std::getline(ss, item, ',') && i < 3)
		{
			vec[i] = atof(item.c_str());
			i++;
		}
		result.x = vec[0], result.y = vec[1], result.z = vec[2];
	}
	return result;
}

std::vector<char*> INIReader::getSectionData()
{
	std::vector<char*> stringlist;
	char* returnBuffer = new char[255];
	memset(returnBuffer, 0x00, 255);
	if (PathFileExists(m_filename))
	{
		GetPrivateProfileSectionNames(returnBuffer, 255, m_filename);
		char* pNextSection = returnBuffer;
		while (*pNextSection != 0x00)
		{
			//if (*pNextSection != 0x00)
			stringlist.push_back(pNextSection);
			pNextSection = pNextSection + strlen(pNextSection) + 1;
		}
	}
	return stringlist;
}

void INIReader::setFilename(char* filename)
{
	m_filename = filename;
}