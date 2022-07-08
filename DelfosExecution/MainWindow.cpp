#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QString>
#include "../DelfosInspect/mat2qimage.h"
#include <opencv2/core/persistence.hpp>
#include <QFileDialog>
#include <thread>
#include <QPixmap>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	
}

MainWindow::MainWindow(QStringList qstrlstUserName, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	
	xInit = false;
	
	this->setWindowTitle(qstrlstUserName[0] + " | Delfos Execution");
	ui->lblPicture->setVisible(false);
	ui->grpConfigs->setEnabled(false);
	ui->grpConfigs->setVisible(false);
	ui->cmdRun->setVisible(false);
	ui->cmdStop->setVisible(false);
	ui->cmdClear->setVisible(false);
	ui->txtSerial->setEnabled(false);
	
	ui->cmdRelease->setVisible(false);
	ui->cmdRelease->setEnabled(false);
	ui->cmdLights->setVisible(false);
	qstrUser = qstrlstUserName[0];
	qstrManufacturerName = "A company";
	qstrManufacturerLocation = "Mexico";
	if ("Debug" == qstrlstUserName[0]) {
		qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
		ui->cmdDebug->setVisible(true);
		ui->cmdRelease->setVisible(true);
	}
	else
		ui->cmdDebug->setVisible(false);
	if ("Developer" == qstrlstUserName[0])
		ui->cmdRelease->setVisible(true);
	if ("Manager" == qstrlstUserName[0])
		ui->cmdRelease->setVisible(true);
	
	qRegisterMetaType<pair<QPixmap, vector<pair<int, Point>>>>("pair<QPixmap, vector<pair<int, Point>>>");
	qRegisterMetaType<InspectMachine>("InspectMachine");
	qRegisterMetaType<Size>("Size");
	
	// Conecta el slot de la ventana principal con una acción en el thread
	// MainWindow -> Thread
	connect(this,				SIGNAL(	sg_RunEngine(InspectMachine)), 
			&InspectionMachine,	SLOT(	sl_EngineInspection(InspectMachine)));
	connect(this,				SIGNAL(	sg_StopEngine()), 
			&InspectionMachine,	SLOT(	sl_StopEngine()));
	connect(this,				SIGNAL(	sg_Test()), 
			&InspectionMachine,	SLOT(	sl_Test()));
	connect(this,				SIGNAL(	sg_Readed()), 
			&InspectionMachine,	SLOT(	sl_Readed()));
	connect(this,				SIGNAL(	sg_Clear()),
			&InspectionMachine,	SLOT(	sl_Clear()));
	
	// Conecta un slot del thread con una señal de la ventana principal
	// Thread -> MainWindow
	connect(&InspectionMachine,	SIGNAL(	sg_ShowResult(pair<QPixmap, vector<pair<int, Point>>>)),
			this,				SLOT(	sl_ShowResult(pair<QPixmap, vector<pair<int, Point>>>)));
	connect(&InspectionMachine,	SIGNAL(	sg_ShowTime(double)),
			this,				SLOT(	sl_ShowTime(double)));
	connect(&InspectionMachine,	SIGNAL(	sg_ClearStatus()),
			this,				SLOT(	sl_ClearStatus()));
	connect(&InspectionMachine,	SIGNAL(	sg_NoFound()),
			this,				SLOT(	sl_NoFound()));
	connect(&InspectionMachine,	SIGNAL(	sg_Initialized()),
			this,				SLOT(	sl_Initialized()));
	connect(&InspectionMachine,	SIGNAL(	sg_NoFound()),
			this,				SLOT(	sl_NoFound()));
	connect(&InspectionMachine,	SIGNAL(	sg_Finished()),
			this,				SLOT(	sl_Finished()));
	connect(&InspectionMachine,	SIGNAL(	sg_SizeModel(Size)),
			this,				SLOT(	sl_SizeModel(Size)));
	
	InspectionMachine.start();
	string strOpenPath = "/home/pi/battery/default/default.yml";
	FileStorage fs_OpenStream("OpenPath.xml", FileStorage::READ);
	if (fs_OpenStream.isOpened())
	{
		fs_OpenStream["Path"] >> strOpenPath;
	}
	fs_OpenStream.release();
	ui->txtPathFile->setText(strOpenPath.c_str());
	
	if ("Developer" == qstrlstUserName[1])
		ui->cmdConfigs->setVisible(true);
	else
		ui->cmdConfigs->setVisible(false);
	
}
MainWindow::~MainWindow()
{
    delete ui;
	
	// Le decimos al bucle de mensajes del hilo que se detenga
	InspectionThread.quit();
	// Ahora esperamos a que el hilo de trabajo termine
	InspectionThread.wait();
	
}
void MainWindow::OpenFile()
{
	if(Filter::chkStatus())
	{
		ptr_Machine = new InspectMachine();
		ulong ulFault = ptr_Machine->ul_LoadFile(ui->txtPathFile->text().toStdString());
		
		if (0 == ulFault)
		{
			FileStorage fs_SaveStream("OpenPath.xml", FileStorage::WRITE);
			fs_SaveStream << "Path" << ui->txtPathFile->text().toStdString();
			fs_SaveStream.release();
			
			QStringList qstrlstData = ui->txtPathFile->text().split("/");
			
			QString Name = qstrlstData[qstrlstData.count() - 1];
			
			this->setWindowTitle(this->windowTitle() + " | " + Name);
			
			ui->lblPicture->setVisible(true);
			ui->cmdOpen->setVisible(false);
			ui->txtPathFile->setVisible(false);
		
			map<string, int> mstriIO = ptr_Machine->GetIO();
		
			auto aIterator = mstriIO.find("UP");
			ui->txtLightUp->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("DOWN");
			ui->txtLightDown->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("LEFT");
			ui->txtLightLeft->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("RIGHT");
			ui->txtLightRight->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("SENSORU");
			ui->txtSensorUnit->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("STOPPER");
			ui->txtSolenoid->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("BUTTON");
			ui->txtButton->setText(QString::number(aIterator->second));
			aIterator = mstriIO.find("WINDOWS");
			ui->txtWindows->setText(QString::number(aIterator->second));

			ui->cmdRun->setVisible(true);
			ui->cmdOpen->setVisible(false);
			ui->cmdFind->setVisible(false);
			xInit = true;
			ui->cmdLights->setVisible(true);
		
			if (ui->cmdRelease->isVisible())
				ui->cmdRelease->setEnabled(true);
			
			QMessageBox::information(this,
				tr("Success"),
				tr("Resources have been loaded correctly."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
		else
		{		
			QMessageBox::information(this,
				tr("Load Failed"),
				"Check the resource files, data: 0x" +
				QString::number(ulFault,16),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
	}
	else
	QMessageBox::warning(this,
		tr("Error"),
		tr("Memory access forbidden."),
		QMessageBox::Ok,
		QMessageBox::Ok);
}
void MainWindow::Run()
{
	if (Filter::chkStatus())
	{
		if (false == xInit )
		{
			ptr_Machine = new InspectMachine();
			ulong ulFault = ptr_Machine->ul_LoadFile(ui->txtPathFile->text().toStdString());
			if (0 == ulFault)
			{
				ui->lblPicture->setVisible(true);
		
				map<string, int> mstriIO = ptr_Machine->GetIO();
		
				auto aIterator = mstriIO.find("UP");
				ui->txtLightUp->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("DOWN");
				ui->txtLightDown->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("LEFT");
				ui->txtLightLeft->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("RIGHT");
				ui->txtLightRight->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("SENSORU");
				ui->txtSensorUnit->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("STOPPER");
				ui->txtSolenoid->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("BUTTON");
				ui->txtButton->setText(QString::number(aIterator->second));
				aIterator = mstriIO.find("WINDOWS");
				ui->txtWindows->setText(QString::number(aIterator->second));
				xInit = true;
			}
			else
			{		
				QMessageBox::information(this,
					tr("Load Failed"),
					"Check the resource files, data: 0x" +
					QString::number(ulFault,16),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
		}
		ui->cmdLights->setEnabled(false);
		ptr_Machine->v_LightOff();
		ui->txtSerial->setEnabled(true);
		ui->txtSerial->setFocus();
		if (xInit)
			emit sg_RunEngine(*ptr_Machine);
	}
	else
		QMessageBox::warning(this,
			tr("Error"),
			tr("Memory access forbidden."),
			QMessageBox::Ok,
			QMessageBox::Ok);
		
}
void MainWindow::Stop()
{
	emit sg_StopEngine();
	ui->txtSerial->setEnabled(false);
	ui->cmdLights->setEnabled(true);
}
void MainWindow::Find()
{
	QString qstrPath = QFileDialog::getOpenFileName(this,
		tr("Open Model"),
		"/home/pi/battery",
		tr("Config Files (*.yml)"));
	ui->txtPathFile->setText(qstrPath);
}
void MainWindow::Configs()
{
	ui->grpConfigs->setVisible(!ui->grpConfigs->isVisible());
	ui->txtSerial->setFocus();
}
void MainWindow::Debug()
{
	emit sg_Test();
}
void MainWindow::Lights(bool xCheck)
{
	if (xCheck)
		ptr_Machine->v_LightOn();
	else
		ptr_Machine->v_LightOff();
	ui->txtSerial->setFocus();
}
void MainWindow::Release(bool xCheck)
{
	int STOPPER = ptr_Machine->GetIO().find("STOPPER")->second;
	if (xCheck)
		digitalWrite(STOPPER, HIGH);
	else
		digitalWrite(STOPPER, LOW);
	ui->txtSerial->setFocus();
}
void MainWindow::SendSerial()
{
	if (!ui->txtSerial->text().isEmpty())
	{
		ui->txtSerial->setEnabled(false);
		emit sg_Readed();
		ui->cmdClear->setVisible(true);
	}
}
void MainWindow::Clear()
{
	emit sg_Clear();
	ui->txtSerial->setEnabled(true);
	ui->cmdClear->setVisible(false);
	ui->txtSerial->setFocus();
}
void MainWindow::sl_ShowResult(const pair<QPixmap, vector<pair<int, Point>>>& data)
{
	ui->lblPicture->setPixmap(data.first);
	
	QFont font;
	font.setPointSize(120);
	ui->lblStatus->setFont(font);
	
	if (0 == data.second.size())
	{
		ui->lblStatus->setText("PASS");
		QPalette palette;
		QBrush brush1(GlobalColor::green);
		brush1.setStyle(Qt::SolidPattern);
		palette.setBrush(QPalette::Active, QPalette::WindowText, brush1);
		ui->lblStatus->setPalette(palette);
	}
	else
	{
		ui->lblStatus->setText("FAIL");
		QPalette palette;
		QBrush brush1(GlobalColor::red);
		brush1.setStyle(Qt::SolidPattern);
		palette.setBrush(QPalette::Active, QPalette::WindowText, brush1);
		ui->lblStatus->setPalette(palette);
		
		ui->lstErrors->clear();
		for (auto aIterator = data.second.begin(); 
		aIterator != data.second.end(); aIterator++)
		{
			if (0 > aIterator->first)
				ui->lstErrors->addItem(QString::number(aIterator->second.x) + 
					" counted units at row " + QString::number(aIterator->second.y));					
			else
				ui->lstErrors->addItem(QString::number(aIterator->first) +
					"|X:" + QString::number(aIterator->second.x) +
					" Y:" + QString::number(aIterator->second.y));
		}
		
	}
}
void MainWindow::sl_ShowTime(double dEnlapsedTime)
{
	ui->lblTime->setText(QString::number(dEnlapsedTime) + " seconds");
	string strLogTime;
	
	int iCount = 0;
	time_t ttime = time(0);
	tm *local_time = localtime(&ttime);
	
	int iTmp = local_time->tm_mon + 1;
	string strMonth = to_string(iTmp);
	if (10 > iTmp)
		strMonth = "0" + strMonth;
	iTmp = local_time->tm_mday;
	string strDay = to_string(iTmp);
	if (10 > iTmp)
		strDay = "0" + strDay;
	iTmp = local_time->tm_year-100;
	string strYear = to_string(iTmp);
	if (10 > iTmp)
		strYear = "0" + strYear;

	
	iTmp = local_time->tm_hour;
	string strHour = to_string(iTmp);
	if (10 > iTmp)
		strHour = "0" + strHour;
	iTmp = local_time->tm_min;
	string strMin = to_string(iTmp);
	if (10 > iTmp)
		strMin = "0" + strMin;
	iTmp = local_time->tm_sec;
	string strSec = to_string(iTmp);
	if (10 > iTmp)
		strSec = "0" + strSec;
	
	string strCommand = "/home/pi/battery/.log/log-UUT.xml";
	string strLongDate = strGetDay(local_time->tm_wday) + ", " +
		strGetMonth(local_time->tm_mon) + " " + to_string(local_time->tm_mday) + 
		", " + to_string(1900 + local_time->tm_year);
	strLogTime = to_string(local_time->tm_hour) + ":" + 
		to_string(local_time->tm_min) + ":" + to_string(local_time->tm_sec);
		
	FileStorage fs_LogStream(strCommand, FileStorage::WRITE);
	fs_LogStream << "Sation_ID" << "TEST_STATION";
	fs_LogStream << "Serial_Number" << ui->txtSerial->text().toStdString();
	fs_LogStream << "Date" << strLongDate;
	fs_LogStream << "Time" << strLogTime;
	fs_LogStream << "Operator" << qstrUser.toStdString();
	fs_LogStream << "Execution_Time" << to_string(dEnlapsedTime) + " seconds";
	fs_LogStream << "Number_of_Results" << ui->lstErrors->count();
	fs_LogStream << "UUT_Result" << ui->lblStatus->text().toStdString();
	fs_LogStream << "Manufacter_Name" << qstrManufacturerName.toStdString();
	fs_LogStream << "Manufacter_Location" << qstrManufacturerLocation.toStdString();
	fs_LogStream.release();
	
	strLogTime = strHour + strMin + strSec;
	
	strCommand = "~/battery/createLog.sh " + ui->txtSerial->text().toStdString();
	system(strCommand.c_str());
	
	ui->txtSerial->setText("");
	ui->txtSerial->setEnabled(true);
	ui->cmdClear->setVisible(false);
	ui->txtSerial->setFocus();
}
void MainWindow::sl_ClearStatus()
{
	QFont font;
	font.setPointSize(80);
	ui->lblStatus->setFont(font);
	ui->lblPicture->clear();
	ui->lblStatus->setText("PROCESS");
	QPalette palette;
	QBrush brush1(GlobalColor::darkBlue);
	brush1.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::WindowText, brush1);
	ui->lblStatus->setPalette(palette);
	ui->lstErrors->clear();
	ui->lblTime->clear();
	ui->cmdClear->setVisible(false);
	
}
void MainWindow::sl_NoFound()
{
	ui->lblStatus->setText("FAULT");
	QPalette palette;
	QBrush brush1(GlobalColor::darkRed);
	brush1.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::WindowText, brush1);
	ui->lblStatus->setPalette(palette);
}
void MainWindow::sl_Initialized()
{
	ui->cmdRun->setText("Online");
	ui->cmdRun->setEnabled(false);
	ui->cmdStop->setVisible(true);
}
void MainWindow::sl_SizeModel(Size sSize)
{
	ui->lblNumberRows->setText(QString::number(sSize.height)); // y alto de la bateria
	ui->lblNumberColumns->setText(QString::number(sSize.width));
}
void MainWindow::sl_Finished()
{
	ui->cmdStop->setVisible(false);
	ui->cmdRun->setText("Run");
	ui->cmdRun->setEnabled(true);
	xInit = false;
}
string MainWindow::strGetDay(int iDay)
{
	string strLongDate = "";
	switch (iDay)
	{
	case 0:
		strLongDate = "Sunday, ";
		break;
	case 1:
		strLongDate = "Monday, ";
		break;
	case 2:
		strLongDate = "Tuesday, ";
		break;
	case 3:	
		strLongDate = "Wednesday, ";
		break;
	case 4:	
		strLongDate = "Thursday, ";
		break;
	case 5:	
		strLongDate = "Friday, ";
		break;
	case 6:	
		strLongDate = "Saturday, ";
		break;
	default:
		break;
	}
	return strLongDate;
}
string MainWindow::strGetMonth(int iMonth)
{
	string strLongDate = "";
	switch (iMonth)
	{
	case 0:
		strLongDate = "January ";
		break;
	case 1:
		strLongDate = "February ";
		break;
	case 2:
		strLongDate = "March ";
		break;
	case 3:
		strLongDate = "April ";
		break;
	case 4:
		strLongDate = "May ";
		break;
	case 5:
		strLongDate = "June ";
		break;
	case 6:
		strLongDate = "July ";
		break;
	case 7:
		strLongDate = "August ";
		break;
	case 8:
		strLongDate = "September ";
		break;
	case 9:
		strLongDate = "October ";
		break;
	case 10:
		strLongDate = "November ";
		break;
	case 11:
		strLongDate = "December ";
		break;
	default:
		break;
	}
	return strLongDate;
}




//void MainWindow::v_FillErrors(vector<tuple<int, Point>> vipDataErrors)
//{
//	int iError;
//	Point pLocation;
//	ui->lstErrors->clear();
//	for (auto aIterator = vipDataErrors.begin(); 
//	aIterator != vipDataErrors.end(); aIterator++)
//	{
//		tie(iError, pLocation) = *aIterator;
//		if (0 > iError)
//			ui->lstErrors->addItem(QString::number(pLocation.x) + 
//				" counted units at row " + QString::number(pLocation.y));					
//		else
//			ui->lstErrors->addItem(QString::number(iError) +
//				"|X:" + QString::number(pLocation.x) +
//				" Y:" + QString::number(pLocation.y));
//	}
//}