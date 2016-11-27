#pragma once
#include "DataStruct.h"
#include "ReadFile.h"

extern SatPoint SatPosition(const Time& obsTime, const Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr);
//根据卫星PRN，寻找观测时间与星历参考时刻TOE间隔最小的卫星导航电文
extern NFileRecord GetNFileRecordByObsTime(const GTime& GobsTime, const vector<NFileRecord>& nDatas, string PRN);
extern bool CalculationPostion(Point PXYZ, OEpochData &oData, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds, double &Rr, map<string, double> &SatCLKs, double elvation);
extern bool OutputResult(ReadFile read, string output,double elevation);
extern bool ChebyShev_SatPosition(const Time& obsTime, string PRN, const map<string, ChebyCoeff> &m, SatPoint& satpoint);
//根据广播星历和要求时间计算卫星坐标
extern void BroadcastSatPosition(const GTime& time, const NFileRecord& nData, SatPoint& satpoint);
extern bool CoefficientofChebyShev(const vector<NFileRecord>& nDatas, map<string, ChebyCoeff>& m);
