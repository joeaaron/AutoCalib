#include "LaserWidget.h"
#include "ui_laserwidget.h"
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include "network/network.h"
#include <vector>

using namespace ECAT::Network;

#define PI 3.1415926
const int DETECTACCURACY = 200;
const double STANDARD = 1500;

LaserWidget::LaserWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::LaserWidget),
laserCenterX(0),
laserCenterY(0),
laserRadius(0),
mouseClick(0)
{
	ui->setupUi(this);   //display the UI
	initUi();
	initVariables();
	initSignals();
}

LaserWidget::~LaserWidget()
{


	delete ui;
}

void LaserWidget::initUi()
{
	
}

void LaserWidget::initSignals()
{
	connect(ui->btnLaserDataClear, SIGNAL(clicked()), this, SLOT(onUpLaserClearBtnClicked()));
	connect(ui->btnLaserDataTest, SIGNAL(clicked()), this, SLOT(onUpLaserTestBtnClicked()));
	connect(ui->btnLaserDataGet, SIGNAL(clicked()), this, SLOT(onUpLaserGetBtnClicked()));
	connect(ui->btnLaserDataClear_2, SIGNAL(clicked()), this, SLOT(onBottomLaserClearBtnClicked()));
	connect(ui->btnLaserDataTest_2, SIGNAL(clicked()), this, SLOT(onBottomLaserTestBtnClicked()));
	connect(ui->btnLaserDataGet_2, SIGNAL(clicked()), this, SLOT(onBottomLaserGetBtnClicked()));
	connect(ui->btnclearlist, SIGNAL(clicked()), this, SLOT(onListClearBtnClicked()));
	connect(ui->btnRefEnsure, SIGNAL(clicked()), this, SLOT(onRefEnsureBtnClicked()));
	connect(ui->btnRefStop, SIGNAL(clicked()), this, SLOT(onRefStopBtnClicked()));
	connect(ui->laserOneBtn, SIGNAL(clicked()), this, SLOT(onLaserOneBtnClicked()));
	connect(ui->laserTwoBtn, SIGNAL(clicked()), this, SLOT(onLaserTwoBtnClicked()));
	connect(ui->laserThreeBtn, SIGNAL(clicked()), this, SLOT(onLaserThreeBtnClicked()));
	connect(ui->laserFourBtn, SIGNAL(clicked()), this, SLOT(onLaserFourBtnClicked()));
	connect(ui->laserCrossOneBtn, SIGNAL(clicked()), this, SLOT(onLaserCrossOneBtnClicked()));
	connect(ui->laserCrossTwoBtn, SIGNAL(clicked()), this, SLOT(onLaserCrossTwoBtnClicked()));
	connect(&CMDParser::getInstance(), SIGNAL(updateLaserData(short*, int)),
		this, SLOT(onUpdateLaserData(short*, int)));
	connect(dataRefresh, SIGNAL(timeout()), this, SLOT(onDataRefreshCallBack()));
	
}

void LaserWidget::initVariables()
{
	mLaserCnt = 0;
	dataRefresh = new QTimer(this);

	onUpdateLaserData(NULL, 0);
}

void LaserWidget::onUpLaserClearBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().topLaserClear();
}

void LaserWidget::onUpLaserTestBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().topLaserTest();
	//resultsQualityJudge(1580);
}

void LaserWidget::onUpLaserGetBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().topLaserGet();
}

void LaserWidget::onBottomLaserClearBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().bottomLaserClear();
}

void LaserWidget::onBottomLaserTestBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().bottomLaserTest();
}

void LaserWidget::onBottomLaserGetBtnClicked()
{
	dataRefresh->stop();
	CMDParser::getInstance().bottomLaserGet();
}

void LaserWidget::onListClearBtnClicked()
{
	ui->position_listWidget->clear();
}

