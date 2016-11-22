#pragma once
#include "DataStruct.h"
#include "ReadFile.h"

extern SatPoint SatPosition(const Time& obsTime, const Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr);
extern NFileRecord GetNFileRecordByObsTime(const Time& obsTime, const vector<NFileRecord>& nDatas, string &PRN);
extern bool CalculationPostion(Point PXYZ, OEpochData &oData, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds, double &Rr, map<string, double> &SatCLKs, double elvation);
extern bool OutputResult(ReadFile read, string output,double elevation);
