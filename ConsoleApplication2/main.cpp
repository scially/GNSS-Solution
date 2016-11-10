#include <iostream>
#include <stdlib.h>
#include "ReadFile.h"
#include "Matrix.h"
#include "Tools.h"
#include "Position.h"
using namespace std;
int main(){
	//测试数据读取
	/*string o= "D:\\系统文件\\桌面\\o文件\\unb31090.09o";
	string n = "D:\\系统文件\\桌面\\o文件\\unb31090.09n";
	ReadFile read = ReadFile(o, n);
	vector<OEpochData> odata = read._ofile.ReadData();
	vector<NFileRecord> ndata = read._nfile.ReadNRecord();*/

	//测试矩阵库
	/*double m[][3] = { { 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 }, { 7.0, 8.0, 8.0 } };
	Matrix M1(*m, 3, 3);
	cout<<M1.Inverse();*/

	//测试时间转换
	/*Time time;
	time.year = 2007;
	time.month = 10;
	time.day = 26;
	time.hour = 9;
	time.minute = 30;
	time.second = 0;
	cout.precision(16);
	cout << UTC2JD(time)<<endl;
	system("pause");*/

	string o = "d:\\系统文件\\桌面\\o文件\\site247j.01o";
	string n = "d:\\系统文件\\桌面\\o文件\\site247j.01n";
	//string o = "D:\\系统文件\\桌面\\o文件\\unb31090.09o";
	//string n = "D:\\系统文件\\桌面\\o文件\\unb31090.09n";
	string w = "D:\\系统文件\\桌面\\spp.pos";
	ReadFile reader(o, n);
	if (OutputResult(reader,w))
	{
		cout << "计算完毕！" << endl;
	}
	system("pause");
	return 0;
}