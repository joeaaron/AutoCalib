#include "XYZP2PWidget.h"
#include <QMessageBox>
XYZP2PWidget::XYZP2PWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::xyzP2PWidget), 
	xyzPtr(new XYZ()),
	axesIndex(new QVector<int>(4, -1)),
	axesOffset(new QVector<int>(4, 0)),
	updateTimer(new QTimer(this))
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

XYZP2PWidget::~XYZP2PWidget(){}

void XYZP2PWidget::initUi(){
	ui->controllBox->setEnabled(false);
}

void XYZP2PWidget::initSignals(){
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onStartBtnToggled(bool)));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));
}

void XYZP2PWidget::initVariables(){
	xyzPtr->setAxes(axesIndex);
}

void XYZP2PWidget::onMotionConnected(){
	ui->controllBox->setEnabled(true);
}

void XYZP2PWidget::onStartBtnToggled(bool checked){
	if (checked){
		if (!xyzPtr->enabled()){
			if (!xyzPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
			else
			{
				xyzPtr->home(axesIndex-> at(0));
				xyzPtr->home(axesIndex-> at(1));
				xyzPtr->home(axesIndex-> at(2));
				xyzPtr->home(axesIndex-> at(3));
			}
		}
		updateTimer->start(100);
		//
		/*qint32 expXPos = ui->xPosSlider->value() * 10000 / M_PI / 23.87 * 3;
		qint32 expYPos = ui->yPosSlider->value() * 10000 / M_PI / 23.87 * 3;
		qint32 expZPos = ui->zPosSlider_2->value() * 10000 / M_PI / 23.87 * 40;
		qint32 expRPos = ui->rPosSlider->value() * 10000 / 360 * 120;
		qint32 expVel = ui->refVelSlider->value() * 10000 / M_PI / 23.87 * 3;*/
		qint32 expXPos = ui->xPosSlider->value() * (1 << 17) / 45 * 60;
		qint32 expYPos = ui->yPosSlider->value()* (1 << 17) / 20 * 60;
		qint32 expZPos = ui->zPosSlider_2->value() * (1 << 17) / 10 * 60;
		qint32 expRPos = ui->rPosSlider->value() * (1 << 17) / 360 * 100 * 60;
		qint32 expVel = ui->refVelSlider->value() * (1 << 17) / 45 * 60;
		if (ui->axisXCheckBox->isChecked()){
			if (!xyzPtr->moveXP2P(expXPos, expVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ moveXP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}
		if (ui->axisYCheckBox->isChecked())
			if (!xyzPtr->moveYP2P(expYPos, expVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ moveYP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		if (ui->axisZCheckBox_2->isChecked())
			if (!xyzPtr->moveZP2P(expZPos, expVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ moveZP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		if (ui->axisRCheckBox->isChecked())
			if (!xyzPtr->moveRP2P(expRPos, expVel)){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("XYZ moveRP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
	
	}
	else{
		if (!xyzPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("XYZ is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
	}
}

void XYZP2PWidget::updateAxisIndex(DEVICE_AXIS axis, int index){
	if (DEVICE_AXIS::XYZ_X == axis)
		(*axesIndex)[0] = index;
	else if (DEVICE_AXIS::XYZ_Y == axis)
		(*axesIndex)[1] = index;
	else if (DEVICE_AXIS::XYZ_Z == axis)
		(*axesIndex)[2] = index;
	else if (DEVICE_AXIS::XYZ_R == axis)
		(*axesIndex)[3] = index;
	else
		return;
}

void XYZP2PWidget::updateAxisOffset(DEVICE_AXIS axis, int offset){
	if (DEVICE_AXIS::XYZ_X == axis)
		(*axesOffset)[0] = offset;
	else if (DEVICE_AXIS::XYZ_Y == axis)
		(*axesOffset)[1] = offset;
	else if (DEVICE_AXIS::XYZ_Z == axis)
		(*axesOffset)[2] = offset;
	else if (DEVICE_AXIS::XYZ_R == axis)
		(*axesOffset)[3] = offset;
	else
		return;
}

void XYZP2PWidget::onUpdateTimerOut(){
	qint32 pos, vel;
	if (xyzPtr->readXVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 3;
		ui->xVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_X velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readYVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 3;
		ui->yVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Y velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readZVel(&vel)){
		vel = vel * M_PI * 23.87 / 10000 / 40;
		ui->zVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Z velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readRVel(&vel)){
		vel = vel * 360 / 10000 / 120;
		ui->rVelLcdNum->display(vel);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_R velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readXPos(&pos)){
		pos = (pos - (*axesOffset)[0]) * M_PI * 23.87 / 10000 / 3;
		ui->xPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_X position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readYPos(&pos)){
		pos = (pos - (*axesOffset)[1]) * M_PI * 23.87 / 10000 / 3;
		ui->yPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Y position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readZPos(&pos)){
		pos = (pos - (*axesOffset)[1]) * M_PI * 23.87 / 10000 / 40;
		ui->zPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_Z position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	//
	if (xyzPtr->readRPos(&pos)){
		pos = (pos - (*axesOffset)[1]) * 10000 / 360 * 120;
		ui->rPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read XYZ_R position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
}