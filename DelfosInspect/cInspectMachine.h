#pragma once

#include <string>
#include <array>
#include <list>
#include <map>
#include <set>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QString>
#include <QImage>


#define Positive 1
#define Negative 0
#define Percent 1
#define Pixels 0

using namespace std;
using namespace cv;
using namespace Qt;

/************** ENUMERADORES ********************/
enum Filters:int
{
	Binary    = 0,
	HSV       = 1,
	Dilate,
	Erode,
	Bilateral,
	Blur
};
/************** CLASES **************************/
class ImageProcessSettings
{
public:
	/************** SOBECARGA DE OPERADORES *****/
	void operator=(const ImageProcessSettings &imageprocesssettings);
	/************** ATRIBUTOS *******************/
	string str_ISO;
	string str_SS;
	string str_Path;
	string str_ResWidth;
	string str_ResHeight;
	int i_Loop;
	int i_OffsetX;
	int i_OffsetY;
	int i_Width;
	int i_Height;
	
	/************** RETORNOS ********************/
	QString qISO() {return QString(str_ISO.c_str());}
	QString qISS() {return QString(str_SS.c_str());}
	QString qPath() {return QString(str_Path.c_str());}
	QString qResWidth() {return QString(str_ResWidth.c_str());}
	QString qResHeight() {return QString(str_ResHeight.c_str());}
	QString qLoop() {return QString::number(i_Loop);}
	QString qOffsetX() {return QString::number(i_OffsetX);}
	QString qOffsetY() {return QString::number(i_OffsetY);}
	QString qWidth() {return QString::number(i_Width);}
	QString qHeight() {return QString::number(i_Height);}
	/************** METODOS *********************/
	ImageProcessSettings();
	ImageProcessSettings(array<string, 6> astrData);
	ImageProcessSettings(array<string,10> astrData);
	void v_SaveFile(string strPath);
	bool x_LoadFile(string strPath);
	/*************************************************/
private:
	/************** ATRIBUTOS ************************/
	string strData[10];
	
	/*************************************************/
};
class CalibImage
{
public:
	/************** SOBECARGA DE OPERADORES *****/
	void operator=(const CalibImage &calibimage);
	/************** RETORNOS ********************/
	bool x_Calibrated() { return x_Success;}
	Size s_GetSize() { return s_Size;}
	QString qHeight() { return QString::number(s_Size.height);}
	QString qWidth() { return QString::number(s_Size.width);}
	/************** METODOS *********************/
	CalibImage();
	CalibImage(string strPath, Size sSize);
	bool x_Start(Mat MChessBoard, Mat &MUndistort);
	void v_SaveFile(string strPath);
	bool x_LoadFile(string strPath);
	Mat MImage(Mat MOriginalImage);
	/*************************************************/
private:
	/************** ATRIBUTOS *******************/
	string str_Path;
	Size s_Size;
	Mat M_CameraMatrix;
	Mat M_DistCoeffs;
	bool x_Success;
	/*************************************************/
};
enum Polar : int
{
	NOEXIST = -10,
	UNDEF   = -1,
	POS     = 0,
	NEG,
	ALLP    = 8
};
struct DataLocation
{
	DataLocation();
	DataLocation(Point pCenter, Point pFirst, Point pLast, Polar pPole);
	void operator =(const DataLocation &datalocation);
	Point CenterPoint;
	Point FirstPoint;
	Point LastPoint;
	Polar Pole;
};
struct AttribsObjects
{
	void operator=(const AttribsObjects &attribsobjects);
	vector<pair<int, int>> vii_FirstCol;
	vector<vector<int>> vvi_LastInspect;
	vector<vector<int>> vviStatisticValues;
	vector<int> vi_SizeCols;
	array<double, 2> ad_DStandar;
};
class ObjectUnit
{
public:	
	/************** SOBECARGA DE OPERADORES *****/
	void operator=(const ObjectUnit &objectunit);
	
