#include "Position.h"
#include "Tools.h"
#include "Matrix.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>

//根据广播星历计算卫星位置
void BroadcastSatPosition(const GTime& time, const NFileRecord& nData, SatPoint& satpoint)
{
	satpoint.PRN = nData.PRN;
	//1.计算卫星运动的平均角速度
	double n0 = sqrt(GM) / pow(nData.sqrtA, 3); //参考时刻TOE的平均角速度
	double n = n0 + nData.DetaN;
	GTime GTOE;
	GTOE.week = nData.GPSWeek;  GTOE.seconds = nData.TOE;
	double diffTime = const_cast<GTime&>(time)-GTOE;
	//2.计算观测瞬间卫星的平近点角M0,这里注意是严格 观测瞬间-参考时间
	double M = nData.M0 + n*diffTime;
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
	double i = nData.i0 + kesi_i + nData.IDOT*diffTime;
	//8.计算卫星在轨道面坐标系中的位置
	double sat_x = r*cos(u);
	double sat_y = r*sin(u);
	//9.计算观测瞬间升交点的经度L
	double L = nData.OMEGA + (nData.OMEGA_DOT - We)*diffTime - We*nData.TOE;
	//10.计算卫星在瞬时地球坐标系中的位置
	satpoint.point.x = sat_x*cos(L) - sat_y*cos(i)*sin(L);
	satpoint.point.y = sat_x*sin(L) + sat_y*cos(i)*cos(L);
	satpoint.point.z = sat_y*sin(i);
}

