#include "XYZInchingWidget.h"
#include <QMessageBox>
XYZInchingWidget::XYZInchingWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::xyzInchingWidget), 
	xyzPtr(new XYZ()),
	axesIndex(new QVector<int>(4,-1)),
	updateTimer(new QTimer(this))
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

XYZInchingWidget::~XYZInchingWidget(){}

void XYZInchingWidget::initUi(){
	QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
	for (auto btn : buttons)
		btn->setEnabled(false);
}

void XYZInchingWidget::initSignals(){
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onStartBtnToggled(bool)));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));

	QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
	for (auto btn : buttons)
		if (btn != ui->startBtn){
			connect(btn, SIGNAL(pressed()), this, SLOT(onDirectionBtnPressed()));
			connect(btn, SIGNAL(released()), this, SLOT(onDirectionBtnReleased()));
		}
}

void XYZInchingWidget::initVariables(){
	xyzPtr->setAxes(axesIndex);
}

void XYZInchingWidget::onMotionConnected(){
	ui->startBtn->setEnabled(true);
}

void XYZInchingWidget::onStartBtnToggled(bool checked){
	if (checked){
		if (!xyzPtr->enable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("XYZ is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			/*xyzPtr->home(axesIndex->at(0));
			xyzPtr->home(axesIndex->at(1));
			xyzPtr->home(axesIndex->at(2));
			xyzPtr->home(axesIndex->at(3));*/

			QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
			for (auto btn : buttons)
				btn->setEnabled(true);
			updateTimer->start(100);
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
			QList<QPushButton*> buttons = ui->controlBox->findChildren< QPushButton* >();
			for (auto btn : buttons)
				if (btn != ui->startBtn)
					btn->setEnabled(false);
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
	}
}

void XYZInchingWidget::updateAxisIndex(DEVICE_AXIS axis, int index){
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

void XYZInchingWidget::onDirectionBtnPressed(){
	QObject* sender = QObject::sender();
	qint32 linearVel = ui->lvelSpinBox->value();
	qint32 angularVel = ui->rvelSpinBox->value();
	if (sender == ui->rightBtn || sender == ui->leftBtn){
		// x+/x-
		if (sender == ui->leftBtn)
			linearVel = -linearVel;
		//need conversion to motor speed
		linearVel = linearVel * (1 << 17) / 45;
		if (!xyzPtr->moveX(linearVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Move X-Axis has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->backwardBtn || sender == ui->forwardBtn){
		// y+/ya-
		if (sender == ui->forwardBtn)
			linearVel = -linearVel;
		//need conversion to motor speed
		//linearVel = linearVel * 10000 / M_PI / 23.87 * 3;
		linearVel = linearVel * (1 << 17) / 20;
		if (!xyzPtr->moveY(linearVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Move Y-Axis has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->upBtn || sender == ui->downBtn){
		// z+/z-
		if (sender == ui->upBtn)
			linearVel = -linearVel;
		//need conversion to motor speed
		linearVel = linearVel * (1 << 17) / 10;
		if (!xyzPtr->moveZ(linearVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Move Z-Axis has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->counterClockwiseBtn || sender == ui->clockwiseBtn){
		// rotation z
		if (sender == ui->clockwiseBtn)
			angularVel = -angularVel;
		//angularVel = angularVel * 10000 / 360 * 120;
		angularVel = angularVel * (1 << 17) / 360 * 100;
		if (!xyzPtr->moveR(angularVel)){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Rotation Z-Axis has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else
		return;

}

void XYZInchingWidget::onDirectionBtnReleased(){
	QObject* sender = QObject::sender();
	if (sender == ui->rightBtn || sender == ui->leftBtn){
		// X
		if (!xyzPtr->stopX()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop X has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->backwardBtn || sender == ui->forwardBtn){
		// Y
		if (!xyzPtr->stopY()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop Y has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->upBtn || sender == ui->downBtn){
		// Z
		if (!xyzPtr->stopZ()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop Z has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else if (sender == ui->counterClockwiseBtn || sender == ui->clockwiseBtn){
		// R
		if (!xyzPtr->stopR()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Stop R has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
	}
	else
		return;
}

void XYZInchingWidget::onUpdateTimerOut(){
	qint32 pos, vel;
	// X-Axis
	if (xyzPtr->readXPos(&pos)){
		pos = pos * 45.0 / (1 << 17) ;
		ui->xPosLcdNum->display(pos);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read X position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}

	if (xyzPtr->readXVel(&vel)){
		vel = vel * 45.0 / (1 << 17);
		ui->xVelLcdNum->display(vel);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read X velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}
	// Y-Axis
	if (xyzPtr->readYPos(&pos)){
		pos = pos * 20.0 / (1 << 17);
		ui->yPosLcdNum->display(pos);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read Y position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}

	if (xyzPtr->readYVel(&vel)){
		vel = vel * 20.0 / (1 << 17);
		ui->yVelLcdNum->display(vel);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read Y velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}
	// Z-Axis
	if (xyzPtr->readZPos(&pos)){
		pos = pos * 10.0 / (1 << 17);
		ui->zPosLcdNum->display(pos);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read Z position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}

	if (xyzPtr->readZVel(&vel)){
		vel = vel * 10.0 / (1 << 17);
		ui->zVelLcdNum->display(vel);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read Z velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}
	// R-Axis
	if (xyzPtr->readRPos(&pos)){
		pos = pos * 360.0  / (1 << 17) / 100;
		ui->rPosLcdNum->display(pos);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read R position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}

	if (xyzPtr->readRVel(&vel)){
		vel = vel * 360.0 / (1 << 17) / 100;
		ui->rVelLcdNum->display(vel);
	}
	else{
		/*QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read R velocity has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);*/
	}
}