	/************** RETORNOS ********************/
	int i_GetVersion();
	int i_GetMax(); 
	int i_GetMin(); 
	int i_GetSquare();
	int i_GetMargin();
	QString qMax();
	QString qMin();
	QString qSquare();
	QString qMargin();
	QString qElongation();
	Size sGetSize(Polar Pole);
	vector<pair<int, int>> vi_GetColValues(Polar Pole);
	/************** METODOS *********************/
	ObjectUnit();
	ObjectUnit(Polar Pole, array<int, 5> aiMaxMin);
	Mat M_FindContourns(Mat MOriginalPicture);
	int i_Find();
	void v_ExtractSizes(Polar Pole, Mat MOriginalImage);
	void v_ExtractSizes(Polar Pole, vector<Mat> vMOriginalImage, bool xAdd = false);
	void v_ShowData(Polar Pole, Mat &MOriginalImage);
	Mat M_CreateArray(Mat MOriginalImage);
	Mat M_PaintLocations(Mat MOriginalImage, Polar pPole = ALLP);
	Polar pl_CheckPolarity(Point p_First, Point p_Last);
	bool x_ChangePolarity(Point p_First, Point p_Last, 
		Polar pPole);
	vector<Point> vp_MatchArray(Polar Pole, vector<map<int, 
		vector<Point>>> & v_map_ivpArrayCheck);
	void v_DrawMissing(Polar Pole, Mat & MOriginalImage, vector<Point> vpData);
	Mat M_ShowUnits(Polar Pole, Mat MOriginalImage);
	bool x_Evaluate(ObjectUnit &ouDataCompare,
		vector<pair<int,
		Point>> &vipDataErrors,
		Mat MOriginalImage, 
		Mat &MResult);
	void v_SaveFile(string strPath);
	bool x_LoadFile(string strPath);
	int i_MaxColumn(Polar Pole);
	int i_MaxColumn();
	int i_GetRowPolarity(int iIndex);
	Point p_GetFirstPoint(Polar Pole, int iIndex);
	double d_CalculateDStandar(vector<int> viData);
	double d_CalculateMedia(vector<int> viData);
	Point p_CalculateMaxMin(vector<int> viData);
	vector<vector<int>> vvi_GetStadistics(Polar Pole);
	void v_LogExecution(Polar Pole, QString qstrPath);
	void v_SetDStandar(Polar Pole, int iIndex, double dData);
	void v_ClearDStandar(Polar Pole);
	double d_GetDStandar(Polar Pole, int iIndex);
	void v_SetPole(array<int, 5> aiData);
	bool x_EmptyArray();
	/*************************************************/

private:	
	/************** ATRIBUTOS *******************/
	array<AttribsObjects,2> q;
	int i_MaxContour;
	int i_MinContour;
	int i_Square;
	int i_Margin;
	double d_Elongation;
	Point pArray;
	vector<map<int, DataLocation>> * v_map_idl_BatteryLocation;
	multimap<int, DataLocation> map_idl_BatteryPosition;
	vector<vector<Point> > vvp_FilterContour;
	vector<vector<Point> > vvp_Contours;
	vector<Vec4i> vv4_Hierarchy;
	int iVersion;
	//QString qstr_LocalFile;
	/*************************************************/
};
class FilterParams
{
public:
	/************** SOBECARGA DE OPERADORES *****/
	void operator=(const FilterParams &filterparams);
	void operator<(const FilterParams &filterparams);
	/************** ATRIBUTOS *******************/
	int i_Param[6];
	int * iptr_Filters;
	vector<Mat> vM_Image;
	/************** RETORNOS ********************/
	void v_SetType(int iType) { i_Type = iType; }
	int i_GetType() { return i_Type; }
	/************** METODOS *********************/
	FilterParams();
	FilterParams(vector<Mat> vMImages,
		array<int, 6> iParam,											
		int * iptrFilters = 0);
	~FilterParams();		
	/*************************************************/
private:
	/************** ATRIBUTOS *******************/
	int i_Type;
	
