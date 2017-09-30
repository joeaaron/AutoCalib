#ifndef _ECAT_GUI_MotionWidget_H_
#define _ECAT_GUI_MotionWidget_H_
#include "ui_motion.h"
class MotionWidget:public QWidget
{
	Q_OBJECT
public:
	MotionWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~MotionWidget();
private:
	void initUi();
	void initSignals();
	void initVariables();
signals:
	void updatePosition(int);
	void updateVelocity(int);
private slots:
	void onMotionConnected();
	void onMotionDisconnected();
	void onAxisComboActivated(int);
	void onEnableBtnClicked();
	void onHomeBtnClicked();
	void onVelMoveBtnToggled(bool);
	void onPosMoveBtnToggled(bool);
	void onUpdateTimerOut();
	void onAngluarRadioBtnToggled(bool);
private:
	Ui::motionWidget* ui;
	quint32 axisID;
	QTimer* updateTimer;
	bool motionConnected;
};
#endif