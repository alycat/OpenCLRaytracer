#ifndef _INIREADER_H
#define _INIREADER_H

#include <vector>
#include <CL\cl.h>

class INIReader{
public:
	INIReader();
	INIReader(char* filename);
	INIReader(const INIReader& copy);
	~INIReader();
	INIReader& operator=(const INIReader& copy);

	cl_float3 readFloat3(char* section, char* key);
	char* readString(char* section, char* key);
	int readInt(char* section, char* key);
	float readFloat(char* section, char* key);
	bool readBool(char* section, char* key);
	std::vector<char*> getSectionData();

	void setFilename(char* filename);
protected:
	char* m_filename;
private:
};
#endif