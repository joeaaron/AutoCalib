#ifndef _ECAT_GUI_LaserDetectWidget_H_
#define _ECAT_GUI_LaserDetectWidget_H_

#include <QWidget>

namespace Ui
{
	class LaserWidget;
}
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
	bool resultsQualityJudge(double);
	bool resultsCrossQuality(double);
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
	void onUpdateLaserData(short*, int);
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
};

#endif