void LaserWidget::onRefEnsureBtnClicked()
{
	QString cycle_str;
	int cycle;
	bool ok;
	cycle_str = ui->leDataRefCycle->text();
	cycle = cycle_str.toInt(&ok, 10);
	if (cycle > 10 && cycle < 100000)
	{
		dataRefresh->start(cycle);
	}
}

void LaserWidget::onRefStopBtnClicked()
{
	dataRefresh->stop();
}

void LaserWidget::onLaserOneBtnClicked()
{
	
	
	if(resultsQualityJudge(distAvrFirst))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("1st laser is calibrated right.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("1st laser is calibrated wrong.")
		);
}

void LaserWidget::onLaserTwoBtnClicked()
{

	if (resultsQualityJudge(distAvrSecond))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("2nd laser is calibrated right.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("2nd laser is calibrated wrong.")
		);
}

void LaserWidget::onLaserThreeBtnClicked()
{
	if (resultsQualityJudge(distAvrThird))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("3rd laser is calibrated right.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("3rd laser is calibrated wrong.")
		);
}

void LaserWidget::onLaserFourBtnClicked()
{
	
	if (resultsQualityJudge(distAvrFourth))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("4th laser is calibrated right.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("4th laser is calibrated wrong.")
		);
}

void LaserWidget::onLaserCrossOneBtnClicked()
{
	double distGap = distFirstLeft - distSecondRight;
	if (resultsCrossQuality(distGap))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("1st&2nd laser gap is resonable.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("1st&2nd laser gap is ridiculous!")
		);
}

void LaserWidget::onLaserCrossTwoBtnClicked()
{
	double distGap = distFirstRight - distFourthLeft;
	if (resultsCrossQuality(distGap))
		QMessageBox::information(this,
		tr("Quality OK"),
		QString("1st&4th laser gap is resonable.")
		);
	else
		QMessageBox::critical(this,
		tr("Quality Bad"),
		QString("1st&4th laser gap is ridiculous!")
		);
}
/******************************
* 函数名：onDataRefreshCallBack
* 函数功能：dataRefesh的周期回调函数
* 输入：无
* ***************************/
void LaserWidget::onDataRefreshCallBack()
{
	if (ui->checkBox->isChecked() && ui->checkBox_2->isChecked())
	{
		//pMainTabWidget->pMainWindow->writeCmd(9, "AC+DATA");
		CMDParser::getInstance().allLaserGet();
	}
	else if (ui->checkBox->isChecked())
	{
		//pMainTabWidget->pMainWindow->writeCmd(7, "AC+DATA");
		CMDParser::getInstance().topLaserGet();
	}
	else if (ui->checkBox_2->isChecked())
	{
		//pMainTabWidget->pMainWindow->writeCmd(8, "AC+DATA");
		CMDParser::getInstance().bottomLaserGet();
	}
	else
	{
		ui->position_listWidget->insertItem(0, "none");
	}
}

void LaserWidget::addCoordinate(QPaintDevice *pPaintDev, QRect rect)
{
	int rectX = rect.x();
	int rectY = rect.y();
	int width = rect.width();
	int height = rect.height();

	laserCenterX = rectX + width/ 3;
	laserCenterY = rectY + height/ 3;
	int radiusCnt = 4;
	int radiusX = width / 3;
	int radiusY = height / 3;
	laserRadius = radiusX < radiusY ? radiusX : radiusY;

	QPainter painter(pPaintDev);
	QPen pen = painter.pen();
	pen.setWidth(1);
	pen.setColor(QColor(255, 255, 255, 100));
	pen.setStyle(Qt::DashDotDotLine);
	painter.setPen(pen);

	for (int i = 0; i < radiusCnt; i++)
	{
		if (i == (radiusCnt - 1))
		{
			pen.setStyle(Qt::SolidLine);
			painter.setPen(pen);
		}
		painter.drawEllipse(QPoint(laserCenterX, laserCenterY),
			laserRadius / radiusCnt*(i + 1), laserRadius / radiusCnt*(i + 1));
		painter.drawText(laserCenterX + (laserRadius / radiusCnt*(i + 1) + 5)*cos(PI / 4),
			laserCenterY - (laserRadius / radiusCnt*(i + 1) + 5)*sin(PI / 4),
			QString::number(i + 1) + "000mm");
	}
	pen.setStyle(Qt::DashDotDotLine);
	painter.setPen(pen);
	for (double i = 0; i < 360; i += 15)
	{
		painter.drawLine(laserCenterX, laserCenterY,
			laserCenterX + (laserRadius - 30)*cos(2 * PI*i / 360),
			laserCenterY - (laserRadius - 30)*sin(2 * PI*i / 360));
		painter.drawText(laserCenterX - 5 + (laserRadius - 15)*cos(2 * PI*i / 360),
			laserCenterY - (laserRadius - 15)*sin(2 * PI*i / 360),
			QString::number(i));
	}
}

