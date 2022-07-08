#include "cInspectMachine.h"
#include <opencv2/core.hpp>
#include <opencv2/core/persistence.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <wiringPi.h>
#include "cCamera.h"
#include <stdlib.h>
#include <fstream>
#include <QDebug>
#include <ctime>
#include <iostream>

/****************** FILTER CALIB IMAGE ******************************/
CalibImage::CalibImage(string strPath, Size sSize)
{
	str_Path = strPath;
	s_Size = sSize;
	x_Success = false;
}
CalibImage::CalibImage()
{
	str_Path = "Calib.xml";
	s_Size = Size(2, 2);
	x_Success = false;
}
bool CalibImage::x_Start(Mat MChessBoard, Mat &MUndistort)
{
	Mat MR, MT;
	Mat MGrayImage;
	
	vector<Point2f> vpCorners;

	Mat NewCameraMatrix;
	vector<vector<Point3f> > vvp3_objpoints;											// Creating vector to store vectors of 3D points for each checkerboard image
	vector<vector<Point2f> > vvp2_imgpoints;											// Creating vector to store vectors of 2D points for each checkerboard image
	std::vector<cv::Point3f> vp3_objp;
	for (int i{ 0 }; i < s_Size.height; i++)											// Defining the world coordinates for 3D points
	{
		for (int j{ 0 }; j < s_Size.width; j++)
			vp3_objp.push_back(cv::Point3f(j, i, 0));
	}
		
	cvtColor(MChessBoard, MGrayImage, CV_BGR2GRAY);
	
	x_Success = findChessboardCorners(MGrayImage,
		Size(s_Size.width, s_Size.height),
		vpCorners);
		
	if (x_Success)
	{
		TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 
			30, 
			0.001);
		cornerSubPix(MGrayImage, 
			vpCorners, 
			Size(11, 11), 
			Size(-1, -1), 
			criteria);
		drawChessboardCorners(MChessBoard,
			Size(s_Size.width, s_Size.height), 
			vpCorners,
			x_Success);
		vvp3_objpoints.push_back(vp3_objp);
		vvp2_imgpoints.push_back(vpCorners);
		calibrateCamera(vvp3_objpoints,
			vvp2_imgpoints, 
			Size(MGrayImage.rows, MGrayImage.cols),
			M_CameraMatrix,
			M_DistCoeffs,
			MR,
			MT);
		
		undistort(MChessBoard, 
			MUndistort, 
			M_CameraMatrix, 
			M_DistCoeffs);
		return x_Success;
	}
	else
	{
		MUndistort = Mat();
		return x_Success;
	}
}
void CalibImage::v_SaveFile(string strPath)
{
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	fs_SaveStream << "Calibration" << "{";
	fs_SaveStream << "Matrix" << M_CameraMatrix;
	fs_SaveStream << "Distances" << M_DistCoeffs;
	fs_SaveStream << "Size" << s_Size;
	fs_SaveStream << "Calibrated" << (int)x_Success;
	fs_SaveStream << "}";
	fs_SaveStream.release(); 
}
bool CalibImage::x_LoadFile(string strPath)
{
	bool xFail;
	FileStorage fs_OpenStream;
	try
	{
		fs_OpenStream = FileStorage(strPath, FileStorage::READ);
	}
	catch (cv::Exception& e)
	{
		xFail = true;
		x_Success = false;
		qDebug() << e.what();
	}
	if (fs_OpenStream.isOpened() && !xFail)
	{
		FileNode fnData = fs_OpenStream["Calibration"];
		
		fnData["Matrix"] >> M_CameraMatrix;
		fnData["Distances"] >> M_DistCoeffs;
		fnData["Size"] >> s_Size;
		x_Success = (int)fnData["Calibrated"];
		fs_OpenStream.release();
		x_Success = true;
		return true;
	}
	fs_OpenStream.release();
	return false;
}
void CalibImage::operator =(const CalibImage &calibimage)
{		
	this->s_Size = calibimage.s_Size;
	this->M_CameraMatrix = calibimage.M_CameraMatrix;
	this->M_DistCoeffs = calibimage.M_DistCoeffs;
}
Mat CalibImage::MImage(Mat MOriginalImage)
{
	Mat MUndistort;
	if (0 < M_CameraMatrix.cols && 0 < M_CameraMatrix.rows)
		undistort(MOriginalImage, MUndistort, M_CameraMatrix, M_DistCoeffs);
	return MUndistort;
}
/****************** FILTER IMAGE SETTINGS ***************************/
ImageProcessSettings::ImageProcessSettings()
{
	str_ISO = "500";
	str_SS = "100000";
	str_Path = "/home/pi/picture.jpg";
	i_Loop = 1;
	str_ResWidth = "800";
	str_ResHeight = "600";
	i_OffsetX = 0;
	i_OffsetY = 0;
	i_Width = 800;
	i_Height = 600;
}
ImageProcessSettings::ImageProcessSettings(array<string, 10> astrData)
{
	str_ISO = astrData[0];
	str_SS = astrData[1];
	str_Path = astrData[2];
	i_Loop = atoi(astrData[3].c_str());
	str_ResWidth = astrData[4];
	str_ResHeight = astrData[5];
	i_OffsetX = atoi(astrData[6].c_str());
	i_OffsetY = atoi(astrData[7].c_str());
	i_Width = atoi(astrData[8].c_str());
	i_Height = atoi(astrData[9].c_str());
}
ImageProcessSettings::ImageProcessSettings(array<string, 6> astrData)
{
	str_ISO = astrData[0];
	str_SS = astrData[1];
	str_Path = astrData[2];
	i_Loop = atoi(astrData[3].c_str());
	str_ResWidth = astrData[4];
	str_ResHeight = astrData[5];
	i_OffsetX = 0;
	i_OffsetY = 0;
	i_Width = atoi(astrData[4].c_str());
	i_Height = atoi(astrData[5].c_str());
}
void ImageProcessSettings::operator =(const ImageProcessSettings &imageprocesssettings)
{
	this->str_ISO = imageprocesssettings.str_ISO;
	this->str_SS = imageprocesssettings.str_SS;
	this->i_Loop = imageprocesssettings.i_Loop;
	this->str_ResWidth = imageprocesssettings.str_ResWidth;
	this->str_ResHeight = imageprocesssettings.str_ResHeight;
	this->i_OffsetX = imageprocesssettings.i_OffsetX;
	this->i_OffsetY = imageprocesssettings.i_OffsetY;
	this->i_Width = imageprocesssettings.i_Width;
	this->i_Height = imageprocesssettings.i_Height;
}
bool ImageProcessSettings::x_LoadFile(string strPath)
{
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
		FileNode fnData = fs_OpenStream["Image"];
		
		str_ISO = (string)fnData["ISO"];
		str_SS = (string)fnData["SS"];
		str_Path = (string)fnData["Path"];
		i_Loop = (int)fnData["Loop"];
		str_ResWidth = (string)fnData["ResWidth"];
		str_ResHeight = (string)fnData["ResHeight"];
		i_OffsetX = (int)fnData["OffsetX"];
		i_OffsetY = (int)fnData["OffsetY"];
		i_Width = (int)fnData["Width"];
		i_Height = (int)fnData["Height"];

		fs_OpenStream.release();
		return true;
	}
	fs_OpenStream.release();
	return false;
}
void ImageProcessSettings::v_SaveFile(string strPath)
{
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	fs_SaveStream <<  "Image" << "{"
		<< "ISO" << str_ISO
		<< "SS" << str_SS
		<< "Path" << str_Path
		<< "Loop" << i_Loop
		<< "ResWidth" << str_ResWidth
		<< "ResHeight" << str_ResHeight
		<< "OffsetX" << i_OffsetX
		<< "OffsetY" << i_OffsetY
		<< "Width" << i_Width
		<< "Height" << i_Height
		<< "}";
	fs_SaveStream.release(); 
}
/****************** FILTER PARAMS ***********************************/
void FilterParams::operator=(const FilterParams &filterparams)
{
	for (int i = 0; i < 6; i++)
		this->i_Param[i] = filterparams.i_Param[i];
	this->i_Type = filterparams.i_Type;
}
void FilterParams::operator<(const FilterParams &filterparams)
{
	for (int i = 0; i < 6; i++)
		this->i_Param[i] = filterparams.i_Param[i];
	this->i_Type = filterparams.i_Type;
	this->vM_Image = filterparams.vM_Image;
}
FilterParams::FilterParams()
{
	vM_Image.push_back(Mat());
	vM_Image.push_back(Mat());
	iptr_Filters = new int();
}
FilterParams::FilterParams(vector<Mat> vMImages, 
	array<int,6> iParam, 
	int * iptrFilters)
{
	for (int i = 0; i < 6; i++) 
		i_Param[i] = iParam[i];
	vM_Image = vMImages;
	iptr_Filters = iptrFilters;
}
FilterParams::~FilterParams()
{
	//delete iptr_Param;
}
/****************** FILTER CARTRIDGE ********************************/
void FilterCartridge::operator=(const FilterCartridge &filtercartridge)
{
	this->vfp_Params = filtercartridge.vfp_Params;
}
FilterCartridge::FilterCartridge()
{
	i_IndexExecution = 0;
}
FilterCartridge::~FilterCartridge()
{
	//delete iptr_Filters;
}
void FilterCartridge::v_Update(int iIndex, FilterParams fpFilters)
{
	for (int i = 0; i < 6; i++)
		vfp_Params[iIndex].i_Param[i] = fpFilters.i_Param[i];
}
void FilterCartridge::v_Push(void(* vFunction)(FilterParams&), 
	FilterParams fpFilters)
{
	*fpFilters.iptr_Filters = *(int*)&vFunction;
	vfp_Params.push_back(fpFilters);
}
void FilterCartridge::v_SetStart()
{
	i_IndexExecution = 0;
}
void FilterCartridge::v_Execute(Mat &MOriginal, int iStop)
{
	if (15 > i_IndexExecution)
	{
		if (0 == i_IndexExecution)
			vfp_Params[i_IndexExecution].vM_Image[0] = MOriginal;

		void(* vFilterExecute)(FilterParams&);
		*(&vFilterExecute) = (void(*)(FilterParams&)) * vfp_Params[
			i_IndexExecution].iptr_Filters;
		vFilterExecute(vfp_Params[i_IndexExecution]);
		
		i_IndexExecution++;
		
		//if (iStop == i_IndexExecution)
		MOriginal = vfp_Params[i_IndexExecution - 1].vM_Image[1];
		if (i_IndexExecution < vfp_Params.size() && 15 > i_IndexExecution)
			vfp_Params[i_IndexExecution].vM_Image[0] = vfp_Params[
			i_IndexExecution-1].vM_Image[1];
	}
}
void FilterCartridge::v_Swap(int iIndexFrom, int iIndexTo)
{
	FilterParams fpFrom = vfp_Params[iIndexFrom];
	vfp_Params[iIndexFrom] = vfp_Params[iIndexTo];
	vfp_Params[iIndexTo] = fpFrom;
}
void FilterCartridge::v_Delete(int iDelete)
{
	int iIterator = 0;
	auto aIterator = vfp_Params.begin();
	for ( iIterator = 0; iIterator < iDelete; iIterator++)
		aIterator++;
	vfp_Params.erase(aIterator);
}
void FilterCartridge::v_SaveFile(string strPath)
{
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	int iSize = vfp_Params.size();
	fs_SaveStream <<  "Param_Config" << "{";
	fs_SaveStream << "Size" << (int)iSize;
	for (int i = 0; i < iSize; i++)
	{
		fs_SaveStream <<  "Index" + to_string(i) << "{";
		fs_SaveStream <<  "Param" << "[";
		fs_SaveStream << (int)vfp_Params[i].i_Param[0];
		fs_SaveStream << (int)vfp_Params[i].i_Param[1];
		fs_SaveStream << (int)vfp_Params[i].i_Param[2];
		fs_SaveStream << (int)vfp_Params[i].i_Param[3];
		fs_SaveStream << (int)vfp_Params[i].i_Param[4];
		fs_SaveStream << (int)vfp_Params[i].i_Param[5] << "]";
		fs_SaveStream << "Type" << (int)vfp_Params[i].i_GetType();
		fs_SaveStream << "}";
	}
	fs_SaveStream << "}";
	fs_SaveStream.release(); 
}
bool FilterCartridge::x_LoadFile(string strPath)
{
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
		FileNode fnData = fs_OpenStream["Param_Config"];
		int iSize = (int)fnData["Size"];
		if (0 < iSize) {
			vfp_Params.clear();		
			for(int i = 0 ; i < iSize ; i++)
			{
				array<int, 6> iParam;
				FileNode fnFilter = fnData["Index" + to_string(i)];
				int iType = (int)fnFilter["Type"];
				FileNode fnParam = fnFilter["Param"];
				FileNodeIterator it = fnParam.begin();
				FileNodeIterator it_end = fnParam.end();
				vector<Mat> vMImages;
				vMImages.push_back(Mat());
				vMImages.push_back(Mat());
				void(* vFunction)(FilterParams&);
				int iCount = 0;
				for (; it != it_end; ++it)
					iParam[iCount++] = (int)*it;
				switch (iType)
				{
				case Binary:
					vFunction = Filter::Binary;
					break;
				case HSV:
					vFunction = Filter::HSV;
					break;
				case Dilate:
					vFunction = Filter::Dilate;
					break;
				case Erode:
					vFunction = Filter::Erode;
					break;
				case Bilateral:
					vFunction = Filter::Bilateral;
					break;
				case Blur:
					vFunction = Filter::Blur;
					break;
				default:
					break;
				}
				FilterParams fpNewFilter = FilterParams(vMImages, 
					iParam, new int);
				*fpNewFilter.iptr_Filters = *(int*)&vFunction;
				fpNewFilter.v_SetType(iType);
				vfp_Params.push_back(fpNewFilter);
			}
		}
		fs_OpenStream.release();
		return true;
	}
	fs_OpenStream.release();
	return false;
}
//***************** DATA LOCATION ***********************************/

