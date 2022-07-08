#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pthread.h>
#include "TestEngine.h"
#include <QVector>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
	explicit MainWindow(QStringList qstrlstUserName, QWidget *parent = 0);
    ~MainWindow();

signals :
	// Manda señal para que inicie el motor de inspección
	void sg_RunEngine(const InspectMachine& machine);
	// Manda señal para detender el motor de inspección
	void sg_StopEngine();
	void sg_Test();
	void sg_Readed();
	void sg_Clear();
		
protected slots :
	// Recibe el resultado de la imagen y la lista de errores
	void sl_ShowResult(const pair<QPixmap, vector<pair<int, Point>>>& data);
	void sl_ClearStatus();
	void sl_NoFound();
	void sl_Initialized();
	void sl_Finished();
	void sl_SizeModel(Size sSize);
	void sl_ShowTime(double dEnlapsedTime);
	// slots de interfaz
    void OpenFile();
	void Run();
	void Lights(bool xCheck);
	void Release(bool xCheck);
	void SendSerial();
	void Stop();
	void Clear();
	void Find();
	void Configs();
	void Debug();

private:
    Ui::MainWindow *ui;
	bool xInit;
	InspectMachine * ptr_Machine;
	QString qstrUser;
	QString qstrManufacturerName;
	QString qstrManufacturerLocation;
	ManagerEngine InspectionMachine;
	string strGetDay(int iDay);
	string strGetMonth(int iMonth);
	QThread InspectionThread;
	
};

#endif // MAINWINDOW_H
