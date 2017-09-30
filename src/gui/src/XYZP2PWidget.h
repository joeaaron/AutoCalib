#ifndef _ECAT_GUI_XYZP2PWidget_H_
#define _ECAT_GUI_XYZP2PWidget_H_
#include "ui_xyzp2p.h"
#include "network/device.h"
using namespace ECAT::Network;
class XYZP2PWidget :public QWidget{
	Q_OBJECT
public:
	XYZP2PWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~XYZP2PWidget();
	void updateAxisIndex(DEVICE_AXIS axis, int index);
	void updateAxisOffset(DEVICE_AXIS axis, int offset);
private:
	void initUi();
	void initSignals();
	void initVariables();

private slots:
	void onMotionConnected();
	void onStartBtnToggled(bool);
	void onUpdateTimerOut();
private:
	Ui::xyzP2PWidget* ui;
	shared_ptr<QVector<int>> axesIndex;
	shared_ptr<QVector<int>> axesOffset;
	shared_ptr<XYZ> xyzPtr;

	QTimer* updateTimer;
};
#endif