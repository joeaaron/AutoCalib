/*!
 * \file AutoCalibWidget.cpp
 * \date 2017/10/23 14:09
 *
 * \author JohnShua
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/
#ifndef _ECAT_GUI_AutoCalibWidget_H_
#define _ECAT_GUI_AutoCalibWidget_H_
#include "AutoCalibWidget.h"
#include "ui_autocalibpage.h"
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include "network/network.h"
#include "calib.h"
#include "file.h"
#include "image.h"
//#include "cowaCalib.h"
#include <QDataStream>
#include <fstream>
#include <windows.h>

const int LUGGAGESIZE = 18;
const int SMALLPANSIZE = 24;
const int BIGPANSIZE = 20;
const int SAVEDIRNUM = 8; 
const int CAMERAS = 4;
const int FALLLASER = 5;
const QString p[] = { "UP", "DOWN", "MIDDLE", "LEFT", "RIGHT" };
enum chessBoardPos{
	POS_UP,
	POS_DOWN,
	POS_MIDDLE,
	POS_LEFT,
	POS_RIGHT,
};

//class Cowa_Calib;

//using namespace ECAT::CALIB;
using namespace JA::CV;
using namespace JA::CPLUSPLUS;

#define HAND_EYE_CALIBRATION_ROTATION_ANGLE  (-CV_PI/6)

AutoCalibWidget::AutoCalibWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoCalibWidget),
	xyzPtr(new XYZ()),
	bigPanTiltPtr(new PanTilt()),
	smallPanTiltPtr(new PanTilt()),
	xyzaxesIndex(new QVector<int>()),
	smallaxesIndex(new QVector<int>()),
	bigaxesIndex(new QVector<int>()),
	axesOffset(new QVector<int>(2, 0)),
	updateTimer(new QTimer(this))
{
    ui->setupUi(this);
	InitSignals();
	initVariables();
	initUi();
} 

//************************************
// Method:    initUi
// FullName:  AutoCalibWidget::initUi
// Access:    private 
// Returns:   void
// Qualifier:
//************************************
void AutoCalibWidget::initUi(){
	//useless button hided
	ui->saveBtn->hide();
	ui->importBtn->hide(); 
	//Init display Frame
	displayView = new DisplayView(ui->displayFrame);
	ui->displayLayout->addWidget(displayView, 0, 0, 1, 1);
	displayView->show();
}

void AutoCalibWidget::InitSignals()
{
	connect(ui->controlConnectBtn, SIGNAL(clicked()), this, SLOT(onConnectClicked()));
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onStartBtnToggled(bool)));
	connect(ui->stopBtn, SIGNAL(clicked()), this, SLOT(onStopBtnClicked()));
	connect(ui->calibBtn, SIGNAL(clicked()), this, SLOT(onCalibBtnClicked()));

	connect(ui->testBtn, SIGNAL(clicked()), this, SLOT(onTestBtnClicked()));
	connect(ui->importBtn, SIGNAL(clicked()), this, SLOT(onImportBtnClicked()));
	connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(onSaveBtnClicked()));
	connect(ui->displayBtn, SIGNAL(clicked()), this, SLOT(onDisplayBtnClicked()));
	//connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));

	connect(&CMDParser::getInstance(), SIGNAL(recvImage(quint16, quint16, quint16, QByteArray)),
		this, SLOT(onRecvImage(quint16, quint16, quint16, QByteArray)));
	connect(&CMDParser::getInstance(), SIGNAL(suitcaseNum(QByteArray)), this, SLOT(onGetSuitcaseNum(QByteArray)));
	connect(this, SIGNAL(ReachLocation(qint32, bool)), this, SLOT(onImgTook(qint32, bool)));
	connect(this, SIGNAL(progValue(double)), this, SLOT(onValueChanged(double)));
	connect(this, SIGNAL(openTopLaser()), this, SLOT(onOpenTopLaser()));
	connect(this, SIGNAL(openTopFallLaser()), this, SLOT(onOpenTopFallLaser()));
	connect(this, SIGNAL(openBottomLaser()), this, SLOT(onOpenBottomLaser()));
	connect(this, SIGNAL(getDeviation()), this, SLOT(onGetDeviation()));
	
	connect(ui->boardCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBoardCheckBoxStateChanged(int)));
	connect(ui->fallCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFallCheckBoxStateChanged(int)));
	connect(ui->calibIDCBox, SIGNAL(activated(int)), this, SLOT(onCamComboActivated(int)));
}

AutoCalibWidget::~AutoCalibWidget()
{
    delete ui;
}


void AutoCalibWidget::onCamComboActivated(int index){

	calibID = index + 1;
}

void AutoCalibWidget::initVariables(){

	//initialize the saving settings
	//savePath = "./images/top";
	savePath = "./image";
	saveDir = "1";
	num = 0;
	cameraID = 0;         //default cameraID is 0

	imgArea = 640 * 480;
	lrOffset = 0.0;
	udOffset = 0.0;
	areaOffset = 0.0;

	endPoint = 5;
	calibID = 1;
	//flags
	smallPanSaveFinish = false;
	bigPanSaveFinish = false;
	recvFinish = false;
	isSaveImage = true;
	isSmallBoardCalibrated = false;
	isFallLaserCalibrate = false;
	bGetDeviation = false;
	bFind = false;
	//bin to the ptr
	bigaxesIndex->push_back(5);
	bigaxesIndex->push_back(4);
	bigPanTiltPtr->setAxes(bigaxesIndex);

	smallaxesIndex->push_back(7);
	smallaxesIndex->push_back(6);
	smallPanTiltPtr->setAxes(smallaxesIndex);

	xyzaxesIndex->push_back(1);
	xyzaxesIndex->push_back(0);
	xyzaxesIndex->push_back(2);
	xyzaxesIndex->push_back(3);
	xyzPtr->setAxes(xyzaxesIndex);

	//read default counterpoints xml file
	QString filename = QDir::currentPath() + "/counterpoint.xml";
	if (QFile::exists(filename)){
		if (readXMLFile(filename)){
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
		onImportBtnClicked();
	}
	
}

void AutoCalibWidget::onSynchroIP(QString address)
{
	ipAddress = address;
}

void AutoCalibWidget::onMotionConnected()
{
	ui->startBtn->setEnabled(true);
}

void AutoCalibWidget::onMotionDisconnected()
{
	QMessageBox::critical(this,
		tr("Motion Error"),
		QString("Connect Failed!")
		);
}

void AutoCalibWidget::onDisplayBtnClicked(){
	//ui->displayBtn->setEnabled(false);
	CMDParser::getInstance().requestImage({0});
}

void AutoCalibWidget::onBoardCheckBoxStateChanged(int state)
{
	if (Qt::Unchecked == state)
		isSmallBoardCalibrated = false;
	else
		isSmallBoardCalibrated = true;
}

void AutoCalibWidget::onFallCheckBoxStateChanged(int state)
{
	if (Qt::Unchecked == state)
		isFallLaserCalibrate = false;
	else
		isFallLaserCalibrate = true;
}

void calib(int n)
{
	char cmd[128];
	sprintf(cmd, "calibrate.exe image %d", n);
	system(cmd);
}

void AutoCalibWidget::onTestBtnClicked(){
	//Do something
	//onSmallBoardMotionPro();     //test vision servo

	/*cv::Mat dst = cv::imread("temp.bmp");
	cv::Mat viewGray;
	cvtColor(dst, viewGray, cv::COLOR_BGR2GRAY);

	calib.GetDeviationPara(dst, para);*/
	//qDebug() << xyz_xPoint_2.at(0);
	//onCalibBtnClicked();
	//pushFiles();
	//DWORD start_time = GetTickCount();
	//cv::Mat srcImg = cv::imread("./temp/x2.bmp");
	//Image::ImgAvgGrayValue(srcImg, bFind);
	//DWORD end_time = GetTickCount();
	//qDebug() << "The run time is:" << (end_time - start_time) << "ms!" << endl;
	//bool bFind;
	//cv::Mat srcImg = cv::imread("./temp/x2.bmp");
	//if (srcImg.empty())
	//{
	//	return;
	//}
	//Calib::FindBoardCorner(srcImg, bFind);
	//qDebug() << bFind;

	//QString logInfo4 = QString("calib %1 camera-laser calibration finished :").arg(1);
	//printLog(1, logInfo4);

	//sleep(10000);
	//ui->processLog->clear();
	//pushFiles();
	std::thread threads[3];

	threads[0] = std::thread(calib, 1);
	threads[1] = std::thread(calib, 2);
	threads[2] = std::thread(calib, 4);

	for (auto &pThread : threads){
		pThread.join();
	}
		
	QMessageBox::information(this,
		tr("CalibInformation"),
		QString("Calib Finished!"));
	//backUpFile();
	/*std::string dirPath = "./images/";
	File::delAllFiles(dirPath);*/
	//CMDParser::getInstance().getSuitcaseNum();
}

