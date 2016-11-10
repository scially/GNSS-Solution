#include "Position.h"
#include "Tools.h"
#include "Matrix.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>

//CLK卫星钟差(s),初始值为0
//SendTime就是接收到信号时卫星的信号发射时间
//LEAPSeconds 跳秒
//stationPoinst 测站坐标，初始值为(0,0,0)
//Rr接收机钟差
SatPoint SatPosition(const Time& obsTime, Point& stationPoint, double pL, const NFileRecord& nData, double &SatCLK, double LEAPSeconds, double Rr)
{
	SatPoint satpoint;
	satpoint.PRN = nData.PRN;
	/*****************************信号从卫星到接收机传播时间***************************************/
	double TransTime0 = 0, TransTime1 = 0;
	GTime GobsTime = UTC2GTime(obsTime);
	//计算卫星信号传播时间
	TransTime1 = pL / C - Rr + SatCLK;
	//卫星信号传播时间需要收敛
	//第一次假设测站坐标为(0,0,0)这样就就差了几毫秒，仍然可以收敛
	while (abs(TransTime0 - TransTime1) > 1.0e-6)
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
		double timeInterval = (SendTime.week - nData.GPSWeek) * 7 * 86400 + (SendTime.seconds - nData.TOE);// +LEAPSeconds;
		//顾及一周（604800）开始或结束
		if (timeInterval > 302400)   timeInterval -= 604800;
		if (timeInterval < -302400)  timeInterval += 604800;
		double M = nData.M0 + n*timeInterval;
		//3.计算偏近点角
		double E = 0, E0 = M;
		while (abs(E - E0) > 1.0e-14)
		{
			//牛顿迭代法
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
		double i = nData.i0 + kesi_i + nData.IDOT*timeInterval;
		//8.计算卫星在轨道面坐标系中的位置
		double sat_x = r*cos(u);
		double sat_y = r*sin(u);
		//9.计算观测瞬间升交点的经度L
		double L = nData.OMEGA + (nData.OMEGA_DOT - We)*timeInterval - We*nData.TOE;
		//10.计算卫星在瞬时地球坐标系中的位置
		satpoint.x = sat_x*cos(L) - sat_y*cos(i)*sin(L);
		satpoint.y = sat_x*sin(L) + sat_y*cos(i)*cos(L);
		satpoint.z = sat_y*sin(i);
		//11.地球自转改正,将卫星位置规划到接受信号时刻的位置
		/*double deta_a = We*timeInterval;
		double earth_x = satpoint.x, earth_y = satpoint.y;
		satpoint.x = earth_x * cos(deta_a) + earth_y  * sin(deta_a);
		satpoint.y = earth_y * cos(deta_a) - earth_x * sin(deta_a);
		satpoint.z = satpoint.z;*/
		//12.卫星钟差改正
		//计算相对论效应
		//由于卫星非圆形轨道引起的相对论效应改正项
		double deta_tr = -2 * sqrt(GM)*nData.sqrtA / pow(C, 2)*nData.e*sin(E);
		//根据星历数据计算卫星钟差
		double deta_toc = (SendTime.week - UTC2GTime(nData.TOC).week) * 7 * 86400 + (SendTime.seconds - UTC2GTime(nData.TOC).seconds);
		SatCLK = nData.ClkBias + nData.ClkDrift*deta_toc + nData.ClkDriftRate*pow(deta_toc, 2) + deta_tr;
		//13.再次计算信号传播时间，考虑卫星钟差
		//卫星到接收机观测距离
		double sat2rec_x2 = pow((satpoint.x - stationPoint.x), 2);
		double sat2rec_y2 = pow((satpoint.y - stationPoint.y), 2);
		double sat2rec_z2 = pow((satpoint.z - stationPoint.z), 2);
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
bool CalculationPostion(Point PXYZ, const OEpochData &oDatas, Point &Position, const vector<NFileRecord>& nDatas, double LeapSeconds,double &Rr)
{
	Point Position1;
	//星历中的测站近似坐标
	Position = PXYZ;
	//对流层、电离层改正
	double ion = 0;
	double trop = 0;
	//卫星数是否大于4
	if (oDatas.satsums < 4) return false;
	//初始化卫星钟差
	map<string, double> SatCLKs;
	for (vector<NFileRecord>::size_type i = 0; i < nDatas.size();i++)
	{
		SatCLKs[nDatas[i].PRN] = 0.0;
	}
	//检查观测值是否有效
	OEpochData oDatas_copy = oDatas;
	for (vector<ASatData>::size_type i = 0; i < oDatas_copy.AllTypeDatas[C1].size(); i++)
	{
		//跳过非GPS卫星
		if (oDatas_copy.AllTypeDatas[C1][i].PRN.substr(0, 1) != "G")
		{
			i--;
			oDatas_copy.AllTypeDatas[C1].erase(oDatas_copy.AllTypeDatas[C1].begin() + i + 1);
			oDatas_copy.AllTypeDatas[P1].erase(oDatas_copy.AllTypeDatas[P1].begin() + i + 1);
			oDatas_copy.AllTypeDatas[P2].erase(oDatas_copy.AllTypeDatas[P2].begin() + i + 1);
			oDatas_copy.AllTypeDatas[L1].erase(oDatas_copy.AllTypeDatas[L1].begin() + i + 1);
			oDatas_copy.AllTypeDatas[L2].erase(oDatas_copy.AllTypeDatas[L2].begin() + i + 1);
			oDatas_copy.satsums--;
		}
	}
	//是否多余4颗卫星
	if (oDatas_copy.satsums<4) return false;
	//构建方程系数
	Matrix B(oDatas_copy.satsums, 4);
	Matrix L(oDatas_copy.satsums, 1);
	Matrix detaX(4, 1);
	Matrix V(oDatas_copy.satsums, 1);
	//迭代计算测站坐标,控制迭代次数
	int iter_count = 0;
	while (abs(Position.x - Position1.x)>1e-5 || abs(Position.y - Position1.y) > 1e-5 || abs(Position.z - Position1.z) > 1e-5)
	{	
		if (++iter_count>50)
		{
			std::cout << "迭代次数过多，该历元！\n";
			break;
		}
		Position1 = Position;
		for (vector<ASatData>::size_type i = 0; i < oDatas_copy.AllTypeDatas[C1].size(); i++)
		{
			//根据观测时间选择最佳轨道卫星
			NFileRecord _nData = GetNFileRecordByObsTime(oDatas_copy.gtime, nDatas, oDatas_copy.AllTypeDatas[C1].at(i).PRN);
			//卫星钟差，每颗卫星钟差均不一样，因此要根据PRN来找上次计算出的这颗卫星钟差
			double &SatCLK = SatCLKs[_nData.PRN];

			//有的接收机可能没有P码，所以改用C/A码
			double PObs = oDatas_copy.AllTypeDatas[C1].at(i).Obs;
			//无电离层模型
			if (oDatas_copy.AllTypeDatas[P1].at(i).Obs != 0)
			{
				PObs = oDatas_copy.AllTypeDatas[P1].at(i).Obs;
				if (oDatas_copy.AllTypeDatas[P2].at(i).Obs != 0)
				{
					PObs = 2.54573*PObs - 1.54573*oDatas_copy.AllTypeDatas[P2].at(i).Obs;
				}
			}
			//卫星位置
			SatPoint satpoint = SatPosition(oDatas_copy.gtime, Position, PObs, _nData, SatCLK, LeapSeconds, Rr);
			//测试卫星位置
			std::cout.precision(10);
			//std::cout << satpoint.PRN << ": " << satpoint.x << ", " << satpoint.y << ", " << satpoint.z << std::endl;
			//方向余弦
			double deta_x = satpoint.x - Position.x;
			double deta_y = satpoint.y - Position.y;
			double deta_z = satpoint.z - Position.z;
			double Rj = sqrt(deta_x*deta_x + deta_y*deta_y + deta_z*deta_z);
			double li = deta_x / Rj;
			double mi = deta_y / Rj;
			double ni = deta_z / Rj;
			double l = PObs - Rj + C*SatCLK - C*Rr; //常数项
			B.set(i, 0, -li); B.set(i, 1, -mi); B.set(i, 2, -ni); B.set(i, 3, 1.0);
			L.set(i, 0, l);
		}
		detaX = (B.Trans()*B).Inverse()*(B.Trans()*L);
		//改正测站坐标以及接收机钟差
		Position.x += detaX.get(0, 0);
		Position.y += detaX.get(1, 0);
		Position.z += detaX.get(2, 0);
		Rr += detaX.get(3, 0) / C;
	}
	B.Matrix_free();
	L.Matrix_free();
	detaX.Matrix_free();
	V.Matrix_free();
	//测试结果
	//std::cout << Position.x << "  " << Position.y << "  " << Position.z << "\n";
	return true;
}
bool OutputResult(ReadFile read,string output)
{
	using namespace std;
	//获取星历和观测数据
	OHeader oHeader = read._ofile.ReadHeader();
	vector<OEpochData> oDatas = read._ofile.ReadData();
	NFileHeader nHeader = read._nfile.ReadNHeader();
	vector<NFileRecord> nDatas = read._nfile.ReadNRecord();
	//接收机钟差
	double Rr = 0;
	//测试计算结果
	//CalculationPostion(oHeader.PXYZ, oDatas[0], result,nDatas, nHeader.LeapSeconds,Rr);
	//输出每个观测历元的计算结果
	ofstream Output(output);
	cout << "总历元数： " << oDatas.size() << endl;
	if (!Output.is_open()) return false;
	Output << "        观测历元       " << "    卫星数   " << "      X       " << "      Y       " << "      Z       "
		<< "    接收机钟差     \n";
	for (vector<OEpochData>::size_type i = 0; i < oDatas.size(); i++)
	{
		cout << "正在处理： " << i << endl;
		Point result;
		if (CalculationPostion(oHeader.PXYZ, oDatas[i], result, nDatas, nHeader.LeapSeconds, Rr))
		{
			Output << setw(4) << oDatas[i].gtime.year << setw(3) << oDatas[i].gtime.month << setw(3) << oDatas[i].gtime.day
				<< setw(3) << oDatas[i].gtime.hour << setw(3) << oDatas[i].gtime.minute << setw(11) << fixed << setprecision(7)
				<< oDatas[i].gtime.second  ;
			Output << setw(3) << oDatas[i].satsums;
			Output << setw(18) << fixed << setprecision(4) << result.x
				<< setw(15) << fixed << setprecision(4) << result.y
				<< setw(15) << fixed << setprecision(4) << result.z
                << setw(16) << fixed << setprecision(10) << Rr<<endl;
		}
		else
		{
			Output << "卫星数量小于4颗，具体信息：\n";
			Output << setw(4) << oDatas[i].gtime.year << setw(3) << oDatas[i].gtime.month << setw(3) << oDatas[i].gtime.day
				<< setw(3) << oDatas[i].gtime.hour << setw(3) << oDatas[i].gtime.minute << setw(11) << fixed << setprecision(7) 
				<< oDatas[i].gtime.second<< endl;
		}
	}
	Output.close();
	return true;
}