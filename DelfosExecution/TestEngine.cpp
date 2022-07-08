#include "TestEngine.h"
#include "MainWindow.h"
#include "../DelfosInspect/mat2qimage.h"
#include <iostream>


using namespace cv;
using namespace Qt;

ManagerEngine::ManagerEngine()
{

	
}

ManagerEngine::~ManagerEngine()
{
	i_StatusEngine = _Exit;
	this_thread::sleep_for(chrono::milliseconds(500));
	xDebug = false;
}

void ManagerEngine::sl_EngineInspection(const InspectMachine & machine)
{
	Machine = machine;
	i_StatusEngine = _Initialized;
}
void ManagerEngine::sl_StopEngine()
{
	i_StatusEngine = _Halt;
}
void ManagerEngine::sl_Test()
{
	xDebug = true;
}
void ManagerEngine::sl_Readed()
{
	xReaded = true;
}
void ManagerEngine::sl_Clear()
{
	xReaded = false;
	i_StatusEngine = _ReadSerial;
}
void ManagerEngine::run()
{
	Mat MOriginalPicture, MResult, MTemp;
	time_t timer_begin, timer_end;
	QImage image;
	QPixmap pixmap;
	vector<Point> vpMissing;
	ObjectUnit BatteryTest;
	vector<pair<int, Point>> vipDataErrors;
	vector<map<int, vector<Point>>> v_map_vipData;
	pair<QPixmap, vector<pair<int, Point>>> pMVpData;
	xReaded = false;
	xDebug = false;
	double secondsElapsed;
	
	i_StatusEngine = _Idle;
	do {
		while (_Halt > i_StatusEngine)
		{
			switch (i_StatusEngine)
			{
			case _Initialized:
				emit sg_Initialized();
				i_StatusEngine = _ReadSerial;
				break;
			case _Idle:
				this_thread::sleep_for(chrono::milliseconds(50));
				break;
			case _ReadSerial:
				this_thread::sleep_for(chrono::milliseconds(50));
				if (xReaded) {
					xReaded = false;
					i_StatusEngine = _StandBy;
				}
				break;
			case _StandBy:
				if ((HIGH == digitalRead(Machine.i_GetButton()) && HIGH == digitalRead(Machine.i_GetSensor()) || xDebug))
				{
					unsigned int count = 0;
					while (count <= 20)
					{
						if (LOW == digitalRead(Machine.i_GetSnWindow1()))
						{
							count = count + 1;
							digitalWrite(Machine.i_GetWindows(), 1);
							this_thread::sleep_for(chrono::milliseconds(200));
						}
						else
						{
							count = 0;
							digitalWrite(Machine.i_GetWindows(), 0);
						}
					}
					//digitalWrite(Machine.i_GetWindows(), HIGH);
					this_thread::sleep_for(chrono::milliseconds(500));
					time(&timer_begin);
					xDebug = false;
					emit sg_ClearStatus();
					this_thread::sleep_for(chrono::milliseconds(500));
					i_StatusEngine = _FindPositions;
				}
				this_thread::sleep_for(chrono::milliseconds(50));
				break;
			case _FindPositions:
				//cout << "FP1" << endl;
				MOriginalPicture = Machine.CutPicture(Machine.PictureSettings, true); // Toma la primer imagen para posición
				//cout << "FP2" << endl;
				MOriginalPicture.copyTo(MResult);
				//cout << "FP3" << endl;
				Machine.ExecuteFilters(Machine.PictureFilters, MOriginalPicture); // Ejecuta Filtro de posición
				//cout << "FP4" << endl;
				BatteryTest = Machine.BatteryFound;
				BatteryTest.M_FindContourns(MOriginalPicture); // Busca contornos
				//cout << "FP5" << endl;
				vpMissing.clear();
				if (0 < BatteryTest.i_Find())
					i_StatusEngine = _Capture;
				else
					i_StatusEngine = _Fault;
				break;
			case _Capture:
				// Si encuentra contornos
				//cout << "C1" << endl;
				BatteryTest.M_CreateArray(MOriginalPicture); // Crea el arreglo de las unidades 
				//cout << "C2" << endl;
				v_map_vipData = BatteryTest.v_map_ivp_GetArray();
				//cout << "C3" << endl;
				vpMissing = Machine.BatteryFound.vp_MatchArray(v_map_vipData);
				//cout << "C4" << endl;
				MOriginalPicture = Machine.StabilizedImage(Machine.i_GetLoop());
				//cout << "C5" << endl;
				MOriginalPicture.copyTo(Machine.M_Segmented);
				//cout << "C6" << endl;
				BatteryTest.v_ExtractSizes(MOriginalPicture); // Extrae los datos de dimensiones de la bateria
				i_StatusEngine = _Evaluate;
				break;
			case _Evaluate:
				//cout << "E1" << endl;
				bool xResult;
				vipDataErrors.clear();
				//cout << "E2" << endl;
				xResult = BatteryTest.x_Evaluate(Machine.BatteryFound,
					vipDataErrors,
					MOriginalPicture, 
					MResult);
				//cout << "E3" << endl;
				if (0 < vpMissing.size())
					BatteryTest.v_DrawMissing(MResult, vpMissing);
				Machine.v_SavePicture(MResult);
				//cout << "E4" << endl;
				Machine.BatteryFound.v_LogExecution(QString(Machine.PictureSettings.str_Path.c_str()));
				//Machine.BatteryFound.v_LogUnit();
				
				i_StatusEngine = _ProcessImage;
				break;
			case _ProcessImage:
				rotate(MResult, MTemp, ROTATE_90_CLOCKWISE);
				MTemp.copyTo(Machine.M_Original);
				image = Mat2QImage(MTemp);
				pixmap = QPixmap::fromImage(image.scaled(900,
					900, 
					Qt::KeepAspectRatio));
				i_StatusEngine = _ShowResult;
				break;
			case _ShowResult:
				digitalWrite(Machine.i_GetWindows(), LOW);
				//digitalWrite(23, LOW);	// Subir ventanas
				pMVpData = make_pair(pixmap, vipDataErrors);	
				emit sg_ShowResult(pMVpData);
				this_thread::sleep_for(chrono::milliseconds(500));
				emit sg_SizeModel(Machine.BatteryFound.sGetSize());
				time(&timer_end);
				secondsElapsed = difftime(timer_end, timer_begin);
				emit sg_ShowTime(secondsElapsed);
				//digitalWrite(Machine.i_GetWindows(), LOW);
				if (xResult)
					i_StatusEngine = _Release;
				else
					i_StatusEngine = _Retry;
				break;
			case _Release:
				while (HIGH == digitalRead(Machine.i_GetSensor()))
					digitalWrite(Machine.i_GetStopper(), HIGH);
				
				// Opción de proceso de Release más largo
				/*while (HIGH == digitalRead(Machine.i_GetSensor()))
				{
					digitalWrite(Machine.i_GetStopper(), HIGH);
					this_thread::sleep_for(chrono::milliseconds(2000));
				}*/
				
				this_thread::sleep_for(chrono::milliseconds(1500));
				digitalWrite(Machine.i_GetStopper(), LOW);
				this_thread::sleep_for(chrono::milliseconds(500));
				i_StatusEngine = _ReadSerial;
				break;
			case _Retry:
				this_thread::sleep_for(chrono::milliseconds(1000));
				i_StatusEngine = _StandBy;
				break;
			case _Fault:
				emit sg_NoFound();
				this_thread::sleep_for(chrono::milliseconds(1000));
				i_StatusEngine = _ReadSerial;
				emit sg_SizeModel(Size(0, 0));
				break;
			default:
				break;
			}
		}
		if (_Halt == i_StatusEngine) {
			emit sg_Finished();
			i_StatusEngine = _Offline;
		}
		
	} while (_Exit != i_StatusEngine);
}

