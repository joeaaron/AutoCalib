#include "NetworkWidget.h"
#include <QMessageBox>
NetworkWidget::NetworkWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */) 
	:ui(new Ui::networkPage), QWidget(parent, flags), heartBeatTimer(new QTimer(this))
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

NetworkWidget::~NetworkWidget(){
	if (isMotionConnect){
		int axisNum = 7;
		for (int i = 0; i < axisNum; i++){   
			char flag;
			MOTION_ERROR err = MotionController::getInstance().readAxisStopped(i, &flag);
			if (MOTION_ERROR_NONE == err && !flag)
				MotionController::getInstance().stopAxis(i);
			err = MotionController::getInstance().readAxisEnabled(i, &flag);
			if (MOTION_ERROR_NONE == err && flag)
				MotionController::getInstance().disableAxis(i);
		}
		MotionController::getInstance().releaseControl();
	}
}

void NetworkWidget::initUi(){
	// init sensor host 127.0.0.1
	ui->visionHostCombo->addItem("192.168.1.100");
	ui->visionHostCombo->addItem("192.168.1.101");
	ui->visionHostCombo->addItem("192.168.1.102");
	ui->visionHostCombo->addItem("192.168.1.103");
	ui->visionHostCombo->addItem("192.168.1.104");
	ui->visionHostCombo->addItem("192.168.1.105");
	ui->visionHostCombo->addItem("192.168.1.106");
	ui->visionHostCombo->addItem("192.168.1.107");
	ui->visionHostCombo->addItem("192.168.1.108");
	ui->visionHostCombo->addItem("192.168.1.109");
	ui->visionHostCombo->addItem("192.168.1.110");
	ui->visionHostCombo->addItem("127.0.0.1");
	// init sensor port 6000
	ui->visionPortLineEdit->setValidator(new QIntValidator(1, 65535, this));
	ui->visionPortLineEdit->setText("6000");
	// init motion controller host 127.0.0.1
	ui->motionHostCombo->addItem("127.0.0.1");
	// init motion controller port 9000
	ui->motionPortLineEdit->setText("5");
}

void NetworkWidget::initVariables(){
	isMotionConnect = false;
}

void NetworkWidget::initSignals(){
	// vision
	connect(ui->visionUSBRedirectBtn, SIGNAL(clicked()), this, SLOT(onUSBRedirectClicked()));
	connect(ui->visionConnectBtn, SIGNAL(clicked()), this, SLOT(onConnectBtnClicked()));
	connect(ui->visionOpenCamBtn, SIGNAL(clicked()), this, SLOT(onOpencamBtnClicked()));
	connect(ui->wifiCheckBox, SIGNAL(clicked()), this, SLOT(onWifiChecked()));
	connect(&CMDParser::getInstance(), SIGNAL(connected()), this, SLOT(onHandleConnection()));
	connect(&CMDParser::getInstance(), SIGNAL(disconnected()), this, SLOT(onHandleDisconnection()));
	connect(&CMDParser::getInstance(), SIGNAL(socketerror(QString)), this, SLOT(onHandleSocketError(QString)));
	connect(&CMDParser::getInstance(), SIGNAL(camOpened(bool)), this, SLOT(onCamOpened(bool)));
	connect(&CMDParser::getInstance(), SIGNAL(camClosed(bool)), this, SLOT(onCamClosed(bool)));
	connect(this, SIGNAL(autodirect()), ui->visionUSBRedirectBtn, SIGNAL(clicked()));
	connect(this, SIGNAL(autocamconnect()), ui->visionConnectBtn, SIGNAL(clicked()));
	connect(this, SIGNAL(autocamopen()), ui->visionOpenCamBtn, SIGNAL(clicked()));
	connect(this, SIGNAL(autocontrolconnect()), ui->motionConnectBtn, SIGNAL(clicked()));
	connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(onHeartBeatTimeOut()));
	// motion
	connect(ui->motionConnectBtn, SIGNAL(clicked()), this, SLOT(onMotionConnectBtn()));
}
// vision
void NetworkWidget::onUSBRedirectClicked(){
	QString port = ui->visionPortLineEdit->text();
	QString adbcmd = "adb forward tcp:"+port+" tcp:"+port;
	QString output;
	bool ret = CMDParser::getInstance().adbCommand(adbcmd, output);
	if (ret){
		ui->visionConnectBtn->setEnabled(true);
		ui->visionUSBRedirectBtn->setEnabled(false);
		//QMessageBox::information(this, tr("USBDirect"), tr("Direct Successful!"));
	}
	else
		QMessageBox::critical(this, tr("Adb Error"), tr("The following error occurred: %1.").arg(output));
}

void NetworkWidget::onConnectBtnClicked(){
	auto lambda = [this](){
		if ("OPEN_CAM" == ui->visionOpenCamBtn->text())
			CMDParser::getInstance().exitNetwork();
		else
			QMessageBox::warning(this, tr("CAM Error"), tr("The following error occurred:close CAM first before disconnection."));
	};
	ui->visionConnectBtn->setEnabled(false);
	if ("CONNECT" == ui->visionConnectBtn->text()){
		CMDParser::getInstance().setupNetwork(ui->visionHostCombo->currentText(), ui->visionPortLineEdit->text().toInt());
		QString ipAddress = ui->visionHostCombo->currentText();
		emit synchroIP(ipAddress);
	}
		
	else{
		if ("OPEN_CAM" == ui->visionOpenCamBtn->text()){
			CMDParser::getInstance().exitNetwork();
			return;
		}
		else{
			//before disconnect from sensor, close CAM first
			CMDParser::getInstance().closeCam();
			QTimer::singleShot(1000, lambda);
		}
	}
}