DataLocation::DataLocation()
{
	CenterPoint = Point(0, 0);
	FirstPoint  = Point(0, 0);
	LastPoint	= Point(0, 0);
	Pole		= UNDEF;
}
DataLocation::DataLocation(Point pCenter, Point pFirst, Point pLast, Polar pPole)
{
	CenterPoint = pCenter;
	FirstPoint  = pFirst;
	LastPoint	= pLast;
	Pole		= pPole;
}

void DataLocation::operator =(const DataLocation &datalocation)
{
	this->CenterPoint	= datalocation.CenterPoint;	
	this->FirstPoint	= datalocation.FirstPoint;
	this->LastPoint		= datalocation.LastPoint;	
	this->Pole			= datalocation.Pole;	
}



//***************** OBJECT UNIT *************************************/

void AttribsObjects::operator =(const AttribsObjects &attribsobjects)
{
	this->vi_SizeCols			= attribsobjects.vi_SizeCols;
	this->vii_FirstCol			= attribsobjects.vii_FirstCol;
	this->vviStatisticValues	= attribsobjects.vviStatisticValues;
	this->ad_DStandar			= attribsobjects.ad_DStandar;
}
ObjectUnit::ObjectUnit()
{
	
}
ObjectUnit::ObjectUnit(Polar Pole, array<int, 5> aiMaxMin)
{
	i_MaxContour = aiMaxMin[0];
	i_MinContour = aiMaxMin[1];
	i_Square = aiMaxMin[2];
	i_Margin = aiMaxMin[3];
	d_Elongation = (double)aiMaxMin[4] / 100;
	v_map_idl_BatteryLocation = new vector<map<int, DataLocation > >();
}
void ObjectUnit::operator = (const ObjectUnit &objectunit)
{		
	this->i_MaxContour				= objectunit.i_MaxContour;
	this->i_MinContour				= objectunit.i_MinContour;
	this->i_Square					= objectunit.i_Square;
	this->i_Margin					= objectunit.i_Margin;
	this->d_Elongation				= objectunit.d_Elongation;
	this->v_map_idl_BatteryLocation = objectunit.v_map_idl_BatteryLocation;
	this->q[0]						= objectunit.q[0];
	this->q[1]						= objectunit.q[1];
	this->pArray					= objectunit.pArray;
}
Mat ObjectUnit::M_FindContourns(Mat MOriginalPicture)
{
	Mat MWritePicture;
	findContours(MOriginalPicture,														// Busca bordes de imagen segmantada
		vvp_Contours,															// --------
		vv4_Hierarchy,															// --------
		CV_RETR_LIST,																	// --------
		CV_CHAIN_APPROX_SIMPLE);														// --------

	if (1 == MOriginalPicture.channels())
		cvtColor(MOriginalPicture, MWritePicture, COLOR_GRAY2BGR);						// Convierte la imagen dilatada a imagen BGR
	else
		MOriginalPicture.copyTo(MWritePicture);

	int count = 0;
	vvp_FilterContour.clear();
		
	for (int idx = 0; idx < vvp_Contours.size(); idx++)							// Busca y filtra todos los contornos superiores
	{
		if (vvp_Contours[idx].size() > i_MinContour && 					// --------
			vvp_Contours[idx].size() < i_MaxContour)					// --------
		{			
			switch (count)
			{
			case 0:
				drawContours(MWritePicture, 											// --------	
					vvp_Contours, 												// --------	
					idx, 																// --------	
					Scalar(0, 0, 255), 													// --------	
					5);																	// --------	
				break;
			case 1:
				drawContours(MWritePicture, 											// --------	
					vvp_Contours, 												// --------	
					idx, 																// --------	
					Scalar(0, 255, 0), 													// --------	
					5);																	// --------	
				break;
			case 2:
				drawContours(MWritePicture, 											// --------
					vvp_Contours, 												// --------
					idx, 																// --------
					Scalar(255, 0, 0), 													// --------
					5);																	// --------
				break;
			default:
				break;
			}
			vvp_FilterContour.push_back(vvp_Contours[idx]);				// se agrega el contorno a un vector
			count++;
			if (3 == count)
				count = 0;
		}
	}
	return MWritePicture;
}
int ObjectUnit::i_Find()
{
	set<int> siBorderX, siBorderY;
	set<int>::iterator siiIteratorStart, siiIteratorEnd;
	map_idl_BatteryPosition.clear();
	int iMaxArea = (i_Square * 2)	*(i_Square * 2);
	int iMinArea = (i_Square * 0.5)	*(i_Square * 0.5);
	double dMaxElon = (double)1 + d_Elongation;
	double dMinElon = (double)1 - d_Elongation;
	for (int idx = 0; idx < vvp_FilterContour.size(); idx++) {					// Inicia un recorrido por todos los contronos encontrados
		siBorderX.clear();																// Limpia los vectores en X
		siBorderY.clear();																// Limpia los vectores en Y
		for (int idy = 0; idy < vvp_FilterContour[idx].size(); idy++) {			// Vierte todos los puntos del contorno
			siBorderX.insert(vvp_FilterContour[idx][idy].x);					// sobre vectores en X
			siBorderY.insert(vvp_FilterContour[idx][idy].y);					// y sobre vectores en Y
		}		
		
		siiIteratorStart = siBorderX.begin();											// Obtiene el punto mas alto
		siiIteratorEnd = siBorderX.end();												// Obtiene el punto mas bajo
		int centerPointX = (*siiIteratorStart + *(--siiIteratorEnd)) / 2;				// Genera coordenada media X		
		int iX = *siiIteratorEnd - *siiIteratorStart;
		
		siiIteratorStart = siBorderY.begin();											// Obtiene el punto mas cercano
		siiIteratorEnd = siBorderY.end();												// Obtiene el punto mas lejano
		int centerPointY = (*siiIteratorStart + *(--siiIteratorEnd)) / 2;				// Genera coordenada media Y
		int iY = *siiIteratorEnd - *siiIteratorStart;
		
		int iArea = iX*iY;
		double dSquare = (double)iX / iY;
		if (iArea < iMaxArea && iArea > iMinArea && 
			dSquare > dMinElon && dSquare < dMaxElon)
		{
			DataLocation dlUnit(Point(centerPointX, centerPointY),						// Genera la estructura de puntos y polaridad
				Point(centerPointX - i_Square, centerPointY - i_Square),
				Point(centerPointX + i_Square, centerPointY + i_Square),
				UNDEF);
			
			map_idl_BatteryPosition.insert({ centerPointY, {dlUnit} });					// Incerta en el map los puntos				
		}
	}
	return map_idl_BatteryPosition.size();
}
Mat ObjectUnit::M_CreateArray(Mat MOriginalImage)
{
	map<int, DataLocation> map_idl_RowBattery;
	v_map_idl_BatteryLocation->clear();
	Mat MImage;
	MOriginalImage.copyTo(MImage);
	Scalar sColors[3];
	int iRule = 0;
	int iCols = 0;
	int iCount = 0;
	int iRows = 0;
	
	sColors[0] = Scalar(0, 255, 255);
	sColors[1] = Scalar(0, 255, 0);
	sColors[2] = Scalar(255, 0, 0);
	
	int i_Top =  map_idl_BatteryPosition.begin()->second.FirstPoint.y +					// Genera rango superior para detectar baterias en la fila
		(i_Margin / 2);
	int i_Bottom =  map_idl_BatteryPosition.begin()->second.LastPoint.y +				// Genera rango inferior para detectar baterias en la fila
		(i_Margin / 2);
	
	
	for (auto aiterator = map_idl_BatteryPosition.begin();						// Inicia recorrido por todo el map con
	aiterator != map_idl_BatteryPosition.end();									// las posiciones de las baterias
	aiterator++)																		// --------
	{	
		switch (aiterator->second.Pole)
		{
		case UNDEF:
			iRule = 0;
			break;
		case POS:
			iRule = 1;
			break;
		case NEG:
			iRule = 2;
			break;			
		default:
			break;
		}
		if (i_Top < aiterator->first && i_Bottom > aiterator->first)					// Si el centro del controno esta dentro de rango
		{
			rectangle(MImage,
			aiterator->second.FirstPoint,													// Dibuja en la nueva imagen el rectangulo del contorno
			aiterator->second.LastPoint, 
			sColors[iRule], 
			5);							// --------
			
			iCount++;
			map_idl_RowBattery.insert({
				aiterator->second.CenterPoint.x, // Agrega el contorno a una fila
				aiterator->second });											// --------
		}
		else																			// Si está fuera de rango
		{	
			if (0 == map_idl_RowBattery.size())
			{
				rectangle(MImage,
					aiterator->second.FirstPoint,												// Dibuja en la nueva imagen el rectangulo del contorno
					aiterator->second.LastPoint, 
					sColors[iRule],
					5); // --------
				
				iCount++;
				map_idl_RowBattery.insert( { 
					aiterator->second.CenterPoint.x, // Agrega el contorno a una fila
					aiterator->second });											// --------
				aiterator++;
			}
			iRows++;
			if (iCount > iCols)
				iCols = iCount;
			iRule++;
			v_map_idl_BatteryLocation->push_back(map_idl_RowBattery); // Agrega fila al arrego
			map_idl_RowBattery = map<int, DataLocation>(); // Crea nueva fila
			i_Top = aiterator->second.FirstPoint.y + (i_Margin / 2);					// Genera nuevo rango superior para detectar baterias en la fila
			i_Bottom = aiterator->second.LastPoint.y + (i_Margin / 2);					// Genera nuevo rango inferior para detectar baterias en la fila
			aiterator--;																// Regresa el iterador uno para que vuelva a empezar la fila
		}
	}
	iRows++;
	pArray = Point(iCols, iRows);
	if (0 < map_idl_RowBattery.size())
		v_map_idl_BatteryLocation->push_back(map_idl_RowBattery);
	return MImage;
}
Mat ObjectUnit::M_PaintLocations(Mat MOriginalImage, Polar pPole)
{
	Scalar sColors[3];
	int iRule = 0;
	Mat MImage;
	
	if (1 < MOriginalImage.channels())
		MOriginalImage.copyTo(MImage);
	else
		cvtColor(MOriginalImage, MImage, COLOR_GRAY2BGR);
	
	sColors[0] = Scalar(0, 255, 255);
	sColors[1] = Scalar(0, 255, 0);
	sColors[2] = Scalar(0, 0, 255);
	
	for (auto aRow = v_map_idl_BatteryLocation->begin();
	aRow != v_map_idl_BatteryLocation->end();
	aRow++)
	{
		for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
		{
			switch (aCol->second.Pole)
			{
			case UNDEF:
				iRule = 0;
				break;
			case POS:
				iRule = 1;
				break;
			case NEG:
				iRule = 2;
				break;			
			default:
				break;
			}
			
			if (ALLP == pPole)
			{
				rectangle(MImage,
					aCol->second.FirstPoint,													// Dibuja en la nueva imagen el rectangulo del contorno
					aCol->second.LastPoint, 
					sColors[iRule], 
					5);
			}
			else if(aCol->second.Pole == pPole)
			{
				rectangle(MImage,
					aCol->second.FirstPoint,													// Dibuja en la nueva imagen el rectangulo del contorno
					aCol->second.LastPoint, 
					sColors[iRule], 
					5);
			} 
		}
	}
	return MImage;
}
Polar ObjectUnit::pl_CheckPolarity(Point p_First, Point p_Last)
{
	Polar pPole = NOEXIST;
	for (auto aRow = v_map_idl_BatteryLocation->begin();
	aRow != v_map_idl_BatteryLocation->end();
	aRow++)
	{
		auto aCol = aRow->begin();
		if (aCol->second.CenterPoint.y >= p_First.y &&
				aCol->second.CenterPoint.y <= p_Last.y)
		{
			for (; aCol != aRow->end(); aCol++)
			{
				if (aCol->second.CenterPoint.x >= p_First.x &&
					aCol->second.CenterPoint.x <= p_Last.x)
				{
					pPole = aCol->second.Pole;
					aRow = v_map_idl_BatteryLocation->end()-1;
					break;
				}
			}
		}
	}
	return pPole;
}
bool ObjectUnit::x_ChangePolarity(Point p_First, Point p_Last, Polar pPole)
{
	bool xSuccess = false;
	
	for (auto aRow = v_map_idl_BatteryLocation->begin();
	aRow != v_map_idl_BatteryLocation->end();
	aRow++)
	{
		auto aCol = aRow->begin();
		if (aCol->second.CenterPoint.y >= p_First.y &&
				aCol->second.CenterPoint.y <= p_Last.y)
		{
			for (; aCol != aRow->end(); aCol++)
			{
				if (aCol->second.CenterPoint.x >= p_First.x &&
				aCol->second.CenterPoint.x <= p_Last.x)
				{
					aCol->second.Pole = pPole;
					xSuccess = true;
				}
			}
		}
	}
	return xSuccess;
}
vector<Point> ObjectUnit::vp_MatchArray(Polar Pole, vector<map<int, 
	vector<Point>>> & v_map_ivpArrayCheck)
{
//	auto xRow = v_map_ivpArrayCheck.begin();
	vector<Point> vpMissingUnit;
//	int iRows = 0;
//	int iCols = 0;
//	
//	cout << "aR" << q[Pole].v_map_ivp_ArrayBattery->size() << endl;
//	cout << "xR" << v_map_ivpArrayCheck.size() << endl;
//	if (q[Pole].v_map_ivp_ArrayBattery->size() == v_map_ivpArrayCheck.size())
//	{
//		for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//		aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//		{
//			if (xRow->size() != aRow->size())
//			{
//				auto xCol = xRow->begin();
//				auto aCol = aRow->begin();
//				//cout << "R" << iRows << endl;
//				while (aCol != aRow->end() &&  q[Pole].vi_SizeCols[iRows] > iCols)
//				{
//					//cout << "C" << iCols << endl;
//					iCols++;
//					if (xCol != xRow->end())
//					{
//						try
//						{
//							//cout << "aC" << aCol->second[0].x << endl;
//							//cout << "xC" << xCol->second[0].x << endl;
//							if (aCol->second[0].x - i_Square > xCol->second[0].x ||
//								aCol->second[0].x + i_Square < xCol->second[0].x)
//							{
//								vpMissingUnit.push_back(aCol->second[0]);
//								if (xRow->size() > aRow->size())
//									xCol++;
//								else
//									aCol++;
//							}
//							else
//							{
//								aCol++;
//								xCol++;
//								if (aCol == aRow->end() && xCol != xRow->end())
//								{
//									do
//									{
//										vpMissingUnit.push_back(xCol->second[0]);
//										xCol++;
//									} while (xCol != xRow->end());
//								}
//							}
//						}
//						catch (exception& e)
//						{
//							aCol = aRow->end();
//						}
//					}
//					else 
//					{
//						vpMissingUnit.push_back(aCol->second[0]);
//						aCol++;
//					}
//				}
//			}
//			iCols = 0;
//			iRows++;
//			xRow++;
//		}
//	}
//	else
//	{
//		for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//		aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//		{
//			for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
//				vpMissingUnit.push_back(aCol->second[0]);
//		}
//	}
	return vpMissingUnit;
}
void ObjectUnit::v_DrawMissing(Polar Pole, Mat & MOriginalImage, vector<Point> vpData)
{
	Point pFirst;
	Point pLast;
	
	int iwidth = MOriginalImage.size().width;
	int iheight = MOriginalImage.size().height;
	
	for (auto aIterator = vpData.begin(); aIterator != vpData.end(); aIterator++)
	{
		
		pFirst = Point(aIterator->x - i_Square, 
			aIterator->y - i_Square); // Genera el cuadro de evaluación
		pLast = Point(aIterator->x + i_Square, 
			aIterator->y + i_Square);
			
		if (0 > pFirst.x)																// Ajusta a rectangulo en caso de que salga
		pFirst.x = 0; // las dimansiones de la imagen
		if (0 > pFirst.y)
			pFirst.y = 0;
		if (0 > pLast.x)
			pLast.x = 0;
		if (0 > pLast.y)
			pLast.y = 0;
		if (pFirst.x > iwidth)
			pFirst.x = iwidth;
		if (pLast.x > iwidth)
			pLast.x = iwidth;
		if (pFirst.y > iheight)
			pFirst.y = iheight;
		if (pLast.y > iheight)
			pLast.y = iheight;
		
		
		rectangle(MOriginalImage, pFirst, pLast, Scalar(0, 0, 255), 10);
	}
}
void ObjectUnit::v_SaveFile(string strPath) ///******************************************************/ Guardar las polaridades en position
{
	array<int,2> aiSize;
	int i;
	int iRows;
	int iCols;
	string strPosition;
	if (0 == q[POS].ad_DStandar[0])
		q[POS].ad_DStandar[0] = 5;
	if (0 == q[POS].ad_DStandar[1])
		q[POS].ad_DStandar[1] = 5;
	if (0 == q[NEG].ad_DStandar[0])
		q[NEG].ad_DStandar[0] = 5;
	if (0 == q[NEG].ad_DStandar[1])
		q[NEG].ad_DStandar[1] = 5;
	aiSize[POS] = q[POS].vi_SizeCols.size();
	aiSize[NEG] = q[NEG].vi_SizeCols.size();
	
	FileStorage fs_SaveStream(strPath, FileStorage::WRITE);
	fs_SaveStream	<<  "Param_Config"	<< "{";
	fs_SaveStream		<<  "Version"		<< 2.3;
	fs_SaveStream		<<  "Max"			<< i_MaxContour;
	fs_SaveStream		<<  "Min"			<< i_MinContour;
	fs_SaveStream		<<  "Square"		<< i_Square;
	fs_SaveStream		<<  "Margin"		<< i_Margin;
	fs_SaveStream		<<  "Elongation"	<< d_Elongation;				
	
	
	iRows = (int)v_map_idl_BatteryLocation->size();
	if (0 < iRows)
	{
		fs_SaveStream		<<  "Battery_Location"	<< "{";
		fs_SaveStream			<<  "Rows"			<< iRows;
		fs_SaveStream			<<  "Cols"			<< i_MaxColumn();
		iRows = 0;
		for (auto aRow = v_map_idl_BatteryLocation->begin(); 
		aRow != v_map_idl_BatteryLocation->end(); aRow++)
		{
			strPosition = "Row" + to_string(iRows);
			iCols = 0;
			fs_SaveStream		<<  strPosition		<< "{";
			for (auto aCol = aRow->begin(); aCol != aRow->end();
			aCol++)
			{
				strPosition = "Index" + to_string(iCols++);
				fs_SaveStream <<  strPosition		<< "{";
				fs_SaveStream <<  "Locate"		<< (int)aCol->first;
				fs_SaveStream <<  "Polar"		<< (int)aCol->second.Pole;
				fs_SaveStream <<  "Center"		<< "[:";
				fs_SaveStream << (int)aCol->second.CenterPoint.x;
				fs_SaveStream << (int)aCol->second.CenterPoint.y;
				fs_SaveStream << "]";
				fs_SaveStream <<  "First"		<< "[:";
				fs_SaveStream << (int)aCol->second.FirstPoint.x;
				fs_SaveStream << (int)aCol->second.FirstPoint.y;
				fs_SaveStream << "]";
				fs_SaveStream <<  "Last"		<< "[:";
				fs_SaveStream << (int)aCol->second.LastPoint.x;
				fs_SaveStream << (int)aCol->second.LastPoint.y;
				fs_SaveStream << "]";
				fs_SaveStream << "}";
			}
			fs_SaveStream		<<  "}";
			iRows++;
		}
		fs_SaveStream		<< "}";
	}
	
//	fs_SaveStream		<<  "Positive"		<< "{";
//						
//	fs_SaveStream			<<  "MaxDS"			<< q[POS].ad_DStandar[0];
//	fs_SaveStream			<<  "MinDS"			<< q[POS].ad_DStandar[1];
//	fs_SaveStream			<<  "Column"		<< "{";
//	fs_SaveStream				<<  "Size"			<< aiSize[POS];
//	fs_SaveStream				<<  "Data"			<< "[";
//	for (auto aIterator = q[POS].vi_SizeCols.begin(); 
//	aIterator != q[POS].vi_SizeCols.end(); aIterator++)
//	{
//		i = (int)*aIterator;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream			<< "}";
//	fs_SaveStream			<<  "Values"		<< "{";
//	fs_SaveStream				<<  "Size"			<< 
//		(int)q[POS].vii_FirstCol.size();
//	fs_SaveStream				<<  "Media"			<< "[:";
//	for (auto aIterator = q[POS].vii_FirstCol.begin(); 
//	aIterator != q[POS].vii_FirstCol.end(); aIterator++)
//	{
//		i = aIterator->first;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream				<<  "D_Standar"		<< "[:";
//	for (auto aIterator = q[POS].vii_FirstCol.begin(); 
//	aIterator != q[POS].vii_FirstCol.end(); aIterator++)
//	{
//		i = aIterator->second;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream			<< "}";
//	
//	iRows = (int)q[POS].vviStatisticValues.size();
//	if (0 < iRows)
//	{
//		fs_SaveStream		<<  "Training"		<< "{";
//		fs_SaveStream			<<  "Rows"			<< iRows;
//		fs_SaveStream			<<  "Cols"			<< 
//			(int)q[POS].vviStatisticValues.begin()->size();
//		iRows = 0;
//		for (auto aRow = q[POS].vviStatisticValues.begin(); 
//		aRow != q[POS].vviStatisticValues.end(); aRow++)
//		{
//			strPosition = "Row" + to_string(iRows);
//			fs_SaveStream		<< strPosition		<< "[:";
//			for (auto aCol = aRow->begin(); aCol != aRow->end();
//			aCol++)
//			{
//				i = *aCol;
//				fs_SaveStream << i;
//			}
//			fs_SaveStream		<<  "]";
//			iRows++;
//		}
//		fs_SaveStream		<< "}";
//	}	
//	fs_SaveStream		<< "}";
//	//********************************************************* NEGATIVE ******************/
//	fs_SaveStream		<<  "Negative"		<< "{";
//						
//	fs_SaveStream			<<  "MaxDS"			<< q[NEG].ad_DStandar[0];
//	fs_SaveStream			<<  "MinDS"			<< q[NEG].ad_DStandar[1];
//	fs_SaveStream			<<  "Column"		<< "{";
//	fs_SaveStream				<<  "Size"			<< aiSize[NEG];
//	fs_SaveStream				<<  "Data"			<< "[";
//	for (auto aIterator = q[NEG].vi_SizeCols.begin(); 
//	aIterator != q[NEG].vi_SizeCols.end(); aIterator++)
//	{
//		i = (int)*aIterator;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream			<< "}";
//	fs_SaveStream			<<  "Values"		<< "{";
//	fs_SaveStream				<<  "Size"			<< 
//		(int)q[NEG].vii_FirstCol.size();
//	fs_SaveStream				<<  "Media"			<< "[:";
//	for (auto aIterator = q[NEG].vii_FirstCol.begin(); 
//	aIterator != q[NEG].vii_FirstCol.end(); aIterator++)
//	{
//		i = aIterator->first;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream				<<  "D_Standar"		<< "[:";
//	for (auto aIterator = q[NEG].vii_FirstCol.begin(); 
//	aIterator != q[NEG].vii_FirstCol.end(); aIterator++)
//	{
//		i = aIterator->second;
//		fs_SaveStream << i;
//	}
//	fs_SaveStream				<< "]";
//	fs_SaveStream			<< "}";
//	
//	iRows = (int)q[NEG].vviStatisticValues.size();
//	if (0 < iRows)
//	{
//		fs_SaveStream		<<  "Training"		<< "{";
//		fs_SaveStream			<<  "Rows"			<< iRows;
//		fs_SaveStream			<<  "Cols"			<< 
//			(int)q[NEG].vviStatisticValues.begin()->size();
//		iRows = 0;
//		for (auto aRow = q[NEG].vviStatisticValues.begin(); 
//		aRow != q[NEG].vviStatisticValues.end(); aRow++)
//		{
//			strPosition = "Row" + to_string(iRows);
//			fs_SaveStream		<< strPosition		<< "[:";
//			for (auto aCol = aRow->begin(); aCol != aRow->end();
//			aCol++)
//			{
//				i = *aCol;
//				fs_SaveStream << i;
//			}
//			fs_SaveStream		<<  "]";
//			iRows++;
//		}
//		fs_SaveStream		<< "}";
//	}	
	fs_SaveStream		<< "}";
	
	fs_SaveStream.release(); 
}
bool ObjectUnit::x_LoadFile(string strPath)
{
	int X;
	int Y;
	int iRows;
	int iCols;
	int iSize;
	int iCount;
	bool xFail = false;
	string strNodeName;
	FileNode fnData;
	FileNode fnPole;
	FileNode fnLocate;
	FileNode fnRows;
	FileNode fnCols;
	FileNode fnVector;
	FileNode fnValues;
	string strPosition;
	vector<int> viTMP;
	DataLocation dlTMP;
	Polar pPole;
	map<int, DataLocation> map_idlpData;
	FileStorage fs_OpenStream;
	double dMax = 0, dMin = 0;
	double dVersion = 0;
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
		v_map_idl_BatteryLocation = new vector<map<int, DataLocation> >();
		fnData					= fs_OpenStream["Param_Config"];
		dVersion				= (double)fnData["Version"];
		iVersion				= dVersion * 100;
		if (2 > dVersion)
		{
			i_MaxContour	= (int)fnData["Max"];
			i_MinContour	= (int)fnData["Min"];		
			i_Square		= (int)fnData["Square"];
			i_Margin		= (int)fnData["Margin"];
			d_Elongation	= (double)fnData["Elongation"];
		
			dMax = (double)fnData["MaxDS"];
			dMin = (double)fnData["MinDS"];
			v_ClearDStandar(POS);
			v_SetDStandar(POS, 0, dMax);
			v_SetDStandar(POS, 1, dMin);
			
			fnVector = fnData["Column"];
			iSize = (int)fnVector["Size"];
			q[POS].vi_SizeCols.clear();
			fnValues = fnVector["Data"];
			for (int i = 0; i < iSize; i++)
				q[POS].vi_SizeCols.push_back((int)fnValues[i]);
		
		
			fnVector = fnData["Values"];
			iSize = (int)fnVector["Size"];
			q[POS].vii_FirstCol.clear();
		
			fnValues = fnVector["Media"];
			for (int i = 0; i < iSize; i++)
				viTMP.push_back((int)fnValues[i]);
		
			auto aIterator = viTMP.begin();
			fnValues = fnVector["D_Standar"];
			for (int i = 0; i < iSize; i++)
				q[POS].vii_FirstCol.push_back(make_pair(*aIterator++, (int)fnValues[i]));
		
			fnVector = fnData["Training"];
			iRows = (int)fnVector["Rows"];
			iCols = (int)fnVector["Cols"];
			pArray = Point(iCols, iRows);
			q[POS].vviStatisticValues.clear();
			iCount = 0;		
			for (int i = 0; i < iRows; i++)
			{
				viTMP.clear();
				strPosition = "Row" + to_string(iCount);
				fnValues = fnVector[strPosition];
				for (int i = 0; i < iCols; i++)
					viTMP.push_back((int)fnValues[i]);
				q[POS].vviStatisticValues.push_back(viTMP);
				iCount++;
			}
		
			fnVector = fnData["Array_Position"];
			iRows = (int)fnVector["Rows"];
			iCols = (int)fnVector["Cols"];
		
			iCount = 0;
			for (int i = 0; i < iRows; i++)
			{
			
				strPosition = "Row" + to_string(iCount);
				fnValues = fnVector[strPosition];
				for (int i = 0; i < iCols * 3; i++)
				{
					iSize = (int)fnValues[i++];
					X = (int)fnValues[i++];
					Y = (int)fnValues[i];
					dlTMP = DataLocation(	Point(X, Y), 
											Point(X - i_Square, Y - i_Square),
											Point(X + i_Square, Y + i_Square),
											UNDEF);
					map_idlpData.insert(make_pair(iSize, dlTMP));
				}
				v_map_idl_BatteryLocation->push_back(map_idlpData);
				map_idlpData.clear();
				iCount++;
			}
			q[1] = q[0];
		}
		else if (2 <= dVersion)
		{
			i_MaxContour	= (int)fnData["Max"];
			i_MinContour	= (int)fnData["Min"];
			i_Square		= (int)fnData["Square"];
			i_Margin		= (int)fnData["Margin"];
			d_Elongation	= (double)fnData["Elongation"];
			
			fnLocate = fnData["Battery_Location"];
			
			iRows = (int)fnLocate["Rows"];
			iCols = (int)fnLocate["Cols"];
			pArray = Point(iCols, iRows);
			
			for (int i = 0; i < iRows; i++)
			{
				strNodeName = "Row" + to_string(i);
				fnRows = fnLocate[strNodeName];
				for (int j = 0; j < iCols; j++)
				{
					strNodeName = "Index" + to_string(j);
					fnCols = fnRows[strNodeName];
					iSize = (int)fnCols["Locate"];
					pPole = (Polar)((int)fnCols["Polar"]);
					X = (int)fnCols["Center"][0];
					Y = (int)fnCols["Center"][1];
					Point pCenter(X,Y);
					X = (int)fnCols["First"][0];
					Y = (int)fnCols["First"][1];
					Point pFirst(X, Y);
					X = (int)fnCols["Last"][0];
					Y = (int)fnCols["Last"][1];
					Point pLast(X, Y);
					dlTMP = DataLocation(pCenter, pFirst, pLast, pPole);
					map_idlpData.insert(make_pair(iSize, dlTMP));
				}
				v_map_idl_BatteryLocation->push_back(map_idlpData);
				map_idlpData.clear();
			}
			
//			fnPole = fnData["Positive"];
//
//			dMax = (double)fnPole["MaxDS"];
//			dMin = (double)fnPole["MinDS"];
//		
//			v_ClearDStandar(POS);
//			v_SetDStandar(POS, 0, dMax);
//			v_SetDStandar(POS, 1, dMin);
//		
//			fnVector = fnPole["Column"];
//			iSize = (int)fnVector["Size"];
//			q[POS].vi_SizeCols.clear();
//			fnValues = fnVector["Data"];
//			for (int i = 0; i < iSize; i++)
//				q[POS].vi_SizeCols.push_back((int)fnValues[i]);
//		
//		
//			fnVector = fnPole["Values"];
//			iSize = (int)fnVector["Size"];
//			q[POS].vii_FirstCol.clear();
//		
//			fnValues = fnVector["Media"];
//			for (int i = 0; i < iSize; i++)
//				viTMP.push_back((int)fnValues[i]);
//		
//			auto aIterator = viTMP.begin();
//			fnValues = fnVector["D_Standar"];
//			for (int i = 0; i < iSize; i++)
//				q[POS].vii_FirstCol.push_back(make_pair(*aIterator++, (int)fnValues[i]));
//		
//			fnVector = fnPole["Training"];
//			iRows = (int)fnVector["Rows"];
//			iCols = (int)fnVector["Cols"];
//			q[POS].vviStatisticValues.clear();
//			iCount = 0;		
//			for (int i = 0; i < iRows; i++)
//			{
//				viTMP.clear();
//				strPosition = "Row" + to_string(iCount);
//				fnValues = fnVector[strPosition];
//				for (int i = 0; i < iCols; i++)
//					viTMP.push_back((int)fnValues[i]);
//				q[POS].vviStatisticValues.push_back(viTMP);
//				iCount++;
//			}
//		
//			fnVector = fnPole["Array_Position"];
//			iRows = (int)fnVector["Rows"];
//			iCols = (int)fnVector["Cols"];
//		
//			q[POS].v_map_ivp_ArrayBattery->clear();
//			iCount = 0;
//			for (int i = 0; i < iRows; i++)
//			{
//			
//				strPosition = "Row" + to_string(iCount);
//				fnValues = fnVector[strPosition];
//				for (int i = 0; i < iCols * 3; i++) {
//					iSize = (int)fnValues[i++];
//					X = (int)fnValues[i++];
//					Y = (int)fnValues[i];
//					vpTMP.push_back(Point(X, Y));
//					map_ivpData.insert(make_pair(iSize, vpTMP));
//					vpTMP.clear();
//				}
//				q[POS].v_map_ivp_ArrayBattery->push_back(map_ivpData);
//				map_ivpData.clear();
//				iCount++;
//			}
//		
//			{
//				fnPole = fnData["Negative"];
//		
//				dMax = (double)fnPole["MaxDS"];
//				dMin = (double)fnPole["MinDS"];
//		
//				v_ClearDStandar(POS);
//				v_SetDStandar(POS, 0, dMax);
//				v_SetDStandar(POS, 1, dMin);
//
//				fnVector = fnPole["Column"];
//				iSize = (int)fnVector["Size"];
//				q[NEG].vi_SizeCols.clear();
//				fnValues = fnVector["Data"];
//				for (int i = 0; i < iSize; i++)
//					q[NEG].vi_SizeCols.push_back((int)fnValues[i]);
//		
//		
//				fnVector = fnPole["Values"];
//				iSize = (int)fnVector["Size"];
//				q[NEG].vii_FirstCol.clear();
//		
//				fnValues = fnVector["Media"];
//				for (int i = 0; i < iSize; i++)
//					viTMP.push_back((int)fnValues[i]);
//		
//				auto aIterator = viTMP.begin();
//				fnValues = fnVector["D_Standar"];
//				for (int i = 0; i < iSize; i++)
//					q[NEG].vii_FirstCol.push_back(make_pair(*aIterator++, (int)fnValues[i]));
//		
//				fnVector = fnPole["Training"];
//				iRows = (int)fnVector["Rows"];
//				iCols = (int)fnVector["Cols"];
//				q[NEG].vviStatisticValues.clear();
//				iCount = 0;		
//				for (int i = 0; i < iRows; i++)
//				{
//					viTMP.clear();
//					strPosition = "Row" + to_string(iCount);
//					fnValues = fnVector[strPosition];
//					for (int i = 0; i < iCols; i++)
//						viTMP.push_back((int)fnValues[i]);
//					q[NEG].vviStatisticValues.push_back(viTMP);
//					iCount++;
//				}
//		
//				fnVector = fnPole["Array_Position"];
//				iRows = (int)fnVector["Rows"];
//				iCols = (int)fnVector["Cols"];
//		
//				q[NEG].v_map_ivp_ArrayBattery->clear();
//				iCount = 0;
//				for (int i = 0; i < iRows; i++)
//				{
//			
//					strPosition = "Row" + to_string(iCount);
//					fnValues = fnVector[strPosition];
//					for (int i = 0; i < iCols * 3; i++) {
//						iSize = (int)fnValues[i++];
//						X = (int)fnValues[i++];
//						Y = (int)fnValues[i];
//						vpTMP.push_back(Point(X, Y));
//						map_ivpData.insert(make_pair(iSize, vpTMP));
//						vpTMP.clear();
//					}
//					q[NEG].v_map_ivp_ArrayBattery->push_back(map_ivpData);
//					map_ivpData.clear();
//					iCount++;
//				}
//			}
		}
		fs_OpenStream.release();
		return true;
	}
	fs_OpenStream.release();
	return false;
}
void ObjectUnit::v_ExtractSizes(Polar Pole, Mat MOriginalImage)
{	
//	Mat MBattery;
//	Point pFirst, pLast;
//	vector<int> viArea;
//	int iwidth = MOriginalImage.size().width;
//	int iheight = MOriginalImage.size().height;
//	int iBlackPixels, iTotalArea, iAcomulate, iElements;
//	double dMedia, dSumatory, dDesviationS;
//	q[Pole].vii_FirstCol.clear();
//	q[Pole].vi_SizeCols.clear();
////	int ic = 0;
//	
//	for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//	aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//	{
//		iAcomulate = 0;
//		dSumatory = 0;
//		iElements = aRow->size();
//		viArea.clear();
////		ic++;
////		if (8 == ic)
////			ic = 0;
//			
//		for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
//		{
////			ic++;
////			if (21 == ic)
////				ic = 0;
////			
//			pFirst = Point(aCol->second[0].x - i_Square, 
//				aCol->second[0].y - i_Square);									// Genera el cuadro de evaluación
//			pLast = Point(aCol->second[0].x + i_Square, 
//				aCol->second[0].y + i_Square);
//			
//			if (0 > pFirst.x)															// Ajusta a rectangulo en caso de que salga
//				pFirst.x = 0;															// las dimansiones de la imagen
//			if (0 > pFirst.y)
//				pFirst.y = 0;
//			if (0 > pLast.x)
//				pLast.x = 0;
//			if (0 > pLast.y)
//				pLast.y = 0;
//			if (pFirst.x > iwidth)
//				pFirst.x = iwidth;
//			if (pLast.x > iwidth)
//				pLast.x = iwidth;
//			if (pFirst.y > iheight)
//				pFirst.y = iheight;
//			if (pLast.y > iheight)
//				pLast.y = iheight;
//			
//			MBattery = (MOriginalImage)(Rect(pFirst, pLast));							// Recorta la unidad de la imagen
//			iTotalArea = ((pLast.x - pFirst.x) * (pLast.y - pFirst.y));					// Calcula el area del cuadro de evaluación		
//			iBlackPixels = iTotalArea - countNonZero(MBattery);							// Calcula los pixeles negros
//			viArea.push_back(iBlackPixels);
//			
//
//		}
//		
//		
//		
//		dMedia = (double)iAcomulate / iElements;
//		
//		for (auto aIterator = viArea.begin(); aIterator != viArea.end(); aIterator++)
//			dSumatory += pow((double)(*aIterator) - dMedia, 2);
//		
//		dSumatory = dSumatory / iElements;
//		
//		dDesviationS = sqrt(dSumatory);
//		
//		q[Pole].vii_FirstCol.push_back(make_pair((int)dMedia, (int)dDesviationS));		// Guarda el valor de la primer columna
//		q[Pole].vi_SizeCols.push_back(iElements);
//	}
}
void ObjectUnit::v_ExtractSizes(Polar Pole, vector<Mat> vMOriginalImage, bool xAdd)
{
//	Mat MBattery;
//	Point pFirst, pLast;
//	vector<int> viArea;
//	int iwidth = vMOriginalImage[0].size().width;
//	int iheight = vMOriginalImage[0].size().height;
//	auto aIterator = vMOriginalImage.begin();
//	int iBlackPixels, iTotalArea, iElements;
//	double dMedia, dSumatory, dDesviationS;
//	bool xFirst = true;
//	q[Pole].vii_FirstCol.clear();
//	q[Pole].vi_SizeCols.clear();
//	if (!xAdd)
//		q[Pole].vviStatisticValues.clear();
//	while(aIterator != vMOriginalImage.end())
//	{	
//		int iCount = 0;
//		for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//		aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//		{
//			dSumatory = 0;
//			if (0 == iCount)
//				iElements = aRow->size();
//			vector<int> viArea;
//			if (iCount < q[Pole].vviStatisticValues.size())
//				viArea = q[Pole].vviStatisticValues[iCount];
//			
//			for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
//			{			
//				pFirst = Point(aCol->second[0].x - i_Square, 
//					aCol->second[0].y - i_Square);								// Genera el cuadro de evaluación
//				pLast = Point(aCol->second[0].x + i_Square, 
//					aCol->second[0].y + i_Square);
//			
//				if (0 > pFirst.x)														// Ajusta a rectangulo en caso de que salga
//					pFirst.x = 0;														// las dimansiones de la imagen
//				if (0 > pFirst.y)
//					pFirst.y = 0;
//				if (0 > pLast.x)
//					pLast.x = 0;
//				if (0 > pLast.y)
//					pLast.y = 0;
//				if (pFirst.x > iwidth)
//					pFirst.x = iwidth;
//				if (pLast.x > iwidth)
//					pLast.x = iwidth;
//				if (pFirst.y > iheight)
//					pFirst.y = iheight;
//				if (pLast.y > iheight)
//					pLast.y = iheight;
//			
//				MBattery = (*aIterator)(Rect(pFirst, pLast));							// Recorta la unidad de la imagen
//				iTotalArea = ((pLast.x - pFirst.x) * (pLast.y - pFirst.y));				// Calcula el area del cuadro de evaluación		
//				iBlackPixels = iTotalArea - countNonZero(MBattery);						// Calcula los pixeles negros
//				viArea.push_back(iBlackPixels);
//			}
//			
//			if (iCount >= q[Pole].vviStatisticValues.size())
//				q[Pole].vviStatisticValues.push_back(viArea);
//			else
//				q[Pole].vviStatisticValues[iCount] = viArea;
//			
//			if (xFirst)
//				q[Pole].vi_SizeCols.push_back(iElements);
//			iCount++;
//		}
//		xFirst = false;
//		aIterator++;
//	}
//	
//	for (auto aIterator = q[Pole].vviStatisticValues.begin(); 
//	aIterator != q[Pole].vviStatisticValues.end(); aIterator++)
//	{
//		dDesviationS = d_CalculateDStandar(*aIterator);
//		dMedia = d_CalculateMedia(*aIterator);
//		q[Pole].vii_FirstCol.push_back(make_pair((int)dMedia, (int)dDesviationS));		// Guarda el valor de la primer columna	
//	}	
}
Point ObjectUnit::p_CalculateMaxMin(vector<int> viData)
{
	int iMax = *viData.begin();
	int iMin = iMax;
	
	for (auto aIterator = viData.begin(); aIterator != viData.end(); aIterator++) 
	{
		if (*aIterator > iMax)
			iMax = *aIterator;
		if (*aIterator < iMin)
			iMin = *aIterator;
	}
	
	return Point(iMax, iMin);
}
double ObjectUnit::d_CalculateDStandar(vector<int> viData)
{
	double dSumatory, dDesviationS, dMedia = d_CalculateMedia(viData);
		
	for (auto aIterator = viData.begin(); aIterator != viData.end(); aIterator++)
		dSumatory += pow((double)(*aIterator) - dMedia, 2);
		
	dSumatory = dSumatory / viData.size();
		
	dDesviationS = sqrt(dSumatory);
	
	return dDesviationS;
}
double ObjectUnit::d_CalculateMedia(vector<int> viData)
{
	double dAcomulate;
	
	for (auto aIterator = viData.begin(); aIterator != viData.end(); aIterator++)
		dAcomulate += *aIterator;
	
	double dMedia = dAcomulate / viData.size();
	
	return dMedia;
}
void ObjectUnit::v_ShowData(Polar Pole, Mat &MOriginalImage)
{
//	string strSing = "";
//	int iCount = 1;
//	for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//	aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//	{
//		auto aCol = aRow->begin();
//	
//		putText(MOriginalImage,
//			to_string(iCount++),
//			aCol->second[0],
//			cv::FONT_HERSHEY_DUPLEX,
//			3.0,
//			CV_RGB(255, 100, 0),
//			2);
//		
//		putText(MOriginalImage,
//			to_string(aRow->size()),
//			Point(aCol->second[0].x + (i_Square * 4), aCol->second[0].y),
//			cv::FONT_HERSHEY_DUPLEX,
//			3.0,
//			CV_RGB(255, 100, 0),
//			2);
//	}
}
Mat ObjectUnit::M_ShowUnits(Polar Pole, Mat MOriginalImage)
{
//	Mat MBattery;
//	Point pFirst, pLast;
//	int iwidth = MOriginalImage.size().width;
//	int iheight = MOriginalImage.size().height;
//	int iBlackPixels, iTotalArea;
//	
//	for (auto aRow = q[Pole].v_map_ivp_ArrayBattery->begin(); 
//	aRow != q[Pole].v_map_ivp_ArrayBattery->end(); aRow++)
//	{
//		for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
//		{
//			pFirst = Point(aCol->second[0].x - i_Square, 
//				aCol->second[0].y - i_Square);									// Genera el cuadro de evaluación
//			pLast = Point(aCol->second[0].x + i_Square, 
//				aCol->second[0].y + i_Square);
//			
//			if (0 > pFirst.x)															// Ajusta a rectangulo en caso de que salga
//				pFirst.x = 0;															// las dimansiones de la imagen
//			if (0 > pFirst.y)
//				pFirst.y = 0;
//			if (0 > pLast.x)
//				pLast.x = 0;
//			if (0 > pLast.y)
//				pLast.y = 0;
//			if (pFirst.x > iwidth)
//				pFirst.x = iwidth;
//			if (pLast.x > iwidth)
//				pLast.x = iwidth;
//			if (pFirst.y > iheight)
//				pFirst.y = iheight;
//			if (pLast.y > iheight)
//				pLast.y = iheight;
//		
//			rectangle(MOriginalImage,													// Dibuja en la nueva imagen el rectangulo del contorno
//				pFirst, 
//				pLast, 
//				Scalar(0, 255, 0), 
//				5);
//		}
//	}
	return MOriginalImage;
}
int ObjectUnit::i_MaxColumn(Polar Pole)
{
	int iMax = 0;
	for (auto aIterator = q[Pole].vi_SizeCols.begin(); 
	aIterator != q[Pole].vi_SizeCols.end(); aIterator++)
	{
		if (iMax < *aIterator)
			iMax = *aIterator;
	}
	return iMax;
}
int ObjectUnit::i_MaxColumn()
{
	int iMax = 0;
	for (auto aIterator = v_map_idl_BatteryLocation->begin();
	aIterator != v_map_idl_BatteryLocation->end(); aIterator++)
	{
		if (iMax < aIterator->size())
			iMax = aIterator->size();
	}
	return iMax;
}
void ObjectUnit::v_LogExecution(Polar Pole, QString qstrPath)
{
	int iCount = 0;
	string strCommand = "/home/pi/battery/.log/lastLog.xml";
	
	FileStorage fs_SaveStream(strCommand, FileStorage::WRITE);
	fs_SaveStream <<  "Pair_Rows" << "{";
	for (auto aIterator = q[Pole].vii_FirstCol.begin(); 
	aIterator != q[Pole].vii_FirstCol.end(); aIterator++)
	{
		fs_SaveStream << "Row" + to_string(iCount) << "[";
		fs_SaveStream << (int)aIterator->first << (int)aIterator->second;
		fs_SaveStream << "]";
		iCount++;
	}
	fs_SaveStream << "}";
	
	iCount = 0;
	fs_SaveStream <<  "Data_Array" << "{";
	for (auto aRows = q[Pole].vvi_LastInspect.begin(); 
	aRows != q[Pole].vvi_LastInspect.end(); aRows++)
	{
		fs_SaveStream << "Row" + to_string(iCount) << "[";
		for (auto aCols = aRows->begin(); aCols != aRows->end(); aCols++)
		{
			fs_SaveStream << (int)*aCols;
		}
		fs_SaveStream << "]";
		iCount++;
	}
	q[Pole].vvi_LastInspect.clear();
	fs_SaveStream << "}";	
	fs_SaveStream.release();		
}
bool ObjectUnit::x_Evaluate(ObjectUnit &ouDataCompare,
	vector<pair<int, Point>> &vipDataErrors,
	Mat MOriginalImage,
	Mat &MResult)
{
//	Mat MBattery;
//	Point pFirst;
//	Point pLast;
//	Point pMid;
//	int iTotalArea;
//	int iBlackPixels;
//	int MaxStandar, MinStandar;
//	bool xFirst;
//	bool xResult = true;
//	auto aiterator = ouDataCompare.vii_FirstCol.begin();
//	auto aSizes = ouDataCompare.v_map_ivp_ArrayBattery->begin();
//	int iwidth = MOriginalImage.size().width;
//	int iheight = MOriginalImage.size().height;
//	int iRow = 0;
//	int iCountRow;
//	int iRes = 0;
//	string strCount = "";
//	
//	for (auto aRow = v_map_ivp_ArrayBattery->begin(); 
//	aRow != v_map_ivp_ArrayBattery->end(); aRow++)
//	{
//		MaxStandar = aiterator->first + (aiterator->second * ouDataCompare.ad_DStandar[0]);
//		MinStandar = aiterator->first - (aiterator->second * ouDataCompare.ad_DStandar[1]);
//		xFirst = true;
//		iCountRow = 0;
//		vector<int> viDataInspect;
//		for (auto aCol = aRow->begin(); aCol != aRow->end(); aCol++)
//		{
//			pFirst = Point(aCol->second[0].x - i_Square, 
//				aCol->second[0].y - i_Square);
//			pLast = Point(aCol->second[0].x + i_Square, 
//				aCol->second[0].y + i_Square);
//			if (xFirst) {
//				pMid = aCol->second[0];
//				xFirst = false;
//			}
//			if (0 > pFirst.x)
//				pFirst.x = 0;
//			if (0 > pFirst.y)
//				pFirst.y = 0;
//			if (0 > pLast.x)
//				pLast.x = 0;
//			if (0 > pLast.y)
//				pLast.y = 0;
//			if (pFirst.x > iwidth)
//				pFirst.x = iwidth;
//			if (pLast.x > iwidth)
//				pLast.x = iwidth;
//			if (pFirst.y > iheight)
//				pFirst.y = iheight;
//			if (pLast.y > iheight)
//				pLast.y = iheight;
//			MBattery = (MOriginalImage)(Rect(pFirst, pLast));
//			iTotalArea = ((pLast.x - pFirst.x) * (pLast.y - pFirst.y));
//			iBlackPixels = iTotalArea - countNonZero(MBattery);
//			
//			viDataInspect.push_back(iBlackPixels);
//			if (iBlackPixels > MinStandar && iBlackPixels < MaxStandar)
//				rectangle(MResult, pFirst, pLast, Scalar(0, 255, 0), 5);
//			else
//			{
//				vipDataErrors.push_back(
//						make_pair(iBlackPixels, 
//						Point(iCountRow, iRow)));
//				rectangle(MResult, pFirst, pLast, Scalar(0, 0, 255), 10);
//				xResult = false;
//			}
//			iCountRow++;
//		}
//		ouDataCompare.vvi_LastInspect.push_back(viDataInspect);
//		if (aSizes->size() != iCountRow)
//		{		
//			iRes = aSizes->size() - iCountRow;
//			if (0 < iRes)
//				strCount = "Missing units: " + to_string(iRes);
//			else
//				strCount = "Excess units: " + to_string(abs(iRes));
//			
//			putText(MResult,
//				strCount,
//				pMid,
//				cv::FONT_HERSHEY_DUPLEX,
//				3.0,
//				CV_RGB(255,0,0),
//				4);
//			vipDataErrors.push_back(
//						make_pair(-1, Point(iCountRow*-1, iRow)));
//			xResult = false;
//		}
//		iRow++;
//		aSizes++;
//		aiterator++;
//	}
//	return xResult;
return true;
}
/*
 * 1) Promedio
 * 2) Dato menos promedio al cuadrado
 * 3) La sumatoria del resultado anterior
 * 4) Se divide entre la cantidad de datos
 * 5) Se saca la raiz cuadrada
 */
