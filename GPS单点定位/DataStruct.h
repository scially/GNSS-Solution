#ifndef _DATASTRUCT_H_
#define _DATASTRUCT_H_

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <map>

#define GM 3.986005E14 //万有引力常数G与地球质量M之乘积
#define C  2.99792458E8 //光速
#define PI atan(1)*4
#define We 7.292115E-5 //地球自转角速度

#define WGS84A  6378137.0     //WGS84坐标系长半轴
#define WGS84f  1/298.257223565 //WGS84坐标系偏心率
//v2.x Rinex文件头
using std::string;
using std::vector;
using std::map;
enum ObsType{
	C1,
	P1,
	P2,
	L1,
	L2=4,
};
struct Point  //空间坐标系
{
	double x;
	double y;
	double z;
	Point()
	{
		x = 0;
		y = 0;
		z = 0;
	}
};
struct SatPoint   //卫星坐标
{ 
	Point point;
	string PRN;
};
struct BLHPoint  //大地坐标系,单位为弧度
{
	double B;
	double L;
	double H;
	BLHPoint()
	{
		B = 0;
		L = 0;
		H = 0;
	}
};
struct ENUPoint  //站心地平坐标系
{
	double E;
	double N;
	double U;
	ENUPoint()
	{
		E = 0;
		N = 0;
		U = 0;
	}
};
struct Time{  //正常时UTC
	int year;
	int month;
	int day;
	int hour;
	int minute;
	double second;
};
struct GTime  //GPS时
{
	int week;
	double seconds;
};
struct OHeader{  //O文件文件头
	string version;
	char type; //文件类型
	char sattype; //卫星类型
	string markername; //测站名
	string markernumber;//测站编号
	string observer;//观测人姓名
	string agency;//观测单位
	string rec;//接收机序列号
	string rectype;//接收机型号
	string recvers;//接收机版本号
	float antenna_h;//天线高
	float interval;//历元间隔
	string ant;//天线序列号
	string anttype;//天线型号
	Point PXYZ; //测站近似坐标
	int sigsnums;//GPS接收机信号类型数，这里即便是M，我们也只考虑GPS信号类型
	vector<string> sigtypes; //信号列表
	Time gtime;//第一个观测记录时刻，GPS时
};
struct ASatData //每颗卫星的观测值以及编号
{
	string PRN; //卫星编号
	double Obs; //观测值
};
struct OEpochData //每一个历元的观测值
{
	Time gtime; //历元时刻
	int satsums;//卫星数量
	int flag;//历元标志，0表示正常，1表示当前历元与上一历元发生异常
	map<ObsType, vector<ASatData>> AllTypeDatas;  //星历中所有类型的观测值
};
struct NFileHeader        //N文件文件头
{
	string version;  //RINEX格式的版本号
	char type;       //文件类型
	char system;     //文件所属卫星系统
	double IonAlpha[4];  //星历中的电离层参数A1-A4
	double IonBeta[4];   //星历中的电离层参数B2-B4
	double DeltaUtaA[2]; //用于计算UTC时的历书时的多项式系数A1,A2
	double DeltaUtcT;   //用于计算UTC时的历书时的参考时刻
	double DeltaUtcW;  //用于计算UTC时的历书时 参考周数
	double LeapSeconds;  //跳秒 计算UTC和GPS时的差异
};
struct NFileRecord  //PRN编号的卫星的广播星历
{
	string PRN;     //  卫星的Prn号	
	Time TOC;   //卫星钟的参考时刻
	double ClkBias; //  卫星钟的偏差s
	double ClkDrift;  //  卫星钟的漂移 s/s
	double ClkDriftRate;  //  卫星钟的漂移速度s/(s2)
	//广播轨道1
	double IODE;
	double Crs;
	double DetaN; //摄动参数
	double M0;
	//广播轨道2
	double Cuc;
	double e; //轨道偏心率
	double Cus;
	double sqrtA;
	//广播轨道3
	double TOE;  //星历参考时刻，GPS周内的秒数
	double Cic;
	double OMEGA;
	double Cis;
	//广播轨道4
	double i0;
	double Crc;
	double omega;
	double OMEGA_DOT;
	//广播轨道5
	double IDOT; //i的变化率
	double L2;
	double GPSWeek; //GPS周数，与TOE一同表示星历的参考时刻
	double L2_P;
	//广播轨道6
	double SatAccuracy;
	double SatHealthy;
	double TGD;
	double IODC;
	//广播轨道7
	double TransmissionTimeofMessage;
	double FitInterval;
	double Reamrk1;
	double Remark2;
};
#endif