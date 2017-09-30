#include "SensorWidget.h"
#include "network/network.h"
#include "switchbutton.h"
using namespace ECAT::Network;
SensorWidget::SensorWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) 
	:ui(new Ui::sensorWidget), QWidget(parent, flags)
{
	ui->setupUi(this);
	initUi();
	initSignals();
	cameraID = 0;	//default cameraID is 0
}

SensorWidget::~SensorWidget(){}

void SensorWidget::initUi(){ 

	ui->getValueBtn->hide();
	
}

void SensorWidget::initSignals(){
	connect(ui->displayBtn, SIGNAL(clicked()), this, SLOT(onDisplayBtnClicked()));
	connect(ui->camCombo, SIGNAL(activated(int)), this, SLOT(onCamComboActivated(int)));
	connect(ui->camCombo, SIGNAL(activated(int)), this, SIGNAL(updateCameraID(int)));
	connect(ui->triggerBtn, SIGNAL(clicked()), this, SLOT(onTriggerBtnClicked()));
	connect(ui->videoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onVideoCheckBoxStateChanged(int)));
	connect(ui->gainSpinBox, SIGNAL(editingFinished()), this, SLOT(onGainEditingFinished()));
	connect(ui->exposureSpinBox, SIGNAL(editingFinished()), this, SLOT(onExposureEditingFinished()));
	connect(ui->gainSlider, SIGNAL(sliderReleased()), this, SLOT(onGainSliderReleased()));
	connect(ui->exposureSlider, SIGNAL(sliderReleased()), this, SLOT(onExposureSliderReleased()));
	connect(&CMDParser::getInstance(), SIGNAL(connected()), this, SLOT(onNetworkConnected()));
	connect(&CMDParser::getInstance(), SIGNAL(disconnected()), this, SLOT(onNetworkDisconnected()));
	connect(&CMDParser::getInstance(), SIGNAL(camOpened(bool)), this, SLOT(onCamOpened(bool)));
	connect(&CMDParser::getInstance(), SIGNAL(camClosed(bool)), this, SLOT(onCamClosed(bool)));
	connect(&CMDParser::getInstance(), SIGNAL(gainValue(quint32)), this, SLOT(onGainValue(quint32)));
	connect(&CMDParser::getInstance(), SIGNAL(exposureValue(quint32)), this, SLOT(onExposureValue(quint32)));
	connect(&CMDParser::getInstance(), SIGNAL(recvImage(quint16, quint16, quint16, QByteArray)), this, SLOT(onRecvImage()));
	connect(ui->getValueBtn, SIGNAL(clicked()), this, SLOT(onGetValueBtnClicked()));
	connect(ui->laserBtn, SIGNAL(clicked()), this, SLOT(onLaserBtnClicked()));
}

void SensorWidget::onNetworkConnected(){
	setEnabled(true);
	ui->cameraSelectFrame->setEnabled(true);
	ui->gainExposureFrame->setEnabled(false);
}

void SensorWidget::onNetworkDisconnected(){
	ui->cameraSelectFrame->setEnabled(false);
	setEnabled(false);
}

void SensorWidget::onCamOpened(bool flag){
	ui->gainExposureFrame->setEnabled(flag);
	ui->triggerBtn->setDisabled(flag);
}

void SensorWidget::onCamClosed(bool flag){
	ui->gainExposureFrame->setDisabled(flag);
	ui->triggerBtn->setEnabled(flag);
}

void SensorWidget::onCamComboActivated(int index){
	cameraID = index;
	ui->laserBtn->setEnabled(true);
	CMDParser::getInstance().setLaserStatus(8);
}

void SensorWidget::onTriggerBtnClicked(){
	CMDParser::getInstance().triggerCamera(cameraID);
}

void SensorWidget::onGainSliderReleased(){
	CMDParser::getInstance().setGainValue(ui->gainSlider->value());
}

void SensorWidget::onExposureSliderReleased(){
	CMDParser::getInstance().setExposureValue(ui->exposureSlider->value());
}

void SensorWidget::onGainEditingFinished(){
	CMDParser::getInstance().setGainValue(ui->gainSpinBox->value());
}

void SensorWidget::onExposureEditingFinished(){
	CMDParser::getInstance().setExposureValue(ui->exposureSpinBox->value());
}

void SensorWidget::onGetValueBtnClicked(){
	ui->getValueBtn->setEnabled(false);
	CMDParser::getInstance().getExposureValue();
	CMDParser::getInstance().getGainValue();
}

void SensorWidget::onLaserBtnClicked(){
	ui->laserBtn->setEnabled(false);
	CMDParser::getInstance().closeLaser();
}

void SensorWidget::onGainValue(quint32 val){
	if (!ui->getValueBtn->isEnabled())
		ui->getValueBtn->setEnabled(true);
	ui->gainSpinBox->setValue(val);
}

void SensorWidget::onExposureValue(quint32 val){
	if (!ui->getValueBtn->isEnabled())
		ui->getValueBtn->setEnabled(true);
	ui->exposureSpinBox->setValue(val);
}

void SensorWidget::onDisplayBtnClicked(){
	ui->displayBtn->setEnabled(false);
	CMDParser::getInstance().requestImage({cameraID});
}

void SensorWidget::onVideoCheckBoxStateChanged(int state){
	emit captureModeChanged(state);
	if (Qt::Unchecked == state)
		ui->displayBtn->setEnabled(true);
}

void SensorWidget::onRecvImage(){
	if (!ui->videoCheckBox->isChecked())
		ui->displayBtn->setEnabled(true);
}
