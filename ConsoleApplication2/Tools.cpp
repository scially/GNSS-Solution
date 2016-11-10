#include "Tools.h"
#include <istream>

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