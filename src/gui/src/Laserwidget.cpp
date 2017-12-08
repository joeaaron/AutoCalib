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
const int POINTNUM = 6;

LaserWidget::LaserWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::LaserWidget),
laserCenterX(0),
laserCenterY(0),
laserRadius(0),
mouseClick(0),
xyzIndex(new QVector<int>()),
xyzPtr(new XYZ()),
updateTimer(new QTimer(this))
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
	ui->resultBtn->hide();
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
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onCalibDetectToggled(bool)));
	connect(&CMDParser::getInstance(), SIGNAL(updateLaserData(short*, int)),
		this, SLOT(onUpdateLaserData(short*, int)));
	connect(dataRefresh, SIGNAL(timeout()), this, SLOT(onDataRefreshCallBack()));
	connect(this, SIGNAL(reachLocation(qint32)), this, SLOT(onCalibDetect(qint32)));
	
}

void LaserWidget::initVariables()
{
	mLaserCnt = 0;
	dataRefresh = new QTimer(this);

	xyzIndex->push_back(1);
	xyzIndex->push_back(0);
	xyzIndex->push_back(2);
	xyzIndex->push_back(3);
	xyzPtr->setAxes(xyzIndex);

	QString fileName = QDir::currentPath() + "/calibDetectPoint.xml";
	if (QFile::exists(fileName)){
		if (readXMLFile(fileName)){
			updatePoints();
		}
		else
			QMessageBox::warning(
			this,
			tr("Config Error"),
			QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	else{
		QMessageBox::warning(
			this,
			tr("Config Error"),
			QString("No xml file called \"axis-driver.xml\", \
					please select right xml file and save it as \"axis-driver.xml\" in current directory for convenience.")
					);
	}
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
	bool result = true;
	if (resultsQualityJudge(distAvrFirst, result))
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
	bool result = true;
	if (resultsQualityJudge(distAvrSecond, result))
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
	bool result = true;
	if (resultsQualityJudge(distAvrThird, result))
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
	bool result = true;
	if (resultsQualityJudge(distAvrFourth, result))
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
	bool result = true;
	if (resultsCrossQuality(distGap, result))
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
	bool result = true;
	if (resultsCrossQuality(distGap, result))
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

bool LaserWidget::resultsQualityJudge(double distAvr, bool result)
{
	QIcon icon;
	if (abs(distAvr - 1500) > DETECTACCURACY)
	{
		icon.addFile(QStringLiteral(":/images/images/cry.png"), QSize(), QIcon::Normal, QIcon::On);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		result = false;
	}
		
	else
	{
		icon.addFile(QStringLiteral(":/images/images/smile.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		result = true;
	}
		
	return result;
}

bool LaserWidget::resultsCrossQuality(double distSide, bool result)
{
	QIcon icon;
	if (abs(distSide) > DETECTACCURACY)
	{
		icon.addFile(QStringLiteral(":/images/images/cry.png"), QSize(), QIcon::Normal, QIcon::On);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		result = false;
	}

	else
	{
		icon.addFile(QStringLiteral(":/images/images/smile.png"), QSize(), QIcon::Normal, QIcon::Off);
		ui->resultBtn->setIcon(icon);
		ui->resultBtn->setIconSize(QSize(120, 120));
		result = true;
	}
	return result;
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

void LaserWidget::sleep(unsigned int msec)
{
	QTime reachTime = QTime::currentTime().addMSecs(msec);

	while (QTime::currentTime() < reachTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void LaserWidget::autoMotion()
{
	//reach to specified location and make judge
	for (int pointNum = 0; pointNum < POINTNUM; pointNum++)
	{
		suitcaseMotion(pointNum);
		emit reachLocation(pointNum);
	}
	//sum up all the results;
	for (QVector<bool>::iterator it = results.begin(); it != results.end(); it++)
	{
		QIcon icon;
		qint32 NGNum = 0;
		if (false == *it)
			NGNum += 1;
		if (NGNum > 0)
		{
			icon.addFile(QStringLiteral(":/images/images/smile.png"), QSize(), QIcon::Normal, QIcon::Off);
			ui->resultsBtn->setIcon(icon);
			ui->resultsBtn->setIconSize(QSize(120, 120));
		}
		else
		{
			icon.addFile(QStringLiteral(":/images/images/cry.png"), QSize(), QIcon::Normal, QIcon::On);
			ui->resultsBtn->setIcon(icon);
			ui->resultsBtn->setIconSize(QSize(120, 120));
		}
	}
	//clear the vector array
	clearVectorArray();
}

void LaserWidget::onCalibDetect(qint32 pointNum)
{
	onUpLaserGetBtnClicked();
	bool resultOne = true;
	bool resultTwo = true;
	bool resultThree = true;
	bool resultFour = true;
	bool resultFive = true;

	double distGapOne = distFirstLeft - distSecondRight;
	double distGapTwo = distFirstRight - distFourthLeft;
	//2017-12-07
	switch (pointNum)
	{
	case 0:
		resultsQualityJudge(distAvrFirst, resultOne);
		results.push_back(resultOne);
		break;
	case 1:
		resultsCrossQuality(distGapOne, resultTwo);
		results.push_back(resultTwo);
		break;
	case 2:
		resultsQualityJudge(distAvrSecond, resultThree);
		results.push_back(resultThree);
		break;
	case 3:
		resultsCrossQuality(distGapTwo, resultFour);
		results.push_back(resultFour);
		break;
	case 4:
		resultsQualityJudge(distAvrFourth, resultFive);
		results.push_back(resultFive);
		break;
	default:
		break;
	}
}

void LaserWidget::suitcaseMotion(qint32 i)
{
	qint32 expXPos, expYPos, expZPos, expRPos;
	qint32 expXVel, expYVel, expZVel, expRVel;

	expXPos = xyz_xPoint.at(i) * (1 << 17) / 45;
	expYPos = xyz_yPoint.at(i) * (1 << 17) / 20;
	expZPos = xyz_zPoint.at(i) * (1 << 17) / 10;
	expRPos = xyz_rPoint.at(i) * (1 << 17) / 360 * 100;
	
	expXVel = 80 * (1 << 17) / 45;
	expYVel = 65 * (1 << 17) / 20;
	expZVel = 65 * (1 << 17) / 10;
	expRVel = 65 * (1 << 17) / 360 * 100;					//INCREMENTAL ENCODER
	//expRVel = 5 * (1 << 17) / 360 * 100;					//ABSOLUTE ENCODER

	if (!xyzPtr->moveRP2P(expRPos, expRVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveRP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	sleep(100);
	xyzPtr->waitFinished(xyzIndex->at(3));

	if (!xyzPtr->moveZP2P(expZPos, expZVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveZP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	sleep(100);
	xyzPtr->waitFinished(xyzIndex->at(2));

	if (!xyzPtr->moveXP2P(expXPos, expXVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveXP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	sleep(100);
	xyzPtr->waitFinished(xyzIndex->at(1));

	if (!xyzPtr->moveYP2P(expYPos, expYVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveYP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	sleep(100);
	xyzPtr->waitFinished(xyzIndex->at(0));

}

void LaserWidget::onMotionConnected()
{
	ui->startBtn->setEnabled(true);
}

void LaserWidget::onMotionDisconnected()
{
	//QMessageBox::critical(this,
	//	tr("Motion Error"),
	//	QString("Connect Failed!")
	//	);
}

void LaserWidget::onCalibDetectToggled(bool checked)
{
	if (checked)
	{
		if (!xyzPtr->enabled()){
			if (!xyzPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}

		updateTimer->start(100);

		if (!motionThread.joinable())
			motionThread = std::thread(&LaserWidget::autoMotion, this);

		
	}
	else
	{
		if (!xyzPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("XYZPantilt is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
	}
}

bool LaserWidget::readXMLFile(QString filename){
	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		LOG(ERROR) << "FILE READ ERROR:" << filename.toStdString() << ":" << file.errorString().toStdString();
		return false;
	}

	QXmlStreamReader xmlReader(&file);
	if (xmlReader.readNextStartElement()){
		if ("Config" == xmlReader.name()){
			while (xmlReader.readNextStartElement()){
				if (xmlReader.name() == "suitcase"){
					while (xmlReader.readNextStartElement()){
						if (xmlReader.name() == "XYZ_X"){
							QString xyz_x = xmlReader.readElementText();
							xyz_xList.append(xyz_x);
						}
						else if (xmlReader.name() == "XYZ_Y"){
							QString xyz_y = xmlReader.readElementText();
							xyz_yList.append(xyz_y);
						}
						else if (xmlReader.name() == "XYZ_Z"){
							QString xyz_z = xmlReader.readElementText();
							xyz_zList.append(xyz_z);
						}
						else{
							QString xyz_r = xmlReader.readElementText();
							xyz_rList.append(xyz_r);
						}
					}
				}
				
			}
		}
		else{
			xmlReader.raiseError(tr("Incorrect file"));
			return false;
		}
	}
	else{
		LOG(WARNING) << "XML EMPTY WARNING:" << filename.toStdString();
		return false;
	}
	return true;
}

void LaserWidget::updatePoints(){
	QString xyz_xEntries = xyz_xList.at(0);
	QString xyz_yEntries = xyz_yList.at(0);
	QString xyz_zEntries = xyz_zList.at(0);
	QString xyz_rEntries = xyz_rList.at(0);

	
	while (xyz_xPoint.size() < POINTNUM)
	{
		int xyz_xSize = xyz_xEntries.size();
		int xyz_xPos = xyz_xEntries.indexOf(' ');
		xyz_xPoint.push_back(xyz_xEntries.left(xyz_xPos).toInt());
		xyz_xEntries = xyz_xEntries.right(xyz_xSize - xyz_xPos - 1);

		int xyz_ySize = xyz_yEntries.size();
		int xyz_yPos = xyz_yEntries.indexOf(' ');
		xyz_yPoint.push_back(xyz_yEntries.left(xyz_yPos).toInt());
		xyz_yEntries = xyz_yEntries.right(xyz_ySize - xyz_yPos - 1);

		int xyz_zSize = xyz_zEntries.size();
		int xyz_zPos = xyz_zEntries.indexOf(' ');
		xyz_zPoint.push_back(xyz_zEntries.left(xyz_zPos).toInt());
		xyz_zEntries = xyz_zEntries.right(xyz_zSize - xyz_zPos - 1);

		int xyz_rSize = xyz_rEntries.size();
		int xyz_rPos = xyz_rEntries.indexOf(' ');
		xyz_rPoint.push_back(xyz_rEntries.left(xyz_rPos).toInt());
		xyz_rEntries = xyz_rEntries.right(xyz_rSize - xyz_rPos - 1);
	}
}

void LaserWidget::clearVectorArray()
{
	xyz_xPoint.clear();
	xyz_yPoint.clear();
	xyz_zPoint.clear();
	xyz_rPoint.clear();

	results.clear();
}