//CLK卫星钟差(s)
//SendTime 接收到信号时卫星的信号发射时间
//LEAPSeconds 跳秒
//stationPoinst 测站坐标，初始值为(0,0,0)
//Rr 接收机钟差
SatPoint SatPosition(const Time& obsTime, const Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr)
{
	SatPoint satpoint;
	satpoint.PRN = nData.PRN;
	/********************信号从卫星到接收机传播时间******************************/
	double TransTime0 = 0, TransTime1 = 0;
	GTime GobsTime = const_cast<Time&>(obsTime).UTC2GTime();
	//计算卫星信号传播时间
	TransTime1 = pL / C - Rr + SatCLK;
	//卫星信号传播时间需要收敛
	//第一次假设测站坐标为(0,0,0)这样就就差了几毫秒，仍然可以收敛
	while (abs(TransTime0 - TransTime1) > 1.0e-10)
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
		double deta_toc = (SendTime.week - const_cast<Time&>(nData.TOC).UTC2GTime().week) * 7 * 86400 + (SendTime.seconds - const_cast<Time&>(nData.TOC).UTC2GTime().seconds);
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
//前两个元素代表起始和结束时间
//PRN 卫星编号 计算编号为PRN的拟合系数
bool CoefficientofChebyShev(const vector<NFileRecord>& nDatas, map<string, ChebyCoeff>& m)
{
	//计算每个参考时刻的卫星的位置
	map<string, vector<NFileRecord>> nDatas_sort;
	for (auto iter = nDatas.begin(); iter != nDatas.end(); iter++)
	{
		nDatas_sort[iter->PRN].push_back(*iter);
	}
	for (auto iter = nDatas_sort.begin(); iter != nDatas_sort.end(); iter++)
	{
		//计算起始结束时间
		//起始和结束分别外推半个小时，防止只有一个卫星星历
		double start_sec = iter->second.begin()->GPSWeek * WeekSecond + iter->second.begin()->TOE /*- 1800*/;
		double end_sec = (iter->second.end() - 1)->GPSWeek * WeekSecond + (iter->second.end() - 1)->TOE /*+ 1800*/;
		ChebyCoeff xyzcoeff;
		xyzcoeff.XCoeff[0] = start_sec;
		xyzcoeff.XCoeff[1] = end_sec;
		//采样间隔10min
		int row = int(end_sec - start_sec) / 600 > NMAX ? int(end_sec - start_sec) / 600 : NMAX * 30;
		Matrix mB(row + 1, NMAX);
		Matrix xl(row + 1, 1);
		Matrix yl(row + 1, 1);
		Matrix zl(row + 1, 1);
		for (int i = 0; i < mB.getRowNum(); i++)
		{
			GTime GObsTime = Sec2GTime((end_sec - start_sec) / row * i + start_sec);
			//区间[ts,te]变换到[-1,1]
			double change_sec = (2.0 *((end_sec - start_sec) / row * i + start_sec) - (start_sec + end_sec)) / (end_sec - start_sec);
			//B
			for (int ncofee = 0; ncofee < NMAX; ncofee++)
				mB.set(i, ncofee, Chebyshev(change_sec, ncofee));
			//星历中PRN卫星的所有历元的位置
			SatPoint satpoint;
			NFileRecord nData = GetNFileRecordByObsTime(GObsTime, nDatas, iter->first);
			BroadcastSatPosition(GObsTime, nData, satpoint);
			//L
			xl.set(i, 0, satpoint.point.x);
			yl.set(i, 0, satpoint.point.y);
			zl.set(i, 0, satpoint.point.z);
		}
		//解算拟合系数
		Matrix Ninv = (mB.Trans()*mB).Inverse();
		Matrix Xcoeff = Ninv*mB.Trans()*xl;
		std::cout << Xcoeff << std::endl;
		Matrix Ycoeff = Ninv*mB.Trans()*yl;
		Matrix Zcoeff = Ninv*mB.Trans()*zl;
		for (int i = 0; i < NMAX; i++)
		{
			xyzcoeff.XCoeff[i + 2] = Xcoeff.get(i, 0);
			xyzcoeff.YCoeff[i + 2] = Ycoeff.get(i, 0);
			xyzcoeff.ZCoeff[i + 2] = Zcoeff.get(i, 0);
		}
		m[iter->first] = xyzcoeff;
	}
	return true;
}

//切比雪夫多项式拟合卫星位置
bool ChebyShev_SatPosition(const Time& obsTime, string PRN, const map<string, ChebyCoeff> &m, SatPoint& satpoint)
{
	satpoint.PRN = PRN;
	double start_sec = m.at(PRN).XCoeff[0];
	double end_sec = m.at(PRN).XCoeff[1];
	GTime Gobstime = const_cast<Time&>(obsTime).UTC2GTime();
	//将观测时间变换为[-1,1]
	double change_Gobstime = (2.0 * (Gobstime.week * WeekSecond + Gobstime.seconds) - (start_sec + end_sec)) / (end_sec - start_sec);
	for (int i = 0; i < NMAX; i++)
	{
		satpoint.point.x += m.at(PRN).XCoeff[i + 2] * Chebyshev(change_Gobstime, i);
		satpoint.point.y += m.at(PRN).YCoeff[i + 2] * Chebyshev(change_Gobstime, i);
		satpoint.point.z += m.at(PRN).ZCoeff[i + 2] * Chebyshev(change_Gobstime, i);
	}
	return true;
}
//前两个元素代表起始和结束时间
//PRN 卫星编号 计算编号为PRN的拟合系数
bool LagrangeCoeff(const vector<NFileRecord>& nDatas, map<string, vector<LargnageCoeff>>& L)
{
	//计算每个参考时刻的卫星的位置
	map<string, vector<NFileRecord>> nDatas_sort;
	for (auto iter = nDatas.begin(); iter != nDatas.end(); iter++)
	{
		nDatas_sort[iter->PRN].push_back(*iter);
	}
	for (auto iter = nDatas_sort.begin(); iter != nDatas_sort.end(); iter++)
	{
		//计算起始结束时间
		//起始和结束分别外推半个小时，防止只有一个卫星星历
		double start_sec = iter->second.begin()->GPSWeek * WeekSecond + iter->second.begin()->TOE /*- 1800*/;
		double end_sec = (iter->second.end() - 1)->GPSWeek * WeekSecond + (iter->second.end() - 1)->TOE /*+ 1800*/;
		//采样间隔
		int row = int(end_sec - start_sec) / (LMAX - 1);
		vector<LargnageCoeff> singleL;
		for (int i = 0; i < LMAX; i++)
		{
			GTime GObsTime = Sec2GTime(row * i + start_sec);
			//星历中PRN卫星的所有历元的位置
			SatPoint satpoint;
			NFileRecord nData = GetNFileRecordByObsTime(GObsTime, nDatas, iter->first);
			BroadcastSatPosition(GObsTime, nData, satpoint);

			LargnageCoeff lcoeff = { GObsTime, satpoint.point };
			singleL.push_back(lcoeff);
		}
		L[iter->first] = singleL;
	}
	return true;
}
bool Lagrange_SatPosition(const Time& obsTime, string PRN, const map<string, vector<LargnageCoeff>>& L, SatPoint& satpoint)
{
	satpoint.PRN = PRN;
	GTime GobsTime = const_cast<Time&>(obsTime).UTC2GTime();
	for (vector<LargnageCoeff>::size_type i = 0; i < L.at(PRN).size(); i++)
	{
		double coeff = 1.0;
		for (vector<LargnageCoeff>::size_type j = 0; j < L.at(PRN).size(); j++)
		{
			if (j == i) continue;
			coeff *= (GobsTime.seconds - L.at(PRN).at(j).gtime.seconds) / (L.at(PRN).at(i).gtime.seconds - L.at(PRN).at(j).gtime.seconds);
		}
		satpoint.point.x += coeff* L.at(PRN).at(i).point.x;
		satpoint.point.y += coeff* L.at(PRN).at(i).point.y;
		satpoint.point.z += coeff* L.at(PRN).at(i).point.z;
	}
	return true;
}
NFileRecord GetNFileRecordByObsTime(const GTime& GobsTime, const vector<NFileRecord>& nDatas, string PRN)
{
	//星历中编号为PRN的所有卫星星历数据
	vector<double> timeInterval;
	vector<NFileRecord> waitGet;
	for (vector<NFileRecord>::size_type i = 0; i < nDatas.size(); i++)
	{
		if (nDatas.at(i).PRN == PRN)
		{
			double timebias = abs(nDatas.at(i).TOE - GobsTime.seconds) + abs(nDatas.at(i).GPSWeek - GobsTime.week) * WeekSecond;
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
bool CalculationPostion(Point PXYZ, OEpochData &oData, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds, double &Rr, map<string, double> &SatCLKs, double elvation)
{
	Point Position1;
	//星历中的测站近似坐标
	Position = PXYZ;
	//检查观测值
	if (!CheckDatas(oData)) return false;

	//初始化卫星钟差
	for (vector<NFileRecord>::size_type i = 0; i < nDatas.size(); i++)
	{
		SatCLKs[nDatas[i].PRN] = 0.0;
	}
	//控制迭代次数
	int iter_count = 0;
	while (abs(Position.x - Position1.x)>1e-8 || abs(Position.y - Position1.y) > 1e-8 || abs(Position.z - Position1.z) > 1e-8)
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
			NFileRecord _nData = GetNFileRecordByObsTime(const_cast<Time&>(oData.gtime).UTC2GTime(), nDatas, oData.AllTypeDatas[C1].at(i).PRN);
			//卫星钟差，每颗卫星钟差均不一样，因此要根据PRN来找上次计算出的这颗卫星钟差
			double &SatCLK = SatCLKs[_nData.PRN];
			double PObs = oData.AllTypeDatas[C1].at(i).Obs;
			//卫星位置
			SatPoint satpoint = SatPosition(oData.gtime, Position, PObs, _nData, SatCLK, LeapSeconds, Rr);
			if (oData.AllTypeDatas[P1].at(i).Obs != 0)   satpoint = SatPosition(oData.gtime, Position, PObs, _nData, SatCLK, LeapSeconds, Rr);
			//卫星高度角
			if (!Elevation(Position, satpoint, elvation)) continue;
#if 1
			//无电离层模型
			PObs = oData.AllTypeDatas[C1].at(i).Obs;
			if (oData.AllTypeDatas[P1].at(i).Obs != 0)
			{
				PObs = oData.AllTypeDatas[P1].at(i).Obs;
				if (oData.AllTypeDatas[P2].at(i).Obs != 0)  PObs = 2.54573*PObs - 1.54573*(oData.AllTypeDatas[P2].at(i).Obs);
			}
#endif

#if 0  
			//对流层改正
			double trop = Trop(Position, satpoint, azel);
			Pobs -= trop;
			if (odata.alltypedatas[p1].at(i).obs != 0)
			{
				pobs = odata.alltypedatas[p1].at(i).obs - trop;
				if (odata.alltypedatas[p2].at(i).obs != 0)  pobs = 2.54573*pobs - 1.54573*(odata.alltypedatas[p2].at(i).obs - trop);
			}
#endif

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
	//接收机钟差,接收机钟差
	double Rr = 0;
	map<string, double> SatCLKs;
	//输出每个观测历元的计算结果
	ofstream ResFile(output);
	cout << "总历元数： " << oDatas.size() << endl;
	if (!ResFile.is_open()) return false;
	ResFile << setw(16) << "观测历元" << setw(18) << "卫星数" << setw(8) << "X" << setw(16) << "Y" << setw(12) << "Z" << setw(23) << "接收机钟差\n";

	for (vector<OEpochData>::size_type i = 0; i < oDatas.size(); i++)
	{
		cout << "正在处理： " << i << endl;
		Point result;
		if (CalculationPostion(oHeader.PXYZ, oDatas[i], result, nDatas, nHeader.LeapSeconds, Rr, SatCLKs, elevation))
			EString::OutPut(ResFile, oDatas[i], Rr, &result);
		else
			EString::OutPut(ResFile, oDatas[i], Rr);
	}
	ResFile.flush();
	ResFile.close();
	return true;
}
