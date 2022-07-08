#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	QString qstrPath = QString(*argv);
	MainWindow w(qstrPath);
    w.show();
    
    return a.exec();
}
