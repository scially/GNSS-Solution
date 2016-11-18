#ifndef ESTRING_H_
#define ESTRING_H_

#include <string>
#include <fstream>
#include <iomanip>
#include "DataStruct.h"
using std::string;
namespace EString
{
	string TrimLeft(const string& str);
	string TrimRight(const string& str);
	string Trim(const string& str);
	std::ofstream& OutPut(std::ofstream &ofs, const OEpochData& oData, double Rr, const Point* result = nullptr);
}

#endif 

