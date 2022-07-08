#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QApplication>
#include <QString>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "mat2qimage.h"
#include "cInspectMachine.h"
#include <QStringList>

#define _BINARY				{true, true, true, true, false, false, false, false, false, false, false, false}
#define _HSV				{true, true, true, true, true, true, true, true, true, true, true, true}
#define _DILATE				{true, true, false, false, false, false, false, false, false, false, false, false}
#define _ERODE				{true, true, false, false, false, false, false, false, false, false, false, false}
#define _BILATERAL			{true, true, true, true, true, true, false, false, false, false, false, false}
#define _BLUR				{true, true, false, false, false, false, false, false, false, false, false, false}
#define _BINARYINIT			{"255", "0", "", "", "", ""}
#define _HSVINIT			{"255", "0", "255", "0", "255", "0"}
#define _DILATEINIT			{"2", "", "", "", "", ""}
#define _ERODEINIT			{"2", "", "", "", "", ""}
#define _BILATERALINIT		{"7", "25", "25", "","", ""}
#define _BLURINIT			{"2", "", "", "", "", ""}
#define _BINARYNAMES		{"High", "Low", "", "", "", ""}
#define _HSVNAMES			{"H High", "H Low", "S High", "S Low", "V High", "V Low" }
#define _DILATENAMES		{"Size", "", "", "", "", ""}
#define _ERODENAMES			{"Size", "", "", "", "", ""}
#define _BILATERALNAMES		{"Pixels", "S Color", "S Space", "", "", ""}
#define _BLURNAMES			{"Pixels", "", "", "", "", ""}
#define _PATH				"/home/pi/battery/"
#define OUTPUT				1
#define INPUT				0
#define _SENSORU			14
#define	_BUTTON				13
#define _STOPPER			22	//12 in MARK 1	
#define _WINDOWS			23
#define _SENSORW1			21
#define _SENSORW2			30

using namespace cv;
using namespace Qt;

namespace Ui {
class MainWindow;
}


enum Status:int
{
	Empty         = 0,
	Opened        = 1,
	FailMatrix,
	FailDistances,
	NoOpened
};


class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	explicit MainWindow(QStringList qstrlstUserName, QWidget *parent = 0);
	~MainWindow();
	
	void setUser(QString qstrUserName);
protected slots :
	/************* TAB OPEN MODEL ********************/
	void OpenModel();
	void OpenModelNew();
	void OpenModelNext();
	void OpenModelExit();
	void OpenLastModel();
	void SaveModel();
	void OpenModelSaveIO();
	void SelectedColor(int xindex);
	void SetColor();
	
	void TestUp(bool xSignal);
	void TestDown(bool xSignal);
	void TestLeft(bool xSignal);
	void TestRight(bool xSignal);
	void ReadSensor();
	void TestStopper(bool xSignal);
	void TestWindows(bool xSignal);
	void ReadButton();
	
	/************** TAB CALIBRATE ********************/
	void Calibrate();
	void CalibrateTakePicture();
	void CalibrateNext();
	void CalibrateExit();
	
	/************** TAB PICTURE SETTING **************/
	void PictureSettingTakePicture();
	void PictureSettingRefresh();
	void PictureSettingCalibrate();
	void PictureSettingLoadCalib();
	void PictureSettingNext();
	void InspectionProcess(bool xCheck);
	
	/************** TAB SET FILTER *******************/
	void SetFilterTake();
	void SetFilterFilter(int index);
	void SetFilterApply();
	void SetFilterAdd();
	void SetFilterSelect(int iIndex);
	void SetFilterUp();
	void SetFilterDown();
	void SetFilterDelete();
	void SetFilterNext();
	void SetFilterClear();
	//void FilterPosChangePolarity(int iIndex);
	
	/************** TAB FIND POSITION ****************/
	void FindPositionTake();
	void FindPositionShow();
	void FindPositionEdges();
	void FindPositionNext();
	void FilterUnitsChangePolarity(int iIndex);
	
	/************** TAB IMAGE SETTING ****************/
	void ImageSettingTake(bool xCheck);
