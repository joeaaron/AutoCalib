#ifndef _ECAT_GUI_XYZInchingWidget_H_
#define _ECAT_GUI_XYZInchingWidget_H_
#include "ui_xyzinching.h"
#include "network/device.h"
using namespace ECAT::Network;
class XYZInchingWidget :public QWidget{
	Q_OBJECT
public:
	XYZInchingWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~XYZInchingWidget();
	void updateAxisIndex(DEVICE_AXIS axis, int index);
private:
	void initUi();
	void initSignals();
	void initVariables();
private slots:
	void onMotionConnected();
	void onStartBtnToggled(bool);
	void onDirectionBtnPressed();
	void onDirectionBtnReleased();
	void onUpdateTimerOut();
private:
	Ui::xyzInchingWidget* ui;
	shared_ptr<QVector<int>> axesIndex;
	shared_ptr<XYZ> xyzPtr;
	QTimer* updateTimer;
};
#endif