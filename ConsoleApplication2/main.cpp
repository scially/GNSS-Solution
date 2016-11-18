#include <iostream>
#include <stdlib.h>
#include "ReadFile.h"
#include "Matrix.h"
#include "Tools.h"
#include "Position.h"
using namespace std;
int main()
{
	string o = "SITE247J.01O";
	string n = "SITE247J.01N";
	//string o = "unb31090.09o";
	//string n = "unb31090.09n";
	string w = "spp.pos";
	ReadFile reader(o, n);
	if (OutputResult(reader,w,15))
	{
		cout << "¼ÆËãÍê±Ï£¡" << endl;
	}
	system("pause");
	return 0;
}