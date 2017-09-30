#ifndef _ECAT_GUI_InchingWidget_H_
#define _ECAT_GUI_InchingWidget_H_
#include "ui_inching.h"
#include "network/device.h"
#include <memory> 
using namespace ECAT::Network;
using namespace std;
class InchingWidget:public QWidget{
	Q_OBJECT
public:
	InchingWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~InchingWidget();
	void initUi();
	void initSignals();
	void initVariables();
	void updateAxisIndex(DEVICE_AXIS axis, int index);
	void updateAxisOffset(DEVICE_AXIS axis, int offset);
private slots:
	void onMotionConnected();
	void onStartBtnToggled(bool);
	void onDirectionBtnPressed();
	void onDirectionBtnReleased();
	void onUpdateTimerOut();
	void onUDCheckBoxStateChanged(int);
private:
	Ui::inchingWidget* ui;
	shared_ptr<QVector<int>> axesIndex;/*[0]:axisX,[1]:axisZ*/
	shared_ptr<QVector<int>> axesOffset;/*[0]:axisX,[1]:axisZ*/
	shared_ptr<PanTilt> panTiltPtr;
	QTimer* updateTimer;
	qint32 vel;
};
#endif