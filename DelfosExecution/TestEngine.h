#pragma once
#include <QString>
#include <wiringPi.h>
#include "../DelfosInspect/cInspectMachine.h"
#include <QPixmap>
#include <QObject>
#include <QVector>
#include <QDebug>
#include <QThread>
#include <QString>

using namespace std;

#define _Run			1
#define _Initialized	16
#define _Idle			32
#define _ReadSerial		254
#define _StandBy		255
#define _FindPositions	256
#define _Capture		257
#define _Evaluate		258
#define _ProcessImage	259
#define _ShowResult		260
#define _Release		261
#define _Retry			262
#define _Fault			512
#define _Halt			1024
#define _Offline		1025
#define _Exit			2048

class ManagerEngine : public QThread
{
	Q_OBJECT
		
public :		
	ManagerEngine();
	~ManagerEngine();
	
protected:
	void run();
	
signals :
	void sg_ShowResult(const pair<QPixmap, vector<pair<int, Point>>>& data);
	void sg_ShowTime(double dEnlapsedTime);
	void sg_ClearStatus();
	void sg_NoFound();
	void sg_Initialized();
	void sg_SizeModel(Size sSize);
	void sg_Finished();
	
public slots :
	// Recibe la orden de inspección
	void sl_EngineInspection(const InspectMachine& machine);
	void sl_StopEngine();
	void sl_Test();
	void sl_Readed();
	void sl_Clear();
	
private:
	int i_StatusEngine;
	bool xDebug;
	bool xReaded;
	InspectMachine Machine;

};