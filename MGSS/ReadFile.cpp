#include "ReadFile.h"


ReadFile::ReadFile(std::string ofilename, std::string nfilename)
{
	_ofile = ReadOFile(ofilename);
	_nfile = ReadNFile(nfilename);
}

ReadFile::~ReadFile()
{
}
