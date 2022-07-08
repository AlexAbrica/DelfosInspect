#pragma once

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_Login
{	
public:
	
	QPushButton *cmdLogin;
	QPushButton *cmdExit;
	QLabel *lblLogo;
	QLineEdit *txtPassword;
	QLineEdit *txtUser;
	
	void setupUi(QDialog *Dialog)
	{
		
		QFont font;
		font.setPointSize(20);
		
		if (Dialog->objectName().isEmpty())
			Dialog->setObjectName(QStringLiteral("Login"));
		Dialog->setWindowTitle("Welcome to Delfos Inspect");
		Dialog->resize(480, 340);
		Dialog->setMinimumSize(QSize(480, 340));
		Dialog->setMaximumSize(QSize(480, 340));
		Dialog->setFont(font);
		
		/*********************************** Lineas de texto ***********************************************/
		
		txtUser = new QLineEdit(Dialog);
		txtUser->setObjectName(QStringLiteral("User"));
		txtUser->setGeometry(QRect(140, 20, 200, 40));
		//txtUser->setFont(font);
		
		txtPassword = new QLineEdit(Dialog);
		txtPassword->setObjectName(QStringLiteral("Password"));
		txtPassword->setGeometry(QRect(140, 65, 200, 40));
		txtPassword->setEchoMode(QLineEdit::Password);
		//txtPassword->setFont(font);
		
		/*********************************** Botones de comando ***********************************************/
		
		cmdLogin = new QPushButton(Dialog);
		cmdLogin->setObjectName(QStringLiteral("Login"));
		cmdLogin->setText(QStringLiteral("Login"));
		cmdLogin->setGeometry(QRect(140, 110, 95, 40));
		
		cmdExit = new QPushButton(Dialog);
		cmdExit->setObjectName(QStringLiteral("Exit"));
		cmdExit->setText(QStringLiteral("Exit"));
		cmdExit->setGeometry(QRect(245, 110, 95, 40));
		
		/*********************************** Imagen de logo ***********************************************/
		
		lblLogo = new QLabel(Dialog);
		lblLogo->setObjectName(QStringLiteral("logo"));
		lblLogo->setGeometry(QRect(165, 170, 150, 150));
		QPixmap pixmap("/home/pi/logo.png");
		lblLogo->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio));
		
		QObject::connect(cmdLogin, SIGNAL(clicked()), Dialog, SLOT(Login()));
		QObject::connect(cmdExit, SIGNAL(clicked()), Dialog, SLOT(Exit()));

		QMetaObject::connectSlotsByName(Dialog);
		
	}
	
};

namespace Ui {
	class LogIn : public Ui_Login {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H