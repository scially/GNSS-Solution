#include <iostream>
#include <stdlib.h>
#include "ReadFile.h"
#include "Matrix.h"
#include "Tools.h"
#include "Position.h"

using namespace std;
int main()
{

#if 1
	string n = "wuhn1230.16n";
	string o = "wuhn1230.16o";
	ReadFile r(o, n);
	vector<NFileRecord> nDatas = r._nfile.ReadNRecord();
	map<string, vector<LargnageCoeff>> m;
	LagrangeCoeff(nDatas, m);
	Time time(2016,5,2,4,0);
	SatPoint p;	
	Lagrange_SatPosition(time, "G10", m, p);
	cout << fixed<<setprecision(3) << p.point.x << " " << setw(20) << p.point.y << " " << setw(20)<< p.point.z << endl;
#endif
#if 0
	string n = "wuhn1230.16n";
	string o = "wuhn1230.16o";
	ReadFile r(o, n);
	vector<NFileRecord> nDatas = r._nfile.ReadNRecord();
	map<string, ChebyCoeff> MC;
	CoefficientofChebyShev(nDatas, MC);
	Time time(2016,5,2,5);
	SatPoint p;	
	ChebyShev_SatPosition(time, "G10",MC,p);
	cout << fixed<<setprecision(3) << p.point.x << " " << setw(20) << p.point.y << " " << setw(20)<< p.point.z << endl;
#endif

#if	0  //伪据定位
	string o = "wuhn1230.16o";
	string n = "wuhn1230.16n";
	string w = "spp.pos";
	ReadFile reader(o, n);
	if (OutputResult(reader,w,15))
	{
		cout << "计算完毕！" << endl;
	}
#endif
	system("pause");
	return 0;
}
