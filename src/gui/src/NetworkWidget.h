#ifndef _ECAT_GUI_NetworkWidget_H_
#define _ECAT_GUI_NetworkWidget_H_
#include "config.h"
#include "ui_networkpage.h"
#ifdef NETWORK_EXPORT
#include "network_import.h"
#else
#include "network/network.h"
#include "network/motioncontroller.h"
#endif
using namespace ECAT::Network;
class NetworkWidget :public QWidget{
	Q_OBJECT
public:
	NetworkWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~NetworkWidget();
private:
	void initUi();
	void initSignals();
	void initVariables();
signals:
	void motionConnected();
	void motionDisconnected();
	void autodirect();
	void directSuccessful();
	void autocamconnect();
	void autocamopen();
	void autocontrolconnect();
private slots :
	// vision
	void onConnectBtnClicked();
	void onUSBRedirectClicked();
	void onHandleConnection();
	void onHandleDisconnection();
	void onOpencamBtnClicked();
	void onHandleSocketError(QString);
	void onCamOpened(bool);
	void onCamClosed(bool);
	void onHeartBeatTimeOut();
	void onWifiChecked();
	// motion
	void onMotionConnectBtn();
private:
	Ui::networkPage *ui;
	QTimer *heartBeatTimer;
	bool isMotionConnect;
};
#endif