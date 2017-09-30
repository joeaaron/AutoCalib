#ifndef _ECAT_GUI_MainWindow_H_
#define _ECAT_GUI_MainWindow_H_
#include "ui_mainwindow.h"
#include <QMainWindow>
#include "mytitlebar.h"
#include "NcFramelessHelper.h"

class MainWindow : public QMainWindow{
	Q_OBJECT
public:
	MainWindow( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
	~MainWindow();
	void initUi();
	void initSignals();
public slots:
	void onConnectAction();
	void onSetupAction();
	void onCalibrtionAction();
	void onAutoCalibAction();
	void onLaserDetectAction();

private:
	Ui::ECATMainWindow *ui_mainwindow;
	QWidget *networkWidget;
	QWidget *setupWidget;
	QWidget *calibrationWidget;
	QWidget *autocalibWidget;
	QWidget *laserdetectWidget;
	NcFramelessHelper* fh;
public:
	MyTitleBar *m_titleBar;
};
#endif