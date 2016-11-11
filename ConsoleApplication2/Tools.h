#pragma once
#include <ostream>
#include "DataStruct.h"
extern double UTC2JD(Time time);
extern GTime UTC2GTime(Time time);
extern void CheckDatas(Point Station, SatPoint satpoint, OEpochData &oData);
extern int MinOfNums(const vector<double>& nums);
extern BLHPoint PointToBLHPoint(const Point& point);