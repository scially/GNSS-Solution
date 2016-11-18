#include "Position.h"
#include "Tools.h"
#include "Matrix.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>

//CLK卫星钟差(s)
//SendTime就是接收到信号时卫星的信号发射时间
//LEAPSeconds 跳秒
//stationPoinst 测站坐标，初始值为(0,0,0)
//Rr接收机钟差
SatPoint SatPosition(const Time& obsTime, const Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr)
{
	SatPoint satpoint;
	satpoint.PRN = nData.PRN;
	/********************信号从卫星到接收机传播时间******************************/
	double TransTime0 = 0, TransTime1 = 0;
	GTime GobsTime = UTC2GTime(obsTime);
	//计算卫星信号传播时间
	TransTime1 = pL / C - Rr + SatCLK;
	//卫星信号传播时间需要收敛
	//第一次假设测站坐标为(0,0,0)这样就就差了几毫秒，仍然可以收敛
	while (abs(TransTime0 - TransTime1) > 1.0e-8)
	{
		TransTime0 = TransTime1;
		//1.计算卫星运动的平均角速度
		double n0 = sqrt(GM) / pow(nData.sqrtA, 3); //参考时刻TOE的平均角速度
		double n = n0 + nData.DetaN;
		//接收机接收时刻-信号传播时间=计算卫星发送信号的时刻
		GTime SendTime;
		if ((GobsTime.seconds - TransTime1) < 0)
		{
			SendTime.week = GobsTime.week - 1;
			SendTime.seconds = GobsTime.seconds - TransTime1 + 7 * 86400;
		}
		else
		{
			SendTime.week = GobsTime.week;
			SendTime.seconds = GobsTime.seconds - TransTime1;
		}
		//2.计算观测瞬间卫星的平近点角M0,这里注意是严格 观测瞬间-参考时间
		double TransTime = (SendTime.week - nData.GPSWeek) * 7 * 86400 + (SendTime.seconds - nData.TOE);// +LEAPSeconds;
		//顾及一周（604800）开始或结束
		if (TransTime > 302400)   TransTime -= 604800;
		if (TransTime < -302400)  TransTime += 604800;
		double M = nData.M0 + n*TransTime;
		//3.计算偏近点角
		double E = 0, E0 = M;
		while (abs(E - E0) > 1.0e-14)
		{
			E0 = E;
			E -= (M + nData.e*sin(E0) - E0) / (nData.e*cos(E0) - 1);
		}
		//4.计算真近点角
		double f = atan2(sqrt(1.0 - nData.e*nData.e)*sin(E), cos(E) - nData.e);
		//5.计算升交角距
		double u1 = nData.omega + f;
		//6.计算摄动改正项
		double kesi_u = nData.Cuc*cos(2 * u1) + nData.Cus*sin(2 * u1);
		double kesi_r = nData.Crc*cos(2 * u1) + nData.Crs*sin(2 * u1);
		double kesi_i = nData.Cic*cos(2 * u1) + nData.Cis*sin(2 * u1);
		//7.对 u' r' i'进行摄动改正
		double u = u1 + kesi_u;
		double r = nData.sqrtA*nData.sqrtA *(1 - nData.e*cos(E)) + kesi_r;
		double i = nData.i0 + kesi_i + nData.IDOT*TransTime;
		//8.计算卫星在轨道面坐标系中的位置
		double sat_x = r*cos(u);
		double sat_y = r*sin(u);
		//9.计算观测瞬间升交点的经度L
		double L = nData.OMEGA + (nData.OMEGA_DOT - We)*TransTime - We*nData.TOE;
		//10.计算卫星在瞬时地球坐标系中的位置
		satpoint.point.x = sat_x*cos(L) - sat_y*cos(i)*sin(L);
		satpoint.point.y = sat_x*sin(L) + sat_y*cos(i)*cos(L);
		satpoint.point.z = sat_y*sin(i);
		//11.地球自转改正,将卫星位置归化到接受信号时刻的位置
		double deta_a = We*TransTime1;
		double earth_x = satpoint.point.x, earth_y = satpoint.point.y;
		satpoint.point.x = earth_x * cos(deta_a) + earth_y  * sin(deta_a);
		satpoint.point.y = earth_y * cos(deta_a) - earth_x * sin(deta_a);
		satpoint.point.z = satpoint.point.z;
		//12.卫星钟差改正
		//计算相对论效应
		//由于卫星非圆形轨道引起的相对论效应改正项
		double deta_tr = -2 * sqrt(GM)*nData.sqrtA / pow(C, 2)*nData.e*sin(E);
		//根据星历数据计算卫星钟差
		double deta_toc = (SendTime.week - UTC2GTime(nData.TOC).week) * 7 * 86400 + (SendTime.seconds - UTC2GTime(nData.TOC).seconds);
		SatCLK = nData.ClkBias + nData.ClkDrift*deta_toc + nData.ClkDriftRate*pow(deta_toc, 2) + deta_tr;
		//13.再次计算信号传播时间，考虑卫星钟差
		//卫星到接收机观测距离
		double sat2rec_x2 = pow((satpoint.point.x - stationPoint.x), 2);
		double sat2rec_y2 = pow((satpoint.point.y - stationPoint.y), 2);
		double sat2rec_z2 = pow((satpoint.point.z - stationPoint.z), 2);
		double pL1 = sqrt(sat2rec_x2 + sat2rec_y2 + sat2rec_z2);
		//考虑卫星钟差、接收机钟差
		TransTime1 = pL1 / C + SatCLK - Rr;
	}
	return satpoint;
}

