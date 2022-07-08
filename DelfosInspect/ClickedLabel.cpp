#include "ClickedLabel.h"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
	: QLabel(parent) {
    
}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
	emit clicked();
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
	emit released();
}

void ClickableLabel::mouseMoveEvent(QMouseEvent* event) {
	emit positionChanged();
}

/*
 *
 *
 
 ClickableLabel *picImageShow;
 picImageShow = new ClickableLabel(centralWidget);
 QObject::connect(picImageShow, SIGNAL(clicked()), MainWindow, SLOT(StartPosition()));
 QObject::connect(picImageShow, SIGNAL(released()), MainWindow, SLOT(Position()));
 QObject::connect(picImageShow, SIGNAL(positionChanged()), MainWindow, SLOT(Draw()));
 
 *
 *
 *
 *
 **/