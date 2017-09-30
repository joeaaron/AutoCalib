#ifndef _ECAT_GUI_DisplayView_H_
#define _ECAT_GUI_DisplayView_H_
#include <QGraphicsView>
#include <QGraphicsScene>
class DisplayView :public QGraphicsView{
	Q_OBJECT
public:
	DisplayView(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~DisplayView();
	void showImage(QImage);
private:
	QGraphicsScene *scene;
};
#endif