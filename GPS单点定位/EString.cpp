
#include "EString.h"

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
std::ofstream& EString::OutPut(std::ofstream &ofs, const OEpochData& oData, double Rr,const Point* result)
{
	if (result == nullptr)
	{
		ofs << "该历元数据有误：";
		ofs << std::setw(4) << oData.gtime.year << std::setw(3) << oData.gtime.month << std::setw(3) << oData.gtime.day
			<< std::setw(3) << oData.gtime.hour << std::setw(3) << oData.gtime.minute << std::setw(11) << std::fixed << std::setprecision(7)
			<< oData.gtime.second << std::endl;
	}
	else
	{
		ofs << std::setw(4) << oData.gtime.year << std::setw(3) << oData.gtime.month << std::setw(3) << oData.gtime.day
			<< std::setw(3) << oData.gtime.hour << std::setw(3) << oData.gtime.minute << std::setw(11) << std::fixed << std::setprecision(7)
			<< oData.gtime.second;
		ofs << std::setw(4) << oData.satsums;
		ofs << std::setw(16) << std::fixed << std::setprecision(4) << result->x
			<< std::setw(15) << std::fixed << std::setprecision(4) << result->y
			<< std::setw(15) << std::fixed << std::setprecision(4) << result->z
			<< std::setw(16) << std::fixed << std::setprecision(10) << Rr << std::endl;
	}
	return ofs;
}
