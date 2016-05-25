#ifndef _PARSER_H
#define _PARSER_H

#include <iostream>
#include <fstream>

class Parser{
public:
	Parser(){}

	static const char* readFile(const char* filename)
	{
		std::ifstream sourceFilename(filename);
		std::string sourceFile(std::istreambuf_iterator<char>(sourceFilename),
			(std::istreambuf_iterator<char>()));

		int len = sourceFile.size();
		char* charArr = new char[len + 1];
		std::memcpy(charArr, sourceFile.c_str(), sizeof(char) * len);
		charArr[len] = '\0';
		return charArr;
	}
};

#endif