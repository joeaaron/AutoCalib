#include "InchingWidget.h"
#include <QMessageBox>
InchingWidget::InchingWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::inchingWidget), 
	panTiltPtr(new PanTilt()), 
	axesIndex(make_shared<QVector<int>>(2, -1)),
	axesOffset(make_shared<QVector<int>>(2, 0)),
	updateTimer(new QTimer(this))
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

InchingWidget::~InchingWidget(){
}

void InchingWidget::initUi(){
	QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
	for (auto btn : buttons)
		btn->setEnabled(false);
}

void InchingWidget::initSignals(){
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onStartBtnToggled(bool)));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));
	connect(ui->UDcheckBox, SIGNAL(stateChanged(int)), this, SLOT(onUDCheckBoxStateChanged(int)));
	QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
	for (auto btn:buttons)
		if (btn != ui->startBtn){
			connect(btn, SIGNAL(pressed()), this, SLOT(onDirectionBtnPressed()));
			connect(btn, SIGNAL(released()), this, SLOT(onDirectionBtnReleased()));
		}

}

void InchingWidget::initVariables(){
	panTiltPtr->setAxes(axesIndex);
	vel = ui->velSlider->value()*(1 << 17) / 360 * 100;		//convert to pulse/s if given a 17-bits coder
}

void InchingWidget::onMotionConnected(){
	ui->startBtn->setEnabled(true);
}

void InchingWidget::onStartBtnToggled(bool checked){
	if (checked){
		if (!panTiltPtr->enable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Pantilt is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
			for (auto btn : buttons)
				btn->setEnabled(true);
			updateTimer->start(100);
		}
	}
	else{
		if (!panTiltPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Pantilt is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
			for (auto btn : buttons)
				if (btn != ui->startBtn)
					btn->setEnabled(false);
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop();});
		}
	}

}

void InchingWidget::updateAxisIndex(DEVICE_AXIS axis, int index){
	if (DEVICE_AXIS::BIGPANTILT_X == axis || DEVICE_AXIS::SMALLPANTILT_X == axis)
		(*axesIndex)[0] = index;
	else if (DEVICE_AXIS::BIGPANTILT_Z == axis || DEVICE_AXIS::SMALLPANTILT_Z == axis)
		(*axesIndex)[1] = index;
	else
		return;
}

void InchingWidget::updateAxisOffset(DEVICE_AXIS axis, int offset){
	if (DEVICE_AXIS::BIGPANTILT_X == axis || DEVICE_AXIS::SMALLPANTILT_X == axis)
		(*axesOffset)[0] = offset;
	else if (DEVICE_AXIS::BIGPANTILT_Z == axis || DEVICE_AXIS::SMALLPANTILT_Z == axis)
		(*axesOffset)[1] = offset;
	else
		return;
}

void InchingWidget::onDirectionBtnPressed(){
	QObject* sender = QObject::sender();

	if (sender == ui->upBtn){ 
		//if (!panTiltPtr->pitchP2P(5, -vel))
		if(!panTiltPtr->pitch(-vel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Pitch up has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->downBtn){
		if(!panTiltPtr->pitch(vel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Pitch down has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->leftBtn){
		if (!panTiltPtr->yaw(-vel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Yaw left has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->rightBtn){
		if (!panTiltPtr->yaw(vel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Yaw right has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);

		}
	}
	else
		return;
}

void InchingWidget::onDirectionBtnReleased(){
	QObject* sender = QObject::sender();
	if (sender == ui->upBtn || sender == ui->downBtn){
		if (!panTiltPtr->stopPitch()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop pitch has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->leftBtn || sender == ui->rightBtn){
		if (!panTiltPtr->stopYaw()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop yaw has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
}

void InchingWidget::onUpdateTimerOut(){
	qint32 pos, vel;
	if (panTiltPtr->readPitchVel(&vel)){
		vel = vel * 360 / (1 << 17) / 100;
		ui->xVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read pitch velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (panTiltPtr->readYawVel(&vel)){
		vel = vel * 360 / (1 << 17) / 100;
		ui->zVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read yaw velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (panTiltPtr->readPitchPos(&pos)){
		pos = (pos - (*axesOffset)[0])*360 / (1 << 17) / 100;
		ui->xPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read pitch position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (panTiltPtr->readYawPos(&pos)){
		pos = (pos - (*axesOffset)[1])*360 / (1 << 17) / 100;
		ui->zPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read yaw position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}

}

void InchingWidget::onUDCheckBoxStateChanged(int state){
	if (Qt::Checked == state)
		vel = -vel;	
	if (Qt::Unchecked == state)
		vel = -vel;
}
