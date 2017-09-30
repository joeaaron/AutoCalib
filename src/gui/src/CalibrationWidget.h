#ifndef _ECAT_GUI_CalibrationWidget_H_
#define _ECAT_GUI_CalibrationWidget_H_
#include "ui_calibrationpage.h"
#include "PaneWidget.h"
#include "network/device.h"
#include "DisplayView.h"
using namespace ECAT::Network;
class CalibrationWidget :public QWidget{
	Q_OBJECT
public:
	CalibrationWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~CalibrationWidget();
private:
	void initUi();
	void initSignals();
	void initVariables();
	void saveXMLFile(QString filename);
	bool readXMLFile(QString filename);
	void updateComBox();
	void updateLineEdit();
	bool checkAxesMap();
signals:
	void motionConnected();
	void motionDisconnected();
	void requestImage();
	void recvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
private slots:
	void onComBoxIndexChanged(int index);
	void onLineEditTextChanged(const QString);
	void onSaveConfigBtnClicked();
	void onReadConfigBtnClicked();
	void onDisplayBtnClicked();
	void onCamComboActivated(int index);
	void onVideoCheckBoxStateChanged(int);
	void onVideoCheckBox2StateChanged(int);
	void onRecvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
	void onSaveBtnClicked();
public:
	QString savePath;
	qint32 saveImgNum;
	int cameraID;
private:
	Ui::calibrationPage* ui;
	QVector<PaneWidget*> panes;
	QWidget* bigPanInchingWidget;
	QWidget* bigPanP2PWidget;
	QWidget* smallPanInchingWidget;
	QWidget* smallPanP2PWidget;
	QWidget* xyzInchingWidget;
	QWidget* xyzP2PWidget;
	DisplayView* displayView;
	QImage imageToSave;
	bool bVideo;
	bool bSaving;
	QMap<DEVICE_AXIS, int> deviceAxesMap;
	QMap<DEVICE_AXIS, int> axisOffsetMap;
	uint16_t num;
	quint8 data[3];
};

#endif