/*

void * ExecutionThread(void * threadid)
{
	ObjectUnit BatteryTest;
	MainWindow * ui;
	QImage image;
	QPixmap pixmap;
	Mat MOriginalPicture, MResult, MTemp;
	vector<tuple<int, Point>> vipDataErrors;
	
	CommThread * ptrCommExterior = (CommThread*)threadid;
	InspectMachine * Machine = (InspectMachine *)ptrCommExterior->iCommand;
	bool xResult = false;
	digitalWrite(Machine->i_GetStopper(), LOW);
	
	ptrCommExterior->iCommand = NoCommand;
	ptrCommExterior->iResponse = Ack;
	while (NoCommand == ptrCommExterior->iCommand) {
		this_thread::sleep_for(chrono::milliseconds(10));
	}
					
	ui = (MainWindow *)ptrCommExterior->iCommand;
	
	while (Exit != ptrCommExterior->iCommand)
	{
		if((HIGH == digitalRead(Machine->i_GetButton()) &&
			HIGH == digitalRead(Machine->i_GetSensor())))
		//if (Done == ptrCommExterior->iCommand)
		{
			ui->v_ClearStatus();
			ptrCommExterior->iResponse = Testing;
			ptrCommExterior->iCommand = NoCommand;
			this_thread::sleep_for(chrono::milliseconds(500));
			
			MOriginalPicture = Machine->CutPicture(Machine->PictureSettings, true); // Toma la primer imagen para posición
			MOriginalPicture.copyTo(MResult);
			vipDataErrors.clear();
			Machine->ExecuteFilters(Machine->PictureFilters, MOriginalPicture); // Ejecuta Filtro de posición
			BatteryTest = Machine->BatteryFound;
			BatteryTest.M_FindContourns(MOriginalPicture); // Busca contornos
			if (0 < BatteryTest.i_Find()) {
				// Si encuentra contornos
				BatteryTest.M_CreateArray(MOriginalPicture); // Crea el arreglo de las unidades 
				
				
				MOriginalPicture = Machine->StabilizedImage(Machine->i_GetLoop());
				MOriginalPicture.copyTo(Machine->M_Segmented);
				BatteryTest.v_ExtractSizes(MOriginalPicture); // Extrae los datos de dimensiones de la bateria
				
				xResult = BatteryTest.x_Evaluate(Machine->BatteryFound,
					vipDataErrors, 
					MOriginalPicture, 
					MResult);
				rotate(MResult, MTemp, ROTATE_90_CLOCKWISE);
				MTemp.copyTo(Machine->M_Original);
				image = Mat2QImage(MTemp);
				pixmap = QPixmap::fromImage(image.scaled(900,
					900, 
					Qt::KeepAspectRatio));
				Size sBatterySize = Machine->BatteryFound.sGetSize();
				
				ui->v_ShowStatus(xResult);
				ui->v_ShowImage(pixmap);
				ui->v_SetColsRows(sBatterySize);
				this_thread::sleep_for(chrono::milliseconds(500));
				
				if (xResult)
				{
					ptrCommExterior->iCommand = NoCommand;
					ptrCommExterior->iResponse = Success;
					
					this_thread::sleep_for(chrono::milliseconds(500));
					while (HIGH == digitalRead(Machine->i_GetSensor()))
						digitalWrite(Machine->i_GetStopper(), HIGH);
					this_thread::sleep_for(chrono::milliseconds(1500));
					digitalWrite(Machine->i_GetStopper(), LOW);
					this_thread::sleep_for(chrono::milliseconds(500));
				}
				else
				{
					ui->v_FillErrors(vipDataErrors);
					ptrCommExterior->iCommand = NoCommand;
					ptrCommExterior->iResponse = Fail;
				}
			}
			else
				ui->v_ShowStatus(false);
		}
	}
	ptrCommExterior->qstrMessage = "Thread finished";
	pthread_exit(NULL);
}
*/