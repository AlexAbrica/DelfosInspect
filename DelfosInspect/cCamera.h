#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp> 
#include "wiringPi.h"
#include "cInspectMachine.h"

#define _UP					7
#define _LEFT				0
#define _RIGHT				2
#define _DOWN				3 /*23*/
#define _LIGHT				24
#define HIGH				1
#define LOW					0
#define OUTPUT				1
#define INPUT				0


using namespace std;
using namespace cv;

class cCameraPi
{
	cCameraPi();
	~cCameraPi();
	
public:
	//static void v_TakePicture(string imgPath, int iLoop, string strWidth, string strHeight, string strISO, string strSS);
	static void v_TakePicture(ImageProcessSettings DataImage);
	static Mat M_LoadPicture(string imgPath);
	static void v_SetUp();
	static Mat M_CutImage(Mat MOriginalPicture, ImageProcessSettings DataImage);
	
private:
	static void v_TurnOffLights();
	static void v_TurnOnLights();
	
};