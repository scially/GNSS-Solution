#pragma once
#include <string>
#include "DataStruct.h"
class ReadNFile
{
public:
	ReadNFile(std::string filename);
	explicit ReadNFile();
	ReadNFile(const ReadNFile& nfile);
	~ReadNFile();
	NFileHeader ReadNHeader();
	vector<NFileRecord> ReadNRecord();
private:
	std::string _filename;
};

