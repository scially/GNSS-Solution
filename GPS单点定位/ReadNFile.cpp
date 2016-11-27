#include "ReadNFile.h"
#include <string>
#include "DataStruct.h"
#include "EString.h"
#include <iostream>
#include <fstream>

using namespace std;
ReadNFile::ReadNFile(std::string filename) :_filename(filename)
{
}

ReadNFile::ReadNFile()
{
}
ReadNFile::~ReadNFile()
{
}
ReadNFile::ReadNFile(const ReadNFile& nfile) :_filename(nfile._filename)
{
}
NFileHeader ReadNFile::ReadNHeader()
{
	NFileHeader header;
	ifstream fread(_filename);
	if (!fread.is_open())
	{
		cout << "文件打开失败!\n";
		exit(1);
	}
	string line;
	while (getline(fread, line))
	{
		if (line.find("COMMENT") != string::npos) continue;
		if (line.find("RINEX VERSION / TYPE") != string::npos)  //版本号，文件类型
		{
			header.version = EString::Trim(line.substr(0, 9));
			header.type = line[20];
		}
		if (line.find("ION ALPHA") != string::npos)
		{
			for (int i = 0; i < 4; i++)
			{
				header.IonAlpha[i] = stof((line.substr(2 + 12 * i, 12)));
			}
		}
		if (line.find("ION BETA") != string::npos)
		{
			for (int i = 0; i < 4; i++)
			{
				header.IonBeta[i] = stof(line.substr(2 + 12 * i, 12));
			}
		}
		if (line.find("DELTA-UTC: A0,A1,T,W") != string::npos)
		{
			header.DeltaUtaA[0] = stof(line.substr(3, 19));
			header.DeltaUtaA[1] = stof(line.substr(22, 19));
			header.DeltaUtcT = stof(line.substr(41, 9));
			header.DeltaUtcW = stof(line.substr(50, 9));
		}
		if (line.find("LEAP SECONDS") != string::npos)
		{
			header.LeapSeconds = stof(line.substr(0, 6));
		}
		if (line.find("END OF HEADER") != string::npos) break;
	}
	fread.close();
	return header;
}
vector<NFileRecord> ReadNFile::ReadNRecord()
{
	vector<NFileRecord> records;
	fstream fread(_filename);
	string line;
	//跳过文件头
	while (getline(fread, line))
	{
		if (line.find("END OF HEADER") != string::npos)
			break;
	}
	//读取导航文件内容
	while (getline(fread, line))
	{
		NFileRecord record;
		//PRN号/历元/卫星钟
		record.PRN = stoi(line.substr(0, 2)) < 10 ? "G0" + line.substr(1, 1) : "G"+line.substr(0, 2);
		record.TOC.year = stoi(line.substr(3, 2)) >= 0 && stoi(line.substr(3, 2)) < 80 ? stoi(line.substr(3, 2)) + 2000 : stoi(line.substr(3, 2)) + 1900;
		record.TOC.month = stoi(line.substr(6, 2));
		record.TOC.day = stoi(line.substr(9, 2));
		record.TOC.hour = stoi(line.substr(12, 2));
		record.TOC.minute = stoi(line.substr(15, 2));
		record.TOC.second = stod(line.substr(17, 5));
		record.ClkBias = stod(line.substr(22, 19));
		record.ClkDrift = stod(line.substr(41, 19));
		record.ClkDriftRate = stod(line.substr(60, 19));
		//广播轨道1
		getline(fread, line);
		record.IODE = stod(line.substr(3, 19));
		record.Crs = stod(line.substr(22, 19));
		record.DetaN = stod(line.substr(41, 19));
		record.M0 = stod(line.substr(60, 19));
		//广播轨道2
		getline(fread, line);
		record.Cuc = stod(line.substr(3, 19));
		record.e = stod(line.substr(22, 19));
		record.Cus = stod(line.substr(41, 19));
		record.sqrtA = stod(line.substr(60, 19));
		//广播轨道3
		getline(fread, line);
		record.TOE = stod(line.substr(3, 19));
		record.Cic = stod(line.substr(22, 19));
		record.OMEGA = stod(line.substr(41, 19));
		record.Cis = stod(line.substr(60, 19));
		//广播轨道4
		getline(fread, line);
		record.i0 = stod(line.substr(3, 19));
		record.Crc = stod(line.substr(22, 19));
		record.omega = stod(line.substr(41, 19));
		record.OMEGA_DOT = stod(line.substr(60, 19));
		//广播轨道5
		getline(fread, line);
		record.IDOT = stod(line.substr(3, 19));
		record.L2 = stod(line.substr(22, 19));
		record.GPSWeek =int(stod(line.substr(41, 19)));
		record.L2_P = stod(line.substr(60, 19));
		//广播轨道6
		getline(fread, line);
		record.SatAccuracy = stod(line.substr(3, 19));
		record.SatHealthy = stod(line.substr(22, 19));
		record.TGD = stod(line.substr(41, 19));
		record.IODC = stod(line.substr(60, 19));
		//广播轨道7
		getline(fread, line);
		record.TransmissionTimeofMessage = stod(line.substr(3, 19));
		record.FitInterval = atof(line.substr(22, 19).size() == 0 ? "0" : line.substr(22, 19).c_str());
		if (line.substr(41, 38).size() > 0)
		{
			record.Reamrk1 = atof(line.substr(41, 19).c_str());
			record.Remark2 = atof(line.substr(60, 19).c_str());
		}
		else
		{
			record.Reamrk1 = 0;
			record.Remark2 = 0;
		}
		records.push_back(record);
	}
	fread.close();
	return records;
}