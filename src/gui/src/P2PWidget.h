#ifndef _ECAT_GUI_P2PWidget_H_
#define _ECAT_GUI_P2PWidget_H_
#include "ui_p2p.h"
#include "network/device.h"
using namespace ECAT::Network;

extern "C" QMap<QString, QString> formPoint;

class P2PWidget :public QWidget{
	Q_OBJECT
public:
	P2PWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~P2PWidget();
	void updateAxisIndex(DEVICE_AXIS axis, int index);
	void updateAxisOffset(DEVICE_AXIS axis, int offset);
private:
	void initUi();
	void initSignals();
	void initVariables();
	void updatePoints();
	void sleep(unsigned int msec);
	bool readXMLFile(QString filename);
	void onReadConfig();
	bool readXml(QString fileName);
signals:
	void ReachSpecifiedLocation();
private slots:
	void onMotionConnected();
	void onStartBtnToggled(bool);
	void onUpdateTimerOut();
private:
	Ui::p2pWidget* ui;
	shared_ptr<QVector<int>> axesIndex;
	shared_ptr<QVector<int>> axesOffset;
	shared_ptr<PanTilt> panTiltPtr;
	shared_ptr<QMap<QString, QString>>  formPointptr;
	QTimer* updateTimer;

	QMap<DEVICE_AXIS, vector<int>> point_1;
	QMap<DEVICE_AXIS, vector<int>> point_2;
	QMap<DEVICE_AXIS, vector<int>> point_3;
	QMap<DEVICE_AXIS, vector<int>> point_4;
	QMap<DEVICE_AXIS, vector<int>> point_5;

	QVector<qint32> counterpoints_x;
	QVector<qint32> counterpoints_z;
};
#endif