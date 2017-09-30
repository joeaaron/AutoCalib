#include "MainWindow.h"
#include "NetworkWidget.h"
#include "SetupWidget.h"
#include "CalibrationWidget.h"
#include "AutoCalibWidget.h"
#include "LaserWidget.h"

MainWindow::MainWindow(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */):
					   QMainWindow(parent, flags), 
					   ui_mainwindow(new Ui::ECATMainWindow),
					   m_titleBar(NULL)
{
	ui_mainwindow->setupUi(this);
	//windowflag
	//this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	//拖动和缩放
	//fh = new NcFramelessHelper;
	//fh->activateOn(this);

	//init UI
	initUi();
	//init signals and slots
	initSignals();
}
MainWindow::~MainWindow(){
	if (networkWidget)
		delete networkWidget;
	if (setupWidget)
		delete setupWidget;
	if (calibrationWidget)
		delete calibrationWidget;
	if (laserdetectWidget)
		delete laserdetectWidget;
	if (autocalibWidget)
		delete autocalibWidget;
	
	//delete fh;
}

void MainWindow::initUi(){


#if 0	
	//init titleBar
	m_titleBar = new MyTitleBar(this);
	installEventFilter(m_titleBar);
	//m_titleBar->setTitleRoll();
	m_titleBar->setTitleIcon("./images/cowa.png");
	//m_titleBar->setStyleSheet("./qdarkstyle/style.qss");
	m_titleBar->setTitleContent(QStringLiteral("前行之路还需前行"));
	m_titleBar->setButtonType(ONLY_CLOSE_BUTTON);
	m_titleBar->setTitleWidth(this->width());

	QVBoxLayout *pLayout = new QVBoxLayout();
	pLayout->addWidget(m_titleBar, 0, Qt::AlignTop));
	pLayout->addStretch();
	pLayout->setSpacing(0);
	pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(pLayout);
#endif
	
	//set up Network widget 
	networkWidget = new NetworkWidget();
	//set up setup widget
	setupWidget = new SetupWidget();
	//set up calibration widget
	calibrationWidget = new CalibrationWidget();
	//set up autocalib widget
	autocalibWidget = new AutoCalibWidget();
    //set up laserdetect widget
	laserdetectWidget = new LaserWidget();
	//set up init widget
	setCentralWidget(networkWidget);
	

}

void MainWindow::initSignals(){
	connect(ui_mainwindow->connectAct, SIGNAL(triggered()), this, SLOT(onConnectAction()));
	connect(ui_mainwindow->setupAct, SIGNAL(triggered()), this, SLOT(onSetupAction()));
	connect(ui_mainwindow->calibrationAct, SIGNAL(triggered()), this, SLOT(onCalibrtionAction()));
	connect(ui_mainwindow->autocalibAct, SIGNAL(triggered()), this, SLOT(onAutoCalibAction()));
	connect(ui_mainwindow->laserdetectAct, SIGNAL(triggered()), this, SLOT(onLaserDetectAction()));
	connect(networkWidget, SIGNAL(motionConnected()), setupWidget, SIGNAL(motionConnected())); 
	connect(networkWidget, SIGNAL(motionDisconnected()), setupWidget, SIGNAL(motionDisconnected()));
	connect(networkWidget, SIGNAL(motionConnected()), calibrationWidget, SIGNAL(motionConnected()));
	connect(networkWidget, SIGNAL(motionDisconnected()), calibrationWidget, SIGNAL(motionDisconnected()));
	connect(autocalibWidget, SIGNAL(Connect()), networkWidget, SIGNAL(autocontrolconnect()));
}

void MainWindow::onConnectAction(){
	if (centralWidget())
		takeCentralWidget();
	setCentralWidget(networkWidget);
}
void MainWindow::onSetupAction(){
	if (centralWidget())
		takeCentralWidget();
	setCentralWidget(setupWidget);
}
void MainWindow::onCalibrtionAction(){
	if (centralWidget())
		takeCentralWidget();
	setCentralWidget(calibrationWidget);
}
void MainWindow::onAutoCalibAction(){
	if (centralWidget())
		takeCentralWidget();
	setCentralWidget(autocalibWidget);
}
void MainWindow::onLaserDetectAction(){
	if (centralWidget())
		takeCentralWidget();
	setCentralWidget(laserdetectWidget);
}