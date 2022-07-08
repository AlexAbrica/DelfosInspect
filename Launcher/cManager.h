#pragma once

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include <QString>

using namespace std;

class Launcher
{
public:	
	Launcher();
	~Launcher();
	static int i_CheckPID(QString qstrNameProcess);
	QString qstr_Path;
	
	string str_GetPath() { return qstr_Path.toStdString(); }
	
private:
	
};