//	void ImageSettingConfig(int iState);
//	void ImageSettingRefresh();
//	void ImageSettingNext();
	
	/************** TAB FIND UNIT ********************/
	void FindUnitTake();
	void FindUnitFilter(int index);
	void FindUnitApply();
	void FindUnitAdd();
	void FindUnitSelect(int iIndex);
	void FindUnitUp();
	void FindUnitDown();
	void FindUnitDelete();
	void FindUnitShow();
	void FindUnitNext();
	void FindUnitEstabilized();
	void FindUnitsClear();
	void FindUnitsChangePolarity(int iIndex);	
	
	/************** TAB TRAINING MODEL ***************/
	void TrainingEvaluate();
	void TrainingNext();
	void ShowBW(bool xActive);
	void Release(bool xActive);
	void TrainingSelectRow(int iRow);
	
	/************** TAB TEST RUN *********************/
	void TestModel();
	void ChangePolarity(int iIndex);
	
	/*************************************************/
	
	void Position();
	void StartPosition();
	void Draw();
	
private:
	/************** ATRIBUTOS ************************/
    Ui::MainWindow *ui;
	string str_Path;
	Mat M_BackUp;
	Mat M_Segmented;
	int UP;
	int LEFT;
	int RIGHT;
	int DOWN;
	int SENSORU;
	int STOPPER;
	int BUTTON;
	int LIGHT;
	int WINDOWS;
	int SENSORW1;
	int SENSORW2;
	bool xDebug;
	int i_Version;
	double d_Scale;
	QPoint q_First;
	Point p_First;
	Point p_Last;
	
	//ObjectUnit::Polar Pole;
	vector<string> vstr_Licence;
	vector<vector<int>> vvi_Statistics;
	pair<int, QStringList> iqstrlstCurrentUser;
	
	/************** ATRIBUTOS DE CLASES **************/
	array<ImageProcessSettings,3> PictureSettings;
	CalibImage DataCalib;
	array<FilterCartridge,3> PictureFilters;
	ObjectUnit BatteryFound;
	ObjectUnit BatteryTest;
	
	Mat MCurrentPicture;
	
	/************** METODOS **************************/
	// Muestra una imagen de tipo Mat en el panel principal
	void v_ShowImage(Mat &MImage);
	//Mat M_CalibImage(Mat MOriginalImage);
	FilterParams CreateSetFilterParams(Mat InitialPicture);
	FilterParams CreateFindUnitsParams(Mat InitialPicture);
	array<int, 5> CreateObject();
	// Toma una nueva captura de imagen con la cámara
	Mat TakePicture(ImageProcessSettings ipsData);
	// Carga una imagen ya capturada por la cámara
	Mat LoadPicture();
	// Corta la imagen ya sea capturada o por capturar
	// 0) Para tomar una nueva imagen, 1) para cargar la imagen ya tomada
	Mat CutPicture(ImageProcessSettings ipsData, int iTake = 1);
	void ExecuteFilters(FilterCartridge ipsFilters, Mat &MOriginalPicture);
	bool RestoreProcessingImage(bool xLoad = false);
	bool RestoreProcessingPicture(bool xLoad = false);
	bool RestoreFiltersPositions(bool xLoad = false);
	bool RestoreFiltersUnits(bool xLoad = false);
	bool RestorePositions(bool xLoad = false);
	bool RestoreCalib(bool xLoad = false);
	bool RestoreCalib(QString qstrPath, bool xLoad = false);
	bool RestoreIO();
	void v_LightOn();
	void v_LightOff();
	void v_UpdateDataImage();
	void v_FillDataStatistic();
	Mat M_StabilizedImage(int iTimes);
	void v_RestoreListFilters();
	/*************************************************/
};

#endif // MAINWINDOW_H
