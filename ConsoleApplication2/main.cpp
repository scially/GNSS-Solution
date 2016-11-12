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
	//double m[][3] = { { 1.0, 2.0, 3.0 }, { 4.0, 5.0, 6.0 }, { 7.0, 8.0, 8.0 } };
	//Matrix M1(*m, 3, 3);
	//Matrix M2(3,3);
	//M2 = M1*M1;
	//cout << M2;

	string o = "SITE247J.01O";
	string n = "SITE247J.01N";
	//string o = "unb31090.09o";
	//string n = "o文件\\unb31090.09n";
	string w = "spp.pos";
	ReadFile reader(o, n);
	if (OutputResult(reader,w,15))
	{
		cout << "计算完毕！" << endl;
	}
	system("pause");
	return 0;
}