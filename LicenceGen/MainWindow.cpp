#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <fstream>
#include <string>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/core/persistence.hpp>
#include <QString>
#include <vector>

using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ButtonClicked()
{
	string strSerial;
	fstream fReader;
	string strHigh;
	string strLow;
	string strKey;
	uint64_t uiKey = 0;
	stringstream stream;
	
	fReader.open("/sys/firmware/devicetree/base/serial-number", ios::in);
	if (fReader.is_open()) {
		getline(fReader, strSerial);
		fReader.close();
	}
	if ("" != strSerial)
	{		
		uint64_t uiNumber = ui->txtSerial->text().toLong();
		
		strHigh = strSerial.substr(8, 4);
		strLow = strSerial.substr(12, 4);
	
		uiKey = stoll(strHigh, 0, 16) << 16;
		uiKey += stoll(strLow, 0, 16);
		
		uint64_t ulResult = uiKey * uiNumber;
		uint32_t uiFirst = ulResult & 0xFFFFFFFF;
		uint32_t uiSecond = ulResult >> 32;
		string str1, str2, str3;
		
		stream << std::hex << uiFirst;
		str1 = stream.str();
		if (8 > str1.length())
		{
			int left = 8 - str1.length();
			string tmp = "";
			for (int i = 0; i < left; i++)
				tmp += "0";
			str1 = tmp + str1;
		}
		stream = stringstream();
		stream << std::hex << (uint32_t)uiNumber;
		str2 = stream.str();
		if (8 > str2.length())
		{
			int left = 8 - str2.length();
			string tmp = "";
			for (int i = 0; i < left; i++)
				tmp += "0";
			str2 = tmp + str2;
		}
		stream = stringstream();
		stream << std::hex << uiSecond;
		str3 = stream.str();
		if (8 > str3.length())
		{
			int left = 8 - str3.length();
			string tmp = "";
			for (int i = 0; i < left; i++)
				tmp += "0";
			str3 = tmp + str3;
		}
		
		strKey = str1 + str2 + str3;
		
		str1 = "";
		str2 = "";
		str3 = "";
		
		int i = 0;
		while (i < strKey.length())
		{
			str1.push_back(strKey[i++]);
			str2.push_back(strKey[i++]);
			str3.push_back(strKey[i++]);
		}
		
		
		string strPath = "/home/pi/.config/.lic.xml";
		FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
		fs_SaveStream << "Licence" << "{";
		fs_SaveStream << "x" << str1;
		fs_SaveStream << "y" << str2;
		fs_SaveStream << "z" << str3;
		fs_SaveStream << "}";
		fs_SaveStream.release(); 
		
	}
	else
	{
		QMessageBox::warning(this,
			tr("Error"),
			tr("Memory corrupted."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
	
}
