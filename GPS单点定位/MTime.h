#pragma once
#include <iostream>
#define WeekSecond 604800 //一周604800秒

#define  MAXLEAPS 64
#define  NOWYEAR  2016
static int leaps[MAXLEAPS + 1][7] =
{ /* leap seconds (y,m,d,h,m,s,utc-gpst) */
	{ 2016, 1, 1, 0, 0, 0, -17 },
	{ 2015, 7, 1, 0, 0, 0, -17 },
	{ 2012, 7, 1, 0, 0, 0, -16 },
	{ 2009, 1, 1, 0, 0, 0, -15 },
	{ 2006, 1, 1, 0, 0, 0, -14 },
	{ 1999, 1, 1, 0, 0, 0, -13 },
	{ 1997, 7, 1, 0, 0, 0, -12 },
	{ 1996, 1, 1, 0, 0, 0, -11 },
	{ 1994, 7, 1, 0, 0, 0, -10 },
	{ 1993, 7, 1, 0, 0, 0, -9 },
	{ 1992, 7, 1, 0, 0, 0, -8 },
	{ 1991, 1, 1, 0, 0, 0, -7 },
	{ 1990, 1, 1, 0, 0, 0, -6 },
	{ 1988, 1, 1, 0, 0, 0, -5 },
	{ 1985, 7, 1, 0, 0, 0, -4 },
	{ 1983, 7, 1, 0, 0, 0, -3 },
	{ 1982, 7, 1, 0, 0, 0, -2 },
	{ 1981, 7, 1, 0, 0, 0, -1 },
	{ 0 }
};

struct GTime  //GPS时
{
	int week;
	double seconds;
	double operator- (const GTime& Gtime);
};
struct Time{  //正常时UTC
	int year;
	int month;
	int day;
	int hour;
	int minute;
	double second;
	Time(int Y = 0, int M = 0, int D = 0, int H = 0, int MIN = 0, double S = 0)
		:year(Y), month(M), day(D), hour(H), minute(MIN), second(S)	{}
	Time operator+(double seconds);
	double operator-(const Time&);
	Time operator-(double seconds);
	friend std::ostream& operator <<(std::ostream& os,const Time& time);
	GTime UTC2GTime();
};

extern GTime Sec2GTime(double seconds);
