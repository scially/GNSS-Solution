#include "Tools.h"
#include <functional>

double UTC2JD(Time time)
{
	double hour = time.hour + time.minute / 60.0 + time.second / 3600.0;
	return 1721013.5 + 367 * time.year - int(7.0 / 4 * (time.year + (time.month+9) / 12)) + time.day + hour / 24 + (275*time.month / 9);
}
GTime UTC2GTime(Time time)
{
	GTime gt;
	double JD = UTC2JD(time);
	gt.week = int((JD - 2444244.5) / 7.0);
	gt.seconds = (JD - 2444244.5 - gt.week * 7.0) * 86400.0;
	return gt;
}
int MinOfNums(const vector<double>& nums)
{
	double min = nums[0];
	int index = 0;
	//从0开始，至少保证有一个匹配的卫星轨道
	for (vector<double>::size_type i = 0; i < nums.size(); i++)
	{
		if (nums.at(i) < min)
		{
			index = i;
			min = nums.at(i);
		}
	}
	return index;
}
BLHPoint PointToBLHPoint(const Point& point)
{
	BLHPoint BLH;
	double B = 0,B1=1;
	//std::function<double(double)> 
	//若是不支持c++11, 这里需要声明为一个函数
	double e2 = 2 * WGS84f - WGS84f*WGS84f ;
	auto N = [e2](double B){return WGS84A/sqrt(1-e2*sin(B)*sin(B));};
	while (abs(B1 - B) > 1e-8)
	{
		B1 = B;
		B = atan((point.z + N(B)*e2*sin(B)) / sqrt(pow(point.x, 2) + pow(point.y, 2)));
	}
	double L = atan(point.y / point.x);
	double H = sqrt(pow(point.x, 2) + pow(point.y, 2)) / cos(B) - N(B);
	BLH.B = B;
	BLH.H = H;
	BLH.L = L;
	return BLH;
}
void CheckDatas(Point Station, SatPoint satpoint, OEpochData &oData)
{
	//检查观测值是否有效
	for (int i = 0; i < oData.satsums; i++)
	{
		//跳过非GPS卫星
		if (oData.AllTypeDatas[C1][i].PRN.substr(0, 1) != "G")
		{
			i--;
			for (int i = 0; i < 5;i++)
				oData.AllTypeDatas[ObsType(i)].erase(oData.AllTypeDatas[ObsType(i)].begin() + i + 1);
			oData.satsums--;
		}
	}
}