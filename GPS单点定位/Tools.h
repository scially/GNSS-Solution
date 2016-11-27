#pragma once
#include <ostream>
#include <iomanip>
#include "DataStruct.h"

extern double UTC2JD(const Time &time);

//星历文件检测
extern bool CheckDatas(OEpochData &oDatas);
extern int MinOfNums(const vector<double>& nums);
//坐标系转换
extern BLHPoint PointToBLHPoint(const Point& point);
extern bool XYZToENU(const Point& station, const SatPoint& satpoint, ENUPoint &ENU);
//对流层改正
extern double Trop(const Point &station, const SatPoint &satpoint);
extern bool Elevation(const Point &station, const SatPoint &satpoint, double elvation);
//星历拟合
extern double Chebyshev(double x, int n);