	/*************************************************/
};
class FilterCartridge
{
public:
	/************** SOBECARGA DE OPERADORES *****/
	void operator=(const FilterCartridge &filtercartridge);
	/************** RETORNOS ********************/
	FilterParams i_GetIndex(int iIndex) {return vfp_Params[iIndex];	}
	int i_GetCount() {return vfp_Params.size();}
	vector<FilterParams> vfp_GetParams() {return vfp_Params;}
	/************** METODOS *********************/
	FilterCartridge();
	~FilterCartridge();
	void v_Push(void(* vFunction)(FilterParams&), FilterParams fpFilters);
	void v_SetStart();
	void v_Update(int iIndex, FilterParams fpFilters);
	void v_Execute(Mat &MOriginal, int iStop = 0);
	void v_SetLast();
	void v_Swap(int iIndexFrom, int iIndexTo);
	void v_Delete(int iDelete);
	void v_SaveFile(string strPath);
	bool x_LoadFile(string strPath);
	/*************************************************/
private:
	/************** ATRIBUTOS *******************/
	vector<FilterParams> vfp_Params; 													// Guardar todos los filtros
	int i_IndexExecution;
	/*************************************************/
};
class Filter
{
public:
	/************** METODOS *********************/
	static void Binary(FilterParams &fp_Params);
	static void HSV(FilterParams &fp_Params);
	static void Dilate(FilterParams &fp_Params);
	static void Erode(FilterParams &fp_Params);
	static void Bilateral(FilterParams &fp_Params);
	static void Blur(FilterParams &fp_Params);
	static Mat SetText(Mat MImage, Point pLocation, QString qstrText,
		Scalar sColor = { 0, 0, 0 }, double sScale = 1);
	static string SendCommand(string strCmd);
	static bool chkStatus();
	
	/*************************************************/
};
class InspectMachine
{
public:
	string str_Name;
	string str_Path;
	
	
	InspectMachine();
	InspectMachine(string strName, string strPath);
	~InspectMachine();
	
	array<ImageProcessSettings,3> PictureSettings;
	CalibImage DataCalib;
	array<FilterCartridge,3> PictureFilters;
	ObjectUnit BatteryFound;
	Mat M_Original, M_Segmented;

	void v_LightOn();
	void v_LightOff();
	void ExecuteFilters(FilterCartridge ipsFilters, Mat &MOriginalPicture);
	// Toma una nueva captura de imagen con la cámara
	Mat TakePicture(ImageProcessSettings ipsData);
	// Carga una imagen ya capturada por la cámara
	Mat LoadPicture();
	// Corta la imagen ya sea capturada o por capturar
	// 0) Para tomar una nueva imagen, 1) para cargar la imagen ya tomada
	Mat CutPicture(ImageProcessSettings ipsData, bool xCalib, int iTake = 1);
	Mat StabilizedImage(int iTimes);
	void v_SaveFile();
	void v_SavePicture(Mat MSave);
	ulong ul_LoadFile(string strPath);
	void v_SetData(array<ImageProcessSettings,3> _PictureSettings,
		CalibImage _DataCalib,
		array<FilterCartridge,3> _PictureFilter,
		ObjectUnit BatteryFound);
	map<string, int> GetIO() { return m_stri_IO; }
	void v_SetIO(map<string, int> IO);
	int i_GetSensor() { return SENSORU; }
	int i_GetButton() { return BUTTON; }
	int i_GetStopper() { return STOPPER; }
	int i_GetWindows() { return WINDOWS; }
	int i_GetSnWindow1() { return SENSORW1; } 
	int i_GetSnWindow2() { return SENSORW2; } 
	int i_GetLoop() { return i_Loop; }
	map<string, int> m_stri_GetIO();
	void v_SetLoop(int iLoop);
	
private:
	void v_Unpack_IO();
	map<string, int> m_stri_IO;
	
	int i_Loop;
	int UP;	
	int LEFT;
	int RIGHT;
	int DOWN;
	int LIGHT;
	int SENSORU;
	int STOPPER;
	int BUTTON;
	int WINDOWS;
	int SENSORW1; 
	int SENSORW2; 
	
	
};

/*************************************************/