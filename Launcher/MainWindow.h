#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QString>
#include "cManager.h"
#include <QMap>
#include <QStringList>

#define NoUser		0
#define NewUser		1
#define EditUser	2
#define DeleteUser	3

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
	explicit MainWindow(QString qstrPath, QWidget *parent = 0);
    ~MainWindow();
	
	bool x_LoadUsers();
	bool x_SaveUsers();
	int i_FindUser();
	QStringList qstrlst_GetUser(int iIndex);
	void v_DeleteUser();

protected slots :
    void Login();
	
	void LaunchInspection();
	void LaunchExecution();
	
	void EditUsers();
	void DiscardUser();
	void DeleteUsers();
	void CreateUsers();
	void UserListSelected(int iIndex);
	void SaveUser();
	
	void ItemSelected(QTreeWidgetItem * qtwiData, int iIndex);	
	void ShowPrograms(bool xActive);
	void ShowUnits(bool xActive);
	void ShowProcess(bool xActive);
	void OpenFile();
	void Import();
	void Export();
	void Exit();
	
	void SelectedShift(int iIndex);
	void SelectedTitle(int iIndex);
	
private:
    Ui::MainWindow *ui;
	Launcher Manager;
	
	QStringList qstrlst_Encript(QString qstrData);
	QString qstr_Decrypt(QString qstrKey, QString qstrData);
	
	QMap<int, QStringList> qm_iqstrlst_Users;
	int iUserStatus;
	
	void v_StartSeed();
	void v_UpdateSeed();
	
	uint ui_Seed;
	pair<int, QStringList> iqstrlstCurrentUser;
	pair<int, QStringList> iqstrlstLogedUser;
	
	int iUbermensch;
	
};

#endif // MAINWINDOW_H
