#include <iostream>
#include <stdlib.h>
#include "ReadFile.h"
#include "Matrix.h"
#include "Tools.h"
#include "Position.h"

using namespace std;
int main()
{
	string o = "wuhn1230.16o";
	string n = "wuhn1230.16n";
	string w = "spp.pos";
	ReadFile reader(o, n);
	if (OutputResult(reader,w,15))
	{
		cout << "¼ÆËãÍê±Ï£¡" << endl;
	}
	system("pause");
	return 0;
}
