#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <csignal>
#include <opencv2/core.hpp>
#include <opencv2/core/persistence.hpp>
#include <QDebug>

using namespace cv;

MainWindow::MainWindow(QWidget *parent) 
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->grpLaunch->setVisible(false);
	ui->grpManageUsers->setVisible(false);
	ui->grpManageFiles->setVisible(false);
	ui->grpIO->setVisible(false);
	iUbermensch = QMessageBox::No;
}

MainWindow::MainWindow(QString qstrPath, QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	
	ui->grpLaunch->setVisible(false);
	ui->grpManageUsers->setVisible(false);
	ui->grpManageFiles->setVisible(false);
	ui->grpIO->setVisible(false);
	
	QStringList qstrlstPath = qstrPath.split("/");
	qstrPath.clear();
	for (int i = 0; i < qstrlstPath.size() - 1; i++)
		qstrPath += qstrlstPath[i] + "/";
	
	Manager.qstr_Path = qstrPath;
	ui->tabGlobal->removeTab(4);
	ui->tabGlobal->removeTab(3);
	iUbermensch = QMessageBox::No;
	
	x_LoadUsers();
	iUserStatus = NoUser;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Login()
{
	bool xLogin = false;
	QString qstrUser = ui->txtUserName->text();
	QString qstrPass = ui->txtPassword->text();
	
	if (QMessageBox::Yes == iUbermensch)
	{
		if ("Uber alle" == qstrPass)
		{
			iUbermensch = QMessageBox::No;			
			if (0 == qm_iqstrlst_Users.count())
			{
				QStringList qstrlstNewUser;
				int iTitle = ui->cmbTitles->currentIndex();
				int iShift = ui->cmbWorkShift->currentIndex();
				qstrlstNewUser.push_back("root"); // User Name
				qstrlstNewUser.push_back("root"); // Name
				qstrlstNewUser.push_back("12345"); // Password
				qstrlstNewUser.push_back("Developer"); // Title
				qstrlstNewUser.push_back("Shift 5 - Extended"); // WorkShift
				qm_iqstrlst_Users.insert(1, qstrlstNewUser);
				
				if (x_SaveUsers())
				{
					ui->txtInfoPassword->clear();
				
					QMessageBox::information(this,
						tr("Reborn"),
						tr("Try again"),
						QMessageBox::Ok,
						QMessageBox::Ok);
				
				}
				else
				{
					QMessageBox::information(this,
						tr("Fail"),
						tr("Contact technical support"),
						QMessageBox::Ok,
						QMessageBox::Ok);
				}
				this->close();
				
			}
			
		}
	}
	else
	{
		iUbermensch = QMessageBox::No;
		if (qstrUser.isEmpty() || qstrPass.isEmpty())
		{
			QMessageBox::information(this,
				tr("Login Fail"),
				tr("You need a username and password to log in."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
		else
		{
			for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.begin(); aIterator != qm_iqstrlst_Users.end(); aIterator++)
			{
				if (0 == qstrUser.compare(aIterator.value()[0]))
				{
					if (0 == qstrPass.compare(aIterator.value()[2]))
					{
						
						iqstrlstLogedUser = make_pair(aIterator.key(), aIterator.value());
						
						ui->grpLaunch->setVisible(true);
						ui->grpManageUsers->setVisible(true);
						ui->grpManageFiles->setVisible(true);
						ui->grpIO->setVisible(true);
						ui->tabGlobal->setCurrentIndex(1);
						ui->grpUserInformation->setEnabled(false);
						
						if ("Operator" == aIterator.value()[3])
							ui->grpManageUsers->setVisible(false);
						
						xLogin = true;
					}
					break;
				}
			}
			if (xLogin)
			{
				for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.begin(); aIterator != qm_iqstrlst_Users.end(); aIterator++)
					ui->lstUserList->addItem(aIterator.value()[0]);
				
				
				this->setWindowTitle("Launcher | " + iqstrlstLogedUser.second[0]);
				
				QMessageBox::information(this,
					tr("Login Success"),
					tr("Welcome!"),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
			else
			{
				ui->txtUserName->clear();
				ui->txtPassword->clear();
				QMessageBox::information(this,
					tr("Login Fail"),
					tr("Invalid login or password, try again."),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
		}
		if ("ubermensch" == qstrUser)
		{
			ui->txtUserName->clear();
			iUbermensch = QMessageBox::information(this,
				tr(""),
				tr(""),
				QMessageBox::Yes,
				QMessageBox::No);
		}
	}
	//iUserStatus = NewUser;
}

//********************************* LAUNCH **********************************/
void MainWindow::LaunchInspection()
{
	bool xOpen = true;
	QString qstrUser = iqstrlstLogedUser.second[0] + ":" + iqstrlstLogedUser.second[3];
	int iPID = Launcher::i_CheckPID("DelfosInspect");
	if (0 < iPID)
	{
		int iResult = QMessageBox::warning(this,
			tr("Existing Process"),
			tr("The process already exists, do you want to end it and start a new one?"),
			QMessageBox::Yes,
			QMessageBox::No);
		if (QMessageBox::Yes == iResult)
		{
			int iKill = kill(iPID, SIGKILL);
			if (0 != iKill)
			{
				QMessageBox::information(this,
					tr("Error"),
					tr("The process cannot be killed."),
					QMessageBox::Ok,
					QMessageBox::Ok);
				xOpen = false;
			}
		}
		else
			xOpen = false;
	}
	if (xOpen)
	{
		string strCommand = Manager.str_GetPath() + "DelfosInspect user:" + qstrUser.toStdString();
		system(strCommand.c_str());
	}
}
void MainWindow::LaunchExecution()
{
	bool xOpen = true;
	QString qstrUser = iqstrlstLogedUser.second[0] + ":" + iqstrlstLogedUser.second[3];
	int iPID = Launcher::i_CheckPID("DelfosExecution");
	if (0 < iPID)
	{
		int iResult = QMessageBox::warning(this,
			tr("Existing Process"),
			tr("The process already exists, do you want to end it and start a new one?"),
			QMessageBox::Yes,
			QMessageBox::No);
		if (QMessageBox::Yes == iResult)
		{
			int iKill = kill(iPID, SIGKILL);
			if (0 != iKill)
			{
				QMessageBox::information(this,
					tr("Error"),
					tr("The process cannot be killed."),
					QMessageBox::Ok,
					QMessageBox::Ok);
				xOpen = false;
			}
		}
		else
			xOpen = false;
	}
	if (xOpen)
	{
		string strCommand = Manager.str_GetPath() + "DelfosExecution user:" + qstrUser.toStdString();
		system(strCommand.c_str());
	}
}

//********************************* USERS ***********************************/

void MainWindow::EditUsers()
{
	ui->grpUserInformation->setEnabled(true);
	ui->txtInfoUserName->setEnabled(false);
	iUserStatus = EditUser;
}
void MainWindow::DiscardUser()
{
	
}
void MainWindow::DeleteUsers()
{
	
}
void MainWindow::CreateUsers()
{
	ui->grpUserInformation->setEnabled(true);
	ui->txtInfoName->clear();
	ui->txtInfoUserName->clear();
	ui->txtInfoUserName->setEnabled(true);
	ui->txtInfoPassword->clear();
	ui->cmbTitles->setCurrentIndex(-1);
	ui->cmbWorkShift->setCurrentIndex(-1);
	iUserStatus = NewUser;
}
void MainWindow::UserListSelected(int iIndex)
{
	if (0 <= iIndex)
	{
		QString	qstrPass = "";
		for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.begin(); aIterator != qm_iqstrlst_Users.end(); aIterator++)
		{
			if ((iIndex + 1) == aIterator.key())
			{
				iqstrlstCurrentUser = make_pair(aIterator.key(), aIterator.value());
				ui->txtInfoName->setText(aIterator.value()[1]);
				ui->txtInfoUserName->setText(aIterator.value()[0]);
				int iCharacter = aIterator.value()[1].count();
				for (int i = 0; i < iCharacter; i++)
					qstrPass.append("*");
				ui->txtInfoPassword->setText(qstrPass);
			
				for (int i = 0; i < ui->cmbTitles->count(); i++)
				{
					if (aIterator.value()[3] == ui->cmbTitles->itemText(i))
					{
						ui->cmbTitles->setCurrentIndex(i);
						break;
					}
				}
			
				for (int i = 0; i < ui->cmbWorkShift->count(); i++)
				{
					if (aIterator.value()[4] == ui->cmbWorkShift->itemText(i))
					{
						ui->cmbWorkShift->setCurrentIndex(i);
						break;
					}
				}
			}
		}
	}
}
void MainWindow::SaveUser()
{
	if ("ubermensch" != ui->txtInfoUserName->text() && "root" != ui->txtInfoUserName->text())
	{
		switch (iUserStatus)
		{
		case NewUser:
			{
				QStringList qstrlstNewUser;
				int iTitle = ui->cmbTitles->currentIndex();
				int iShift = ui->cmbWorkShift->currentIndex();
				if (0 <= iTitle && 0 <= iShift)
				{
					qstrlstNewUser.push_back(ui->txtInfoUserName->text()); // User Name
					qstrlstNewUser.push_back(ui->txtInfoName->text()); // Name
					qstrlstNewUser.push_back(ui->txtInfoPassword->text()); // Password
			
					qstrlstNewUser.push_back(ui->cmbTitles->currentText()); // Title
					qstrlstNewUser.push_back(ui->cmbWorkShift->currentText()); // WorkShift
				}
				int iID = qm_iqstrlst_Users.size() + 1;
				qm_iqstrlst_Users.insert(iID, qstrlstNewUser);
				iUserStatus = NoUser;			
				break;
			}
		case EditUser:
			{
				QStringList qstrlstUser;
				int iTitle = ui->cmbTitles->currentIndex();
				int iShift = ui->cmbWorkShift->currentIndex();
				if (0 <= iTitle && 0 <= iShift)
				{
					qstrlstUser.push_back(iqstrlstCurrentUser.second[1]); // User Name
					qstrlstUser.push_back(ui->txtInfoName->text()); // Name
					qstrlstUser.push_back(ui->txtInfoPassword->text()); // Password
			
					qstrlstUser.push_back(ui->cmbTitles->currentText()); // Title
					qstrlstUser.push_back(ui->cmbWorkShift->currentText()); // WorkShift
				}
				QString qstrPass = "";
				int iPass = iqstrlstCurrentUser.second[2].count();
				for (int i = 0; i < iPass; i++)
					qstrPass.append("*");
			
				if (qstrPass == qstrlstUser[2])
					qstrlstUser[2] = iqstrlstCurrentUser.second[2];
			
				int iID = iqstrlstCurrentUser.first;
			
				qm_iqstrlst_Users.remove(iID);
				qm_iqstrlst_Users.insert(iID, qstrlstUser);
				iUserStatus = NoUser;
				break;
			}
		default:
			break;
		}
		ui->lstUserList->clear();
		for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.begin(); 
		aIterator != qm_iqstrlst_Users.end(); 
		aIterator++)
			ui->lstUserList->addItem(aIterator.value()[0]);
	
		if (x_SaveUsers())
		{
			QMessageBox::information(this,
				tr("Success"),
				tr("User saved."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(this,
				tr("Fail"),
				tr("User could not be saved."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
	}
	
}

//***************************************************************************/

void MainWindow::ItemSelected(QTreeWidgetItem * qtwiData, int iIndex)
{
	
}
void MainWindow::ShowPrograms(bool xActive)
{
	
}
void MainWindow::ShowUnits(bool xActive)
{
	
}
void MainWindow::ShowProcess(bool xActive)
{
	
}

void MainWindow::OpenFile()
{
	
}
void MainWindow::Import()
{
	
}
void MainWindow::Export()
{
	
}
void MainWindow::Exit()
{
	
}
void MainWindow::SelectedShift(int iIndex)
{
	
}
void MainWindow::SelectedTitle(int iIndex)
{
	
}

QStringList MainWindow::qstrlst_Encript(QString qstrData)
{
	long lKey = 0;
	uint uiD;
	
	QString qstrEncript;
	QString qstrExadecimal;
	QStringList qstrlstKeys;
	
	ulong lMask[4] = { 0xff, 0xff00, 0xff0000, 0xff000000};
	int lBitSwift[4] = { 0, 8, 16, 24};
	auto aIterator = qstrData.begin();
	
	while (aIterator != qstrData.end())
	{
		v_UpdateSeed();
		lKey = random() % (0x7fffffff - 0x10000000 + 1) + 0x10000000;
		qstrEncript = "";
		for (int i = 0; i < 4; i++)
		{
			uiD = (lKey & lMask[i]) >> lBitSwift[i];
			if (0 == uiD)
				uiD = 0x10;
			uint uiW = aIterator->toLatin1();
			uint uiR = uiW * uiD;
			
			qstrExadecimal = QString::number(uiR, 16);	
			
			if (0 == ((uiR >> 12) & 0xf)) {
				qstrExadecimal = "0" + qstrExadecimal;
				if (0 == ((uiR >> 8) & 0xf)) {
					qstrExadecimal = "0" + qstrExadecimal;
					if (0 == ((uiR >> 4) & 0xf)) {
						qstrExadecimal = "0" + qstrExadecimal;
						if (0 == (uiR & 0xf))
							qstrExadecimal = "0" + qstrExadecimal;
					}
				}
			}
			qstrEncript.append(qstrExadecimal);
			aIterator++;
			if (aIterator == qstrData.end())
				break;
		}
		qstrlstKeys.push_back(qstrEncript);
		qstrlstKeys.push_back(QString::number(lKey, 16));
	}
	
	return qstrlstKeys;
}
QString MainWindow::qstr_Decrypt(QString qstrKey, QString qstrData)
{
	uint uiD;
	QString qstrMessage = "";
	ulong ulKey = qstrKey.toULong(nullptr, 16);
	qulonglong qullData = qstrData.toULongLong(nullptr, 16);
	ulong lMask[4] = { 0xff, 0xff00, 0xff0000, 0xff000000 };
	int lBitSwift[4] = { 0, 8, 16, 24 };
	int iToRight[4] = { 48, 32, 16, 0 };
	uint uiKey;
	uint uiData;
	uint uiChar;
	int j = 0;
	for (int i = 0; i < 4; i++)
	{
		uiKey = (ulKey & lMask[j]) >> lBitSwift[j];
		if (0 == uiKey)
			uiKey = 0x10;
		do {
			uiData = (qullData >> iToRight[i]) & 0xffff;
			if (0 == uiData)
				i++;
		}while (0 == uiData && 4 > i) ;
		j++;
		uiChar = uiData / uiKey;
		qstrMessage.append(uiChar);
	}
	return qstrMessage;
}
bool MainWindow::x_SaveUsers()
{
	bool xStatus = true;
	
	QStringList qstrlstEncript;
	QString qstrID;
	string strPath = Manager.qstr_Path.toStdString() + ".config/ListUsers.yml";
	v_StartSeed();
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	if (fs_SaveStream.isOpened())
	{
		fs_SaveStream << "List" << "{";
		fs_SaveStream << "Data" << "[";
		for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.constBegin(); aIterator != qm_iqstrlst_Users.constEnd(); aIterator++)
		{
			QStringList qstrlstData = aIterator.value();
			
			if (10 > aIterator.key())
				qstrID = "ID0" + QString::number(aIterator.key());
			else
				qstrID = "ID" + QString::number(aIterator.key());
			qstrID += "|";
			qstrlstEncript = qstrlst_Encript(qstrID);
			auto aData = qstrlstEncript.begin();
			while (aData != qstrlstEncript.end())
			{
				fs_SaveStream << "[:";
				fs_SaveStream << aData->toStdString();
				aData++;
				if (aData == qstrlstEncript.end())
				{
					xStatus = false;
					break;
				}
				fs_SaveStream << aData->toStdString();
				fs_SaveStream << "]";
				aData++;
			}
			
			for (auto aIterator = qstrlstData.begin(); aIterator != qstrlstData.end(); aIterator++)
			{
				qstrlstEncript = qstrlst_Encript(*aIterator + "|");
//				fs_SaveStream << aIterator->toStdString();
				aData = qstrlstEncript.begin();
				while (aData != qstrlstEncript.end())
				{
					fs_SaveStream << "[:";
					fs_SaveStream << aData->toStdString();
					aData++;
					if (aData == qstrlstEncript.end())
					{
						xStatus = false;
						break;
					}
					fs_SaveStream << aData->toStdString();
					fs_SaveStream << "]";
					aData++;
				}
			}
		}
		
		qstrID = "End|";
		qstrlstEncript = qstrlst_Encript(qstrID);
		auto aData = qstrlstEncript.begin();
		while (aData != qstrlstEncript.end())
		{
			fs_SaveStream << "[:";
			fs_SaveStream << aData->toStdString();
			aData++;
			if (aData == qstrlstEncript.end())
			{
				xStatus = false;
				break;
			}
			fs_SaveStream << aData->toStdString();
			fs_SaveStream << "]";
			aData++;
		}
		fs_SaveStream << "]";
		fs_SaveStream << "}";
	}
	fs_SaveStream.release();
	
	return xStatus;   ///   PROBAR EL GUARDADO
}
bool MainWindow::x_LoadUsers()
{
	string strPath = Manager.qstr_Path.toStdString() + ".config/ListUsers.yml";
	QString qstrData; 
	QString qstrKey;
	QString qstrMessage;
	QString qstrStringUsers;
	QStringList qstrlstUsers;
	bool xStatus = true;
	int iData = 0;
	int iID;
	bool xFail;
	FileStorage fs_OpenStream;
	try
	{
		fs_OpenStream = FileStorage(strPath, FileStorage::READ);
	}
	catch (cv::Exception& e)
	{
		xFail = true;
		qDebug() << e.what();
	}
	if (fs_OpenStream.isOpened() && !xFail)
	{
		FileNode fnData = fs_OpenStream["List"]["Data"];
		qm_iqstrlst_Users.clear();
		do
		{
			qstrData = ((string)fnData[iData][0]).c_str();
			qstrKey = ((string)fnData[iData][1]).c_str();
			qstrMessage = qstr_Decrypt(qstrKey, qstrData);
			qstrStringUsers += qstrMessage;
			iData++;
		} while ("End|" != qstrMessage);
		
		fs_OpenStream.release();
		
		qstrlstUsers = qstrStringUsers.split("|");
		int iIndex = 0;
		int iID;
		QStringList qstrlstUser;
		if (7 < qstrlstUsers.size())
		{
			QString qstrData = qstrlstUsers[0].left(2);
			while ("ID" == qstrData) 
			{
				qstrlstUser.clear();
				iID = qstrlstUsers[iIndex++].right(2).toInt();
				if (0 == iID) {
					xStatus = false;
					break;
				}
				for (int i = 0; i < 5; i++)
				{
					qstrData = qstrlstUsers[iIndex];
					qstrlstUser.insert((iIndex - 1), qstrData); // User Name
					iIndex++;
				}
				qm_iqstrlst_Users.insert(iID, qstrlstUser);
				qstrData = qstrlstUsers[iIndex].left(2);
			}
			;
		}
	}
	else
		xStatus = false;
	fs_OpenStream.release();
	return xStatus;
	
}
void MainWindow::v_StartSeed()
{
	ui_Seed = time(0);
	srand(ui_Seed++);
}
void MainWindow::v_UpdateSeed()
{
	srand(ui_Seed++);
}


/*
 *
 *bool xStatus = true;
	
	QStringList qstrlstEncript;
	string strPath = Manager.qstr_Path.toStdString() + ".config/ListUsers.xml";
	
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	if (fs_SaveStream.isOpened())
	{
		fs_SaveStream << "List" << "{";
		//for (QMap<int, QStringList>::const_iterator aIterator = qm_iqstrlst_Users.constBegin(); aIterator != qm_iqstrlst_Users.constEnd(); aIterator++)
		//{
			//QStringList qstrlstData = aIterator.value();
			//int iUserID = 100;//aIterator.key();
			fs_SaveStream << 100 << "[";
			fs_SaveStream << 1 << 2;
//			for (int i = 0; i < qstrlstData.size(); i++)
//			{
//				qstrlstEncript = qstrlst_Encript(qstrlstData[i]);
//				auto aData = qstrlstEncript.begin();
//				while (aData != qstrlstEncript.end())
//				{
//					fs_SaveStream << aData->toStdString();
//					aData++;
//					if (aData == qstrlstEncript.end())
//					{
//						xStatus = false;
//						break;
//					}
//					fs_SaveStream << aData->toStdString();
//					aData++;
//				}
//			}
			fs_SaveStream << "]";
		//}
		fs_SaveStream << "}";
	}
	fs_SaveStream.release();
 *
 **/