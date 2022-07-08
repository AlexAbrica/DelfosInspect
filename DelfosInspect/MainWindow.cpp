#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <chrono>
#include <thread>
#include <string>
#include "cCamera.h"
#include <dirent.h>
#include <QFileDialog>
#include <QCursor>

using namespace std;
using namespace cv;

/********************* MAIN WINDOW CLASS *******************/
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::MainWindow(QStringList qstrlstUserName, 
	QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	//ui->lblColorBox->setVisible(false);
	//ui->cmdColorBox->setVisible(false);
	ui->lblSetFilterHI1->setVisible(false);
	ui->lblSetFilterHI2->setVisible(false);
	ui->lblSetFilterHI3->setVisible(false);
	ui->lblSetFilterLO1->setVisible(false);
	ui->lblSetFilterLO2->setVisible(false);
	ui->lblSetFilterLO3->setVisible(false);
	
	ui->txtSetFilterHI1->setVisible(false);
	ui->txtSetFilterHI2->setVisible(false);
	ui->txtSetFilterHI3->setVisible(false);
	ui->txtSetFilterLO1->setVisible(false);
	ui->txtSetFilterLO2->setVisible(false);
	ui->txtSetFilterLO3->setVisible(false);
	
	
	ui->lblFindUnitsHI1->setVisible(false);
	ui->lblFindUnitsHI2->setVisible(false);
	ui->lblFindUnitsHI3->setVisible(false);
	ui->lblFindUnitsLO1->setVisible(false);
	ui->lblFindUnitsLO2->setVisible(false);
	ui->lblFindUnitsLO3->setVisible(false);

	ui->txtFindUnitsHI1->setVisible(false);
	ui->txtFindUnitsHI2->setVisible(false);
	ui->txtFindUnitsHI3->setVisible(false);
	ui->txtFindUnitsLO1->setVisible(false);
	ui->txtFindUnitsLO2->setVisible(false);
	ui->txtFindUnitsLO3->setVisible(false);
	
	ui->cmdNewModel->setVisible(false);
	this->setWindowTitle(qstrlstUserName[0] + " | Delfos Execution");
	ui->grpSave->setVisible(false);
	
	str_Path = Filter::SendCommand("pwd");
	str_Path.pop_back();
	str_Path.push_back('/');
	ui->txtPathFile->setText(str_Path.c_str());
	wiringPiSetup();
	
	UP		= _UP;
	LEFT	= _LEFT;
	RIGHT	= _RIGHT;
	DOWN	= _DOWN;
	LIGHT	= _LIGHT;
	SENSORU = _SENSORU;
	BUTTON	= _BUTTON;
	STOPPER = _STOPPER;
	WINDOWS = _WINDOWS;
	SENSORW1 = _SENSORW1;
	SENSORW2 = _SENSORW2;
	
	pinMode(UP, OUTPUT);
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);
	pinMode(DOWN, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	pinMode(SENSORU, INPUT);
	pinMode(BUTTON, INPUT);
	pinMode(SENSORW1, INPUT);
	pinMode(SENSORW2, INPUT);
	
	pinMode(STOPPER, OUTPUT);
	pinMode(WINDOWS, OUTPUT);
	
	xDebug = true;
	d_Scale = 1;
	
	PictureSettings[1].str_Path = str_Path + "Image.jpg";
	PictureSettings[0].str_Path = str_Path + "Picture.jpg";
	
	if ("Debug" == qstrlstUserName[0])
		ui->grpSignals->setVisible(true);
	else
		ui->grpSignals->setVisible(false);
	
	ui->grpAdvancedPropertiesImage->setVisible(false);

	//		qDebug() << Q_FUNC_INFO << QThread::currentThreadId();
//		ui->cmdDebug->setVisible(true);
//	else
//		ui->cmdDebug->setVisible(false);
	
	if ("Operator" == qstrlstUserName[1])
	{
		ui->tabWizard->removeTab(7);
		ui->tabWizard->removeTab(6);
		ui->tabWizard->removeTab(5);
		ui->tabWizard->removeTab(4);
		ui->tabWizard->removeTab(3);
		ui->tabWizard->removeTab(2);
		ui->tabWizard->removeTab(1);
		ui->grpSave->setVisible(false);
		ui->cmdOpenLast->setVisible(false);
		ui->cmdOpenModel->setVisible(false);
		ui->lblFileModel->setVisible(false);
		ui->txtFileModel->setVisible(false);
		ui->cmdOpenModelNext->setVisible(false);
	}
	if ("Manager"  == qstrlstUserName[1])
	{
		ui->lblCalibrationLegend1->setVisible(false);
		ui->lblCalibrationLegend2->setVisible(false);
		ui->lblCalibrationLegend3->setVisible(false);
		ui->lblCalibrationLegend4->setVisible(false);
		ui->cmdCalibrateTake->setVisible(false);
		ui->lblCalibrateHeight->setVisible(false);
		ui->lblCalibrateWidth->setVisible(false);
		ui->txtCalibrateHeight->setVisible(false);
		ui->txtCalibrateWidth->setVisible(false);
		ui->cmdCalibrate->setVisible(false);
	}
	
}
MainWindow::~MainWindow()
{
    delete ui;
}

