#pragma once
#include <ostream>
#include "DataStruct.h"
extern double UTC2JD(Time time);
extern GTime UTC2GTime(Time time);
extern bool CheckDatas(OEpochData &oDatas);
extern int MinOfNums(const vector<double>& nums);
extern BLHPoint PointToBLHPoint(const Point& point);
extern bool XYZToENU(const Point& station, const SatPoint& satpoint, ENUPoint &ENU);
extern double Trop(const Point &station, const SatPoint &satpoint);
extern bool Elevation(const Point &station, const SatPoint &satpoint, double elvation);