#ifndef _ECAT_GUI_SetupWidget_H_
#define _ECAT_GUI_SetupWidget_H_
#include "ui_setuppage.h"
#include "PaneWidget.h"
#include "DisplayView.h"
class SetupWidget :public QWidget{
	Q_OBJECT
public:
	SetupWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~SetupWidget();
private:
	void initUi();
	void initSignals();
signals:
	void motionConnected();
	void motionDisconnected();
private slots:
	void onRecvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
	void onCaptureModeChanged(int state);
	void onUpdateCameraID(int cameraID);
	void onCameraTriggered(bool flag);
private:
	Ui::setupPage *ui;
	QVector<PaneWidget*> panes;
	QWidget* sensorWidget;
	QWidget* motionWidget;
	DisplayView* displayView;
	bool bVideo;
	int cameraID;
};
#endif