void AutoCalibWidget::backUpFile()
{
	char buf[1000];
	GetCurrentDirectory(1000, buf);
	std::string path = buf;
	std::string dirPath = path + "\\images\\";
	//2017/12/7 fix something 
	std::string backup = "D:\\BACKUP\\";
	std::string dirName = suitcaseID.toStdString();
	std::string mkDir = "md " + backup + dirName;
	system(mkDir.c_str());
	std::string suitcasePath = backup.append(dirName);
	const char* dstPath = suitcasePath.c_str();
	////images backup
	File::copyDir(dirPath.c_str(), dstPath);
}

void AutoCalibWidget::onOpenBottomLaser(){
	CMDParser::getInstance().openBottomLaser();
}

void AutoCalibWidget::onOpenTopLaser(){
	CMDParser::getInstance().openTopLaser();
}

void AutoCalibWidget::onOpenTopFallLaser(){
	CMDParser::getInstance().openFallTopLaser();
}

void AutoCalibWidget::onGetDeviation(){
	
	lrOffset = para[0];
	udOffset = para[1];
	areaOffset = para[2];
	bGetDeviation = true;
}

void AutoCalibWidget::saveImg(qint32 cameraNum){

	switch ((cameraNum + 1) % 4)
	{
	case 1:
		saveDir = "1";
		break;
	case 2:
		saveDir = "2";
		break;
	case 3:
		saveDir = "3";
		break;
	case 0:
		saveDir = "4";
		break;
	default:
		break;
	}

	QPixmap pixmap = QPixmap::fromImage(imageToSave);
	QString tmp;
	tmp.sprintf("%s/%s/x%d", savePath.toLatin1().data(),saveDir.toLatin1().data(), ++num);
	QString imagePath = QString(tmp) + ".bmp";
	pixmap.save(imagePath);
	if (smallPanSaveFinish){
		if (10 == num)
			num = 0;
	}
	else{
		if (12 == num)
			num = 0;
	}
	
}

void AutoCalibWidget::updatePoints(){
	QString xyz_xEntries_1 = xyz_xList.at(0);
	QString xyz_yEntries_1 = xyz_yList.at(0);
	QString xyz_zEntries_1 = xyz_zList.at(0);
	QString xyz_rEntries_1 = xyz_rList.at(0);

	QString xyz_xEntries_2 = xyz_xList.at(1);
	QString xyz_yEntries_2 = xyz_yList.at(1);
	QString xyz_zEntries_2 = xyz_zList.at(1);
	QString xyz_rEntries_2 = xyz_rList.at(1);

	QString smallPan_xEntries = smallpan_xList.at(0);
	QString smallPan_zEntries = smallpan_zList.at(0);

	QString bigPan_xEntries = bigpan_xList.at(0);
	QString bigPan_zEntries = bigpan_zList.at(0);

	while (xyz_xPoint_1.size() < LUGGAGESIZE - 6)
	{
		int xyz_xSize = xyz_xEntries_1.size();
		int xyz_xPos = xyz_xEntries_1.indexOf(' ');
		xyz_xPoint_1.push_back(xyz_xEntries_1.left(xyz_xPos).toInt());
		xyz_xEntries_1 = xyz_xEntries_1.right(xyz_xSize - xyz_xPos - 1);
	
		int xyz_ySize = xyz_yEntries_1.size();
		int xyz_yPos = xyz_yEntries_1.indexOf(' ');
		xyz_yPoint_1.push_back(xyz_yEntries_1.left(xyz_yPos).toInt());
		xyz_yEntries_1 = xyz_yEntries_1.right(xyz_ySize - xyz_yPos - 1);
	
		int xyz_zSize = xyz_zEntries_1.size();
		int xyz_zPos = xyz_zEntries_1.indexOf(' ');
		xyz_zPoint_1.push_back(xyz_zEntries_1.left(xyz_zPos).toInt());
		xyz_zEntries_1 = xyz_zEntries_1.right(xyz_zSize - xyz_zPos - 1);
	
		int xyz_rSize = xyz_rEntries_1.size();
		int xyz_rPos = xyz_rEntries_1.indexOf(' ');
		xyz_rPoint_1.push_back(xyz_rEntries_1.left(xyz_rPos).toInt());
		xyz_rEntries_1 = xyz_rEntries_1.right(xyz_rSize - xyz_rPos - 1);
	}

	while (xyz_xPoint_2.size() < 6)
	{
		int xyz_xSize = xyz_xEntries_2.size();
		int xyz_xPos = xyz_xEntries_2.indexOf(' ');
		xyz_xPoint_2.push_back(xyz_xEntries_2.left(xyz_xPos).toInt());
		xyz_xEntries_2 = xyz_xEntries_1.right(xyz_xSize - xyz_xPos - 1);

		int xyz_ySize = xyz_yEntries_2.size();
		int xyz_yPos = xyz_yEntries_2.indexOf(' ');
		xyz_yPoint_2.push_back(xyz_yEntries_2.left(xyz_yPos).toInt());
		xyz_yEntries_2 = xyz_yEntries_2.right(xyz_ySize - xyz_yPos - 1);

		int xyz_zSize = xyz_zEntries_2.size();
		int xyz_zPos = xyz_zEntries_2.indexOf(' ');
		xyz_zPoint_2.push_back(xyz_zEntries_2.left(xyz_zPos).toInt());
		xyz_zEntries_2 = xyz_zEntries_2.right(xyz_zSize - xyz_zPos - 1);

		int xyz_rSize = xyz_rEntries_2.size();
		int xyz_rPos = xyz_rEntries_2.indexOf(' ');
		xyz_rPoint_2.push_back(xyz_rEntries_2.left(xyz_rPos).toInt());
		xyz_rEntries_2 = xyz_rEntries_2.right(xyz_rSize - xyz_rPos - 1);
	}

	while (smallpan_xPoint.size() < SMALLPANSIZE)
	{
		int small_xSize = smallPan_xEntries.size();
		int small_xPos = smallPan_xEntries.indexOf(' ');
		smallpan_xPoint.push_back(smallPan_xEntries.left(small_xPos).toInt());
		smallPan_xEntries = smallPan_xEntries.right(small_xSize - small_xPos - 1);

		int small_zSize = smallPan_zEntries.size();
		int small_zPos = smallPan_zEntries.indexOf(' ');
		smallpan_zPoint.push_back(smallPan_zEntries.left(small_zPos).toInt());
		smallPan_zEntries = smallPan_zEntries.right(small_zSize - small_zPos - 1);
	}

	while (bigpan_xPoint.size() < BIGPANSIZE)
	{
		int big_xSize = bigPan_xEntries.size();
		int big_xPos = bigPan_xEntries.indexOf(' ');
		bigpan_xPoint.push_back(bigPan_xEntries.left(big_xPos).toInt());
		bigPan_xEntries = bigPan_xEntries.right(big_xSize - big_xPos - 1);

		int big_zSize = bigPan_zEntries.size();
		int big_zPos = bigPan_zEntries.indexOf(' ');
		bigpan_zPoint.push_back(bigPan_zEntries.left(big_zPos).toInt());
		bigPan_zEntries = bigPan_zEntries.right(big_zSize - big_zPos - 1);
	}
}

