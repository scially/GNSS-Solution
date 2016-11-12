#pragma once
#include "DataStruct.h"
#include "ReadFile.h"

using std::map;
extern SatPoint SatPosition(const Time& obsTime, Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr);
extern NFileRecord GetNFileRecordByObsTime(const Time& obsTime, const vector<NFileRecord>& nDatas, string &PRN);
extern bool CalculationPostion(Point PXYZ, OEpochData &oDatas, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds, double &Rr, double elelvation);
extern bool OutputResult(ReadFile read, string output,double elevation);

