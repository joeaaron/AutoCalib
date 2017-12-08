#ifndef _ECAT_GUI_LaserDetectWidget_H_
#define _ECAT_GUI_LaserDetectWidget_H_

#include <QWidget>
#include "network/device.h"
#include <thread>
namespace Ui
{
	class LaserWidget;
}
using namespace ECAT::Network;

class LaserWidget :public QWidget
{
	Q_OBJECT
public:
	explicit LaserWidget(QWidget *parent = 0);
	~LaserWidget();
private:
	void initUi();
	void initSignals();
	void initVariables();
protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *e);
public:
	void addCoordinate(QPaintDevice *pPaintDev, QRect rect);
	void setLaserData(QPaintDevice *pPaintDev, QRect rect, short buf[], int cnt);
	void calcAverDist(std::vector<double>, double &, double&, double&);
	bool resultsQualityJudge(double, bool);
	bool resultsCrossQuality(double,bool);
	void autoMotion();
	void suitcaseMotion(qint32 i);
	void sleep(unsigned int);
	bool readXMLFile(QString fileName);
	void updatePoints();
	void clearVectorArray();
signals:
	void reachLocation(qint32);
private slots:
	void onUpLaserClearBtnClicked();
	void onUpLaserTestBtnClicked();
	void onUpLaserGetBtnClicked();
	void onBottomLaserClearBtnClicked();
	void onBottomLaserTestBtnClicked();
	void onBottomLaserGetBtnClicked();
	void onDataRefreshCallBack();
	void onListClearBtnClicked();
	void onRefEnsureBtnClicked();
	void onRefStopBtnClicked();
	void onLaserOneBtnClicked();
	void onLaserTwoBtnClicked();
	void onLaserThreeBtnClicked();
	void onLaserFourBtnClicked();
	void onLaserCrossOneBtnClicked();
	void onLaserCrossTwoBtnClicked();
	void onCalibDetectToggled(bool);
	void onUpdateLaserData(short*, int);
	void onCalibDetect(qint32);
	void onMotionConnected();
	void onMotionDisconnected();
private:
	Ui::LaserWidget *ui;
	short mLaserData[640 * 4 * 2];
	int mLaserCnt;
	int laserCenterX;
	int laserCenterY;
	int laserRadius;
	int mouseClick;

	double distAvrFirst, distFirstLeft, distFirstRight;
	double distAvrSecond, distSecondLeft, distSecondRight;;
	double distAvrThird;
	double distAvrFourth, distFourthLeft, distFourthRight;;

	QTimer *dataRefresh;
private:
	std::thread motionThread;
	shared_ptr<XYZ> xyzPtr;
	shared_ptr<QVector<int>> xyzIndex;
	QVector<qint32> xyz_xPoint;
	QVector<qint32> xyz_yPoint;
	QVector<qint32> xyz_zPoint;
	QVector<qint32> xyz_rPoint;

	QStringList xyz_xList;
	QStringList xyz_yList;
	QStringList xyz_zList;
	QStringList xyz_rList;
	QTimer* updateTimer;
	QVector<bool> results;
};

#endif