bool AutoCalibWidget::readXMLFile(QString filename){
	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		LOG(ERROR) << "FILE READ ERROR:" << filename.toStdString() << ":" << file.errorString().toStdString();
		return false;
	}

	QXmlStreamReader xmlReader(&file);
	if (xmlReader.readNextStartElement()){
		if ("Config" == xmlReader.name()){
			while (xmlReader.readNextStartElement()){
				if (xmlReader.name() == "luggage"){
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
				else if (xmlReader.name() == "smallpantilt"){
					while (xmlReader.readNextStartElement()){
						if (xmlReader.name() == "X1"){
							QString smallpan_x = xmlReader.readElementText();
							smallpan_xList.append(smallpan_x);
						}
						else{
							QString smallpan_z = xmlReader.readElementText();
							smallpan_zList.append(smallpan_z);
						}
					}
				}
				else if (xmlReader.name() == "bigpantilt"){
					while (xmlReader.readNextStartElement()){
						if (xmlReader.name() == "X1"){
							QString bigpan_x = xmlReader.readElementText();
							bigpan_xList.append(bigpan_x);
						}
						else{
							QString bigpan_z = xmlReader.readElementText();
							bigpan_zList.append(bigpan_z);
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

void AutoCalibWidget::saveXMLFile(QString filename){
	//map enum DEVICE_AXIS to QString
	auto axesNameMap = [](DEVICE_AXIS axis) -> QString{
		switch (axis)
		{
		case DEVICE_AXIS::BIGPANTILT_X:
			return QString("BIGPANTILT_X");
		case DEVICE_AXIS::BIGPANTILT_Z:
			return QString("BIGPANTILT_Z");
		case DEVICE_AXIS::SMALLPANTILT_X:
			return QString("SMALLPANTILT_X");
		case DEVICE_AXIS::SMALLPANTILT_Z:
			return QString("SMALLPANTILT_Z");
		case DEVICE_AXIS::XYZ_X:
			return QString("XYZ_X");
		case DEVICE_AXIS::XYZ_Y:
			return QString("XYZ_Y");
		case DEVICE_AXIS::XYZ_Z:
			return QString("XYZ_Z");
		case DEVICE_AXIS::XYZ_R:
			return QString("XYZ_R");
		default:
			return QString("");
		}
	};
	QFile file(filename);
	file.open(QFile::WriteOnly);

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);

	xmlWriter.writeStartDocument();	// start doc
	xmlWriter.writeStartElement("Config"); // start Config

	xmlWriter.writeAttribute("name", "Calibration");

	QList<DEVICE_AXIS> devices = deviceAxesMap.keys();
	for (auto device : devices){
		xmlWriter.writeStartElement(axesNameMap(device));
		xmlWriter.writeAttribute("Axis", QString::number(deviceAxesMap[device]));
		xmlWriter.writeAttribute("Offset", QString::number(axisOffsetMap[device]));
		xmlWriter.writeEndElement();
	}
	xmlWriter.writeEndElement();	// end Config
	xmlWriter.writeEndDocument();	// end doc

	file.close();
}

void AutoCalibWidget::onConnectClicked(){

	emit connectStart();
}

void AutoCalibWidget::onImportBtnClicked(){

	QString filename = QFileDialog::getOpenFileName(this,
		tr("Config File"),
		".",
		tr("Xml files (*.xml)")
		);
	if (readXMLFile(filename)){
		updatePoints();
	}
	else
		QMessageBox::warning(
		this,
		tr("Config Error"),
		QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
		);
}

void AutoCalibWidget::onSaveBtnClicked(){
	QString filename = QFileDialog::getSaveFileName(this,
			tr("Save File"),
			".",
			tr("Xml files (*.xml)")
			);
	if (filename.isEmpty()){
		filename = "axis-driver.xml";
		saveXMLFile(filename);
	}
	else
		QMessageBox::warning(
		this,
		tr("Config Warning"),
		QString("Axes share same drivers, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
		);
}

/*----------------------------
* ���� : ��������λ�õ��ƶ�
*----------------------------
* ���� : suitcaseMotion
* ���� : public
* ���� : NULL
*
* ���� : i[in]      �˶�����i��λ��	
*/
void AutoCalibWidget::suitcaseMotion(qint32 i)
{
	qint32 expXPos, expYPos, expZPos, expRPos;
	qint32 expXVel, expYVel, expZVel, expRVel;
	//QString value = ui->offsetlineEdit->text();
	//qint32 offset = value.toInt();
	if (!smallPanSaveFinish)
	{
		//expXPos = xyz_xPoint_1.at(i) * 10000 / M_PI / 23.87 * 3;
		//expYPos = xyz_yPoint_1.at(i) * 10000 / M_PI / 23.87 * 3;
		//expZPos = xyz_zPoint_1.at(i) * 10000 / M_PI / 23.87 * 40;
		//expRPos = xyz_rPoint_1.at(i) * 10000 / 360 * 120 + offset;					//INCREMENTAL ENCODER
		////expRPos = xyz_rPoint.at(i) * (1 << 17) / 360 * 100 + axesOffset->at(0);    //ABSOLUTE ENCODER
		expXPos = xyz_xPoint_1.at(i) * (1 << 17) / 45;
		expYPos = xyz_yPoint_1.at(i) * (1 << 17) / 20;
		expZPos = xyz_zPoint_1.at(i) * (1 << 17) / 10;
		expRPos = xyz_rPoint_1.at(i) * (1 << 17) / 360 * 100;
	}
	else
	{
		expXPos = xyz_xPoint_2.at(i) * (1 << 17) / 45;
		expYPos = xyz_yPoint_2.at(i) * (1 << 17) / 20;
		expZPos = xyz_zPoint_2.at(i) * (1 << 17) / 10;
		expRPos = xyz_rPoint_2.at(i) * (1 << 17) / 360 * 100;
	}
	
	expXVel = 80 * (1 << 17) / 45;
	expYVel = 65 * (1 << 17) / 20;
	expZVel = 65  * (1 << 17) / 10;
	expRVel = 65 * (1 << 17) / 360 * 100;					//INCREMENTAL ENCODER
	//expRVel = 5 * (1 << 17) / 360 * 100;					//ABSOLUTE ENCODER
	
	if (!xyzPtr->moveRP2P(expRPos, expRVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveRP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	//sleep(100);
	//xyzPtr->waitFinished(xyzaxesIndex->at(3));

	if (!xyzPtr->moveZP2P(expZPos, expZVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveZP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	/*sleep(100);
	xyzPtr->waitFinished(xyzaxesIndex->at(2));*/

	if (!xyzPtr->moveXP2P(expXPos, expXVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveXP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	//sleep(100);
	//xyzPtr->waitFinished(xyzaxesIndex->at(1));

	if (!xyzPtr->moveYP2P(expYPos, expYVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("XYZ moveYP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	//sleep(100);
	//xyzPtr->waitFinished(xyzaxesIndex->at(0));
	//2018-12-24
	for (int i = 0; i < 4; ++i)
		if (xyzPtr->waitFinished(i));
}

void AutoCalibWidget::sleep(unsigned int msec)
{
	QTime reachTime = QTime::currentTime().addMSecs(msec);

	while (QTime::currentTime() < reachTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void AutoCalibWidget::onRecvImage(quint16 camera, quint16 width, quint16 height, QByteArray imageData){
	QImage image = QImage((uchar*)imageData.data(), width, height, QImage::Format_Indexed8);
	if (image.isNull())
		return;
	else{
		imageToSave = image.copy();
		displayView->showImage(image);
		srcImg = QImage2cvMat(image);
		//Calib::GetDeviationPara(srcImg, para);
		cv::Mat img = srcImg.clone();
		//Image::ImgAvgGrayValue(img, bFind);

		//save img or not
		if (!isSaveImage)
		{
			if (!smallPanSaveFinish)
			{
				if (camera < 5)
				{
					if (!bFind)
					{
						CMDParser::getInstance().requestImage({ camera });            //take a picture again
						return;               //exis at once while image is invalid
					}
					else
						saveImg(camera);
				}
				else
					saveImg(camera);
			}

			//saveImg
			else
			{
				if (camera > 5)
				{
					if (!bFind)
					{
						CMDParser::getInstance().requestImage({ camera });            //take a picture again
						return;
					}
					else
						saveImg(camera);
				}
				else
					saveImg(camera);
			}

		}
		
		recvFinish = true;
		//ui->displayBtn->setEnabled(true);
	}
}

void AutoCalibWidget::onGetSuitcaseNum(QByteArray suitcaseNum){
	suitcaseID = suitcaseNum;
}

void AutoCalibWidget::triggerPush(QString pushCommand, qint32 time)
{
	CMDParser::getInstance().adbCommand(pushCommand);
	sleep(time);
}

void AutoCalibWidget::pushFiles()
{
	int pushTimeConsuming;       
	
	//QString connectCommand("adb connect 192.168.200.164");
	QString connectCommand = "adb connect " + ipAddress;
	pushTimeConsuming = 1000;              //find the appropriate time
	triggerPush(connectCommand, pushTimeConsuming);

	QString binFilesCommand("adb push ./image/transformationTable1.bin /data/cowa_cam_config");
	pushTimeConsuming = 5000;              //find the appropriate time
	triggerPush(binFilesCommand, pushTimeConsuming);

	//QString binFilesCommand("adb push ./image/transformationTable1.bin /data/cowa_cam_config");
	//pushTimeConsuming = 5000;              //find the appropriate time
	//triggerPush(binFilesCommand, pushTimeConsuming);

	//QString binFilesCommand("adb push ./image/transformationTable4.bin /data/cowa_cam_config");
	//pushTimeConsuming = 5000;              //find the appropriate time
	//triggerPush(binFilesCommand, pushTimeConsuming);

	QString yDividingCommand("adb push yDividing.txt /data/cowa_cam_config");
	pushTimeConsuming = 500;
	triggerPush(yDividingCommand, pushTimeConsuming);

	/*QString cowarobotCommand("adb push cowarobot /system/bin");
	pushTimeConsuming = 2000;
	triggerPush(cowarobotCommand, pushTimeConsuming);*/

	//QString cameraScCommand("adb push camera.sc8830.so /system/lib/hw");
	//pushTimeConsuming = 2000;
	//triggerPush(cameraScCommand, pushTimeConsuming);

	QMessageBox::information(this,
		tr("Push Finished"),
		QString("The TransformationTableBin files has been pushed successfully."));

	emit pushFinished();
}

void AutoCalibWidget::onCalibBtnClicked(){

	/*if (!calibThread.joinable())
		calibThread = std::thread(&AutoCalibWidget::cowaCalib, this);*/
	std::thread threads[3];

	threads[0] = std::thread(calib, 1);
	threads[1] = std::thread(calib, 2);
	threads[2] = std::thread(calib, 4);

	for (auto &pThread : threads){
		pThread.join();
	}

	QMessageBox::information(this,
		tr("CalibInformation"),
		QString("CalibImgs Finished!"));

	///adb push .bin files to suitcase
	pushFiles();  //to be finished
}

/*----------------------------
* ���� : ��Դ�Ŀ���
*----------------------------
* ���� : onLightSwitch
* ���� : public
* ���� : NULL
*
* ���� : light[in]    ��Դ�Ƿ�򿪣�falseΪ���򿪣�trueΪ��
*/
void AutoCalibWidget::onLightSwitch(bool light)
{
	if (!smallPanSaveFinish)
	{
		quint8 data[4];
	
		if (light)
		{
			data[0] = 1;
			data[1] = 0;
			data[2] = 1;
			data[3] = 0;
			MotionController::getInstance().setDigitalIO(data, 3);
		}
		else{
			data[0] = 0;
			data[1] = 0;
			data[2] = 0;
			data[3] = 0;
			MotionController::getInstance().setDigitalIO(data, 3);
		}
	}
	else
	{
		quint8 _data[4];
		if (light)
		{
			_data[0] = 0;
			_data[1] = 1;
			_data[2] = 0;
			_data[3] = 1;
			MotionController::getInstance().setDigitalIO(_data, 3);
		}
		else{
			_data[0] = 0;
			_data[1] = 0;
			_data[2] = 0;
			_data[3] = 0;
			MotionController::getInstance().setDigitalIO(_data, 3);
		}
	}
	
}

//the signal img took
void AutoCalibWidget::onImgTook(qint32 camera, bool laser){

	/*if (laser == false)
		CMDParser::getInstance().setLaserStatus(0);
		else
		CMDParser::getInstance().setLaserStatus(8);*/

	CMDParser::getInstance().requestImage({ camera });
}

//print calib log
void AutoCalibWidget::printLog(qint32 cameraLaserID, QString& log)
{
	QString id = QString::number(cameraLaserID, 10);
	ui->processLog->addItem("[" + id + "]" + " " +log);
}

void AutoCalibWidget::onValueChanged(double progress)
{
	ui->calibProgBar->setValue(progress);
}

void AutoCalibWidget::onSmallBoardMotionPro()
{
	chessBoardPos pos;
	emit openTopLaser();
	QString value = ui->sCamlineEdit->text();
	qint32 camID = value.toInt();

	for (int i = camID; i <= CAMERAS; ++i)
	{
		if (3 == i)
			continue;
		suitcaseMotion(3 * i - 3);

		savePath = "./images/top";
		QString logInfo1 = QString("top %1 camera-laser calibration starting :").arg(i);
		printLog(i, logInfo1);

		bool isTeachOver = false;

		for (int j = 0; j < SMALLPANSIZE / CAMERAS; ++j)
		{
			qint32 expSmallPitchPos = smallpan_xPoint.at(j + (i - 1) * 6) * (1 << 17) / 360 * 40;
			qint32 expSmallYawPos = smallpan_zPoint.at(j + (i - 1) * 6)* (1 << 17) / 360 * 40;
			qint32 expSmallVel = 30 * (1 << 17) / 360 * 40;

			if (!smallPanTiltPtr->pitchP2P(expSmallPitchPos, expSmallVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("SmallPantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			smallPanTiltPtr->waitFinished(smallaxesIndex->at(0));

			if (!smallPanTiltPtr->yawP2P(expSmallYawPos, expSmallVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("SmallPantilt yawP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			smallPanTiltPtr->waitFinished(smallaxesIndex->at(1));

			if (j < 5)
			{
				if (!isTeachOver)
				{
					onLightSwitch(true);
					sleep(3000);
					emit ReachLocation(i - 1, false);
					while (!recvFinish)
					{
						sleep(100);
					}
					recvFinish = false;

					///compute the deviation to control XYZ
					emit getDeviation();
					while (!bGetDeviation)
					{
						sleep(100);
					}
					///posoiton
					qint32	XPos = xyz_xPoint_1.at(3 * i - 3) * (1 << 17) / 45;
					qint32	YPos = xyz_yPoint_1.at(3 * i - 3) * (1 << 17) / 20;
					qint32	ZPos = xyz_zPoint_1.at(3 * i - 3) * (1 << 17) / 10;
					///velocity
					/*qint32	XYVel = 10 * 10000 / M_PI / 23.87 * 3;
					qint32	ZVel = 10 * 10000 / M_PI / 23.87 * 40;*/
					qint32 XVel = 80 * (1 << 17) / 45;
					qint32 YVel = 65 * (1 << 17) / 20;
					qint32 ZVel = 65 * (1 << 17) / 10;

					if (fabs(lrOffset) > 20)
					{
						YPos = (xyz_yPoint_1.at(3 * i - 3) + lrOffset) * 10000 / M_PI / 23.87 * 3;
						xyzPtr->moveYP2P(YPos, YVel);
					}
					else
						xyzPtr->moveYP2P(YPos, YVel);

					if (areaOffset > 180000 && 2 != i)
					{
						XPos = (xyz_xPoint_1.at(3 * i - 3) - (areaOffset - 180000) / 500) * 10000 / M_PI / 23.87 * 3;
						xyzPtr->moveXP2P(XPos, XVel);
					}
					else
						xyzPtr->moveXP2P(XPos, XVel);

					if (fabs(udOffset) > 10)
					{
						ZPos = (xyz_zPoint_1.at(3 * i - 1) + udOffset) * 10000 / M_PI / 23.87 * 40;
						xyzPtr->moveZP2P(ZPos, ZVel);
					}
					else
						xyzPtr->moveZP2P(ZPos, ZVel);

					para.clear();
				}

				isTeachOver = true;     //ʾ�����
				isSaveImage = true;     //���Ա���ͼ��

				emit ReachLocation(i - 1, false);
				while (!recvFinish)
				{
					sleep(100);
				}
				recvFinish = false;

				onLightSwitch(false);
				sleep(3000);
				emit ReachLocation(i + 8 - 1, true);
				while (!recvFinish)
				{
					sleep(100);
				}
				recvFinish = false;
			}
			switch (j)
			{
			case 0:
				pos = POS_MIDDLE;
				break;
			case 1:
				pos = POS_LEFT;
				break;
			case 2:
				pos = POS_RIGHT;
				break;
			case 3:
				pos = POS_UP;
				break;
			case 4:
				pos = POS_DOWN;
				break;
			case 5:
				pos = POS_MIDDLE;
				break;
			default:
				break;
			}

			QString logInfo2 = QString("Five positons of chess board : %1 ").arg(p[pos]);
			printLog(i, logInfo2);
		}
		onLightSwitch(true);

		suitcaseMotion(3 * i - 2);
		sleep(3000);
		emit ReachLocation(i - 1, false);       //Merge into a function
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;

		suitcaseMotion(3 * i - 1);
		sleep(3000);
		emit ReachLocation(i - 1, false);       //Merge into a function
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;

		calibValue = 100 * i / 8;
		emit progValue(calibValue);

		QString logInfo3 = QString("top %1 camera-laser calibration finished !").arg(i);
		printLog(i, logInfo3);
	}
}

void AutoCalibWidget::onSmallBoardMotion()
{
	chessBoardPos pos;
	emit openTopLaser();
	QString value = ui->sCamlineEdit->text();
	qint32 camID = value.toInt();
	isSaveImage = false;
	for (int i = camID; i <= CAMERAS; ++i)
	{
		if (3 == i)
			continue;
		suitcaseMotion(3 * i - 3);

		savePath = "./images/top";
		QString logInfo1 = QString("top %1 camera-laser calibration starting :").arg(i);
		printLog(i, logInfo1);

		for (int j = 0; j < SMALLPANSIZE / CAMERAS; ++j)
		{
		/*	qint32 expSmallPitchPos = smallpan_xPoint.at(j + (i - 1) * 6) * (1 << 17) / 360 * 40 * 60 + axesOffset->at(0);
			qint32 expSmallYawPos = smallpan_zPoint.at(j + (i - 1) * 6)* (1 << 17) / 360 * 40 * 60 + axesOffset->at(1);
*/
			qint32 expSmallPitchPos = smallpan_xPoint.at(j + (i - 1) * 6) * (1 << 17) / 360 * 40 ;
			qint32 expSmallYawPos = smallpan_zPoint.at(j + (i - 1) * 6)* (1 << 17) / 360 * 40 ;
			qint32 expSmallVel = 35 * (1 << 17) / 360 * 40;

			if (!smallPanTiltPtr->pitchP2P(expSmallPitchPos, expSmallVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("SmallPantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			//smallPanTiltPtr->waitFinished(smallaxesIndex->at(0));

			if (!smallPanTiltPtr->yawP2P(expSmallYawPos, expSmallVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("SmallPantilt yawP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			//smallPanTiltPtr->waitFinished(smallaxesIndex->at(1));

			for (int i = 6; i < 8; ++i)
				if (smallPanTiltPtr->waitFinished(i));

			if (j < 5)
			{
				onLightSwitch(true);
				sleep(3000);
				emit ReachLocation(i - 1, false);
				while (!recvFinish)
				{
					sleep(100);
				}
				recvFinish = false;
				//sleep(3000);           

				onLightSwitch(false);
				sleep(3000);
				emit ReachLocation(i + 8 - 1, true);
				while (!recvFinish)
				{
					sleep(100);
				}
				recvFinish = false;
			}

			switch (j)
			{
			case 0:
				pos = POS_MIDDLE;
				break;
			case 1:
				pos = POS_LEFT;
				break;
			case 2:
				pos = POS_RIGHT;
				break;
			case 3:
				pos = POS_UP;
				break;
			case 4:
				pos = POS_DOWN;
				break;
			case 5:
				pos = POS_MIDDLE;
				break;
			default:
				break;
			}

			QString logInfo2 = QString("Five positons of chess board : %1 ").arg(p[pos]);
			printLog(i, logInfo2);
		}

		onLightSwitch(true);

		suitcaseMotion(3 * i - 2);
		sleep(3000);
		emit ReachLocation(i - 1, false);       //Merge into a function
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;

		suitcaseMotion(3 * i - 1);
		sleep(3000);
		emit ReachLocation(i - 1, false);       //Merge into a function
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;

		calibValue = 100 * i / 8;
		emit progValue(calibValue);
		//ui->calibProgBar->setValue(calibValue);

		QString logInfo3 = QString("top %1 camera-laser calibration finished !").arg(i);
		printLog(i, logInfo3);
	}
	//avoid the collision happened between chessboard and motor
	qint32 smallPitchPos = 13 * (1 << 17) / 360 * 40;
	qint32 smallYawPos = 52 * (1 << 17) / 360 * 40;
	qint32 smallVel = 25 * (1 << 17) / 360 * 40;
	if (!smallPanTiltPtr->pitchP2P(smallPitchPos, smallVel)){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("SmallPantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		return;
	}
	smallPanTiltPtr->waitFinished(smallaxesIndex->at(0));

	smallPanSaveFinish = true;
	suitcaseMotion(endPoint);
}

void AutoCalibWidget::onLargeBoardMotion()
{
	smallPanSaveFinish = true;
	chessBoardPos pos;
	isSaveImage = false;

	emit openBottomLaser();

	QString value = ui->lCamlineEdit->text();
	qint32 camID = value.toInt();

	for (int i = camID; i <= CAMERAS; ++i)
	{
		if (3 == i)
			continue;
		suitcaseMotion(i - 1);

		savePath = "./images/bottom";
		QString logInfo1 = QString("bottom %1 camera-laser calibration starting :").arg(i);
		printLog(i + 4, logInfo1);

		for (int j = 0; j < BIGPANSIZE / CAMERAS; ++j)
		{
			qint32 expBigPitchPos = bigpan_xPoint.at(j + (i - 1) * 5) * (1 << 17) / 360 * 100;
			qint32 expBigYawPos = bigpan_zPoint.at(j + (i - 1) * 5) * (1 << 17) / 360 * 40;
			qint32 expBigPitchVel = 10 * (1 << 17) / 360 * 100;
			qint32 expBigYawVel = 10 * (1 << 17) / 360 * 40;

			if (!bigPanTiltPtr->pitchP2P(expBigPitchPos, expBigPitchVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("BigPantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			bigPanTiltPtr->waitFinished(bigaxesIndex->at(0));

			if (!bigPanTiltPtr->yawP2P(expBigYawPos, expBigYawVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("BigPantilt yawP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			bigPanTiltPtr->waitFinished(bigaxesIndex->at(1));

			sleep(2000);
			onLightSwitch(true);
			sleep(5000);
			emit ReachLocation(i + 8 - 1, false);       //Merge into a function
			while (!recvFinish)
			{
				sleep(100);
			}
			recvFinish = false;
			//sleep(3000);

			onLightSwitch(false);
			sleep(3000);
			emit ReachLocation(i - 1, true);				 //Merge into a function
			while (!recvFinish)
			{
				sleep(100);
			}
			recvFinish = false;

			switch (j)
			{
			case 0:
				pos = POS_MIDDLE;
				break;
			case 1:
				pos = POS_LEFT;
				break;
			case 2:
				pos = POS_RIGHT;
				break;
			case 3:
				pos = POS_UP;
				break;
			case 4:
				pos = POS_DOWN;
				break;
			case 5:
				pos = POS_MIDDLE;
				break;
			default:
				break;
			}

			QString logInfo2 = QString("Five positons of chess board : %1 ").arg(p[pos]);
			printLog(i + 4, logInfo2);
		}

		calibValue = 100 * (i + 4) / 8;
		emit progValue(calibValue);

		QString logInfo3 = QString("bottom %1 camera-laser calibration finished !").arg(i);
		printLog(i + 4, logInfo3);
	}
	suitcaseMotion(endPoint);
	bigPanSaveFinish = true;
}

void AutoCalibWidget::cowaCalib()
{
	char buf[1000];
	GetCurrentDirectory(1000, buf);
	//std::cout << buf << std::endl;

	std::string path = buf;
	std::string dirPath = path + "\\images\\";
	std::string xmlDir = path + "\\xml\\";
	std::string xmlBottomDir = path + "\\xmlbottom\\";
	//std::string backup = "D:\\calibFiles\\";

	std::string mkDir1 = "md " + dirPath + "cowa_cam_config";
	std::string mkDir2 = "md " + dirPath + "cowa_cam_config\\" + "unaligned";
	std::string mkDir3 = "md " + dirPath + "cowa_cam_config\\" + "aligned";
	//std::string mkDir4 = "md " + backup + "cowa_R1";

	system(mkDir1.c_str());
	system(mkDir2.c_str());
	system(mkDir3.c_str());
	//system(mkDir4.c_str());

	//const char* dstPath = "D:\\calibFiles\\cowa_R1";

	for (int i = calibID; i <= 8; i++)
	{
		if (3 == i || 7 == i)
			continue;

		QString logInfo1 = QString("calib %1 camera-laser calibration starting :").arg(i);
		printLog(i, logInfo1);

		std::string currentPath;
		if (i < 5)
		{
			currentPath = dirPath + "top\\" + std::to_string(i);
			//copy the xml files to \\1 \\2 \\3 \\4
			std::string copyXML = "copy " + xmlDir + "* " + currentPath;
			system(copyXML.c_str());
		}
		else
		{
			currentPath = dirPath + "bottom\\" + std::to_string(i - 4);
			//copy the xml files to \\1 \\2 \\3 \\4
			std::string copyXML = "copy " + xmlBottomDir + "* " + currentPath;
			system(copyXML.c_str());
		}
		SetCurrentDirectory(currentPath.c_str());
		std::string fullDirPath = currentPath;
		int indexofCurrentDir = fullDirPath.find_last_of("\\");
		std::string currentDirName = fullDirPath.substr(indexofCurrentDir, fullDirPath.size()); // e.g. \\1 \\2 \\3 \\4
		std::string upperDirPath = fullDirPath.substr(0, indexofCurrentDir);

		int indexofUpperDir = upperDirPath.find_last_of("\\");
		std::string upperDirName = upperDirPath.substr(indexofUpperDir, upperDirPath.size());
		std::string logDirPath = fullDirPath.substr(0, indexofUpperDir);


		if (upperDirName == "\\top") // calibrate top laser
		{
			Calib::RunCalibrateCamera("in_VID5.xml", "out_camera_data.xml");
			Calib::RunCalibrateLaser("out_camera_data.xml", "_VID5.xml", "out_laser_camera.xml", "RawTransformationTable.bin");

			if (currentDirName == "\\5")
			{
				Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top\\1\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}
			else if (currentDirName == "\\6")
			{
				Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top\\3\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}
			else
			{
				Calib::RunStereoCalib("out_camera_data.xml", "config.xml", "out_stereo_data.xml");
				Calib::RunHandEyesCalib("out_stereo_data.xml", HAND_EYE_CALIBRATION_ROTATION_ANGLE, "out_handeyes_data.xml");
				Calib::RunTableChange("RawTransformationTable.bin", "out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}

			QString logInfo2 = QString("calib %1 top calibration fininshed :").arg(i);
			printLog(i, logInfo2);
		}

		else if (upperDirName == "\\bottom") // calibrate bottom laser
		{
			std::string outCameraDataFilePath = upperDirPath + "\\..\\top" + currentDirName + "\\out_camera_data.xml";
			cv::FileStorage fs(outCameraDataFilePath, cv::FileStorage::READ); // Read the settings
			cv::Mat cameraMatrix, distcofficients;
			fs["camera_matrix"] >> cameraMatrix;
			fs["distortion_coefficients"] >> distcofficients;

			Calib::RunCalibrateCamera("in_VID5.xml", "out_camera_data.xml", &cameraMatrix, &distcofficients);
			Calib::RunCalibrateLaser("out_camera_data.xml", "_VID5.xml", "out_laser_camera.xml", "RawTransformationTable.bin");

			if (currentDirName == "\\5")
			{
				Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top\\1\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}
			else if (currentDirName == "\\6")
			{
				Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top\\3\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}
			else
			{
				if (currentDirName == "\\2")
					Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top" + currentDirName + "\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin", 0);
				else
					Calib::RunTableChange("RawTransformationTable.bin", upperDirPath + "\\..\\top" + currentDirName + "\\out_handeyes_data.xml", "out_laser_camera.xml", -HAND_EYE_CALIBRATION_ROTATION_ANGLE / 2, 0, "transformationTable.bin");
			}

			QString logInfo3 = QString("calib %1 bottom calibration fininshed :").arg(i);
			printLog(i, logInfo3);
		}
		else
		{
			CV_Assert(false);
		}

		// copy *.bin file to output folder
		char buffer[256];
		std::string outBinFileName;
		if (upperDirName == "\\top")
		{
			outBinFileName = "transformationTable" + currentDirName.substr(1, 2) + ".bin";
		}
		else if (upperDirName == "\\bottom")
		{
			outBinFileName = "transformationTableBottom" + currentDirName.substr(1, 2) + ".bin";
		}
		else
		{
			CV_Assert(false);
		}

		std::string outDir = upperDirPath + "\\..\\cowa_cam_config\\aligned";
		std::ifstream in(fullDirPath + "\\transformationTable.bin", std::ios_base::in | std::ios_base::binary);
		std::ofstream out(outDir + "\\" + outBinFileName, std::ios_base::out | std::ios_base::binary);
		std::ofstream out2(outDir + "\\..\\" + outBinFileName, std::ios_base::out | std::ios_base::binary);
		while (!in.eof())
		{
			in.read(buffer, 256);       //���ļ��ж�ȡ256���ֽڵ����ݵ�������  
			int n = in.gcount();             //�������һ�в�֪��ȡ�˶����ֽڵ����ݣ������ú�������һ�¡�  
			out.write(buffer, n);       //д���Ǹ��ֽڵ�����  
			out2.write(buffer, n);
		}
		in.close();
		out.close();
		out2.close();

		outDir = upperDirPath + "\\..\\cowa_cam_config\\unaligned";
		in.open(fullDirPath + "\\RawTransformationTable.bin", std::ios_base::in | std::ios_base::binary);
		out.open(outDir + "\\" + outBinFileName, std::ios_base::out | std::ios_base::binary);
		while (!in.eof())
		{
			in.read(buffer, 256);			//���ļ��ж�ȡ256���ֽڵ����ݵ�������  
			int n = in.gcount();			//�������һ�в�֪��ȡ�˶����ֽڵ����ݣ������ú�������һ�¡�  
			out.write(buffer, n);			 //д���Ǹ��ֽڵ�����  
		}
		in.close();
		out.close();

		cv::destroyAllWindows();

		QString logInfo4 = QString("calib %1 camera-laser calibration finished :").arg(i);
		printLog(i, logInfo4);

	}

	///adb push .bin files to suitcase
	pushFiles();  //to be finished

}

void AutoCalibWidget::onTopFallLaserCalib()
{
	emit openTopFallLaser(); 
	savePath = "./images/top";
	for (int j = 0; j < FALLLASER; ++j)
	{
		qint32 expSmallPitchPos = smallpan_xPoint.at(j) * (1 << 17) / 360 * 40;
		qint32 expSmallYawPos = smallpan_zPoint.at(j)* (1 << 17) / 360 * 40;
		qint32 expSmallVel = 20 * (1 << 17) / 360 * 40;

		if (!smallPanTiltPtr->pitchP2P(expSmallPitchPos, expSmallVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("SmallPantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
			return;
		}
		smallPanTiltPtr->waitFinished(smallaxesIndex->at(0));

		if (!smallPanTiltPtr->yawP2P(expSmallYawPos, expSmallVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("SmallPantilt yawP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
			return;
		}
		smallPanTiltPtr->waitFinished(smallaxesIndex->at(1));

		onLightSwitch(true);
		sleep(3000);
		emit ReachLocation(2, false);
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;
		//sleep(3000);           

		onLightSwitch(false);
		sleep(3000);
		emit ReachLocation(10, true);
		while (!recvFinish)
		{
			sleep(100);
		}
		recvFinish = false;
	}

	onLightSwitch(true);

	smallPanSaveFinish = false;
	suitcaseMotion(1);
	sleep(3000);
	emit ReachLocation(2, false);       //Merge into a function
	while (!recvFinish)
	{
		sleep(100);
	}
	recvFinish = false;

	suitcaseMotion(2);
	sleep(3000);
	emit ReachLocation(2, false);       //Merge into a function
	while (!recvFinish)
	{
		sleep(100);
	}

	smallPanSaveFinish = true;
	suitcaseMotion(endPoint);
}

void AutoCalibWidget::clearVectorArray()
{
	xyz_xPoint_1.clear();
	xyz_xPoint_2.clear();
	xyz_yPoint_1.clear();
	xyz_yPoint_2.clear();
	xyz_zPoint_1.clear();
	xyz_zPoint_2.clear();
	xyz_rPoint_1.clear();
	xyz_rPoint_2.clear();

	smallpan_xPoint.clear();
	smallpan_zPoint.clear();

	bigpan_xPoint.clear();
	bigpan_zPoint.clear();

	sleep(3000);
	ui->processLog->clear();
}

void AutoCalibWidget::onMotionStart(){
	//go home
	/*xyzPtr->home(xyzaxesIndex->at(0));
	xyzPtr->home(xyzaxesIndex->at(1));
	xyzPtr->home(xyzaxesIndex->at(2));
	xyzPtr->home(xyzaxesIndex->at(3));
	xyzPtr->waitFinished(xyzaxesIndex->at(2));*/
	
	if (!isSmallBoardCalibrated)
	{
		//calib using the small board
		onSmallBoardMotion();
		//onSmallBoardMotionPro();       //not used yet
		//calib using the big board
		//onLargeBoardMotion();
	}
		
	else
	{
		if (isFallLaserCalibrate)
		{
			endPoint = 4;
			//calib using the big board
			onLargeBoardMotion();
			//fall laser calib
			onTopFallLaserCalib();       		
		}
		else
		{
			endPoint = 5;
			//calib using the big board
			//onLargeBoardMotion();
		}
	}

	//clear the vector array
	clearVectorArray();
	//onSmallBoardMotionPro();
	onStartBtnToggled(false);
	
	///copy files to another dir
	backUpFile();

	//calib the files
	//onCalibBtnClicked();
	
}

void AutoCalibWidget::onStartBtnToggled(bool checked){
	if (checked){
		if (!xyzPtr->enabled()){
			if (!xyzPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}
		if (!smallPanTiltPtr->enabled()){
			if (!smallPanTiltPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("SmallPantilt is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}
		if (!bigPanTiltPtr->enabled()){
			if (!bigPanTiltPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("BigPantilt is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}
		
		updateTimer->start(100);	

		QDateTime time = QDateTime::currentDateTime();					//��ȡϵͳ���ڵ�ʱ��
		ui->processLog->addItem(time.toString("[yyyy-MM-dd hh:mm:ss ddd]"));

		// gain -- exposure
		CMDParser::getInstance().setGainValue(5);													//set gain value of camera
		CMDParser::getInstance().setExposureValue(10);


		if (!motionThread.joinable())
			motionThread = std::thread(&AutoCalibWidget::onMotionStart, this);

	}
	else{

		if (!bigPanTiltPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("BigPantilt is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
		if (!smallPanTiltPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("SmallPantilt is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
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

void AutoCalibWidget::onStopBtnClicked(){

	//ui->processLog->clear();
	
	//close the light
	quint8 _data[4] = {0};
	MotionController::getInstance().setDigitalIO(_data, 4);
	
	//disconnect the controller
	onConnectClicked();
	
	//del all files in dir_images
	std::string dirPath = "./images/";
	File::delAllFiles(dirPath);
	
	exit(0);
}

cv::Mat AutoCalibWidget::QImage2cvMat(QImage image)
{
	cv::Mat mat;
	qDebug() << image.format();
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cv::cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}

void AutoCalibWidget::onUpdateTimerOut(){
	qint32 pos, vel;
	if (bigPanTiltPtr->readPitchVel(&vel)){
		vel = vel * 360 / (1 << 17) / 100;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read pitch velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (bigPanTiltPtr->readPitchPos(&pos)){
		pos = (pos - (*axesOffset)[0]) * 360 / (1 << 17) / 100;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read pitch position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (bigPanTiltPtr->readYawVel(&vel)){
		vel = vel * 360 / (1 << 17) / 100;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read yaw velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (bigPanTiltPtr->readYawPos(&pos)){
		pos = (pos - (*axesOffset)[1]) * 360 / (1 << 17) / 100;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read yaw position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}

	if (xyzPtr->readXVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 3;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_X velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readXPos(&pos)){
		pos = pos * M_PI * 23.87 / 10000 / 3;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_X position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readYVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 3;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Y velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readYPos(&pos)){
		pos = pos * M_PI * 23.87 / 10000 / 3;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Y position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readZVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 40;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Z velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readZPos(&pos)){
		pos = pos * M_PI * 23.87 / 10000 / 40;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Z position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readRVel(&vel)){
		vel = vel * 360 / 10000 / 120;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_R velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readRPos(&pos)){
		pos = pos * 360 / 10000 / 120;
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_R position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
}

#endif