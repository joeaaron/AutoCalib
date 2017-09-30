#ifndef _ECAT_GUI_SensorWidget_H_
#define _ECAT_GUI_SensorWidget_H_
#include "ui_sensor.h"
class SensorWidget :public QWidget{
	Q_OBJECT
public:
	SensorWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~SensorWidget();
private:
	void initUi();
	void initSignals();
signals:
	void captureModeChanged(int);
	void updateCameraID(int);
private slots:
	void onNetworkConnected();
	void onNetworkDisconnected();
	void onCamOpened(bool);
	void onCamClosed(bool);
	void onCamComboActivated(int index);
	void onTriggerBtnClicked();
	void onDisplayBtnClicked();
	void onVideoCheckBoxStateChanged(int);
	void onGainSliderReleased();
	void onExposureSliderReleased();
	void onGainEditingFinished();
	void onExposureEditingFinished();
	void onGetValueBtnClicked();
	void onLaserBtnClicked();
	void onGainValue(quint32 val);
	void onExposureValue(quint32 val);
	void onRecvImage();
private:
	Ui::sensorWidget* ui;
	int cameraID;
};
#endif