Point ObjectUnit::p_GetFirstPoint(Polar Pole, int iIndex)
{
	Point pLocation;// = (*q[Pole].v_map_ivp_ArrayBattery)[iIndex].begin()->second[0];
	return pLocation;
}
int ObjectUnit::i_GetVersion()
{
	return iVersion;
}
int ObjectUnit::i_GetMax() 
{ 
	return i_MaxContour;
}
int ObjectUnit::i_GetMin()
{ 
	return i_MinContour;
}
int ObjectUnit::i_GetSquare() 
{ 
	return i_Square;
}
int ObjectUnit::i_GetMargin()
{ 
	return i_Margin;
}
QString ObjectUnit::qMax()
{
	return QString::number(i_MaxContour); 
}
QString ObjectUnit::qMin()
{
	return QString::number(i_MinContour);
}
QString ObjectUnit::qSquare()
{
	return QString::number(i_Square);
}
QString ObjectUnit::qMargin()
{
	return QString::number(i_Margin);
}
QString ObjectUnit::qElongation() 
{
	return QString::number(d_Elongation * 100);
}
Size ObjectUnit::sGetSize(Polar Pole) 
{ 
	return Size(i_MaxColumn(Pole), q[Pole].vi_SizeCols.size());
}
vector<pair<int, int>> ObjectUnit::vi_GetColValues(Polar Pole) 
{ 
	return q[Pole].vii_FirstCol; 
}
vector<vector<int>> ObjectUnit::vvi_GetStadistics(Polar Pole) 
{ 
	return q[Pole].vviStatisticValues ; 
}
void ObjectUnit::v_SetDStandar(Polar Pole, int iIndex, double dData) 
{ 
	q[Pole].ad_DStandar[iIndex] = dData; 
}
void ObjectUnit::v_ClearDStandar(Polar Pole)
{ 
	q[Pole].ad_DStandar.fill(0);
}
double ObjectUnit::d_GetDStandar(Polar Pole, int iIndex)
{
	return q[Pole].ad_DStandar[iIndex];
}
void ObjectUnit::v_SetPole(array<int, 5> aiData)
{
	i_MaxContour = aiData[0];
	i_MinContour = aiData[1];
	i_Square = aiData[2];
	i_Margin = aiData[3];
	d_Elongation = (double)aiData[4] / 100;
	v_map_idl_BatteryLocation = new vector<map<int, DataLocation > >();
}
bool ObjectUnit::x_EmptyArray()
{
	if (0 < pArray.x && 0 < pArray.y)
		return false;
	else
		return true;
}

