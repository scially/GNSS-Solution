#include "MTime.h"
#include "Tools.h"
//////////////////////////////////////////////////////////////////////////GTime
double  GTime::operator- (const GTime& Gtime)
{
	//顾及一周（604800）开始或结束
	double TransTime = (week - Gtime.week) * WeekSecond + (seconds - Gtime.seconds);
	if (TransTime > WeekSecond / 2.0)   TransTime -= WeekSecond;
	if (TransTime < -WeekSecond / 2.0)  TransTime += WeekSecond;
	return TransTime;
}

//GPS秒转为GPS周和秒
GTime Sec2GTime(double seconds)
{
	GTime Gtime;
	Gtime.week = int(seconds) / WeekSecond;
	Gtime.seconds = seconds - Gtime.week * WeekSecond;
	return Gtime;
}
//////////////////////////////////////////////////////////////////////////Time
Time Time::operator+(double seconds)
{
	Time addTime(year, month, day, hour, minute, second + seconds);
	while (addTime.second >= 60)
	{
		addTime.second -= 60;
		addTime.minute += 1;
		while (addTime.minute >= 60)
		{
			addTime.hour += 1;
			while (addTime.hour >= 24)
			{
				addTime.day += 1;
			}
		}
	}
	return addTime;
}
double Time::operator-(const Time& time)
{
	return (UTC2JD(*this) - UTC2JD(time)) * 24 * 3600 + this->second - time.second;
}
Time Time::operator-(double seconds)
{
	Time insTime(year, month, day, hour, minute, second - seconds);
	while (insTime.second < 0)
	{
		insTime.second += 60;
		insTime.minute -= 1;
		while (insTime.minute < 0)
		{
			insTime.minute += 60;
			insTime.hour -= 1;
			while (insTime.hour < 0)
			{
				insTime.hour += 24;
				insTime.day -= 1;
			}
		}
	}
	return insTime;
}
std::ostream& operator <<(std::ostream& os, const Time& time)
{
	os << std::setw(4) << time.year << std::setw(3) << time.month << std::setw(3) << time.day
		<< std::setw(3) << time.hour << std::setw(3) << time.minute << std::setw(11) << std::fixed << std::setprecision(7)
		<< time.second;
	return os;
}
GTime Time::UTC2GTime()
{
	GTime gt;
	//跳秒
	/*Time leapTime(this->year, leaps[this->year - 2016][1], leaps[this->year - 2016][2], leaps[this->year - 2016][3], leaps[this->year - 2016][4], leaps[this->year - 2016][5]);
	int leapseconds = (*this - leapTime) > 0 ? leaps[this->year - 2016][6] : 0;*/
	double JD = UTC2JD(*this);
	gt.week = int((JD - 2444244.5) / 7.0);
	gt.seconds = (JD - 2444244.5 - gt.week * 7.0) * 86400.0;
	return gt;
}