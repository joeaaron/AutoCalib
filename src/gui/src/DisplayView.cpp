#include "DisplayView.h"
DisplayView::DisplayView(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:QGraphicsView(parent)
{
	scene = new QGraphicsScene(this);
	scene->setBackgroundBrush(QColor(127, 127, 127));
	setScene(scene);
}
DisplayView::~DisplayView(){}

void DisplayView::showImage(QImage image){
	//QPixmap pixmap = QPixmap::fromImage(image.mirrored(true,true));
	QPixmap pixmap = QPixmap::fromImage(image);
	scene->clear();
	scene->addPixmap(pixmap);
	scene->setSceneRect(pixmap.rect());
	setScene(scene);
}