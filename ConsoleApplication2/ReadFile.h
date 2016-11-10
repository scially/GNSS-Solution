#pragma once
#include "ReadNFile.h"
#include "ReadOFile.h"
#include <string>
class ReadFile
{
public:
	ReadFile(std::string ofilename,std::string nfilename);
	~ReadFile();
	ReadOFile _ofile;
	ReadNFile _nfile;
};

