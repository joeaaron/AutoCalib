#include "SetupWidget.h"
#include "SensorWidget.h"
#include "MotionWidget.h"
#include <QMessageBox>
#include "network/network.h"
using namespace ECAT::Network;
SetupWidget::SetupWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)\
	:ui(new Ui::setupPage), QWidget(parent, flags)
{
	ui->setupUi(this);
	initUi();
	initSignals();
	bVideo = false;	//default mode is capturing image
	cameraID = 0;	//default cameraID is 0
}
SetupWidget::~SetupWidget(){}
void SetupWidget::initUi(){
	// Init sensor pane
	panes.push_back(new PaneWidget(ui->panesAreaContents, "Sensor", ":/icons/sensor.png",true,false));
	sensorWidget = new SensorWidget(panes[0]);
	panes[0]->setWidget(sensorWidget,false);
	panes[0]->expand();
	ui->panesLayout->addWidget(panes[0]);
	//panes[0]->expand();
	//Init another pane
	panes.push_back(new PaneWidget(ui->panesAreaContents, "Motion", ":/icons/motion.png",true,false));
	motionWidget = new MotionWidget(panes[1]);
	panes[1]->setWidget(motionWidget, false);
	ui->panesLayout->addWidget(panes[1]);
	//panes[1]->expand();
	ui->panesLayout->setAlignment(Qt::AlignTop);
	ui->panesLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	//Init display Frame
	displayView = new DisplayView(ui->displayFrame);
	ui->displayLayout->addWidget(displayView, 0, 0, 1, 1);
	displayView->show();
}

void SetupWidget::initSignals(){
	connect(&CMDParser::getInstance(), SIGNAL(recvImage(quint16, quint16, quint16, QByteArray)), 
			this, SLOT(onRecvImage(quint16, quint16, quint16, QByteArray)));
	connect(&CMDParser::getInstance(), SIGNAL(cameraTriggered(bool)), this, SLOT(onCameraTriggered(bool)));
	connect(sensorWidget, SIGNAL(captureModeChanged(int)), this, SLOT(onCaptureModeChanged(int)));
	connect(sensorWidget, SIGNAL(updateCameraID(int)), this, SLOT(onUpdateCameraID(int)));
	connect(this, SIGNAL(motionConnected()), motionWidget, SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionDisconnected()), motionWidget, SLOT(onMotionDisconnected()));
}

void SetupWidget::onRecvImage(quint16 camera, quint16 width, quint16 height, QByteArray imageData){
	QImage image = QImage((uchar*)imageData.data(), width, height, QImage::Format_Indexed8);
	if (image.isNull())
		return;
	else{
		//if (cameraID >= 4)
		//	return;
		if (camera == cameraID){
			displayView->showImage(image);
			if (bVideo)
				CMDParser::getInstance().requestImage();
		}
	}
}

void SetupWidget::onCaptureModeChanged(int state){
	bVideo = (bool)state;
}

void SetupWidget::onUpdateCameraID(int ID){
	cameraID = ID;
}

void SetupWidget::onCameraTriggered(bool flag){
	if (flag)
		QMessageBox::information(this, 
							tr("Camera Triggered Successfully"), 
							tr("You must power off and restart the sensor before triggered camera being valid.")
							);
	else
		QMessageBox::critical(this,
		tr("Camera Triggered Unsuccessfully"),
		tr("Try again.")
		);
}
