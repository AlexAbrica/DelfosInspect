#include <QLabel>
#include <QWidget>
#include <Qt>
QT_BEGIN_NAMESPACE
class ClickableLabel : public QLabel { 
	Q_OBJECT 

	public :
	    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~ClickableLabel();

signals:
	void clicked();
	void released();
	void positionChanged();

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
};

QT_END_NAMESPACE