/*********************** INSPECT MACHINE ****************************/
InspectMachine::InspectMachine()
{
	wiringPiSetup();
	pinMode(LIGHT, OUTPUT);
	pinMode(UP, OUTPUT);
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);
	pinMode(DOWN, OUTPUT);
	pinMode(BUTTON, INPUT);
	pinMode(SENSORU, INPUT);
	pinMode(STOPPER, OUTPUT);
	pinMode(WINDOWS, OUTPUT);
	pinMode(SENSORW1, INPUT);
	pinMode(SENSORW2, INPUT);
	str_Path = "/home/pi/";
	PictureSettings[0].str_Path = str_Path + "Picture.jpg";
	PictureSettings[1].str_Path = str_Path + "Image.jpg";
}
InspectMachine::InspectMachine(string strName, string strPath)
{
	str_Path = strPath;
	str_Name = strName;
	wiringPiSetup();
	pinMode(LIGHT, OUTPUT);
	pinMode(UP, OUTPUT);
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);
	pinMode(DOWN, OUTPUT);
	pinMode(BUTTON, INPUT);
	pinMode(SENSORU, INPUT);
	pinMode(STOPPER, OUTPUT);
	pinMode(WINDOWS, OUTPUT);
	pinMode(SENSORW1, INPUT);
	pinMode(SENSORW2, INPUT);
	digitalWrite(STOPPER, LOW);
	digitalWrite(WINDOWS, LOW);
	PictureSettings[0].str_Path = str_Path + "Picture.jpg";
	PictureSettings[1].str_Path = str_Path + "Image.jpg";
}
InspectMachine::~InspectMachine()
{
	
}
Mat InspectMachine::StabilizedImage(int iTimes)
{
//	Mat MResultAdd;
//	for (int i = 0; i < iTimes; i++)
//	{
//		Mat MTmp = CutPicture(ImageSettings, true, 1); // Toma la segunda imagen para evaluación
//		ExecuteFilters(ImageFilters, MTmp); // Ejecuta filtro de evaluación
//		if (MResultAdd.cols == 0 && MResultAdd.rows == 0)
//			MResultAdd = MTmp;
//		else
//			add(MResultAdd, MTmp, MResultAdd);
//	}
//	return MResultAdd;
}
Mat InspectMachine::TakePicture(ImageProcessSettings ipsData)
{
	v_LightOn();
	cCameraPi::v_TakePicture(ipsData);
	v_LightOff();
	return Mat(cCameraPi::M_LoadPicture(ipsData.str_Path));
}
Mat InspectMachine::LoadPicture()
{
	return Mat(cCameraPi::M_LoadPicture(PictureSettings[0].str_Path));
}
void InspectMachine::v_SavePicture(Mat MSave)
{
	imwrite("/home/pi/battery/.log/image.jpg", MSave);
}
Mat InspectMachine::CutPicture(ImageProcessSettings ipsData, bool xCalib, int iTake)
{
	Mat MOriginalPicture, MBackup;
	if (1 == iTake)
		MOriginalPicture = TakePicture(ipsData);
	else
		MOriginalPicture = LoadPicture();
	MOriginalPicture.copyTo(MBackup);													// Genera un respaldo de la imagen
	if (true == xCalib)																	// Verifica si necesita calibrar
		MOriginalPicture = DataCalib.MImage(MOriginalPicture);							// Calibra
	if (0 == MOriginalPicture.cols && 0 == MOriginalPicture.rows)						// Verifica que no este vacio el resultado
		MBackup.copyTo(MOriginalPicture);												// Si esta vacio recupera el respaldo

	return Mat(cCameraPi::M_CutImage(MOriginalPicture, 
		PictureSettings[0]));																// Corta la imagen
}
void InspectMachine::v_SetData(array<ImageProcessSettings,3> _PictureSettings,
	CalibImage _DataCalib,
	array<FilterCartridge,3> _PictureFilter,
	ObjectUnit _BatteryFound)
{
	PictureSettings = _PictureSettings;
	DataCalib = _DataCalib;
	PictureFilters = _PictureFilter;
	BatteryFound = _BatteryFound;
}
void InspectMachine::ExecuteFilters(FilterCartridge fcFilters, Mat &MOriginalPicture)
{
	fcFilters.v_SetStart();
	int iSize = fcFilters.i_GetCount();
	
	for (int i = 0; i < iSize; i++)
		fcFilters.v_Execute(MOriginalPicture);
}
void InspectMachine::v_LightOn()
{
	digitalWrite(UP,	HIGH);
	digitalWrite(LEFT,	HIGH);
	digitalWrite(RIGHT,	HIGH);
	
	digitalWrite(DOWN,	HIGH);
	digitalWrite(LIGHT,	HIGH);
}
void InspectMachine::v_LightOff()
{
	digitalWrite(UP,	LOW);
	digitalWrite(LEFT,	LOW);
	digitalWrite(RIGHT,	LOW);
	digitalWrite(DOWN,	LOW);
	digitalWrite(LIGHT,	LOW);
}
void InspectMachine::v_SetLoop(int iLoop)
{
	i_Loop = iLoop;
}
void InspectMachine::v_SaveFile()
{
	string strNameFile = str_Path + str_Name + ".yml";
	FileStorage fs_SaveStream(strNameFile, FileStorage::WRITE);
	fs_SaveStream <<  "Inspect_Machine" << "{";
	fs_SaveStream <<  "Name" << str_Name;
	fs_SaveStream <<  "Path" << str_Path;
	fs_SaveStream <<  "Loop" << i_Loop;
	fs_SaveStream <<  "IO" << "{";
	fs_SaveStream <<  "Size" << (int)m_stri_IO.size();
	fs_SaveStream <<  "Key" << "[:";
	for (auto aIterator = m_stri_IO.begin(); aIterator != m_stri_IO.end(); aIterator++)
		fs_SaveStream << aIterator->first;
	fs_SaveStream <<  "]";
	fs_SaveStream <<  "Data" << "[:";
	for (auto aIterator = m_stri_IO.begin(); aIterator != m_stri_IO.end(); aIterator++)
		fs_SaveStream << aIterator->second;
	fs_SaveStream <<  "]";
	fs_SaveStream <<  "}";
	fs_SaveStream <<  "}";
	fs_SaveStream.release();
	
	PictureSettings[0].v_SaveFile(str_Path + str_Name + ".pic.xml");
	PictureSettings[1].v_SaveFile(str_Path + str_Name + ".imaP.xml");
	PictureSettings[2].v_SaveFile(str_Path + str_Name + ".imaN.xml");
	
	DataCalib.v_SaveFile(str_Path + str_Name + ".cal.xml");
	
	PictureFilters[0].v_SaveFile(str_Path + str_Name + ".ftrP.xml");
	PictureFilters[1].v_SaveFile(str_Path + str_Name + ".untP.xml");
	PictureFilters[2].v_SaveFile(str_Path + str_Name + ".untN.xml");
	
	BatteryFound.v_SaveFile(str_Path + str_Name + ".pos.xml");
	
}
ulong InspectMachine::ul_LoadFile(string strPath)
{
	ulong ulFail = 0;
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
		FileNode fnValues = fs_OpenStream["Inspect_Machine"];
		vector<string> vstrNames;
		
		str_Name = (string)fnValues["Name"];
		str_Path = (string)fnValues["Path"];
		i_Loop = (int)fnValues["Loop"];
		
		FileNode fnVector = fnValues["IO"];
		int iSize = (int)fnVector["Size"];
		FileNode fnNames = fnVector["Key"];
		int i;
		for (i = 0; i < iSize; i++)
			vstrNames.push_back((string)fnNames[i]);
		
		i = 0;
		FileNode fnData = fnVector["Data"];
		for (auto aIterator = vstrNames.begin(); aIterator != vstrNames.end(); aIterator++)
			m_stri_IO.insert(make_pair(*aIterator, (int)fnData[i++]));
		
		v_Unpack_IO();
		
		fs_OpenStream.release();
		
		if (!PictureSettings[0].x_LoadFile(str_Path + str_Name + ".pic.xml"))
			ulFail = ulFail | 0x00000001;
		if (!DataCalib.x_LoadFile(str_Path + str_Name + ".cal.xml"))
			ulFail = ulFail | 0x00000010;
		if (!PictureFilters[0].x_LoadFile(str_Path + str_Name + ".ftr.xml"))
		{	
			if (!PictureFilters[0].x_LoadFile(str_Path + str_Name + ".ftrP.xml"))
				ulFail = ulFail | 0x00000100;
		}
		if (!PictureSettings[1].x_LoadFile(str_Path + str_Name + ".imag.xml"))
		{
			if (!PictureSettings[1].x_LoadFile(str_Path + str_Name + ".imaP.xml"))
				ulFail = ulFail | 0x00001000;
			if (!PictureSettings[1].x_LoadFile(str_Path + str_Name + ".imaN.xml"))
				ulFail = ulFail | 0x00010000;
		}
		if (!PictureFilters[1].x_LoadFile(str_Path + str_Name + ".unt.xml"))
		{
			if (!PictureFilters[1].x_LoadFile(str_Path + str_Name + ".untP.xml"))
				ulFail = ulFail | 0x00100000;
			if (!PictureFilters[2].x_LoadFile(str_Path + str_Name + ".untN.xml"))
				ulFail = ulFail | 0x01000000;
		}
		if (!BatteryFound.x_LoadFile(str_Path + str_Name + ".pos.xml"))
			ulFail = ulFail | 0x10000000;
		return ulFail;
	}
	fs_OpenStream.release();
	
	return 0x11111111;
}
void InspectMachine::v_SetIO(map<string, int> IO)
{
	m_stri_IO = IO;
	v_Unpack_IO();
}
void InspectMachine::v_Unpack_IO()
{
	for (auto aIterator = m_stri_IO.begin(); aIterator != m_stri_IO.end(); aIterator++)
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
	
	pinMode(UP, OUTPUT);
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);
	pinMode(DOWN, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	
	pinMode(STOPPER, OUTPUT);
	pinMode(WINDOWS, OUTPUT);
	
	pinMode(SENSORU, INPUT);
	pinMode(BUTTON, INPUT);
	pinMode(SENSORW1, INPUT);
	pinMode(SENSORW2, INPUT);
}