/********************* TAB OPEN MODEL **********************/
void MainWindow::OpenModel()
{	
	
	QString qstrPath = "";
	qstrPath = QFileDialog::getOpenFileName(this,
		tr("Open Model"),
		"/home/pi/battery",
		tr("Config Files (*.yml)"));
	
	ui->txtFileModel->setText(qstrPath);
	
	if (!qstrPath.isEmpty())
	{	
		QStringList qstrlstPat = qstrPath.split(".");
		qstrPath = qstrlstPat[0];
		
		if (Filter::chkStatus())
		{
			unsigned int count = 0;
			while (count <= 20 && !xDebug)
			{
				if (LOW == digitalRead(SENSORW1))
				{
					count = count + 1;
					digitalWrite(WINDOWS, 1);
					this_thread::sleep_for(chrono::milliseconds(200));
				}
				else
				{
					count = 0;
					digitalWrite(WINDOWS, 0);
				}
			}
			//Pole = (Polar)ui->cmbFindUnitPosPolarity->currentIndex();
			this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
			InspectMachine Machine;
			bool xSuccess = false;
			int iStepLoad = 0;
			Machine.ul_LoadFile(ui->txtFileModel->text().toStdString());
			ui->numTakeStabilize->setValue(Machine.i_GetLoop());
			
			i_Version = Machine.BatteryFound.i_GetVersion();
			
			map<string, int> mstriIO = Machine.m_stri_GetIO();
			
			for (auto aIterator = mstriIO.begin(); aIterator != mstriIO.end(); aIterator++)
			{
				if ("UP" == aIterator->first)
					UP = aIterator->second;
				if ("LEFT" == aIterator->first)
					LEFT = aIterator->second;
				if ("RIGHT" == aIterator->first)
					RIGHT = aIterator->second;
				if ("DOWN" == aIterator->first)
					DOWN = aIterator->second;
				if ("LIGHT" == aIterator->first)
					LIGHT = aIterator->second;
			
				if ("SENSORU" == aIterator->first)
					SENSORU = aIterator->second;
				if ("STOPPER" == aIterator->first)
					STOPPER = aIterator->second;
				if ("BUTTON" == aIterator->first)
					BUTTON = aIterator->second;
				if ("WINDOWS" == aIterator->first)
					WINDOWS = aIterator->second;
				if ("SENSORW1" == aIterator->first)
					SENSORW1 = aIterator->second;
				if ("SENSORW2" == aIterator->first)
					SENSORW2 = aIterator->second;
			}
		
			ui->txtLightUp->setText(QString::number(UP));
			ui->txtLightDown->setText(QString::number(DOWN));
			ui->txtLightLeft->setText(QString::number(LEFT));
			ui->txtLightRight->setText(QString::number(RIGHT));
		
			ui->txtSensorUnit->setText(QString::number(SENSORU));
			ui->txtSolenoid->setText(QString::number(STOPPER));
			ui->txtButton->setText(QString::number(BUTTON));
			ui->txtWindows->setText(QString::number(WINDOWS));
			
			
			
			PictureSettings		= Machine.PictureSettings;			// Configuraciones de Imagen de posicionamiento
			DataCalib			= Machine.DataCalib;				// Calibración de Imagen
			PictureFilters		= Machine.PictureFilters;			// Filtros de posicionamiento
			BatteryFound		= Machine.BatteryFound;				// Datos de procesamiento
	
			
			v_RestoreListFilters();									// Limpia los campos
			if (RestoreProcessingPicture(true))						// Reestablece las configuraciones de posicionamiento
			{
				if (!Machine.DataCalib.x_Calibrated())
				{
					if (RestoreCalib(qstrPath + ".calib.xml"))				// Reestablece la calibración
					ui->chkUseCalibration->setCheckState(Checked);
					else
						ui->chkUseCalibration->setCheckState(Unchecked);
				}
				else
					ui->chkUseCalibration->setCheckState(Checked);
				iStepLoad++;
				if (RestoreFiltersPositions(true))					// Reestablece filtros de posicionamiento
				{
					iStepLoad++;
					if (RestoreProcessingImage(true))				// Reestablece las configuraciones de detección
					{
						iStepLoad++;
						if (RestoreFiltersUnits(true))				// Reestablece filtros de detección
						{
							iStepLoad++;
							if (RestorePositions(true)) {			// Reestablece datos de procesamiento
								xSuccess = true;
								ui->grpSave->setVisible(true);
								v_FillDataStatistic();
							}
						}
					}
				}
			}
			if (xSuccess)
			{
				QMessageBox::information(this,
					tr("Success"),
					tr("Resources have been loaded correctly."),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
			else
			{
				QString qstrStep = "Resources could not be loaded during the step: ";
				switch (iStepLoad)
				{
				case 0:
					qstrStep.append("Picture Settings");
					break;
				case 1:
					qstrStep.append("Set Filter");
					break;
				case 2:
					qstrStep.append("Image Settings");
			
					break;
				case 3:
					qstrStep.append("Filter Units");
					break;
				case 4:
					qstrStep.append("Find Position");
					break;
				default:
					break;
				}
		
				QMessageBox::information(this,
					tr("Failed Load"),
					qstrStep,
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
			digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
		}
		else {
			QMessageBox::warning(this,
				tr("Error"),
				tr("Memory corrupted."),
				QMessageBox::Ok,
				QMessageBox::Ok);
			close();
		}
	}
}
void MainWindow::OpenModelNew()
{
	
}
void MainWindow::OpenModelNext()
{
	ui->tabWizard->setCurrentIndex(1);
}
void MainWindow::OpenModelExit()
{
	this->close();
}
void MainWindow::OpenModelSaveIO()
{
	UP		= ui->txtLightUp->text().toInt();
	LEFT	= ui->txtLightLeft->text().toInt();
	RIGHT	= ui->txtLightRight->text().toInt();
	DOWN	= ui->txtLightDown->text().toInt();
	STOPPER = ui->txtSolenoid->text().toInt();
	SENSORU = ui->txtSensorUnit->text().toInt();
	BUTTON	= ui->txtButton->text().toInt();
	WINDOWS = ui->txtWindows->text().toInt();
}
void MainWindow::TestUp(bool xSignal)
{
	if (xSignal)
		digitalWrite(UP, HIGH);
	else
		digitalWrite(UP, LOW);
	ui->lblLightUpResult->setText(QString::number(xSignal));
}
void MainWindow::TestDown(bool xSignal)
{
	if (xSignal)
		digitalWrite(DOWN, HIGH);
	else
		digitalWrite(DOWN, LOW);
	ui->lblLightDownResult->setText(QString::number(xSignal));
}
void MainWindow::TestLeft(bool xSignal)
{
	if (xSignal)
		digitalWrite(LEFT, HIGH);
	else
		digitalWrite(LEFT, LOW);
	ui->lblLightLeftResult->setText(QString::number(xSignal));
}
void MainWindow::TestRight(bool xSignal)
{
	if (xSignal)
		digitalWrite(RIGHT, HIGH);
	else
		digitalWrite(RIGHT, LOW);
	ui->lblLightRightResult->setText(QString::number(xSignal));
}
void MainWindow::SelectedColor(int xindex)
{
	QStringList qstrlNames, qstrlInit;
	if (0 <= xindex)
	{
		QString qstrItem = ui->cmdColorBox->itemText(
			ui->cmdColorBox->currentIndex());
		if ("RED" == qstrItem) //RGB 023 NORGB 145
		{
			ui->txtLightDown->setText("0");
			ui->txtLightLeft->setText("4"); //
			ui->txtLightRight->setText("5");	//
		}
		else if("GREEN" == qstrItem)
		{
			ui->txtLightDown->setText("1"); //
			ui->txtLightLeft->setText("2");
			ui->txtLightRight->setText("5"); //
		}	
		else if("BLUE" == qstrItem)
		{
			ui->txtLightDown->setText("1"); //
			ui->txtLightLeft->setText("4"); //
			ui->txtLightRight->setText("3"); 
		}
		else if ("YELLOW" == qstrItem) //RGB 023 NORGB 145
		{
			ui->txtLightDown->setText("0");
			ui->txtLightLeft->setText("2");
			ui->txtLightRight->setText("5"); //
		}
		else if("CYAN" == qstrItem)
		{
			ui->txtLightDown->setText("1"); //
			ui->txtLightLeft->setText("2");
			ui->txtLightRight->setText("3");
		}	
		else if("MAGENTA" == qstrItem)
		{
			ui->txtLightDown->setText("0");
			ui->txtLightLeft->setText("4"); //
			ui->txtLightRight->setText("3"); 
		}
		else if("Control" == qstrItem)
		{
			ui->txtLightDown->setText("1");
			ui->txtLightLeft->setText("4");
			ui->txtLightRight->setText("5"); 
		}
	}
}
void MainWindow::SetColor()
{
	LEFT	= ui->txtLightLeft->text().toInt();
	RIGHT	= ui->txtLightRight->text().toInt();
	DOWN	= ui->txtLightDown->text().toInt();
}
void MainWindow::TestWindows(bool xSignal)
{
	if (xSignal) {
		unsigned int count = 0;
		while (count <= 20 && !xDebug)
		{
			if (LOW == digitalRead(SENSORW1))
			{
				count = count + 1;
				digitalWrite(WINDOWS, 1);
				this_thread::sleep_for(chrono::milliseconds(200));
			}
			else
			{
				count = 0;
				digitalWrite(WINDOWS, 0);
			}
		}
		digitalWrite(WINDOWS, HIGH);
	}
	else
		digitalWrite(WINDOWS, LOW);
	ui->lblWindowResult->setText(QString::number(xSignal));
}
void MainWindow::ReadSensor()
{
	int iResult = digitalRead(SENSORU);
	ui->lblSensorUnitResult->setText(QString::number(iResult));
}
void MainWindow::TestStopper(bool xSignal)
{
	if (xSignal)
		digitalWrite(STOPPER, HIGH);
	else
		digitalWrite(STOPPER, LOW);
	ui->lblSolenoidResult->setText(QString::number(xSignal));
}
void MainWindow::ReadButton()
{
	int iResult = digitalRead(BUTTON);
	ui->lblButtonResult->setText(QString::number(iResult));
}
void MainWindow::SaveModel()
{
	if (Filter::chkStatus())
	{
		if ("" != ui->txtPathFile->text() && "" != ui->txtNameModel->text())
		{
			string strName = ui->txtNameModel->text().toStdString();
			string strPath = ui->txtPathFile->text().toStdString();
			string strCheckDir = strPath + strName + "/";
		
			map<string, int> msiIO;
			msiIO.insert(pair<string, int>("UP", UP));
			msiIO.insert(pair<string, int>("LEFT", LEFT));
			msiIO.insert(pair<string, int>("RIGHT", RIGHT));
			msiIO.insert(pair<string, int>("DOWN", DOWN));
			msiIO.insert(pair<string, int>("LIGHT", LIGHT));
			msiIO.insert(pair<string, int>("STOPPER", STOPPER));
			msiIO.insert(pair<string, int>("SENSORU", SENSORU));
			msiIO.insert(pair<string, int>("BUTTON", BUTTON));
			msiIO.insert(pair<string, int>("WINDOWS", WINDOWS));
			msiIO.insert(pair<string, int>("SENSORW1", SENSORW1));
			msiIO.insert(pair<string, int>("SENSORW2", SENSORW2));
		
			if (!opendir(strCheckDir.c_str()))
			{
				string strCommand = "mkdir " + strCheckDir;	
				system(strCommand.c_str());
			}
		
			InspectMachine Machine(strName, strPath + strName + "/");
		
			Machine.v_SetData(PictureSettings,
				DataCalib,
				PictureFilters,
				BatteryFound);
			Machine.v_SetIO(msiIO);
			Machine.v_SetLoop(ui->numTakeStabilize->value());
			
			Machine.v_SaveFile();
		
			QMessageBox::information(this,
				tr("Success"),
				tr("Model successfully saved."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		
		}
		else
			QMessageBox::critical(this,
				tr("Error name"),
				tr("Must have a path or file name."),
				QMessageBox::Ok,
				QMessageBox::Ok);
	}
	else {
		QMessageBox::warning(this,
			tr("Error"),
			tr("Memory corrupted."),
			QMessageBox::Ok,
			QMessageBox::Ok);
		close();
	}
}
void MainWindow::OpenLastModel()
{
	if (Filter::chkStatus())
	{	
		unsigned int count = 0;
		while (count <= 20 && !xDebug)
		{
			if (LOW == digitalRead(SENSORW1))
			{
				count = count + 1;
				digitalWrite(WINDOWS, 1);
				this_thread::sleep_for(chrono::milliseconds(200));
			}
			else
			{
				count = 0;
				digitalWrite(WINDOWS, 0);
			}
		}

		this_thread::sleep_for(chrono::milliseconds(100)); 
		bool xSuccess = false;
		int iStepLoad = 0;
		if (RestoreProcessingPicture())
		{
			if (RestoreCalib()) 
				ui->chkUseCalibration->setCheckState(Checked);
			else
				ui->chkUseCalibration->setCheckState(Unchecked);
		
			iStepLoad++;
			v_RestoreListFilters();
			if (RestoreFiltersPositions())
			{
				iStepLoad++;
				if (RestoreProcessingImage())
				{
					iStepLoad++;
					if (RestoreFiltersUnits()) 
					{
						iStepLoad++;
						if (!RestoreIO()) {
							iStepLoad++;
							xSuccess = true;
						}
						
						if (RestorePositions())
						{
							ui->grpSave->setVisible(true);
							xSuccess = !xSuccess;
							v_FillDataStatistic();
						}
					}
				}
			}
		}
		if (xSuccess)
		{
			QMessageBox::information(this,
				tr("Success"),
				tr("Resources have been loaded correctly."),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
		else
		{
			QString qstrStep = "Resources could not be loaded during the step: ";
			switch (iStepLoad)
			{
			case 0:
				qstrStep.append("Picture Settings");
				break;
			case 1:
				qstrStep.append("Set Filter");
				break;
			case 2:
				qstrStep.append("Image Settings");
			
				break;
			case 3:
				qstrStep.append("Filter Units");
				break;
			case 4:
				qstrStep.append("Find Position");
				break;
			case 5:
				qstrStep.append("I/O Data");
				break;
			default:
				break;
			}
		
			QMessageBox::information(this,
				tr("Failed Load"),
				qstrStep,
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
		digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
	}
	else {
		QMessageBox::warning(this,
			tr("Error"),
			tr("Memory corrupted."),
			QMessageBox::Ok,
			QMessageBox::Ok);
		close();
	}
}

/********************* TAB PICTURE SETTING *****************/
void MainWindow::PictureSettingTakePicture()
{
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MResult;
	array<string,10> strData;
	strData[0] = ui->txtPicISO->text().toStdString();
	strData[1] = ui->txtPicSS->text().toStdString();
	strData[2] = str_Path + "Picture.jpg";
	strData[3] = ui->txtPicToTake->text().toStdString();
	strData[4] = ui->txtPicResWidth->text().toStdString();
	strData[5] = ui->txtPicResHeight->text().toStdString();
	strData[6] = ui->txtPicOffsetX->text().toStdString();
	strData[7] = ui->txtPicOffsetY->text().toStdString();
	strData[8] = ui->txtPicWidth->text().toStdString();
	strData[9] = ui->txtPicHeight->text().toStdString();
	PictureSettings[0] = ImageProcessSettings(strData);									// Recopila datos para tomar imagen
	MResult = CutPicture(PictureSettings[0], 0);
	v_ShowImage(MResult);																// Muestra la imagen
	digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
}
void MainWindow::PictureSettingRefresh()
{
	Mat MOriginalPicture, MBackup;
	array<string, 10> strData;
	strData[0] = ui->txtPicISO->text().toStdString();
	strData[1] = ui->txtPicSS->text().toStdString();
	strData[2] = str_Path + "Picture.jpg";
	strData[3] = ui->txtPicToTake->text().toStdString();
	strData[4] = ui->txtPicResWidth->text().toStdString();
	strData[5] = ui->txtPicResHeight->text().toStdString();
	strData[6] = ui->txtPicOffsetX->text().toStdString();
	strData[7] = ui->txtPicOffsetY->text().toStdString();
	strData[8] = ui->txtPicWidth->text().toStdString();
	strData[9] = ui->txtPicHeight->text().toStdString();
	
	PictureSettings[0] = ImageProcessSettings(strData);									// Recopila datos para tomar imagen
	
	MOriginalPicture = cCameraPi::M_LoadPicture(PictureSettings[0].str_Path);				// Carga la imagen
	MOriginalPicture.copyTo(MBackup);													// Genera un respaldo de la imagen
	
	if (Qt::Checked == ui->chkUseCalibration->checkState())								// Verifica si necesita calibrar
		MOriginalPicture = DataCalib.MImage(MOriginalPicture);							// Calibra
	
	if (0 == MOriginalPicture.cols && 0 == MOriginalPicture.rows)						// Verifica que no este vacio el resultado
	{
		MBackup.copyTo(MOriginalPicture);												// Si esta vacio recupera el respaldo
		QMessageBox::critical(this,
			tr("Calibration Fail"),
			tr("Calibration could not be performed, check for calibration data, \n"
			"recalibrate or upload a valid calibration."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
	
	Mat MCutImage = cCameraPi::M_CutImage(MOriginalPicture, PictureSettings[0]);			// Corta la imagen
	
	v_ShowImage(MCutImage);																// Muestra la imagen
}
void MainWindow::PictureSettingCalibrate()
{
	array<string, 10> strData;
	strData[0] = ui->txtPicISO->text().toStdString();
	strData[1] = ui->txtPicSS->text().toStdString();
	strData[2] = str_Path + "Picture.jpg";
	strData[3] = ui->txtPicToTake->text().toStdString();
	strData[4] = ui->txtPicResWidth->text().toStdString();
	strData[5] = ui->txtPicResHeight->text().toStdString();
	strData[6] = ui->txtPicOffsetX->text().toStdString();
	strData[7] = ui->txtPicOffsetY->text().toStdString();
	strData[8] = ui->txtPicWidth->text().toStdString();
	strData[9] = ui->txtPicHeight->text().toStdString();
	
	PictureSettings[0] = ImageProcessSettings(strData);									// Recopila datos para tomar imagen
	
	ui->tabWizard->setCurrentIndex(2);													// Cambia a la pestaña de calibración
}
void MainWindow::PictureSettingLoadCalib()
{
	QString qstrPath = "";
	qstrPath = QFileDialog::getOpenFileName(this,
		tr("Open Model"),
		"/home/pi/battery",
		tr("Config Files (*.xml)"));
	
	if (RestoreCalib(qstrPath))
		ui->chkUseCalibration->setCheckState(Checked);
	else
		ui->chkUseCalibration->setCheckState(Unchecked);
	
}
void MainWindow::v_ShowImage(Mat &MImage)
{	
	//Mat MResult;
	//rotate(MImage, MResult, ROTATE_90_CLOCKWISE);
	
	QImage image = Mat2QImage(MImage);
	QPixmap pixmap = QPixmap::fromImage(image.scaled(ui->picImageShow->width(), 
		ui->picImageShow->height(), 
		Qt::KeepAspectRatio));
	ui->picImageShow->clear();
	ui->picImageShow->setPixmap(pixmap);
}
void MainWindow::PictureSettingNext()
{
	ui->tabWizard->setCurrentIndex(3); // Cambia a la siguiente pestaña
	array<string, 10> strData;
	strData[0] = ui->txtPicISO->text().toStdString();
	strData[1] = ui->txtPicSS->text().toStdString();
	strData[2] = str_Path + "Picture.jpg";
	strData[3] = ui->txtPicToTake->text().toStdString();
	strData[4] = ui->txtPicResWidth->text().toStdString();
	strData[5] = ui->txtPicResHeight->text().toStdString();
	strData[6] = ui->txtPicOffsetX->text().toStdString();
	strData[7] = ui->txtPicOffsetY->text().toStdString();
	strData[8] = ui->txtPicWidth->text().toStdString();
	strData[9] = ui->txtPicHeight->text().toStdString();
	
	PictureSettings[0] = ImageProcessSettings(strData);
	PictureSettings[0].v_SaveFile(str_Path + "Picture.xml");
}
void MainWindow::InspectionProcess(bool xCheck)
{
	if (xCheck)
	{
		ui->cmbFindUnitPosPolarity->setEnabled(true);
		ui->cmbFilterUnitEvalPolarity->setEnabled(true);
		ui->cmbShowSecondFilter->setEnabled(true);
	}
	else
	{
		ui->cmbFindUnitPosPolarity->setEnabled(false);
		ui->cmbFilterUnitEvalPolarity->setEnabled(false);
		ui->cmbShowSecondFilter->setEnabled(false);
	}
	ui->cmbShowSecondFilter->setCurrentIndex(0);
	ui->cmbFilterUnitEvalPolarity->setCurrentIndex(0);
	ui->cmbFindUnitPosPolarity->setCurrentIndex(0);
	
}

/********************* TAB CALIBRATE ***********************/
void MainWindow::CalibrateTakePicture()
{
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MResult = TakePicture(PictureSettings[0]);
	v_ShowImage(MResult);
	digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
}
void MainWindow::Calibrate()
{
	int iHeight = ui->txtCalibrateHeight->text().toInt();
	int iWidth = ui->txtCalibrateWidth->text().toInt();
	DataCalib = CalibImage(str_Path + "Picture.jpg", Size(iWidth, iHeight));
	Mat MChessBoard = TakePicture(PictureSettings[0]);
	Mat MCalibrated;
	if (DataCalib.x_Start(MChessBoard, MCalibrated))
	{
		ui->chkUseCalibration->setCheckState(Checked);
		QMessageBox::information(this,
			tr("Successful Calibration"),
			tr("Calibration completed correctly."),
			QMessageBox::Ok,
			QMessageBox::Ok);		
		v_ShowImage(MCalibrated);
	}
	else
	{
		ui->chkUseCalibration->setCheckState(Unchecked);
		QMessageBox::information(this,
			tr("Failed Calibration"),
			tr("Calibration could not be completed, \n"
			"verify that the image pattern and number of edges are correct."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}
void MainWindow::CalibrateNext()
{
	ui->tabWizard->setCurrentIndex(1);
	DataCalib.v_SaveFile(str_Path + "Calib.xml");
}
void MainWindow::CalibrateExit()
{
	ui->tabWizard->setCurrentIndex(1);
}

/********************* TAB SET FILTER **********************/
void MainWindow::SetFilterTake()
{
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MResult = CutPicture(PictureSettings[0], 0);
	v_ShowImage(MResult);
	digitalWrite(WINDOWS, 1); //MODIFICACION DEL 24 DE MARZO Ventanas abajo
}
void MainWindow::SetFilterFilter(int index)
{
	bool xOK = true;
	bool xSend[12];
	QStringList qstrlNames, qstrlInit;
	
	if (0 <= index)
	{
		QString qstrItem = ui->cmbSetFilterFilter->itemText(
			ui->cmbSetFilterFilter->currentIndex());
		if ("Binary" == qstrItem)														// Binary
		{
			bool xShow[] =  _BINARY;
			qstrlNames = QStringList(_BINARYNAMES);
			qstrlInit = QStringList(_BINARYINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
		}
		else if("HSV" == qstrItem)														// HSV
		{
			bool xShow[] =  _HSV;
			qstrlNames = QStringList(_HSVNAMES);
			qstrlInit = QStringList(_HSVINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];

		}		
		else if("Dilate" == qstrItem)													// DILATE
		{
			bool xShow[] =  _DILATE;
			qstrlNames = QStringList(_DILATENAMES);
			qstrlInit = QStringList(_DILATEINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
		}		
		else if("Erode" == qstrItem)													// ERODE
		{
			bool xShow[] =  _ERODE;
			qstrlNames = QStringList(_ERODENAMES);
			qstrlInit = QStringList(_ERODEINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
		}		
		else if("Bilateral" == qstrItem)												// BILATERAL
		{
			bool xShow[] =  _BILATERAL;
			qstrlNames = QStringList(_BILATERALNAMES);
			qstrlInit = QStringList(_BILATERALINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
		}
		else if("Blur" == qstrItem)														// BLUR
		{
			bool xShow[] =  _BLUR;
			qstrlNames = QStringList(_BLURNAMES);
			qstrlInit = QStringList(_BLURINIT);
			for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
		}
		else
			xOK = false;
	
		if (xOK)
		{
			ui->lblSetFilterHI1->setVisible(xSend[0]);
			ui->txtSetFilterHI1->setVisible(xSend[1]);
			ui->lblSetFilterLO1->setVisible(xSend[2]);
			ui->txtSetFilterLO1->setVisible(xSend[3]);
			ui->lblSetFilterHI2->setVisible(xSend[4]);
			ui->txtSetFilterHI2->setVisible(xSend[5]);
			ui->lblSetFilterLO2->setVisible(xSend[6]);
			ui->txtSetFilterLO2->setVisible(xSend[7]);
			ui->lblSetFilterHI3->setVisible(xSend[8]);
			ui->txtSetFilterHI3->setVisible(xSend[9]);
			ui->lblSetFilterLO3->setVisible(xSend[10]);
			ui->txtSetFilterLO3->setVisible(xSend[11]);		
			ui->lblSetFilterHI1->setText(qstrlNames[0]);
			ui->lblSetFilterLO1->setText(qstrlNames[1]);
			ui->lblSetFilterHI2->setText(qstrlNames[2]);
			ui->lblSetFilterLO2->setText(qstrlNames[3]);
			ui->lblSetFilterHI3->setText(qstrlNames[4]);		
			ui->lblSetFilterLO3->setText(qstrlNames[5]);
			ui->txtSetFilterHI1->setText(qstrlInit[0]);
			ui->txtSetFilterLO1->setText(qstrlInit[1]);
			ui->txtSetFilterHI2->setText(qstrlInit[2]);
			ui->txtSetFilterLO2->setText(qstrlInit[3]);
			ui->txtSetFilterHI3->setText(qstrlInit[4]);
			ui->txtSetFilterLO3->setText(qstrlInit[5]);
		
		}
	}
	else
	{
		ui->lblSetFilterHI1->setVisible(false);
		ui->txtSetFilterHI1->setVisible(false);
		ui->lblSetFilterLO1->setVisible(false);
		ui->txtSetFilterLO1->setVisible(false);
		ui->lblSetFilterHI2->setVisible(false);
		ui->txtSetFilterHI2->setVisible(false);
		ui->lblSetFilterLO2->setVisible(false);
		ui->txtSetFilterLO2->setVisible(false);
		ui->lblSetFilterHI3->setVisible(false);
		ui->txtSetFilterHI3->setVisible(false);
		ui->lblSetFilterLO3->setVisible(false);
		ui->txtSetFilterLO3->setVisible(false);
	}
}
void MainWindow::SetFilterApply()
{
	Mat MOriginalPicture;
	int iCurrentRow = -1;
	int iSize = 0;
	FilterParams fpPictureParams;
	int iFilter = 0;
	
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	
	this_thread::sleep_for(chrono::milliseconds(100));
	
	MOriginalPicture = CutPicture(PictureSettings[0], 1); // Carga la imagen
	PictureFilters[iFilter].v_SetStart(); // Inicia el carrucel de filtros
	if (0 < ui->lstSetFilterFilter->count())											// Valida que se haya seleccionado algun item en la lista
		iCurrentRow = ui->lstSetFilterFilter->currentRow();								// Selecciona el item
	
	if (0 <= iCurrentRow)																// Si el item es valido
		iSize = ui->lstSetFilterFilter->currentRow();									// Establece cuantos se van a ejectar
	else
		iSize = PictureFilters[iFilter].i_GetCount(); // Si no, ejecuta todos
	
	
	for (int i = 0; i < iSize; i++)														// Realiza de ciclo de ejecución de filtros
		PictureFilters[iFilter].v_Execute(MOriginalPicture, iCurrentRow);
	
	if (0 < MOriginalPicture.cols && 0 < MOriginalPicture.rows)							// Verifica que no este vacio el resultado
	{	
		fpPictureParams < CreateSetFilterParams(MOriginalPicture);						// Captura todos los parametros del filtro
		if (0 <= ui->cmbSetFilterFilter->currentIndex())								// Si es un Parámetro valido en el combo box
		{
			QString qstrItem = ui->cmbSetFilterFilter->itemText(						// Obtiene nombre del parametro para despues 
				ui->cmbSetFilterFilter->currentIndex());								// ejecutar el filtro
			if ("Binary" == qstrItem)													// Binary
			Filter::Binary(fpPictureParams);
			else if("HSV" == qstrItem)													// HSV
				Filter::HSV(fpPictureParams);
			else if("Dilate" == qstrItem)												// DILATE
				Filter::Dilate(fpPictureParams);
			else if("Erode" == qstrItem)												// ERODE
				Filter::Erode(fpPictureParams);
			else if("Bilateral" == qstrItem)											// BILATERAL
				Filter::Bilateral(fpPictureParams);
			else if("Blur" == qstrItem)													// BLUR
				Filter::Blur(fpPictureParams);
		}
		else if(0 <= iCurrentRow)														// Si no es un item válido en el combo box
		{
			FilterParams fpDataIndex = PictureFilters[iFilter].i_GetIndex(// Captura los parámetros del filtro
				ui->lstSetFilterFilter->currentRow());
			int iType = fpDataIndex.i_GetType(); // Obtiene tipo de filtro y ejecuta
			switch (iType)
			{
			case Binary:
				Filter::Binary(fpPictureParams);
				break;
			case HSV:
				Filter::HSV(fpPictureParams);
				break;
			case Dilate:
				Filter::Dilate(fpPictureParams);
				break;
			case Erode:
				Filter::Erode(fpPictureParams);
				break;
			case Bilateral:
				Filter::Bilateral(fpPictureParams);
				break;
			case Blur:
				Filter::Blur(fpPictureParams);
				break;
			default:
				break;
			}
			PictureFilters[iFilter].v_Update(iCurrentRow, fpPictureParams);
		}
		
		if (fpPictureParams.vM_Image[1].cols > 0 && 
			fpPictureParams.vM_Image[1].rows > 0)
			v_ShowImage(fpPictureParams.vM_Image[1]);
		
		//delete fpPictureParams;
		
		//// REVISAR SI LA IMAGEN 0 ES LA CONSECUENTE A FILTRAR
		
		
	}
	else
	{
		QMessageBox::information(this,
			tr("Picture invalid"),
			tr("The captured picture is invalid or corrupted, \n"
			"check the picture or retake a new one."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}
void MainWindow::SetFilterAdd()
{
	FilterParams fpPictureParams = CreateSetFilterParams(Mat());
	QString qstrNameFilter = "";
	void(* vFunction)(FilterParams&);
	QString qstrItem = ui->cmbSetFilterFilter->itemText(
		ui->cmbSetFilterFilter->currentIndex());
	if ("Binary" == qstrItem)															// Binary
	{
		vFunction = Filter::Binary;
		qstrNameFilter = "Binary Filter";
		fpPictureParams.v_SetType(Binary);
	}
	else if("HSV" == qstrItem)															// HSV
	{
		vFunction = Filter::HSV;
		qstrNameFilter = "HSV Filter";
		fpPictureParams.v_SetType(HSV);
	}		
	else if("Dilate" == qstrItem)														// DILATE
	{
		vFunction = Filter::Dilate;
		qstrNameFilter = "Dilate Filter";
		fpPictureParams.v_SetType(Dilate);
	}		
	else if("Erode" == qstrItem)														// ERODE
	{
		vFunction = Filter::Erode;
		qstrNameFilter = "Erode Filter";
		fpPictureParams.v_SetType(Erode);
	}		
	else if("Bilateral" == qstrItem)													// BILATERAL
	{
		vFunction = Filter::Bilateral;
		qstrNameFilter = "Bilateral Filter";
		fpPictureParams.v_SetType(Bilateral);
	}
	else if("Blur" == qstrItem)															// BLUR
	{
		vFunction = Filter::Blur;
		qstrNameFilter = "Blur Filter";
		fpPictureParams.v_SetType(Blur);
	}	
	
	ui->cmbSetFilterFilter->removeItem(ui->cmbSetFilterFilter->currentIndex());
	ui->cmbSetFilterFilter->setCurrentIndex(-1);
	PictureFilters[0].v_Push(vFunction, fpPictureParams);
	ui->lstSetFilterFilter->addItem(qstrNameFilter);
}
void MainWindow::SetFilterSelect(int iIndex)
{
	if (0 <= iIndex)
	{
		bool xSend[12];
		QStringList qstrlNames, qstrlData;
		FilterParams fpDataIndex = PictureFilters[0].i_GetIndex(iIndex);
		int iType = fpDataIndex.i_GetType();
		
		for (int i = 0; 6 > i; i++)
			qstrlData.push_back(QString::number(fpDataIndex.i_Param[i]));
	
		switch (iType)
		{
		case Binary:
			{
				bool xShow[] =  _BINARY;
				qstrlNames = QStringList(_BINARYNAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		case HSV:
			{
				bool xShow[] =  _HSV;
				qstrlNames = QStringList(_HSVNAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		case Dilate:
			{
				bool xShow[] =  _DILATE;
				qstrlNames = QStringList(_DILATENAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		case Erode:
			{
				bool xShow[] =  _ERODE;
				qstrlNames = QStringList(_ERODENAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		case Bilateral:
			{
				bool xShow[] =  _BILATERAL;
				qstrlNames = QStringList(_BILATERALNAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		case Blur:
			{
				bool xShow[] =  _BLUR;
				qstrlNames = QStringList(_BLURNAMES);
				for (int i = 0; i < 12; i++) xSend[i] = xShow[i];
				break;
			}
		default:
			break;
		}
	
		if (0 <= iType)
		{
			ui->lblSetFilterHI1->setVisible(xSend[0]);
			ui->txtSetFilterHI1->setVisible(xSend[1]);
			ui->lblSetFilterLO1->setVisible(xSend[2]);
			ui->txtSetFilterLO1->setVisible(xSend[3]);
			ui->lblSetFilterHI2->setVisible(xSend[4]);
			ui->txtSetFilterHI2->setVisible(xSend[5]);
			ui->lblSetFilterLO2->setVisible(xSend[6]);
			ui->txtSetFilterLO2->setVisible(xSend[7]);
			ui->lblSetFilterHI3->setVisible(xSend[8]);
			ui->txtSetFilterHI3->setVisible(xSend[9]);
			ui->lblSetFilterLO3->setVisible(xSend[10]);
			ui->txtSetFilterLO3->setVisible(xSend[11]);		
			ui->lblSetFilterHI1->setText(qstrlNames[0]);
			ui->lblSetFilterLO1->setText(qstrlNames[1]);
			ui->lblSetFilterHI2->setText(qstrlNames[2]);
			ui->lblSetFilterLO2->setText(qstrlNames[3]);
			ui->lblSetFilterHI3->setText(qstrlNames[4]);		
			ui->lblSetFilterLO3->setText(qstrlNames[5]);
			ui->txtSetFilterHI1->setText(qstrlData[0]);
			ui->txtSetFilterLO1->setText(qstrlData[1]);
			ui->txtSetFilterHI2->setText(qstrlData[2]);
			ui->txtSetFilterLO2->setText(qstrlData[3]);
			ui->txtSetFilterHI3->setText(qstrlData[4]);
			ui->txtSetFilterLO3->setText(qstrlData[5]);	
		}
	}
	else
	{
		ui->lblSetFilterHI1->setVisible(false);
		ui->txtSetFilterHI1->setVisible(false);
		ui->lblSetFilterLO1->setVisible(false);
		ui->txtSetFilterLO1->setVisible(false);
		ui->lblSetFilterHI2->setVisible(false);
		ui->txtSetFilterHI2->setVisible(false);
		ui->lblSetFilterLO2->setVisible(false);
		ui->txtSetFilterLO2->setVisible(false);
		ui->lblSetFilterHI3->setVisible(false);
		ui->txtSetFilterHI3->setVisible(false);
		ui->lblSetFilterLO3->setVisible(false);
		ui->txtSetFilterLO3->setVisible(false);
	}
}
void MainWindow::SetFilterUp()
{
	int iCurrent = ui->lstSetFilterFilter->currentRow();
	int iSize = ui->lstSetFilterFilter->count();
	if (iCurrent > 0 && iCurrent <= iSize - 1)
	{
		PictureFilters[0].v_Swap(iCurrent, iCurrent - 1);
		QString qstrCurrentName = ui->lstSetFilterFilter->item(iCurrent)->text();
		ui->lstSetFilterFilter->item(iCurrent)->setText(
			ui->lstSetFilterFilter->item(iCurrent - 1)->text());
		ui->lstSetFilterFilter->item(iCurrent - 1)->setText(qstrCurrentName);
		
	}
}
void MainWindow::SetFilterDown()
{
	int iCurrent = ui->lstSetFilterFilter->currentRow();
	int iSize = ui->lstSetFilterFilter->count();
	if (iCurrent < iSize - 1 && 0 <= iCurrent)
	{
		PictureFilters[0].v_Swap(iCurrent, iCurrent + 1);
		QString qstrCurrentName = ui->lstSetFilterFilter->item(iCurrent)->text();
		ui->lstSetFilterFilter->item(iCurrent)->setText(
			ui->lstSetFilterFilter->item(iCurrent + 1)->text());
		ui->lstSetFilterFilter->item(iCurrent + 1)->setText(qstrCurrentName);
		
	}
}
void MainWindow::SetFilterDelete()
{
	int iCurrent = ui->lstSetFilterFilter->currentRow();
	if (0 <= iCurrent)
	{
		PictureFilters[0].v_Delete(iCurrent);
		int iSize = ui->lstSetFilterFilter->count();
		QList<QString> qlqstrFilter;
		for (int i = 0; i < iSize; i++)
			qlqstrFilter.push_back(ui->lstSetFilterFilter->item(i)->text());
	
		QString qstrItem = qlqstrFilter[iCurrent].left(
			qlqstrFilter[iCurrent].length() - 7);
		ui->cmbSetFilterFilter->addItem(qstrItem);
		qlqstrFilter.removeAt(iCurrent);
		ui->lstSetFilterFilter->clear();
		ui->lstSetFilterFilter->addItems(qlqstrFilter);
	}
}
void MainWindow::SetFilterNext()
{
	PictureFilters[0].v_SaveFile(str_Path + "FiltersPositions.xml");
	ui->tabWizard->setCurrentIndex(4); // Cambia a la siguiente pestaña
}
void MainWindow::SetFilterClear()
{
	ui->cmbSetFilterFilter->setCurrentIndex(-1);
}

/********************* TAB FIND POSITION *******************/
void MainWindow::FindPositionTake()
{
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MCutPicture = CutPicture(PictureSettings[0], 0);
	ExecuteFilters(PictureFilters[0], MCutPicture);
	v_ShowImage(MCutPicture);
	digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
}
void MainWindow::FindPositionEdges()
{
	if (0 < PictureFilters[0].i_GetCount())
	{
		unsigned int count = 0;
	
		while (count <= 20 && !xDebug)
		{
			if (LOW == digitalRead(SENSORW1))
			{
				count = count + 1;
				digitalWrite(WINDOWS, 1);
				this_thread::sleep_for(chrono::milliseconds(200));
			}
			else
			{
				count = 0;
				digitalWrite(WINDOWS, 0);
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	
		Mat MOriginalPicture = CutPicture(PictureSettings[0], 0);
		BatteryFound.v_SetPole(CreateObject());
		ExecuteFilters(PictureFilters[0], MOriginalPicture);
		
		Mat MEdges = BatteryFound.M_FindContourns(MOriginalPicture);
		v_ShowImage(MEdges);
		digitalWrite(WINDOWS, 0);
	}
	else
	{
		QMessageBox::critical(this,
			tr("Error Filters"),
			tr("Error in the image filters,\n" 
			"make sure that there are filters in the cartridge."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}
void MainWindow::FindPositionShow()
{
	bool xExecute = true;
	if (!BatteryFound.x_EmptyArray())
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this,
			"Overwite Data",
			"Do you want to overwrite the unit matrix?",
			QMessageBox::Yes|QMessageBox::No);
		
		if (reply == QMessageBox::No)
			xExecute = false;
	}
	
	if (xExecute)
	{
		if (0 < PictureFilters[0].i_GetCount())
		{
			unsigned int count = 0;
			while (count <= 20 && !xDebug)
			{
				if (LOW == digitalRead(SENSORW1))
				{
					count = count + 1;
					digitalWrite(WINDOWS, 1);
					this_thread::sleep_for(chrono::milliseconds(200));
				}
				else
				{
					count = 0;
					digitalWrite(WINDOWS, 0);
				}
			}
			this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
			Mat MOriginalPicture = CutPicture(PictureSettings[0], 0);
			Mat MUnits, MEdited;
			MOriginalPicture.copyTo(MEdited);
			MOriginalPicture.copyTo(MCurrentPicture);
			BatteryFound.v_SetPole(CreateObject());
			ExecuteFilters(PictureFilters[0], MOriginalPicture);
			BatteryFound.M_FindContourns(MOriginalPicture);
			if (0 < BatteryFound.i_Find())
				MUnits = BatteryFound.M_CreateArray(MEdited);
			v_ShowImage(MUnits);
			digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
		}
	}
	else if (!BatteryFound.x_EmptyArray())
	{
		if (0 < PictureFilters[0].i_GetCount())
		{
			Mat MOriginalPicture = CutPicture(PictureSettings[0], 0);
			MOriginalPicture.copyTo(MCurrentPicture);
			Mat MImage = BatteryFound.M_PaintLocations(MOriginalPicture);
			v_ShowImage(MImage);
		}
	}
	if (0 >= PictureFilters[0].i_GetCount())
	{
		QMessageBox::critical(this,
			tr("Error Filters"),
			tr("Error in the image filters,\n" 
			"make sure that there are filters in the cartridge."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}
void MainWindow::FindPositionNext()
{
	BatteryFound.v_SaveFile(str_Path + "Positions.xml");
	ui->tabWizard->setCurrentIndex(5); // Cambia a la siguiente pestaña
}

void MainWindow::FilterUnitsChangePolarity(int iIndex)
{
	vector<FilterParams> vfpData;
	vector<FilterParams>::iterator vfp_iIterator;
	int iFilter;
	int iSize = ui->cmbFilterUnitEvalPolarity->count();
	QString qstrType = "";
	bool xPass = false;	
	if (0 <= iIndex)
	{
		switch ((Polar)ui->cmbFilterUnitEvalPolarity->currentIndex())
		{	
		case POS:	
			iFilter = 1;
			break;
		case NEG:
			iFilter = 2;
			break;
		default:
			iFilter = 0;
			break;
		}
		
		vfpData = PictureFilters[iFilter].vfp_GetParams();
		ui->txtImaToTake->setText(PictureSettings[iFilter].qLoop());
		ui->txtImaISO->setText(PictureSettings[iFilter].qISO());
		ui->txtImaSS->setText(PictureSettings[iFilter].qISS());
		
		vfp_iIterator = vfpData.begin();
		ui->listFindUnits->clear();
		for (; vfp_iIterator != vfpData.end(); vfp_iIterator++)
		{
			int iParams[6];
			for (int i = 0; i < 6; i++)
				iParams[i] = vfp_iIterator->i_Param[i];
			int iType = vfp_iIterator->i_GetType();
			qstrType = "";	
			switch (iType)
			{
			case Binary:
				qstrType = "Binary";
				xPass = true;
				break;
			case HSV:
				qstrType = "HSV";
				xPass = true;
				break;
			case Dilate:
				qstrType = "Dilate";
				xPass = true;
				break;
			case Erode:
				qstrType = "Erode";
				xPass = true;
				break;
			case Bilateral:
				qstrType = "Bilateral";
				xPass = true;
				break;
			case Blur:
				qstrType = "Blur";
				xPass = true;
				break;
			default:
				break;
			}
			if (xPass)
			{
				xPass = false;
				for (int i = 0; i < iSize; i++)
				{
					if (qstrType == ui->cmbFindUnitsFilter->itemText(i))
					{
						ui->cmbFindUnitsFilter->removeItem(i);
						iSize--;
						break;
					}
				}
				ui->listFindUnits->addItem(qstrType + " Filter");
			}
		}
	}
}

/********************* TAB IMAGE SETTING *******************/
void MainWindow::ImageSettingTake(bool xCheck)
{
	ui->grpAdvancedPropertiesImage->setVisible(xCheck);
}

/********************* TAB FIND UNITS **********************/
void MainWindow::FindUnitTake()
{
	unsigned int count = 0;	
	array<string, 6> strData;
	Mat MResult;
	strData[0] = ui->txtImaISO->text().toStdString();
	strData[1] = ui->txtImaSS->text().toStdString();
	strData[2] = str_Path + "Image.jpg";
	strData[3] = ui->txtImaToTake->text().toStdString();
	strData[4] = ui->txtPicResWidth->text().toStdString();
	strData[5] = ui->txtPicResHeight->text().toStdString();
	
	int iCurrent = ui->cmbFilterUnitEvalPolarity->currentIndex();
	if (POS == iCurrent)
		PictureSettings[1] = ImageProcessSettings(strData);
	else if(NEG == iCurrent)
		PictureSettings[2] = ImageProcessSettings(strData);
		
	if (iCurrent >= 0)
	{
		while (count <= 20 && !xDebug)
		{
			if (LOW == digitalRead(SENSORW1))
			{
				count = count + 1;
				digitalWrite(WINDOWS, 1);
				this_thread::sleep_for(chrono::milliseconds(200));
			}
			else
			{
				count = 0;
				digitalWrite(WINDOWS, 0);
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100));
		if (POS == iCurrent)
		{
			MResult = CutPicture(PictureSettings[1], 
				PictureSettings[1].i_Loop);
		}
		else
		{
			MResult = CutPicture(PictureSettings[2], 
				PictureSettings[2].i_Loop);
		}
		
		ui->grpSetFilterUnit->setEnabled(true);
		ui->grpAdvancedPropertiesImage->setEnabled(true);
		
		v_ShowImage(MResult);
		digitalWrite(WINDOWS, 0);
	}
	else
	{
		ui->grpSetFilterUnit->setEnabled(false);
		ui->grpAdvancedPropertiesImage->setEnabled(false);
		QMessageBox::information(this,
			tr("Missing information"),
			tr("It is required to select a polarity type"),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}
void MainWindow::FindUnitFilter(int index)
{
	bool xOK = true;
	bool * xptrShow;
	QStringList qstrlNames, qstrlInit;
	
	if (0 <= index)
	{
		QString qstrItem = ui->cmbFindUnitsFilter->itemText(
			ui->cmbFindUnitsFilter->currentIndex());
		if ("Binary" == qstrItem)														// Binary
		{
			bool xShow[] =  _BINARY;
			qstrlNames = QStringList(_BINARYNAMES);
			qstrlInit = QStringList(_BINARYINIT);
			xptrShow = xShow;
		}
		else if("HSV" == qstrItem)														// HSV
		{
			bool xShow[] =  _HSV;
			qstrlNames = QStringList(_HSVNAMES);
			qstrlInit = QStringList(_HSVINIT);
			xptrShow = xShow;

		}		
		else if("Dilate" == qstrItem)													// DILATE
		{
			bool xShow[] =  _DILATE;
			qstrlNames = QStringList(_DILATENAMES);
			qstrlInit = QStringList(_DILATEINIT);
			xptrShow = xShow;
		}		
		else if("Erode" == qstrItem)													// ERODE
		{
			bool xShow[] =  _ERODE;
			qstrlNames = QStringList(_ERODENAMES);
			qstrlInit = QStringList(_ERODEINIT);
			xptrShow = xShow;
		}		
		else if("Bilateral" == qstrItem)												// BILATERAL
		{
			bool xShow[] =  _BILATERAL;
			qstrlNames = QStringList(_BILATERALNAMES);
			qstrlInit = QStringList(_BILATERALINIT);
			xptrShow = xShow;
		}
		else if("Blur" == qstrItem)														// BLUR
		{
			bool xShow[] =  _BLUR;
			qstrlNames = QStringList(_BLURNAMES);
			qstrlInit = QStringList(_BLURINIT);
			xptrShow = xShow;
		}
		else
			xOK = false;
	
		if (xOK)
		{
			ui->lblFindUnitsHI1->setVisible(*xptrShow++);
			ui->txtFindUnitsHI1->setVisible(*xptrShow++);
			ui->lblFindUnitsLO1->setVisible(*xptrShow++);
			ui->txtFindUnitsLO1->setVisible(*xptrShow++);
			ui->lblFindUnitsHI2->setVisible(*xptrShow++);
			ui->txtFindUnitsHI2->setVisible(*xptrShow++);
			ui->lblFindUnitsLO2->setVisible(*xptrShow++);
			ui->txtFindUnitsLO2->setVisible(*xptrShow++);
			ui->lblFindUnitsHI3->setVisible(*xptrShow++);
			ui->txtFindUnitsHI3->setVisible(*xptrShow++);
			ui->lblFindUnitsLO3->setVisible(*xptrShow++);
			ui->txtFindUnitsLO3->setVisible(*xptrShow++);		
			ui->lblFindUnitsHI1->setText(qstrlNames[0]);
			ui->lblFindUnitsLO1->setText(qstrlNames[1]);
			ui->lblFindUnitsHI2->setText(qstrlNames[2]);
			ui->lblFindUnitsLO2->setText(qstrlNames[3]);
			ui->lblFindUnitsHI3->setText(qstrlNames[4]);		
			ui->lblFindUnitsLO3->setText(qstrlNames[5]);
			ui->txtFindUnitsHI1->setText(qstrlInit[0]);
			ui->txtFindUnitsLO1->setText(qstrlInit[1]);
			ui->txtFindUnitsHI2->setText(qstrlInit[2]);
			ui->txtFindUnitsLO2->setText(qstrlInit[3]);
			ui->txtFindUnitsHI3->setText(qstrlInit[4]);
			ui->txtFindUnitsLO3->setText(qstrlInit[5]);
		
		}
	}
	else
	{
		ui->lblFindUnitsHI1->setVisible(false);
		ui->txtFindUnitsHI1->setVisible(false);
		ui->lblFindUnitsLO1->setVisible(false);
		ui->txtFindUnitsLO1->setVisible(false);
		ui->lblFindUnitsHI2->setVisible(false);
		ui->txtFindUnitsHI2->setVisible(false);
		ui->lblFindUnitsLO2->setVisible(false);
		ui->txtFindUnitsLO2->setVisible(false);
		ui->lblFindUnitsHI3->setVisible(false);
		ui->txtFindUnitsHI3->setVisible(false);
		ui->lblFindUnitsLO3->setVisible(false);
		ui->txtFindUnitsLO3->setVisible(false);
	}
}
void MainWindow::FindUnitApply()
{
	
	int iFilter;
	
	if (0 == ui->cmbFilterUnitEvalPolarity->currentIndex())
		iFilter = 1;
	else if (1 == ui->cmbFilterUnitEvalPolarity->currentIndex())
		iFilter = 2;
	
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	
	this_thread::sleep_for(chrono::milliseconds(100));
	Mat MOriginalPicture = CutPicture(PictureSettings[iFilter], 0);
	
	PictureFilters[1].v_SetStart();
	
	int iCurrentRow;
	int iSize = 0;
	
	if (0 < ui->listFindUnits->count())											// Valida que se haya seleccionado algun item en la lista
		iCurrentRow = ui->listFindUnits->currentRow(); // Selecciona el item
	
	if (0 <= iCurrentRow)																// Si el item es valido
		iSize = ui->listFindUnits->currentRow(); // Establece cuantos se van a ejectar
	else
		iSize = PictureFilters[iFilter].i_GetCount(); // Si no, ejecuta todos

	
	for (int i = 0; i < iSize; i++)
		PictureFilters[iFilter].v_Execute(MOriginalPicture, iCurrentRow);
	
	if (0 < MOriginalPicture.cols && 0 < MOriginalPicture.rows)							// Verifica que no este vacio el resultado
	{		
		FilterParams fpPictureParams = CreateFindUnitsParams(
			MOriginalPicture);
		if (0 <= ui->cmbFindUnitsFilter->currentIndex())
		{
			QString qstrItem = ui->cmbFindUnitsFilter->itemText(
				ui->cmbFindUnitsFilter->currentIndex());
			if ("Binary" == qstrItem)													// Binary
			Filter::Binary(fpPictureParams);
			else if("HSV" == qstrItem)													// HSV
				Filter::HSV(fpPictureParams);
			else if("Dilate" == qstrItem)												// DILATE
				Filter::Dilate(fpPictureParams);
			else if("Erode" == qstrItem)												// ERODE
				Filter::Erode(fpPictureParams);
			else if("Bilateral" == qstrItem)											// BILATERAL
				Filter::Bilateral(fpPictureParams);
			else if("Blur" == qstrItem)													// BLUR
				Filter::Blur(fpPictureParams);
		}
		else if (0 <= iCurrentRow)
		{
			iCurrentRow = ui->listFindUnits->currentRow();
			FilterParams fpDataIndex = PictureFilters[2].i_GetIndex(iCurrentRow);
			int iType = fpDataIndex.i_GetType();
			
			switch (iType)
			{
			case Binary:
				Filter::Binary(fpPictureParams);
				break;
			case HSV:
				Filter::HSV(fpPictureParams);
				break;
			case Dilate:
				Filter::Dilate(fpPictureParams);
				break;
			case Erode:
				Filter::Erode(fpPictureParams);
				break;
			case Bilateral:
				Filter::Bilateral(fpPictureParams);
				break;
			case Blur:
				Filter::Blur(fpPictureParams);
				break;
			default:
				break;
			}
			
		}
		if (0 <= iCurrentRow)
			PictureFilters[iFilter].v_Update(iCurrentRow, fpPictureParams);
		
		if (fpPictureParams.vM_Image[1].cols > 0 && 
			fpPictureParams.vM_Image[1].rows > 0)
			v_ShowImage(fpPictureParams.vM_Image[1]);
	}
	else
	{
		QMessageBox::information(this,
			tr("Picture invalid"),
			tr("The captured picture is invalid or corrupted, \n"
			"check the picture or retake a new one."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
	digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
}
void MainWindow::FindUnitAdd()
{
	FilterParams fpPictureParams = CreateFindUnitsParams(Mat());
	QString qstrNameFilter = "";
	void(* vFunction)(FilterParams&);
	QString qstrItem = ui->cmbFindUnitsFilter->itemText(
		ui->cmbFindUnitsFilter->currentIndex());
	if ("Binary" == qstrItem)															// Binary
	{
		vFunction = Filter::Binary;
		qstrNameFilter = "Binary Filter";
		fpPictureParams.v_SetType(Binary);
	}
	else if("HSV" == qstrItem)															// HSV
	{
		vFunction = Filter::HSV;
		qstrNameFilter = "HSV Filter";
		fpPictureParams.v_SetType(HSV);
	}		
	else if("Dilate" == qstrItem)														// DILATE
	{
		vFunction = Filter::Dilate;
		qstrNameFilter = "Dilate Filter";
		fpPictureParams.v_SetType(Dilate);
	}		
	else if("Erode" == qstrItem)														// ERODE
	{
		vFunction = Filter::Erode;
		qstrNameFilter = "Erode Filter";
		fpPictureParams.v_SetType(Erode);
	}		
	else if("Bilateral" == qstrItem)												// BILATERAL
	{
		vFunction = Filter::Bilateral;
		qstrNameFilter = "Bilateral Filter";
		fpPictureParams.v_SetType(Bilateral);
	}
	else if("Blur" == qstrItem)															// BLUR
	{
		vFunction = Filter::Blur;
		qstrNameFilter = "Blur Filter";
		fpPictureParams.v_SetType(Blur);
	}	
	
	ui->cmbFindUnitsFilter->removeItem(ui->cmbFindUnitsFilter->currentIndex());
	ui->cmbFindUnitsFilter->setCurrentIndex(-1);
	PictureFilters[2].v_Push(vFunction, fpPictureParams);
	ui->listFindUnits->addItem(qstrNameFilter);
}
void MainWindow::FindUnitSelect(int iIndex)
{
	if (0 <= iIndex)
	{
		bool * xptrShow;
		QStringList qstrlNames, qstrlData;
		int iFilter;
	
		if (0 == ui->cmbFilterUnitEvalPolarity->currentIndex())
			iFilter = 1;
		else if (1 == ui->cmbFilterUnitEvalPolarity->currentIndex())
			iFilter = 2;
		
		FilterParams fpDataIndex = PictureFilters[iFilter].i_GetIndex(iIndex);
		int iType = fpDataIndex.i_GetType();
		
		for (int i = 0; 6 > i; i++)
			qstrlData.push_back(QString::number(fpDataIndex.i_Param[i]));
	
		switch (iType)
		{
		case Binary:
			{
				bool xShow[] =  _BINARY;
				qstrlNames = QStringList(_BINARYNAMES);
				xptrShow = xShow;
				break;
			}
		case HSV:
			{
				bool xShow[] =  _HSV;
				qstrlNames = QStringList(_HSVNAMES);
				xptrShow = xShow;
				break;
			}
		case Dilate:
			{
				bool xShow[] =  _DILATE;
				qstrlNames = QStringList(_DILATENAMES);
				xptrShow = xShow;
				break;
			}
		case Erode:
			{
				bool xShow[] =  _ERODE;
				qstrlNames = QStringList(_ERODENAMES);
				xptrShow = xShow;
				break;
			}
		case Bilateral:
			{
				bool xShow[] =  _BILATERAL;
				qstrlNames = QStringList(_BILATERALNAMES);
				xptrShow = xShow;
				break;
			}
		case Blur:
			{
				bool xShow[] =  _BLUR;
				qstrlNames = QStringList(_BLURNAMES);
				xptrShow = xShow;
				break;
			}
		default:
			break;
		}
	
		if (0 <= iType)
		{
			ui->lblFindUnitsHI1->setVisible(*xptrShow++);
			ui->txtFindUnitsHI1->setVisible(*xptrShow++);
			ui->lblFindUnitsLO1->setVisible(*xptrShow++);
			ui->txtFindUnitsLO1->setVisible(*xptrShow++);
			ui->lblFindUnitsHI2->setVisible(*xptrShow++);
			ui->txtFindUnitsHI2->setVisible(*xptrShow++);
			ui->lblFindUnitsLO2->setVisible(*xptrShow++);
			ui->txtFindUnitsLO2->setVisible(*xptrShow++);
			ui->lblFindUnitsHI3->setVisible(*xptrShow++);
			ui->txtFindUnitsHI3->setVisible(*xptrShow++);
			ui->lblFindUnitsLO3->setVisible(*xptrShow++);
			ui->txtFindUnitsLO3->setVisible(*xptrShow++);		
			ui->lblFindUnitsHI1->setText(qstrlNames[0]);
			ui->lblFindUnitsLO1->setText(qstrlNames[1]);
			ui->lblFindUnitsHI2->setText(qstrlNames[2]);
			ui->lblFindUnitsLO2->setText(qstrlNames[3]);
			ui->lblFindUnitsHI3->setText(qstrlNames[4]);		
			ui->lblFindUnitsLO3->setText(qstrlNames[5]);
			ui->txtFindUnitsHI1->setText(qstrlData[0]);
			ui->txtFindUnitsLO1->setText(qstrlData[1]);
			ui->txtFindUnitsHI2->setText(qstrlData[2]);
			ui->txtFindUnitsLO2->setText(qstrlData[3]);
			ui->txtFindUnitsHI3->setText(qstrlData[4]);
			ui->txtFindUnitsLO3->setText(qstrlData[5]);	
		}
	}
	else
	{
		ui->lblFindUnitsHI1->setVisible(false);
		ui->txtFindUnitsHI1->setVisible(false);
		ui->lblFindUnitsLO1->setVisible(false);
		ui->txtFindUnitsLO1->setVisible(false);
		ui->lblFindUnitsHI2->setVisible(false);
		ui->txtFindUnitsHI2->setVisible(false);
		ui->lblFindUnitsLO2->setVisible(false);
		ui->txtFindUnitsLO2->setVisible(false);
		ui->lblFindUnitsHI3->setVisible(false);
		ui->txtFindUnitsHI3->setVisible(false);
		ui->lblFindUnitsLO3->setVisible(false);
		ui->txtFindUnitsLO3->setVisible(false);
	}
}
void MainWindow::FindUnitUp()
{
	int iCurrent = ui->listFindUnits->currentRow();
	int iSize = ui->listFindUnits->count();
	if (iCurrent > 0 && iCurrent <= iSize - 1)
	{
		PictureFilters[2].v_Swap(iCurrent, iCurrent - 1);
		QString qstrCurrentName = ui->listFindUnits->item(iCurrent)->text();
		ui->listFindUnits->item(iCurrent)->setText(
			ui->listFindUnits->item(iCurrent - 1)->text());
		ui->listFindUnits->item(iCurrent - 1)->setText(qstrCurrentName);
	}
}
void MainWindow::FindUnitDown()
{
	int iCurrent = ui->listFindUnits->currentRow();
	int iSize = ui->listFindUnits->count();
	if (iCurrent < iSize - 1 && 0 <= iCurrent)
	{
		PictureFilters[2].v_Swap(iCurrent, iCurrent + 1);
		QString qstrCurrentName = ui->listFindUnits->item(iCurrent)->text();
		ui->listFindUnits->item(iCurrent)->setText(
			ui->listFindUnits->item(iCurrent + 1)->text());
		ui->listFindUnits->item(iCurrent + 1)->setText(qstrCurrentName);
	}
}
void MainWindow::FindUnitDelete()
{
	int iCurrent = ui->listFindUnits->currentRow();
	if (0 <= iCurrent)
	{
		PictureFilters[2].v_Delete(iCurrent);
		int iSize = ui->listFindUnits->count();
		QList<QString> qlqstrFilter;
		for (int i = 0; i < iSize; i++)
			qlqstrFilter.push_back(ui->listFindUnits->item(i)->text());
	
		QString qstrItem = qlqstrFilter[iCurrent].left(
			qlqstrFilter[iCurrent].length() - 7);
		ui->listFindUnits->addItem(qstrItem);
		qlqstrFilter.removeAt(iCurrent);
		ui->listFindUnits->clear();
		ui->listFindUnits->addItems(qlqstrFilter);
	}
}
void MainWindow::FindUnitShow()
{
	if (!BatteryFound.x_EmptyArray())
	{
		int iFilter = 0;
		Polar Pole = (Polar)ui->cmbFilterUnitEvalPolarity->currentIndex();
		if (POS == Pole)
			int iFilter = 1;
		else if (NEG == Pole)
			int iFilter = 2;
		
		if (0 < PictureFilters[0].i_GetCount())
		{
			Mat MOriginalPicture = CutPicture(PictureSettings[iFilter], 
				PictureSettings[iFilter].i_Loop);
			MOriginalPicture.copyTo(MCurrentPicture);
			
			ExecuteFilters(PictureFilters[2], MOriginalPicture);									// Ejecuta filtro de evaluación
			
			Mat MImage = BatteryFound.M_PaintLocations(MOriginalPicture, Pole);
			v_ShowImage(MImage);
		}
	}
	if (0 >= PictureFilters[0].i_GetCount())
	{
		QMessageBox::critical(this,
			tr("Error Filters"),
			tr("Error in the image filters,\n" 
			"make sure that there are filters in the cartridge."),
			QMessageBox::Ok,
			QMessageBox::Ok);
	}
}

void MainWindow::FindUnitNext()
{
	int iFilter = -1;
	int iCurrent = ui->cmbFilterUnitEvalPolarity->currentIndex();
	string strPath = "";
	string strIma = "";
	
	if (POS == iCurrent) {
		iFilter = 1;
		strPath = str_Path + "UnitsP.xml";
		strIma = str_Path + "ImaP.xml";
	}
	else if (NEG == iCurrent) {
		iFilter = 2;
		strPath = str_Path + "UnitsN.xml";
		strIma = str_Path + "ImaN.xml";
	}
	if (iFilter > 0)
	{
		PictureFilters[iFilter].v_SaveFile(strPath);
		PictureSettings[iFilter].v_SaveFile(strIma);	
		
		ui->tabWizard->setCurrentIndex(6); // Cambia a la siguiente pestaña
	}
}
void MainWindow::FindUnitEstabilized()
{
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MResult = M_StabilizedImage(ui->numTakeStabilize->value());
	v_ShowImage(MResult);
	QMessageBox::information(this,
		tr("Image"),
		tr("The image was stabilized."),
		QMessageBox::Ok,
		QMessageBox::Ok);
	digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
}

void MainWindow::FindUnitsClear()
{
	ui->cmbFindUnitsFilter->setCurrentIndex(-1);
}

void MainWindow::FindUnitsChangePolarity(int iIndex)
{
	if(0 == MCurrentPicture.cols || 0 == MCurrentPicture.rows)
		MCurrentPicture = CutPicture(PictureSettings[0],1);
	
	if (0 <= iIndex) 
	{
		Mat MImage;
		switch (iIndex)
		{
		case 0:
			MImage = BatteryFound.M_PaintLocations(MCurrentPicture);
			break;
		case 1:
			MImage = BatteryFound.M_PaintLocations(MCurrentPicture, POS);
			break;
		case 2:
			MImage = BatteryFound.M_PaintLocations(MCurrentPicture, NEG);
			break;
		case 3:
			MImage = BatteryFound.M_PaintLocations(MCurrentPicture, UNDEF);
			break;
		default:
			break;
		}
		v_ShowImage(MImage);
	}
}

/********************* TAB TRAINING MODEL ******************/
void MainWindow::TrainingEvaluate()
{
	Polar Pole = POS;
	unsigned int count = 0;
	while (count <= 20 && !xDebug)
	{
		if (LOW == digitalRead(SENSORW1))
		{
			count = count + 1;
			digitalWrite(WINDOWS, 1);
			this_thread::sleep_for(chrono::milliseconds(200));
		}
		else
		{
			count = 0;
			digitalWrite(WINDOWS, 0);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
	Mat MOriginalPicture = CutPicture(PictureSettings[0],0);								// Toma la primer imagen para posición
	vector<Mat> vMCalibPictures;
	Mat MPositions, MResult;															// 
	int iTakes = ui->numTakeTraining->value();
	MOriginalPicture.copyTo(MResult);
	MOriginalPicture.copyTo(M_BackUp);
	ExecuteFilters(PictureFilters[0], MOriginalPicture); // Ejecuta Filtro de posición
	BatteryFound.M_FindContourns(MOriginalPicture);										// Busca contornos
	if (0 < BatteryFound.i_Find()) {													// Si encuentra contornos
		BatteryFound.M_CreateArray(MOriginalPicture);									// Crea el arreglo de las unidades 
		for (int i = 0; i < iTakes; i++)
		{
			Mat MTmp = M_StabilizedImage(ui->numTakeStabilize->value());
			vMCalibPictures.push_back(MTmp);
			if (!ui->chkSameUnit->isChecked())
			{
				digitalWrite(STOPPER, HIGH);
				digitalWrite(WINDOWS, LOW);	// modificacion
				QMessageBox::information(this,
					"Pause: Unit " + QString::number(iTakes+1),
					tr("Change the unit or battery to take a new picture,"
					"and press OK to continue."),
					QMessageBox::Ok,
					QMessageBox::Ok);
				digitalWrite(STOPPER, LOW);
				
				while (HIGH == digitalRead(SENSORU))
					this_thread::sleep_for(chrono::milliseconds(10));
				digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
				this_thread::sleep_for(chrono::seconds(1));
			}
		}
		bool xAdd = false;
		if (Checked == ui->chkAdd->checkState())
			xAdd = true;
		BatteryFound.v_ExtractSizes(Pole, vMCalibPictures, xAdd); // Extrae el primer dato de cada fila
		BatteryFound.v_ShowData(Pole, MResult);
		v_ShowImage(MResult);
		Size sBatterySize = BatteryFound.sGetSize(Pole); // Calcula el tamaño de la bateria
		ui->lblColumnsTest->setText(QString::number(sBatterySize.width)); // Muestra en pantalla el ancho
		ui->lblRowsTest->setText(QString::number(sBatterySize.height)); // y alto de la bateria
		
		v_FillDataStatistic();
		
	}
}
void MainWindow::TrainingNext()
{
	Polar Pole = POS;
	map<string, int> msiIO;
	msiIO.insert(pair<string, int>("UP",		UP));
	msiIO.insert(pair<string, int>("LEFT",		LEFT));
	msiIO.insert(pair<string, int>("RIGHT",		RIGHT));
	msiIO.insert(pair<string, int>("DOWN",		DOWN));
	msiIO.insert(pair<string, int>("LIGHT",		LIGHT));
	msiIO.insert(pair<string, int>("STOPPER",	STOPPER));
	msiIO.insert(pair<string, int>("SENSORU",	SENSORU));
	msiIO.insert(pair<string, int>("BUTTON",	BUTTON));
	msiIO.insert(pair<string, int>("WINDOWS",	WINDOWS));
	
	string strNameFile = "IOs.xml";
	FileStorage fs_SaveStream(strNameFile, FileStorage::WRITE);
	fs_SaveStream <<  "Loop" << ui->numTakeStabilize->value();
	fs_SaveStream <<  "IO" << "{";
	fs_SaveStream <<  "Size" << (int)msiIO.size();
	fs_SaveStream <<  "Key" << "[:";
	for (auto aIterator = msiIO.begin(); aIterator != msiIO.end(); aIterator++)
		fs_SaveStream << aIterator->first;
	fs_SaveStream <<  "]";
	fs_SaveStream <<  "Data" << "[:";
	for (auto aIterator = msiIO.begin(); aIterator != msiIO.end(); aIterator++)
		fs_SaveStream << aIterator->second;
	fs_SaveStream <<  "]";
	fs_SaveStream <<  "}";
	fs_SaveStream.release();
	
	BatteryFound.v_ClearDStandar(Pole);
	BatteryFound.v_SetDStandar(Pole, 0, ui->dsDStandarMax->value()); 
	BatteryFound.v_SetDStandar(Pole, 1, ui->dsDStandarMin->value());
	ui->cmdSaveModel->setVisible(true);
	ui->grpSave->setVisible(true);
	ui->cmdOpenModel->setVisible(false);
	ui->cmdNewModel->setVisible(false);
	ui->cmdOpenModelNext->setVisible(false);
	ui->tabWizard->setCurrentIndex(0);
	
	ui->tabWizard->setCurrentIndex(0); // Cambia a la siguiente pestaña
	BatteryFound.v_SaveFile(str_Path + "Positions.xml");	
}
void MainWindow::ShowBW(bool xActive)
{
	if (xActive)
		v_ShowImage(M_Segmented);
	else
		v_ShowImage(M_BackUp);
}
void MainWindow::Release(bool xActive)
{
	if (xActive)
		digitalWrite(STOPPER, HIGH);
	else
		digitalWrite(STOPPER, LOW);
}
void MainWindow::TrainingSelectRow(int iRow)
{	
	if (iRow >= 0)
	{
		vector<int> viData = vvi_Statistics[iRow];
		QString qstrMessage;
	
		ui->lblShowAverage->setText(QString::number(viData[1]));
		ui->lblShowMaxV->setText(QString::number(viData[2]));
		ui->lblShowMinV->setText(QString::number(viData[3]));
		ui->lblShowMaxT->setText(QString::number(viData[4]));
		ui->lblShowMinT->setText(QString::number(viData[5]));
	
		if (0 == viData[6])
			qstrMessage = "Training OK";
		else
		{
			qstrMessage = "Row " + QString::number(iRow);
			if (0x000f == (viData[6] & 0x000f))
				qstrMessage += "|Max values fail|";
			if (0x00f0 == (viData[6] & 0x00f0))
				qstrMessage += "|Min values fail|";
			if (0x0f00 == (viData[6] & 0x0f00))
				qstrMessage += "|Average above max|";
			if (0xf000 == (viData[6] & 0x0f00))
				qstrMessage += "|Average under min|";
		}
		ui->lblSummary->setText(qstrMessage);
	}
}

/********************* TAB TEST RUN ************************/
void MainWindow::TestModel()
{
	if (Filter::chkStatus())
	{
		Polar Pole = POS;
		ui->cmbShowSecondFilter->currentIndex();
		unsigned int count = 0;
		while (count <= 20 && !xDebug)
		{
			if (LOW == digitalRead(SENSORW1))
			{
				count = count + 1;
				digitalWrite(WINDOWS, 1);
				this_thread::sleep_for(chrono::milliseconds(200));
			}
			else
			{
				count = 0;
				digitalWrite(WINDOWS, 0);
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100)); // MODIFICACION DEL 24 DE MARZO
		Mat MOriginalPicture = CutPicture(PictureSettings[0], 0); // Toma la primer imagen para posición
		Mat MResult, MShow; //
		vector<Point> vpMissing;
		MOriginalPicture.copyTo(MResult);
		vector<pair<int, Point>> vipDataErrors;
		ExecuteFilters(PictureFilters[0], MOriginalPicture); // Ejecuta Filtro de posición
		BatteryFound.v_ClearDStandar(Pole);
		BatteryFound.v_SetDStandar(Pole, 0, ui->dsDStandarMax->value());
		BatteryFound.v_SetDStandar(Pole, 1, ui->dsDStandarMin->value());
		BatteryTest = BatteryFound;
		BatteryTest.M_FindContourns(MOriginalPicture); // Busca contornos
		if (0 < BatteryTest.i_Find()) {
			// Si encuentra contornos
			BatteryTest.M_CreateArray(MOriginalPicture); // Crea el arreglo de las unidades 
			
			vector<map<int, vector<Point>>> v_map_vipData; //= BatteryTest.v_map_ivp_GetArray(Pole);
			vpMissing = BatteryFound.vp_MatchArray(Pole, v_map_vipData);
			
			MOriginalPicture = M_StabilizedImage(ui->numTakeStabilize->value());
			MOriginalPicture.copyTo(M_Segmented);
			BatteryTest.v_ExtractSizes(Pole, MOriginalPicture); // Extrae los datos de dimensiones de la bateria
			
			if (BatteryTest.x_Evaluate(BatteryFound,
				vipDataErrors, 
				MOriginalPicture, 
				MResult))
			{
				ui->lblResult->setText("Success: Inspect Complete.");
				ui->lstErrors->clear();
				QMessageBox::information(this,
					tr("Success"),
					tr("Test successfully completed"),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
			else
			{
				int iError;
				Point pLocation;
				ui->lblResult->setText(QString::number(vipDataErrors.size()) + 
					" errors are found.");
				ui->lstErrors->clear();
				BatteryTest.v_DrawMissing(Pole, MResult, vpMissing);
				for (auto aIterator = vipDataErrors.begin();
				aIterator != vipDataErrors.end(); aIterator++)
				{
					tie(iError, pLocation) = *aIterator;
					if (0 > iError) {
						ui->lstErrors->addItem(QString::number(pLocation.x) + 
							" counted units at row " + QString::number(pLocation.y));
						ui->lblResult->setText(ui->lblResult->text() 
							+ "\nIncomplete rows found.");
					
					}
					else
						ui->lstErrors->addItem(QString::number(iError) +
							"|X:" + QString::number(pLocation.x) +
							" Y:" + QString::number(pLocation.y));
				}
				QMessageBox::information(this,
					tr("Fail"),
					tr("Fail test, check status"),
					QMessageBox::Ok,
					QMessageBox::Ok);
			}
			digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
			MResult.copyTo(M_BackUp);
			v_ShowImage(MResult);
			ui->lblColumnsTest->setText(QString::number(BatteryFound.i_MaxColumn(Pole)));
			Size sBatterySize = BatteryFound.sGetSize(Pole);
			ui->lblColumnsTest->setText(QString::number(sBatterySize.width)); // Muestra en pantalla el ancho
			ui->lblRowsTest->setText(QString::number(sBatterySize.height)); // y alto de la bateria
		}
		digitalWrite(WINDOWS, 0); //MODIFICACION DEL 24 DE MARZO Ventanas arriba
	}
	else {
		QMessageBox::warning(this,
			tr("Error"),
			tr("Memory corrupted."),
			QMessageBox::Ok,
			QMessageBox::Ok);
		close();
	}
}

void MainWindow::ChangePolarity(int iIndex)
{
//	ui->cmbFilterPosUnitPolarity->setCurrentIndex(iIndex);
//	ui->cmbFindUnitsFilter->setCurrentIndex(iIndex);
//	ui->cmbFilterUnitEvalPolarity->setCurrentIndex(iIndex);
//	ui->cmbShowSecondFilter->setCurrentIndex(iIndex);
}

void MainWindow::Position()
{
	if (!BatteryFound.x_EmptyArray())
	{
		Mat MResult, MImage;
		QPoint qNew = (QCursor::pos() - this->window()->pos()) - 
			ui->picImageShow->pos();
		int iSquare = BatteryFound.i_GetSquare();
		
		p_Last = Point(qNew.x() * d_Scale, qNew.y() * d_Scale);
		
		//MImage = BatteryFound.M_CreateArray(MCurrentPicture);
		if ((iSquare * 2) > (p_Last.x - p_First.x)) {
			p_Last.x = p_First.x + 49;
			p_First.x = p_First.x - 49;
		}
		if ((iSquare * 2) > (p_Last.y - p_First.y)) {
			p_Last.y = p_First.y + 49;
			p_First.y = p_First.y - 49;
		}
		Polar pCurrentPole = BatteryFound.pl_CheckPolarity(p_First, p_Last);
		if (UNDEF <= pCurrentPole)
		{
			switch (pCurrentPole)
			{
			case UNDEF:
				pCurrentPole = POS;
				break;
			case POS:
				pCurrentPole = NEG;
				break;
			case NEG:
				pCurrentPole = UNDEF;
				break;
			default:
				break;
			}			
			BatteryFound.x_ChangePolarity(p_First, p_Last, pCurrentPole);
			
			//rotate(MCurrentPicture, MImage, ROTATE_90_CLOCKWISE);
			MImage = BatteryFound.M_PaintLocations(MCurrentPicture);
			//rotate(MImage, MResult, ROTATE_90_COUNTERCLOCKWISE);
		}
	
		v_ShowImage(MImage);
	}
	else
		v_ShowImage(MCurrentPicture);
}

void MainWindow::StartPosition()
{
	QPoint qNew = (QCursor::pos() - this->window()->pos()) - 
			ui->picImageShow->pos();
	int iWidth = ui->txtPicWidth->text().toInt();
	int iHeight = ui->txtPicHeight->text().toInt();
	
	if (iWidth > iHeight) {
		if (900 != iWidth)
			d_Scale = (double)iWidth / 900;
	}
	else
	{
		if (900 != iHeight)
			d_Scale = (double)iHeight / 900;
	}
		
	p_First = Point(qNew.x() * d_Scale, qNew.y() * d_Scale);
}

void MainWindow::Draw()
{
	QPoint qNew = (QCursor::pos() - this->window()->pos()) - 
			ui->picImageShow->pos();
		
	Point p_Last = Point(qNew.x() * d_Scale, qNew.y() * d_Scale);
	
	Mat MTemp;
	
	MCurrentPicture.copyTo(MTemp);
	rectangle(MTemp,
		p_First,													// Dibuja en la nueva imagen el rectangulo del contorno
		p_Last, 
		Scalar(255, 0, 0), 
		3);
	v_ShowImage(MTemp);
}

/********************* SPECIAL FUNCTIONS *********************/

Mat MainWindow::TakePicture(ImageProcessSettings ipsData)
{
	v_LightOn();
	cCameraPi::v_TakePicture(ipsData);
	v_LightOff();
	return Mat(cCameraPi::M_LoadPicture(ipsData.str_Path));
}
Mat MainWindow::LoadPicture()
{
	return Mat(cCameraPi::M_LoadPicture(
		PictureSettings[POS].str_Path));
}
Mat MainWindow::CutPicture(ImageProcessSettings ipsData, int iTake)
{
	Mat MOriginalPicture, MBackup;
	if (0 == iTake)
		MOriginalPicture = TakePicture(ipsData);
	else
		MOriginalPicture = LoadPicture();
	MOriginalPicture.copyTo(MBackup); // Genera un respaldo de la imagen
	
	if (Qt::Checked == ui->chkUseCalibration->checkState())								// Verifica si necesita calibrar
	{
		MOriginalPicture = DataCalib.MImage(MOriginalPicture);							// Calibra
		if (0 == MOriginalPicture.cols && 0 == MOriginalPicture.rows)
		{
			QMessageBox::critical(this,
				tr("Invalid calibration"),
				tr("check for calibration data, recalibrate or upload a valid calibration"),
				QMessageBox::Ok,
				QMessageBox::Ok);
		}
	}
	
	if (0 == MOriginalPicture.cols && 0 == MOriginalPicture.rows)						// Verifica que no este vacio el resultado
		MBackup.copyTo(MOriginalPicture);												// Si esta vacio recupera el respaldo

	return Mat(cCameraPi::M_CutImage(MOriginalPicture, 
		PictureSettings[0]));																// Corta la imagen
}
void MainWindow::ExecuteFilters(FilterCartridge fcFilters, Mat &MOriginalPicture)
{
	fcFilters.v_SetStart();
	int iSize = fcFilters.i_GetCount();
	
	for (int i = 0; i < iSize; i++)
		fcFilters.v_Execute(MOriginalPicture);
}
void MainWindow::v_LightOn()
{
	digitalWrite(UP, 1);
	digitalWrite(LEFT, 1);
	digitalWrite(RIGHT, 1);
	digitalWrite(DOWN, 1);
	digitalWrite(LIGHT, 1);
}
void MainWindow::v_LightOff()
{
	digitalWrite(UP, 0);
	digitalWrite(LEFT, 0);
	digitalWrite(RIGHT, 0);
	digitalWrite(DOWN, 0);
	digitalWrite(LIGHT, 0);
}

/********************* OTHER FUNCTIONS *********************/
void MainWindow::setUser(QString qstrUserName)
{
	this->setWindowTitle(this->windowTitle() + " | " + qstrUserName);
}
FilterParams MainWindow::CreateSetFilterParams(Mat InitialPicture)
{
	array<int, 6> aiParam;
	vector<Mat> vMImages;
	if (InitialPicture.rows <= 0 || InitialPicture.cols <= 0)
		vMImages.push_back(Mat());	
	else
		vMImages.push_back(InitialPicture);
	vMImages.push_back(Mat());
	
	aiParam[0] = ui->txtSetFilterHI1->text().toInt(); 
	aiParam[1] = ui->txtSetFilterLO1->text().toInt(); 
	aiParam[2] = ui->txtSetFilterHI2->text().toInt(); 
	aiParam[3] = ui->txtSetFilterLO2->text().toInt(); 
	aiParam[4] = ui->txtSetFilterHI3->text().toInt();
	aiParam[5] = ui->txtSetFilterLO3->text().toInt();
	
	return FilterParams(vMImages, aiParam, new int);
}
FilterParams MainWindow::CreateFindUnitsParams(Mat InitialPicture)
{
	array<int, 6> aiParam;
	vector<Mat> vMImages;
	if (InitialPicture.rows <= 0 || InitialPicture.cols <= 0)
		vMImages.push_back(Mat());	
	else
		vMImages.push_back(InitialPicture);
	vMImages.push_back(Mat());
	
	aiParam[0] = ui->txtFindUnitsHI1->text().toInt(); 
	aiParam[1] = ui->txtFindUnitsLO1->text().toInt(); 
	aiParam[2] = ui->txtFindUnitsHI2->text().toInt(); 
	aiParam[3] = ui->txtFindUnitsLO2->text().toInt(); 
	aiParam[4] = ui->txtFindUnitsHI3->text().toInt();
	aiParam[5] = ui->txtFindUnitsLO3->text().toInt();
	
	return FilterParams(vMImages, aiParam, new int);
}
array<int, 5> MainWindow::CreateObject()
{
	array<int, 5> aiParam;
	aiParam[0] = ui->txtMaxEdges->text().toInt(); 
	aiParam[1] = ui->txtMinEdges->text().toInt();
	aiParam[2] = ui->txtFindPositionSquare->text().toInt();
	aiParam[3] = ui->txtFindPositionMargin->text().toInt();
	aiParam[4] = ui->txtFindPositionElongation->text().toInt();
	
	return aiParam;
}
bool MainWindow::RestoreProcessingPicture(bool xLoad)
{
	if (!xLoad)
		xLoad = PictureSettings[0].x_LoadFile(str_Path + "Picture.xml");
	if (xLoad)
	{
		ui->txtPicToTake->setText(PictureSettings[0].qLoop());
		ui->txtPicResWidth->setText(PictureSettings[0].qResWidth());
		ui->txtPicResHeight->setText(PictureSettings[0].qResHeight());
		ui->txtPicISO->setText(PictureSettings[0].qISO());
		ui->txtPicSS->setText(PictureSettings[0].qISS());
		ui->txtPicOffsetX->setText(PictureSettings[0].qOffsetX());
		ui->txtPicOffsetY->setText(PictureSettings[0].qOffsetY());
		ui->txtPicWidth->setText(PictureSettings[0].qWidth());
		ui->txtPicHeight->setText(PictureSettings[0].qHeight());
		return true;
	}
	return false;
}
bool MainWindow::RestoreProcessingImage(bool xLoad)
{
	bool xOK;
	if (!xLoad)
	{
		xOK = PictureSettings[1].x_LoadFile(str_Path + "ImaP.xml");
		xLoad = PictureSettings[2].x_LoadFile(str_Path + "ImaN.xml");
		if (!(xOK && xLoad)) {
			xLoad = PictureSettings[1].x_LoadFile(str_Path + "Image.xml");
			xLoad = PictureSettings[2].x_LoadFile(str_Path + "Image.xml");
		}
		else
			xLoad = false;
	}
	if (xLoad)
	{
		ui->txtImaToTake->setText(PictureSettings[1].qLoop());
		ui->txtImaISO->setText(PictureSettings[1].qISO());
		ui->txtImaSS->setText(PictureSettings[1].qISS());
		return true;
	}
	return false;
}
bool MainWindow::RestoreFiltersPositions(bool xLoad)
{
	if (!xLoad)
	{
		if (100 >= i_Version)
		{
			PictureFilters[0].x_LoadFile(str_Path + "FiltersPositions.xml");
			xLoad = true;
		}
		else
		{
			PictureFilters[0].x_LoadFile(str_Path + "FPP.xml");
			xLoad = true;
		}
	}
	
	if (xLoad)
	{
		vector<FilterParams> vfpData = PictureFilters[0].vfp_GetParams();
		vector<FilterParams>::iterator vfp_iIterator = vfpData.begin();
		ui->lstSetFilterFilter->clear();
		int iSize = ui->cmbFindUnitsFilter->count();
		QString qstrType = "";
		bool xPass = false;
		for (; vfp_iIterator != vfpData.end(); vfp_iIterator++)
		{
			int iParams[6];
			for(int i = 0; i < 6; i++)
				iParams[i] = vfp_iIterator->i_Param[i];
			int iType = vfp_iIterator->i_GetType();
			qstrType = "";	
			switch (iType)
			{
			case Binary:
				qstrType = "Binary";
				xPass = true;
				break;
			case HSV:
				qstrType = "HSV";
				xPass = true;
				break;
			case Dilate:
				qstrType = "Dilate";
				xPass = true;
				break;
			case Erode:
				qstrType = "Erode";
				xPass = true;
				break;
			case Bilateral:
				qstrType = "Bilateral";
				xPass = true;
				break;
			case Blur:
				qstrType = "Blur";
				xPass = true;
				break;
			default:
				break;
			}
			if (xPass)
			{
				for (int i = 0; i < iSize; i++)
				{
					if (qstrType == ui->cmbSetFilterFilter->itemText(i))
					{
						ui->cmbSetFilterFilter->removeItem(i);
						iSize--;
						break;
					}
				}
				ui->lstSetFilterFilter->addItem(qstrType + " Filter");
			}
		}
		return true;
	}
	return false;
}
bool MainWindow::RestoreFiltersUnits(bool xLoad)
{
	if (!xLoad)
	{
		if (100 >= i_Version)
		{
			PictureFilters[1].x_LoadFile(str_Path + "Units.xml");
			PictureFilters[2] = PictureFilters[1];
			xLoad = true;
		}
		else 
		{
			PictureFilters[1].x_LoadFile(str_Path + "UnitsP.xml");
			PictureFilters[2].x_LoadFile(str_Path + "UnitsN.xml");
			xLoad = true;
		}
	}
	
	if (xLoad)
	{
		vector<FilterParams> vfpData = PictureFilters[1].vfp_GetParams();
		vector<FilterParams>::iterator vfp_iIterator = vfpData.begin();
		ui->listFindUnits->clear();
		int iSize = ui->cmbFindUnitsFilter->count();
		QString qstrType = "";
		bool xPass = false;
		for (; vfp_iIterator != vfpData.end(); vfp_iIterator++)
		{
			int iParams[6];
			for (int i = 0; i < 6; i++)
				iParams[i] = vfp_iIterator->i_Param[i];
			int iType = vfp_iIterator->i_GetType();
			qstrType = "";	
			switch (iType)
			{
			case Binary:
				qstrType = "Binary";
				xPass = true;
				break;
			case HSV:
				qstrType = "HSV";
				xPass = true;
				break;
			case Dilate:
				qstrType = "Dilate";
				xPass = true;
				break;
			case Erode:
				qstrType = "Erode";
				xPass = true;
				break;
			case Bilateral:
				qstrType = "Bilateral";
				xPass = true;
				break;
			case Blur:
				qstrType = "Blur";
				xPass = true;
				break;
			default:
				break;
			}
			if (xPass)
			{
				xPass = false;
				for (int i = 0; i < iSize; i++)
				{
					if (qstrType == ui->cmbFindUnitsFilter->itemText(i))
					{
						ui->cmbFindUnitsFilter->removeItem(i);
						iSize--;
						break;
					}
				}
				ui->listFindUnits->addItem(qstrType + " Filter");
			}
		}
		return true;
	}
	return false;
}
bool MainWindow::RestorePositions(bool xLoad)
{
	Polar Pole;
	int iVersion = 0;
	if (!xLoad)
		xLoad = BatteryFound.x_LoadFile(str_Path + "Positions.xml");
		
	iVersion = BatteryFound.i_GetVersion();
	
	if (xLoad)
	{
		Pole = POS;
		if (100 >= iVersion) 
		{
			ui->chkInspectionprocess->setCheckState(CheckState::Unchecked);
			ui->cmbFindUnitPosPolarity->setEnabled(false);
			ui->cmbFilterUnitEvalPolarity->setEnabled(false);
			ui->cmbShowSecondFilter->setEnabled(false);
		}
		else 
		{
			ui->chkInspectionprocess->setCheckState(CheckState::Checked);
			ui->cmbFindUnitPosPolarity->setEnabled(true);
			ui->cmbFilterUnitEvalPolarity->setEnabled(true);
			ui->cmbShowSecondFilter->setEnabled(true);
		}
		
		ui->cmbShowSecondFilter->setCurrentIndex(0);
		ui->cmbFilterUnitEvalPolarity->setCurrentIndex(0);
		ui->cmbFindUnitPosPolarity->setCurrentIndex(0);
		
		ui->txtMaxEdges->setText(BatteryFound.qMax());
		ui->txtMinEdges->setText(BatteryFound.qMin());
		ui->txtFindPositionSquare->setText(BatteryFound.qSquare());
		ui->txtFindPositionMargin->setText(BatteryFound.qMargin());
		ui->txtFindPositionElongation->setText(BatteryFound.qElongation());
		
		Mat MOriginalPicture = CutPicture(PictureSettings[0],0);								// Toma la primer imagen para posición
		Mat MPositions, MResult;															// 
		MOriginalPicture.copyTo(M_BackUp);
		ui->dsDStandarMax->setValue(BatteryFound.d_GetDStandar(Pole, 0));
		ui->dsDStandarMin->setValue(BatteryFound.d_GetDStandar(Pole, 1));
		if (0 > PictureFilters[Pole].i_GetCount())
		{
			ExecuteFilters(PictureFilters[Pole], MOriginalPicture); // Ejecuta Filtro de posición
			BatteryFound.M_FindContourns(MOriginalPicture); // Busca contornos
			if (0 < BatteryFound.i_Find())
			{
				// Si encuentra contornos
				//if (0 == BatteryFound.v_map_ivp_GetArray(Pole).size())
					BatteryFound.M_CreateArray(MOriginalPicture);							// Crea el arreglo de las unidades 
				MOriginalPicture = CutPicture(PictureSettings[1], 0);							// Toma la segunda imagen para evaluación
				ExecuteFilters(PictureFilters[Pole], MOriginalPicture);								// Ejecuta filtro de evaluación
				MResult = BatteryFound.M_ShowUnits(Pole, M_BackUp); // Extrae el primer dato de cada fila
				BatteryFound.v_ShowData(Pole, MResult);
			
				Size sBatterySize = BatteryFound.sGetSize(Pole);
				ui->lblColumnsTest->setText(QString::number(sBatterySize.width)); // Muestra en pantalla el ancho
				ui->lblRowsTest->setText(QString::number(sBatterySize.height)); // y alto de la bateria		
			}
			
		}
		else
			MOriginalPicture.copyTo(MResult);
		v_ShowImage(MResult);
		return true;
	}
	return false;
}
bool MainWindow::RestoreCalib(bool xLoad)
{
	if (!xLoad)
		xLoad = DataCalib.x_LoadFile(str_Path + "Calib.xml");
	if (xLoad)
	{
		ui->txtCalibrateHeight->setText(DataCalib.qHeight());
		ui->txtCalibrateWidth->setText(DataCalib.qWidth());
		if (DataCalib.x_Calibrated()) {
			ui->chkUseCalibration->setCheckState(Checked);
			return true;
		}
		else
			ui->chkUseCalibration->setCheckState(Unchecked);
	}
	return false;
}
bool MainWindow::RestoreCalib(QString qstrPath, bool xLoad)
{
	if (!xLoad)
		xLoad = DataCalib.x_LoadFile(qstrPath.toStdString());
	if (xLoad)
	{
		ui->txtCalibrateHeight->setText(DataCalib.qHeight());
		ui->txtCalibrateWidth->setText(DataCalib.qWidth());
		if (DataCalib.x_Calibrated())
		{
			ui->chkUseCalibration->setCheckState(Checked);
			DataCalib.v_SaveFile(str_Path + "Calib.xml");
			return true;
		}
		else
			ui->chkUseCalibration->setCheckState(Unchecked);
	}
	return false;
}
bool MainWindow::RestoreIO()
{
	FileStorage fs_OpenStream("IOs.xml", FileStorage::READ);
	if (fs_OpenStream.isOpened())
	{
		map<string, int> mstriIO;
		vector<string> vstrNames;
		
		ui->numTakeStabilize->setValue((int)fs_OpenStream["Loop"]);
		FileNode fnVector = fs_OpenStream["IO"];
		int iSize = (int)fnVector["Size"];
		FileNode fnNames = fnVector["Key"];
		int i;
		for (i = 0; i < iSize; i++)
			vstrNames.push_back((string)fnNames[i]);
		
		i = 0;
		FileNode fnData = fnVector["Data"];
		for (auto aIterator = vstrNames.begin(); aIterator != vstrNames.end(); aIterator++)
			mstriIO.insert(make_pair(*aIterator, (int)fnData[i++]));
		fs_OpenStream.release();
		
		for (auto aIterator = mstriIO.begin(); aIterator != mstriIO.end(); aIterator++)
		{
			if ("UP" == aIterator->first)
				UP = aIterator->second;
			if ("LEFT" == aIterator->first)
				LEFT = aIterator->second;
			if ("RIGHT" == aIterator->first)
				RIGHT = aIterator->second;
			if ("DOWN" == aIterator->first)
				DOWN = aIterator->second;
			if ("LIGHT" == aIterator->first)
				LIGHT = aIterator->second;
			
			if ("SENSORU" == aIterator->first)
				SENSORU = aIterator->second;
			if ("STOPPER" == aIterator->first)
				STOPPER = aIterator->second;
			if ("BUTTON" == aIterator->first)
				BUTTON = aIterator->second;
		}
		
		ui->txtLightUp->setText(	QString::number(UP));
		ui->txtLightDown->setText(	QString::number(DOWN));
		ui->txtLightLeft->setText(	QString::number(LEFT));
		ui->txtLightRight->setText(	QString::number(RIGHT));
		
		ui->txtSensorUnit->setText(	QString::number(SENSORU));
		ui->txtSolenoid->setText(	QString::number(STOPPER));
		ui->txtButton->setText(		QString::number(BUTTON));
		ui->txtWindows->setText(	QString::number(WINDOWS));
		
		pinMode(UP, OUTPUT);
		pinMode(LEFT, OUTPUT);
		pinMode(RIGHT, OUTPUT);
		pinMode(DOWN, OUTPUT);
		pinMode(LIGHT, OUTPUT);
		pinMode(STOPPER, OUTPUT);
		pinMode(WINDOWS, OUTPUT);
		
		pinMode(SENSORU, INPUT);
		pinMode(BUTTON,  INPUT);
		pinMode(SENSORW1, INPUT);
		pinMode(SENSORW2, INPUT);
		
		return true;
	}
	return false;
}
void MainWindow::v_FillDataStatistic()
{
	Polar Pole = POS;
	vector<vector<int>> vviBruteStatistics = BatteryFound.vvi_GetStadistics(Pole);
	QListWidgetItem * qlwiData;
	QBrush qbError(QColor(255, 0, 0, 255));
	qbError.setStyle(Qt::SolidPattern);
	QBrush qbPass(QColor(0, 0, 0, 255));
	qbPass.setStyle(Qt::SolidPattern);
	QString qstrRow;
	int iDStandar = 0;
	int iMedia = 0;
	Point pMaxMin;
	
	ui->lstTraining->clear();
	vvi_Statistics.clear();
	for (auto aIterator = vviBruteStatistics.begin(); aIterator != vviBruteStatistics.end(); aIterator++) 
	{
		vector<int> viData;
		qlwiData = new QListWidgetItem();
		int iFail = 0;
		pMaxMin = BatteryFound.p_CalculateMaxMin(*aIterator);
		iDStandar = BatteryFound.d_CalculateDStandar(*aIterator);
		viData.push_back(iDStandar);
		iMedia = BatteryFound.d_CalculateMedia(*aIterator);
		viData.push_back(iMedia);
		viData.push_back(pMaxMin.x);
		viData.push_back(pMaxMin.y);
		viData.push_back(iMedia + (BatteryFound.d_GetDStandar(Pole, 0) * iDStandar));
		viData.push_back(iMedia - (BatteryFound.d_GetDStandar(Pole, 1) * iDStandar));
			
		if (viData[2] >= viData[4])
			iFail = iFail | 0x000f;
		if (viData[3] <= viData[5])
			iFail = iFail | 0x00f0;
		if (viData[1] >= viData[4])
			iFail = iFail | 0x0f00;
		if (viData[1] <= viData[5])
			iFail = iFail | 0xf000;
			
		viData.push_back(iFail);
		vvi_Statistics.push_back(viData);
			
		qstrRow =	"MxV:" + QString::number(viData[2]) + "|" +
					"MnV:" + QString::number(viData[3]) + "|" +
					"Avg:" + QString::number(viData[1]) + "|" +
					"MxT:" + QString::number(viData[4]) + "|" +
					"MnT:" + QString::number(viData[5]) + "|" + 
					"Err:" + QString::number(viData[6]);
			
		qlwiData->setText(qstrRow);
		if (0 == viData[6])
			qlwiData->setForeground(qbPass);
		else
			qlwiData->setForeground(qbError);
			
		ui->lstTraining->addItem(qlwiData);
	}
}
Mat MainWindow::M_StabilizedImage(int iTimes)
{
	Mat MResultAdd;
	for (int i = 0; i < iTimes; i++)
	{
		Mat MTmp = CutPicture(PictureSettings[1], 0); // Toma la segunda imagen para evaluación
		ExecuteFilters(PictureFilters[2], MTmp); // Ejecuta filtro de evaluación
		if (MResultAdd.cols == 0 && MResultAdd.rows == 0)
			MResultAdd = MTmp;
		else
			add(MResultAdd, MTmp, MResultAdd);
	}
	return MResultAdd;
}
void MainWindow::v_RestoreListFilters()
{
	ui->cmbFindUnitsFilter->clear();
	ui->cmbFindUnitsFilter->addItem("Binary");
	ui->cmbFindUnitsFilter->addItem("HSV");
	ui->cmbFindUnitsFilter->addItem("Dilate");
	ui->cmbFindUnitsFilter->addItem("Erode");
	ui->cmbFindUnitsFilter->addItem("Bilateral");
	ui->cmbFindUnitsFilter->addItem("Blur");
	ui->cmbFindUnitsFilter->setCurrentIndex(-1);
	
	
	ui->cmbSetFilterFilter->clear();
	ui->cmbSetFilterFilter->addItem("Binary");
	ui->cmbSetFilterFilter->addItem("HSV");
	ui->cmbSetFilterFilter->addItem("Dilate");
	ui->cmbSetFilterFilter->addItem("Erode");
	ui->cmbSetFilterFilter->addItem("Bilateral");
	ui->cmbSetFilterFilter->addItem("Blur");
	ui->cmbSetFilterFilter->setCurrentIndex(-1);
}

/*********************************************************/