void LaserWidget::setLaserData(QPaintDevice *pPaintDev, QRect rect, short buf[], int cnt)
{
	QPainter painter(pPaintDev);
	QPen pen = painter.pen();
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	if (buf == NULL) return;
	QPolygon polygon;
	polygon.clear();
	//qDebug() << "mLaserCnt:" << mLaserCnt;
	std::vector<double> distFirst;
	std::vector<double> distSecond;
	std::vector<double> distThird;
	std::vector<double> distFourth;

	double x, y, dist;
	for (int i = 0; i < cnt; i++)
	{
		x = laserCenterX + (float)laserRadius*buf[2 * i + 1] / 4000;
		y = laserCenterY - (float)laserRadius*buf[2 * i] / 4000;
		dist = sqrt(x * x + y * y);
		
		if ((i % 640) == 0)
		{
			if (i / 640 < 4)
			{
				switch (i / 640)
				{
				case 0:
					pen.setColor(Qt::red);           //no.4 cam
					painter.setPen(pen);
					
					break;
				case 1:
					pen.setColor(Qt::yellow);        //no.1 cam
					painter.setPen(pen);
					//distFirst.push_back(dist);
					break;
				case 2:
					pen.setColor(Qt::green);         //no.2 cam
					painter.setPen(pen);
					//distSecond.push_back(dist);
					break;
				default:
					pen.setColor(Qt::blue);          //no.3 cam
					painter.setPen(pen);
					//distThird.push_back(dist);
					break;
				}
			}
			else
			{
				switch (i / 640)
				{
				case 4:
					pen.setColor(Qt::red);
					painter.setPen(pen);
					break;
				case 5:
					pen.setColor(Qt::yellow);
					painter.setPen(pen);
					break;
				case 6:
					pen.setColor(Qt::green);
					painter.setPen(pen);
					break;
				default:
					pen.setColor(Qt::blue);
					painter.setPen(pen);
				}
			}
		}
		if ((buf[2 * i] > 4000) || (buf[2 * i] < -4000)) continue;
		if ((buf[2 * i + 1] > 4000) || (buf[2 * i + 1] < -4000)) continue;
	//	qDebug()<<"mLaserData:"<<i<<","<< (buf[2 * i])<<","<<(buf[2 * i + 1]);
		painter.drawEllipse(laserCenterX + (float)laserRadius*buf[2 * i+1] / 4000,
			laserCenterY - (float)laserRadius*buf[2 * i ] / 4000, 2, 2);
	//	qDebug() << "x:" << laserCenterX + (float)laserRadius*buf[2 * i + 1] / 4000 << ","
		//	"y:" << laserCenterY - (float)laserRadius*buf[2 * i] / 4000;

		if (i < 640)
			distFourth.push_back(dist);
		else if (i >= 640 && i < 1280)
			distFirst.push_back(dist);
		else if (i >= 1280 && i < 1920)
			distSecond.push_back(dist);
		else if (i >= 1920 && i < 2560)
			distThird.push_back(dist);

	}

	///First laser avr dist
	calcAverDist(distFirst, distAvrFirst, distFirstLeft, distFirstRight);
	//qDebug() << "distAvrFirst:" << distAvrFirst;
	///Second laser avr dist
	calcAverDist(distSecond, distAvrSecond, distSecondLeft, distSecondRight);

	///Fourth laser avr dist
	calcAverDist(distFourth, distAvrFourth, distFourthLeft, distFourthRight);

	distFirst.clear();
	distSecond.clear();
	distFourth.clear();
}