map<string, int> InspectMachine::m_stri_GetIO()
{
	return m_stri_IO;
}

/****************** FILTER FUNCTIONS ********************************/
void Filter::Binary(FilterParams &fp_Params)
{	
	Mat MGray_Image, MResultImage;//, MBlurImage;
	if (1 < fp_Params.vM_Image[0].channels())
		cvtColor(fp_Params.vM_Image[0], MGray_Image, COLOR_BGR2GRAY);					// Crea imagen HSV apartir de la original
	else
		fp_Params.vM_Image[0].copyTo(MGray_Image);
	threshold(MGray_Image, 
		MResultImage, 
		fp_Params.i_Param[1], 
		fp_Params.i_Param[0], 
		THRESH_BINARY);
	
//	blur(MBlurImage, 
//		MResultImage, 
//		Size(5, 5));
	
	fp_Params.vM_Image[1] = MResultImage;
}
void Filter::HSV(FilterParams &fp_Params)
{
	if (3 == fp_Params.vM_Image[0].channels())
	{
		Mat MHSV_Image, * MResultImage;
		MResultImage = new Mat();
	
		Scalar s_Low = Scalar(fp_Params.i_Param[1], 
			fp_Params.i_Param[3], 
			fp_Params.i_Param[5]);
		Scalar s_High = Scalar(fp_Params.i_Param[0], 
			fp_Params.i_Param[2], 
			fp_Params.i_Param[4]);
	
		cvtColor(fp_Params.vM_Image[0], MHSV_Image, COLOR_BGR2HSV);						// Crea imagen HSV apartir de la original
		inRange(MHSV_Image, s_Low, s_High, *MResultImage);								// Segmenta imagen HSV
	
		fp_Params.vM_Image[1] = *MResultImage;
	}
	else
	{
		fp_Params.vM_Image[1] = fp_Params.vM_Image[0];
	}
}
void Filter::Dilate(FilterParams &fp_Params)
{
	Mat MResultImage;
	
	Mat MElement = getStructuringElement( MORPH_RECT,
		Size(2*fp_Params.i_Param[0] + 1, 2*fp_Params.i_Param[0] + 1),
		Point(fp_Params.i_Param[0], fp_Params.i_Param[0]));
	
	dilate(fp_Params.vM_Image[0], MResultImage, MElement);								// Dilata imagen segmentada
	
	fp_Params.vM_Image[1] = MResultImage;
}
void Filter::Erode(FilterParams &fp_Params)
{
	Mat MResultImage;
	
	Mat MElement = getStructuringElement( MORPH_RECT,
		Size(2*fp_Params.i_Param[0] + 1, 2*fp_Params.i_Param[0] + 1),
		Point(fp_Params.i_Param[0], fp_Params.i_Param[0]));
	
	erode(fp_Params.vM_Image[0], MResultImage, MElement);								// Erosiona imagen segmentada
	
	fp_Params.vM_Image[1] = MResultImage;
}
void Filter::Bilateral(FilterParams &fp_Params)
{
	Mat MResultImage;
	
	bilateralFilter(fp_Params.vM_Image[0],
		MResultImage,
		fp_Params.i_Param[0], 
		fp_Params.i_Param[1],
		fp_Params.i_Param[2],
		BORDER_DEFAULT);
	
	fp_Params.vM_Image[1] = MResultImage;
}
void Filter::Blur(FilterParams &fp_Params)
{
	Mat MResultImage, MGray_Image;
	
	if (1 < fp_Params.vM_Image[0].channels())
		cvtColor(fp_Params.vM_Image[0], MGray_Image, COLOR_BGR2GRAY);
	else 
		fp_Params.vM_Image[0].copyTo(MGray_Image);
	
	blur(MGray_Image, 
		MResultImage, 
		Size(fp_Params.i_Param[0], 
			fp_Params.i_Param[0]));
	
	fp_Params.vM_Image[1] = MResultImage;
}
Mat Filter::SetText(Mat MImage, Point pLocation, 
	QString qstrText, Scalar sColor, double sScale)
{
	cv::putText(MImage,
		qstrText.toStdString(),
		pLocation,
		cv::FONT_HERSHEY_DUPLEX,
		sScale,
		sColor, //CV_RGB(255, 0, 0),
		2);
	return MImage;
}
string Filter::SendCommand(string strCmd)
{
	string strData;
	FILE * fStream;
	const int iMax_buffer = 256;
	char cBuffer[iMax_buffer];
	strCmd.append(" 2>&1");

	fStream = popen(strCmd.c_str(), "r");

	if (fStream) {
		while (!feof(fStream))
			if (fgets(cBuffer, iMax_buffer, fStream) != NULL) strData.append(cBuffer);
		pclose(fStream);
	}
	return strData;
}