void NetworkWidget::onOpencamBtnClicked(){
	ui->visionOpenCamBtn->setEnabled(false);
	if ("OPEN_CAM" == ui->visionOpenCamBtn->text())
		CMDParser::getInstance().openCam();
	else
		CMDParser::getInstance().closeCam();
}

void NetworkWidget::onCamOpened(bool bflag){
	if (bflag){
		ui->visionOpenCamBtn->setText("CLOSE_CAM");
		ui->visionOpenCamBtn->setEnabled(true);
		QMessageBox::information(this, tr("OPEN_CAM"), tr("CameraOpened Successful!"));
	}
	else{
		QMessageBox::warning(this, tr("CAM Error"), tr("The following error occurred:can't open CAM."));
	}
}

void NetworkWidget::onCamClosed(bool bflag){
	if (bflag){
		ui->visionOpenCamBtn->setText("OPEN_CAM");
		ui->visionOpenCamBtn->setEnabled(true);
	}
	else{
		QMessageBox::warning(this, tr("CAM Error"), tr("The following error occurred:can't close CAM."));
	}
}

void NetworkWidget::onHandleConnection(){
	//if cmdparse is connected, change state of connectBtn
	ui->visionConnectBtn->setEnabled(true);
	ui->visionConnectBtn->setText("DISCONNECT");
	//if cmdparse is connected, change state of opencamBtn
	//ui->visionOpenCamBtn->setEnabled(true);
	QMessageBox::information(this, tr("Connect"), tr("CameraConnection Successful!"));
	heartBeatTimer->start(1000);
	CMDParser::getInstance().openCam();
}

void NetworkWidget::onHandleDisconnection(){
	//if cmdparse is disconnected, change state of connectBtn
	ui->visionConnectBtn->setEnabled(true);
	ui->visionConnectBtn->setText("CONNECT");
	//if cmdparse is connected, change state of opencamBtn
	ui->visionOpenCamBtn->setEnabled(false);
	QMessageBox::information(this, tr("Connect"), tr("CameraConnection Failed!"));
}

void NetworkWidget::onHandleSocketError(QString errorstr){
	QMessageBox::critical(this, tr("Socket Error"), tr("The following error occurred: %1.").arg(errorstr));
	ui->visionConnectBtn->setEnabled(true);
}

void NetworkWidget::onWifiChecked(){
	if (ui->wifiCheckBox->isChecked())
	{
		ui->visionUSBRedirectBtn->setEnabled(false);
		ui->visionConnectBtn->setEnabled(true);
	}
	else
	{
		ui->visionUSBRedirectBtn->setEnabled(true);
		ui->visionConnectBtn->setEnabled(false);
	}
}

// motion
void NetworkWidget::onMotionConnectBtn(){
	ui->motionConnectBtn->setEnabled(false);
	if ("CONNECT" == ui->motionConnectBtn->text()){
		// connect to controller
		MOTION_ERROR err = MotionController::getInstance().setupNetwork(ui->motionHostCombo->currentText(),
																		ui->motionPortLineEdit->text().toInt());
		if (MOTION_ERROR_NONE != err && MOTION_ERROR_BUSY != err){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Error:failed to connect to rpc server, occurred at line number %1 in function %2 in %3 file.")
				.arg(__LINE__).arg(__FUNCTION__).arg(QString(__FILE__))
				);
			HANDLE_MOTION_ERROR(err);
			emit  motionDisconnected();
			return;
		}
		// take the control of controller
		err = MotionController::getInstance().takeControl();
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Error:control taken by other process, occurred at line number %1 in function %2 in %3 file.")
				.arg(__LINE__).arg(__FUNCTION__).arg(QString(__FILE__))
				);
			HANDLE_MOTION_ERROR(err);
			ui->motionConnectBtn->setEnabled(true);
			return;
		}
		else{
			ui->motionConnectBtn->setText("DISCONNECT");
			ui->motionConnectBtn->setEnabled(true);
			emit motionConnected();
			isMotionConnect = true;
			QMessageBox::information(this, tr("Connect"), tr("ControlConnection Successful!"));
		}
	}
	else{
		// release the control of controller
		MOTION_ERROR err = MotionController::getInstance().releaseControl();
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Error:failed to release control, occurred at line number %1 in function %2 in %3 file.")
				.arg(__LINE__).arg(__FUNCTION__).arg(QString(__FILE__))
				);
			HANDLE_MOTION_ERROR(err);
			ui->motionConnectBtn->setEnabled(true);
			return;
		}
		else{
			ui->motionConnectBtn->setText("CONNECT");
			ui->motionConnectBtn->setEnabled(true);
			emit  motionDisconnected();
			isMotionConnect = false;
			QMessageBox::information(this, tr("Connect"), tr("ControlConnection Failed!"));
		}
	}
}

void NetworkWidget::onHeartBeatTimeOut(){
	CMDParser::getInstance().heartbeat();
}