#pragma once
#include <string>
#include "DataStruct.h"
class ReadNFile
{
public:
	ReadNFile(std::string filename);
	//不要使用默认构造函数！
	ReadNFile();
	~ReadNFile();
	NFileHeader ReadNHeader();
	vector<NFileRecord> ReadNRecord();
private:
	std::string _filename;
};