/**************************** LICENCE *******************************/

bool Filter::chkStatus()
{
	fstream fReader;
	string strSerial = "";
	uint64_t uiKey = 0;
	string strHigh;
	string strLow;
	string str_1, str_2, str_3, strNewKey;
	string strFirst;
	string strNumber;
	string strSecond;
	uint32_t uiFirst;
	uint64_t uiSecond;
	uint32_t uiNumber;
	uint64_t ulResult;
	uint64_t ui32Serial;
	
	FileStorage fs_OpenStream("/home/pi/.config/.lic.xml", FileStorage::READ);
	if (fs_OpenStream.isOpened())
	{
		FileNode fnData = fs_OpenStream["Licence"];
		string strTMP = "";
		fnData["x"] >> str_1;
		fnData["y"] >> str_2;
		fnData["z"] >> str_3;
		fs_OpenStream.release();
	}
	
	fReader.open("/sys/firmware/devicetree/base/serial-number", ios::in);
	if (fReader.is_open()) {
		getline(fReader, strSerial);
		fReader.close();
	}
	
	strHigh = strSerial.substr(8, 4);
	strLow = strSerial.substr(12, 4);
	
	uiKey = stoll(strHigh, 0, 16) << 16;
	uiKey += stoll(strLow, 0, 16);
	
	int i = 0;
	while (i < 8)
	{
		strNewKey.push_back(str_1[i]);
		strNewKey.push_back(str_2[i]);
		strNewKey.push_back(str_3[i++]);
	}
	
	strFirst = strNewKey.substr(0, 8);
	strNumber = strNewKey.substr(8, 8);
	strSecond = strNewKey.substr(16, 8);
	
	uiFirst = stoll(strFirst, 0, 16);
	uiSecond = stoll(strSecond, 0, 16);
	uiNumber = stoll(strNumber, 0, 16);
	
	ulResult = uiFirst;
	ulResult += uiSecond << 32;
	
	ui32Serial = ulResult / uiNumber;
	
	if (ui32Serial == uiKey)
		return true;
	else
		return false;
}


/********************************************************************/