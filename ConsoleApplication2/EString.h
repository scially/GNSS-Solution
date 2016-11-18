#ifndef ESTRING_H_
#define ESTRING_H_

#include <string>
using std::string;
namespace EString
{
	string TrimLeft(const string& str);
	string TrimRight(const string& str);
	string Trim(const string& str);
	//有些编译器中不识别D科学计数法
	string D2E(const string& str);
}

#endif 

