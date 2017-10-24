#ifndef AUTOCALIBWIDGET_H
#define AUTOCALIBWIDGET_H

#include <QMainWindow>
#include "network/device.h"
#include "network/motioncontroller.h"
#include "DisplayView.h"
#include <thread>
#include "network/motioncontroller.h"
#include <opencv2/opencv.hpp>

using namespace ECAT::Network;

namespace Ui {
class AutoCalibWidget;
}

class AutoCalibWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutoCalibWidget(QWidget *parent = 0);
    ~AutoCalibWidget();
private:
	void initUi();
	void InitSignals();
	void initVariables();
public:
	void saveXMLFile(QString filename);
	bool readXMLFile(QString filename);
	void updatePoints();
	void suitcaseMotion(qint32 i);
	void sleep(unsigned int);
	void onLightSwitch(bool light);
	void onSmallBoardMotion();
	void onSmallBoardMotionPro();
	void onLargeBoardMotion();

	void saveImg(qint32 cameraNum);
	void printLog(qint32 cameraLaserID, QString& log);
	cv::Mat QImage2cvMat(QImage image);
	void triggerPush(QString, qint32);
	void pushFiles();
	void cowaCalib();
	void onTopFallLaserCalib();
signals:
	void Connect();
	void Import();
	void recvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
	void ReachLocation(qint32 camera, bool lase);
	void progValue(double value);
	void openBottomLaser();
	void openTopLaser();
	void openTopFallLaser();
	void getDeviation();
private slots:
	void onConnectClicked();
	void onImportBtnClicked();
	void onSaveBtnClicked();
	void onTestBtnClicked();
	void onUpdateTimerOut();
	void onRecvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
	void onStartBtnToggled(bool);
	void onStopBtnClicked();
	void onCalibBtnClicked();
	void onGetDeviation();
	void onDisplayBtnClicked();
	//void onCamComboActivated(int index);
	void onMotionStart();
	void onImgTook(qint32 camera, bool laser);
	void onValueChanged(double progress);
	void onOpenBottomLaser();
	void onOpenTopLaser();
	void onOpenTopFallLaser();
	void onBoardCheckBoxStateChanged(int);
	void onFallCheckBoxStateChanged(int);
private:
    Ui::AutoCalibWidget *ui;
	QMap<DEVICE_AXIS, int> deviceAxesMap;
	QMap<DEVICE_AXIS, int> axisOffsetMap;

	shared_ptr<PanTilt> smallPanTiltPtr;
	shared_ptr<XYZ> xyzPtr;
	shared_ptr<PanTilt> bigPanTiltPtr;
	shared_ptr<QVector<int>> smallaxesIndex;	/*[0]:axisX,[1]:axisZ*/
	shared_ptr<QVector<int>> bigaxesIndex;		/*[0]:axisX,[1]:axisZ*/
	shared_ptr<QVector<int>> xyzaxesIndex;	
	shared_ptr<QVector<int>> axesOffset;

	QStringList xyz_xList;
	QStringList smallpan_xList, bigpan_xList;
	QStringList xyz_yList;
	QStringList xyz_zList;
	QStringList smallpan_zList, bigpan_zList;
	QStringList xyz_rList;

	QVector<qint32> xyz_xPoint_1, xyz_xPoint_2;
	QVector<qint32> xyz_yPoint_1, xyz_yPoint_2;
	QVector<qint32> xyz_zPoint_1, xyz_zPoint_2;
	QVector<qint32> xyz_rPoint_1, xyz_rPoint_2;

	QVector<qint32> smallpan_xPoint, smallpan_zPoint;
	QVector<qint32> bigpan_xPoint, bigpan_zPoint;
	vector<double> para;
	QTimer* updateTimer;

	DisplayView* displayView;
	qint32 cameraID, num;
	qint32 endPoint;
	QImage imageToSave;
	QString savePath, saveDir;

	bool smallPanSaveFinish;
	bool bigPanSaveFinish;
	bool recvFinish;
	bool bGetDeviation;
	bool isSmallBoardCalibrated;
	bool isFallLaserCalibrate;

	double calibValue;
	double lrOffset, udOffset, areaOffset;
	double imgArea;

	std::thread motionThread;
	std::thread calibThread;
	bool isSaveImage;
	QMutex pause;
public:
	cv::Mat srcImg;
	bool bFind;
};

#endif // AUTOCALIBWIDGET_H
