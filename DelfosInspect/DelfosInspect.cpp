#include "MainWindow.h"
#include <iostream>
#include <QStringList>
#include <QApplication>

int main(int argc, char *argv[])
{
	QStringList qstrlstData;
	QApplication a(argc, argv);
	if (0 < argc)
	{
		while (NULL != *argv)
		{
			qstrlstData.push_back(QString(*argv));
			argv++;
		}
	}	
	if (2 == qstrlstData.size() && !qstrlstData[1].isNull())
	{
		QStringList qstrlstUser = qstrlstData[1].split(":");
		QStringList qstrUser;
		if (3 == qstrlstUser.size()) {
			cout << "User loging: " << qstrlstUser[1].toStdString() << endl;
			qstrUser.push_back(qstrlstUser[1]);
			qstrUser.push_back(qstrlstUser[2]);
		}
		else {
			cout << "Unknown parameter." << endl;
			qstrUser[0] = "";
			qstrUser[1] = "";
		}
		if ("" != qstrUser[0])
		{
			MainWindow w(qstrUser);
			w.show();
			
			cout << "Starting form: " << qstrlstData[0].toStdString() << endl;
			cout << "Delfos Inspection Software by Mexus" << endl;
			cout << "Starting..." << endl;
		
			return a.exec();
		}
	}
	else
		cout << "Delfos Inspection need a parameter to start." << endl;
	
	
	
	
//    QApplication a(argc, argv);
//	MainWindow WindowInspect;
//	WindowInspect.show();
//	a.exec();
//	printf("Delfos Inspection Software by Mexus\nStarting...\n\n");
	
	return 0;
}
