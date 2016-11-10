#include "TString.h"
using std::string;
string EString::TrimRight(const string& str)
{
	string strcopy = str;
	return strcopy.erase(strcopy.find_last_not_of(' ') + 1, strcopy.size() - strcopy.find_last_not_of(' ') - 1);
}
string EString::TrimLeft(const string& str)
{
	string strcopy = str;
	return strcopy.erase(0, strcopy.find_first_not_of(' '));
}
string EString::Trim(const string& str)
{
	return TrimRight(TrimLeft(str));
}