//根据卫星PRN，寻找观测时间与星历参考时刻TOE间隔最小的卫星导航电文
NFileRecord GetNFileRecordByObsTime(const Time& obsTime, const vector<NFileRecord>& nDatas, string &PRN)
{
	GTime GobsTime = UTC2GTime(obsTime);
	//星历中编号为PRN的所有卫星星历数据
	vector<double> timeInterval;
	vector<NFileRecord> waitGet;
	for (vector<NFileRecord>::size_type i = 0; i < nDatas.size(); i++)
	{
		if (nDatas.at(i).PRN == PRN)
		{
			double timebias = abs(nDatas.at(i).TOE - GobsTime.seconds) + abs(nDatas.at(i).GPSWeek - GobsTime.week) * 7 * 86400;
			timeInterval.push_back(timebias);
			waitGet.push_back(nDatas.at(i));
		}
	}
	return waitGet.at(MinOfNums(timeInterval));
}
//计算单历元的测站坐标
//PXYZ 测站近似坐标
//SatCLK 卫星钟差
//Rr 接收机钟差
bool CalculationPostion(Point PXYZ, OEpochData &oData, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds, double &Rr, double elvation)
{
	Point Position1;
	//星历中的测站近似坐标
	Position = PXYZ;
	//卫星数是否大于4
	if (oData.satsums < 4) return false;
	//初始化卫星钟差
	map<string, double> SatCLKs;
	for (vector<NFileRecord>::size_type i = 0; i < nDatas.size(); i++)
	{
		SatCLKs[nDatas[i].PRN] = 0.0;
	}
	//检查观测值
	if (!CheckDatas(oData)) return false;
	//迭代计算测站坐标,控制迭代次数
	int iter_count = 0;
	while (abs(Position.x - Position1.x)>1e-10 || abs(Position.y - Position1.y) > 1e-10 || abs(Position.z - Position1.z) > 1e-10)
	{
		//构建方程系数
		vector<double> vB;
		vector<double> vL;
		if (++iter_count > 50) break;

		Position1 = Position;
		for (int i = 0; i < oData.satsums; i++)
		{
			if (oData.AllTypeDatas[C1][i].PRN.substr(0, 1) != "G") continue;
			//根据观测时间选择最佳轨道卫星
			NFileRecord _nData = GetNFileRecordByObsTime(oData.gtime, nDatas, oData.AllTypeDatas[C1].at(i).PRN);
			//卫星钟差，每颗卫星钟差均不一样，因此要根据PRN来找上次计算出的这颗卫星钟差
			double &SatCLK = SatCLKs[_nData.PRN];
			double PObs = oData.AllTypeDatas[C1].at(i).Obs;
			//卫星位置
			SatPoint satpoint = SatPosition(oData.gtime, Position, PObs, _nData, SatCLK, LeapSeconds, Rr);
			if (oData.AllTypeDatas[P1].at(i).Obs != 0)   satpoint = SatPosition(oData.gtime, Position, PObs, _nData, SatCLK, LeapSeconds, Rr);
			//卫星高度角
			double azel[2];
			if (!Elevation(Position, satpoint, elvation, azel)) continue;

			//无电离层模型+对流层改正
			double trop = Trop(Position, satpoint, azel);
			/*PObs -= trop;
			if (oData.AllTypeDatas[P1].at(i).Obs != 0)
			{
			PObs = oData.AllTypeDatas[P1].at(i).Obs - trop;
			if (oData.AllTypeDatas[P2].at(i).Obs != 0)  PObs = 2.54573*PObs - 1.54573*(oData.AllTypeDatas[P2].at(i).Obs - trop);
			}*/

			//无电离层模型
			PObs = oData.AllTypeDatas[C1].at(i).Obs;
			if (oData.AllTypeDatas[P1].at(i).Obs != 0)
			{
				PObs = oData.AllTypeDatas[P1].at(i).Obs;
				if (oData.AllTypeDatas[P2].at(i).Obs != 0)  PObs = 2.54573*PObs - 1.54573*(oData.AllTypeDatas[P2].at(i).Obs);
			}
			double deta_x = satpoint.point.x - Position.x;
			double deta_y = satpoint.point.y - Position.y;
			double deta_z = satpoint.point.z - Position.z;
			double Rj = sqrt(deta_x*deta_x + deta_y*deta_y + deta_z*deta_z);
			double li = deta_x / Rj;
			double mi = deta_y / Rj;
			double ni = deta_z / Rj;
			double l = PObs - Rj + C*SatCLK - C*Rr;
			vB.push_back(-li);
			vB.push_back(-mi);
			vB.push_back(-ni);
			vB.push_back(1.0);
			vL.push_back(l);
		}
		if (vL.size() < 4)  return false;
		Matrix B(&vB[0], vB.size() / 4, 4);
		Matrix L(&vL[0], vL.size(), 1);
		Matrix detaX(4, 1);
		detaX = (B.Trans()*B).Inverse()*(B.Trans()*L);

		Position.x += detaX.get(0, 0);
		Position.y += detaX.get(1, 0);
		Position.z += detaX.get(2, 0);
		Rr += detaX.get(3, 0) / C;
	}
	return true;
}
//elevation 高度截止角
bool OutputResult(ReadFile read, string output, double elevation)
{
	using namespace std;
	//读取星历和观测数据
	OHeader oHeader = read._ofile.ReadHeader();
	vector<OEpochData> oDatas = read._ofile.ReadData();
	NFileHeader nHeader = read._nfile.ReadNHeader();
	vector<NFileRecord> nDatas = read._nfile.ReadNRecord();
	//接收机钟差
	double Rr = 0;

	//输出每个观测历元的计算结果
	ofstream ResFile(output);
	cout << "总历元数： " << oDatas.size() << endl;
	if (!ResFile.is_open()) return false;
	ResFile << setw(16)<< "观测历元" << setw(18) << "卫星数" 
		    << setw(8) << "X" << setw(16) << "Y" << setw(12) << "Z" 
			<< setw(23) << "接收机钟差\n";

	for (vector<OEpochData>::size_type i = 0; i < oDatas.size(); i++)
	{
		cout << "正在处理： " << i << endl;
		Point result;
		if (CalculationPostion(oHeader.PXYZ, oDatas[i], result, nDatas, nHeader.LeapSeconds, Rr, elevation))
		{
			ResFile << setw(4) << oDatas[i].gtime.year << setw(3) << oDatas[i].gtime.month << setw(3) << oDatas[i].gtime.day
				    << setw(3) << oDatas[i].gtime.hour << setw(3) << oDatas[i].gtime.minute << setw(11) << fixed << setprecision(7)
				    << oDatas[i].gtime.second;
			ResFile << setw(4) << oDatas[i].satsums;
			ResFile << setw(16) << fixed << setprecision(4) << result.x
				    << setw(15) << fixed << setprecision(4) << result.y
				    << setw(15) << fixed << setprecision(4) << result.z
				    << setw(16) << fixed << setprecision(10) << Rr << endl;
		}
		else
		{
			ResFile << "该历元数据有误：";
			ResFile << setw(4) << oDatas[i].gtime.year << setw(3) << oDatas[i].gtime.month << setw(3) << oDatas[i].gtime.day
				    << setw(3) << oDatas[i].gtime.hour << setw(3) << oDatas[i].gtime.minute << setw(11) << fixed << setprecision(7)
				    << oDatas[i].gtime.second << endl;
		}
	}
	ResFile.close();
	return true;
}