bool LaserWidget::resultsQualityJudge(double distAvr)
{
	QIcon icon;
	if (abs(distAvr - 1500) > DETECTACCURACY)
	{
		icon.addFile(QStringLiteral(":/images/images/cry.png"), QSize(), QIcon::Normal, QIcon::On);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		return false;
	}
		
	else
	{
		icon.addFile(QStringLiteral(":/images/images/smile.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		return true;
	}
		
}

bool LaserWidget::resultsCrossQuality(double distSide)
{
	QIcon icon;
	if (abs(distSide) > DETECTACCURACY)
	{
		icon.addFile(QStringLiteral(":/images/images/cry.png"), QSize(), QIcon::Normal, QIcon::On);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		return false;
	}

	else
	{
		icon.addFile(QStringLiteral(":/images/images/smile.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		return true;
	}

}

///calc average distance
void LaserWidget::calcAverDist(std::vector<double> dist, double& distAvr, double& distLeft, double& distRight)
{
	
	double distAll = 0.0; 
	distLeft = 0.0;
	distRight = 0.0;
	//qDebug() << dist.size();
	for (int m = 0; m < dist.size(); ++m)
	{
		distAll += dist[m];
		//qDebug() << "distFirst:" << dist[m];
		if (m < 30)
			distLeft += dist[m];
		
		if (m > dist.size() - 30 - 1)
			distRight += dist[m];
	}

	distAvr = distAll / 640;
	distLeft = distLeft / 30;
	distRight = distRight / 30;

}

void LaserWidget::paintEvent(QPaintEvent *)
{
	//QSize widgetSize = this->size();
	//int width = widgetSize.width();
	//int heigth = widgetSize.height();
	int width = 700;
	int heigth = 700;
	//qDebug() << width << "," << heigth;
	int minLength = width < heigth ? width : heigth;
	if (minLength < 500) return;
	QRect laserRect;
	laserRect = QRect(600, 20, minLength - 10, minLength - 10);
	addCoordinate(this, laserRect);
	setLaserData(this, laserRect, mLaserData, mLaserCnt);
}

/*******************************
* 函数名称：mousePressEvent
* 功能：鼠标点击窗口事件重载
* 输入：
*******************************/
void LaserWidget::mousePressEvent(QMouseEvent *e)
{
	if (mouseClick > 10000)
		mouseClick = 0;

	QString temp;
	QPoint global_pos = cursor().pos();                   //全局坐标
	QPoint current_pos = e->pos();                        //窗口坐标
	if (((current_pos.x() - laserCenterX)*(current_pos.x() - laserCenterX)
		+ (current_pos.y() - laserCenterY)*(current_pos.y() - laserCenterY))
		<= laserRadius*laserRadius)
	{
		temp = QString("(" + QString::number(int((current_pos.x() - laserCenterX)*4000.0 / laserRadius)) + ","
			+ QString::number(int((laserCenterY - current_pos.y())*4000.0 / laserRadius)) + ")");
		ui->position_listWidget->insertItem(mouseClick, temp);
		mouseClick++;
	}
}

void LaserWidget::onUpdateLaserData(short buf[], int cnt)
{
	if (buf)
	{
		//qDebug() << "cnt:" << cnt << ", mLaserCnt:" << mLaserCnt;
		mLaserCnt = cnt * 4 < (int)sizeof(mLaserData) ? cnt * 4 : (int)sizeof(mLaserData);
		mLaserCnt /= 4;
		memcpy(mLaserData, buf, 4 * mLaserCnt);
		update();
	}
	else
		mLaserCnt = 0;
}