#include "Tools.h"
#include "Matrix.h"
#include <iostream>

double UTC2JD(const Time& time)
{
	double hour = time.hour + time.minute / 60.0 + time.second / 3600.0;
	return 1721013.5 + 367 * time.year - int(7.0 / 4 * (time.year + (time.month + 9) / 12)) + time.day + hour / 24 + (275 * time.month / 9);
}
int MinOfNums(const vector<double>& nums)
{
	double min = nums[0];
	int index = 0;
	//从0开始，至少保证有一个匹配的卫星轨道
	for (vector<double>::size_type i = 0; i < nums.size(); i++)
	{
		if (nums.at(i) < min)
		{
			index = i;
			min = nums.at(i);
		}
	}
	return index;
}
BLHPoint PointToBLHPoint(const Point& point)
{
	BLHPoint BLH;
	double B = 0, B1 = 1;
	//std::function<double(double)> 
	//若是不支持c++11, 这里需要声明为一个函数
	double e2 = 2 * WGS84f - WGS84f*WGS84f;
	auto N = [e2](double B){return WGS84A / sqrt(1 - e2*sin(B)*sin(B)); };
	while (abs(B1 - B) > 1e-8)
	{
		B1 = B;
		B = atan((point.z + N(B)*e2*sin(B)) / sqrt(pow(point.x, 2) + pow(point.y, 2)));
	}
	double L = atan(point.y / point.x);
	double H = sqrt(pow(point.x, 2) + pow(point.y, 2)) / cos(B) - N(B);
	BLH.B = B;
	BLH.H = H;
	BLH.L = L;
	return BLH;
}
bool XYZToENU(const Point& station, const SatPoint& satpoint, ENUPoint &ENU)
{
	//1.计算向量模
	double distance = 0;
	if (sqrt(pow(satpoint.point.x, 2) + pow(satpoint.point.y, 2) + pow(satpoint.point.z, 2)) < WGS84A) return false;
	double r_vector[3];
	r_vector[0] = satpoint.point.x - station.x;
	r_vector[1] = satpoint.point.y - station.y;
	r_vector[2] = satpoint.point.z - station.z;
	distance = sqrt(pow(r_vector[0], 2) + pow(r_vector[1], 2) + pow(r_vector[2], 2));
	for (int i = 0; i < 3; i++) r_vector[i] /= distance;

	//2.计算转换矩阵
	BLHPoint rBLH = PointToBLHPoint(station);
	double E[9];
	E[0] = -sin(rBLH.L);             E[3] = cos(rBLH.L);              E[6] = 0.0;
	E[1] = -sin(rBLH.B)*cos(rBLH.L); E[4] = -sin(rBLH.B)*sin(rBLH.L); E[7] = cos(rBLH.B);
	E[2] = cos(rBLH.B)*cos(rBLH.L);  E[5] = cos(rBLH.B)*sin(rBLH.L);  E[8] = sin(rBLH.B);
	Matrix enu = Matrix(E, 3, 3)*Matrix(r_vector, 3, 1);

	ENU.E = enu.get(0, 0); ENU.N = enu.get(1, 0); ENU.U = enu.get(2, 0);
	return true;
}
//检查观测值
bool CheckDatas(OEpochData &oData)
{
	if (oData.satsums < 4) return false;
	return true;
}
//计算卫星高度角，并对观测数据进行处理
bool Elevation(const Point &station, const SatPoint &satpoint, double elvation)
{
	ENUPoint enu;
	if (!XYZToENU(station, satpoint, enu)) return true;
	double az = pow(enu.E, 2) + pow(enu.N, 2) < 1E-12 ? 0.0 : atan2(enu.E, enu.N);
	if (az<0.0) az += 2 * PI;
	//卫星高度角
	double el = asin(enu.U);
	return el >elvation*PI / 180 ? true : false;
}
//对流层改正
double Trop(const Point &station, const SatPoint &satpoint)
{
	BLHPoint blh = PointToBLHPoint(station);
	ENUPoint enu;
	if (!XYZToENU(station, satpoint, enu)) return 0;
	/* temparature at sea level */
	const double temp0 = 15.0;
	double hgt, pres, temp, e, z, trph, trpw;

	if (blh.H < -100.0 || 1E4 < blh.B || blh.L <= 0) return 0.0;

	/* standard atmosphere */
	hgt = blh.H < 0.0 ? 0.0 : blh.H;

	pres = 1013.25*pow(1.0 - 2.2557E-5*hgt, 5.2568);
	temp = temp0 - 6.5E-3*hgt + 273.16;
	e = 6.108*0.5*exp((17.15*temp - 4684.0) / (temp - 38.45));

	/* saastamoninen model */
	z = PI / 2.0 - asin(enu.U);
	trph = 0.0022768*pres / (1.0 - 0.00266*cos(2.0*blh.B) - 0.00028*hgt / 1E3) / cos(z);
	trpw = 0.002277*(1255.0 / temp + 0.05)*e / cos(z);
	return trph + trpw;
}
//切比雪夫计算式
//n 多项式阶数
//x 返回Tn(x)
double Chebyshev(double x, int n)
{
	if (n == 0) return 1.0;
	else if (n == 1) return x;
	else return 2 * x * Chebyshev(x, n - 1) - Chebyshev(x, n - 2);
}

