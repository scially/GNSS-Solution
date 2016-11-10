#ifndef READOFILE_H_
#define READOFILE_H_

#include "DataStruct.h"
#include "TString.h"
#include <fstream>
#include <string>
class ReadOFile
{
public:
	ReadOFile(std::string ofile);
	//不要使用默认构造函数！
	ReadOFile();
	~ReadOFile();
	OHeader ReadHeader();
	std::vector<OEpochData> ReadData();
private:
	std::string _ofile;
};

#endif