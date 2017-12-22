#ifndef READOFILE_H_
#define READOFILE_H_

#include "DataStruct.h"
#include "EString.h"
#include <fstream>
#include <string>
class ReadOFile
{
public:
	ReadOFile(std::string ofile);
	ReadOFile(const ReadOFile& ofile);
	explicit ReadOFile();
	~ReadOFile();
	OHeader ReadHeader();
	std::vector<OEpochData> ReadData();
private:
	std::string _ofile;
};

#endif