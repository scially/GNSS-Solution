#ifndef TSTRING_H_
#define TSTRING_H_

#include <string>
using std::string;
namespace EString
{
	string TrimLeft(const string& str);
	string TrimRight(const string& str);
	string Trim(const string& str);
}

#endif 

