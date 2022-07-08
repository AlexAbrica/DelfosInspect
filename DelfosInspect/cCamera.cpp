#include "cCamera.h"


cCameraPi::cCameraPi()
{

	
}


void cCameraPi::v_SetUp()
{
//	wiringPiSetup();
//	pinMode(LIGHT, OUTPUT);
//	pinMode(UP, OUTPUT);
//	pinMode(LEFT, OUTPUT);
//	pinMode(RIGHT, OUTPUT);
//	pinMode(DOWN, OUTPUT);
}

void cCameraPi::v_TakePicture(ImageProcessSettings DataImage)
{
	string strCommand = "raspistill -o " + DataImage.str_Path + 
		" -w " + DataImage.str_ResWidth + 
		" -h " + DataImage.str_ResHeight +  
		" -t 10 -ISO " + DataImage.str_ISO + 
		" -ss " + DataImage.str_SS;
	
	Mat MOriginalImage, MAddImages;
	int iFrames = DataImage.i_Loop; 
	;
	bool xFirstShot = false;

	v_TurnOnLights();
	do
	{
		system(strCommand.c_str());
		if (1 < DataImage.i_Loop)
		{
			MOriginalImage = imread(DataImage.str_Path);
			if (!xFirstShot)
			{
				xFirstShot = true;
				MAddImages = MOriginalImage;
			}
			add(MAddImages, MOriginalImage, MAddImages);
		}
		iFrames--;
	} while (iFrames > 0);
	
	if (1 < DataImage.i_Loop)
		imwrite(DataImage.str_Path, MAddImages);
	v_TurnOffLights();
}

void cCameraPi::v_TurnOffLights()
{
//	digitalWrite(LIGHT, LOW);
//	digitalWrite(UP, LOW);
//	digitalWrite(LEFT, LOW);
//	digitalWrite(RIGHT, LOW);
//	digitalWrite(DOWN, LOW);
}

void cCameraPi::v_TurnOnLights()
{
//	digitalWrite(LIGHT, HIGH);
//	digitalWrite(UP, HIGH);
//	digitalWrite(LEFT, HIGH);
//	digitalWrite(RIGHT, HIGH);
//	digitalWrite(DOWN, HIGH);
}

Mat cCameraPi::M_LoadPicture(string imgPath)
{
	return imread(imgPath);
}

Mat cCameraPi::M_CutImage(Mat MOriginalPicture, ImageProcessSettings DataImage)
{
	if ((DataImage.i_OffsetX + DataImage.i_Width) > MOriginalPicture.cols)
		DataImage.i_Width = MOriginalPicture.cols - DataImage.i_OffsetX;
	
	if ((DataImage.i_OffsetY + DataImage.i_Height) > MOriginalPicture.rows)
		DataImage.i_Height = MOriginalPicture.rows - DataImage.i_OffsetY;
	
	return (MOriginalPicture)(Rect(DataImage.i_OffsetX, DataImage.i_OffsetY, DataImage.i_Width, DataImage.i_Height)); 
}