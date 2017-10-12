#include "network/motioncontroller.h"
#include "MotionWidget.h"
#include <QMessageBox>
using namespace ECAT::Network;
MotionWidget::MotionWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::motionWidget), QWidget(parent, flags), updateTimer(new QTimer(this))
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

MotionWidget::~MotionWidget(){
	if (motionConnected){
		int axisNum = ui->axisCombo->count();
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

void MotionWidget::initUi(){
	ui->axisBox->setEnabled(true);
	ui->tabWidget->setEnabled(true);
	ui->parameterBox->hide();
}

void MotionWidget::initSignals(){
	connect(ui->axisCombo, SIGNAL(activated(int)), this, SLOT(onAxisComboActivated(int)));
	connect(ui->enableBtn, SIGNAL(clicked()), this, SLOT(onEnableBtnClicked()));
	connect(ui->homeBtn, SIGNAL(clicked()), this, SLOT(onHomeBtnClicked()));
	connect(ui->velMoveBtn, SIGNAL(toggled(bool)), this, SLOT(onVelMoveBtnToggled(bool)));
	connect(ui->posMoveBtn, SIGNAL(toggled(bool)), this, SLOT(onPosMoveBtnToggled(bool)));
	//connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));
	connect(this,SIGNAL(updatePosition(int)),ui->posLcdNumber,SLOT(display(int)));
	connect(this,SIGNAL(updateVelocity(int)), ui->velLcdNumber, SLOT(display(int)));
	connect(this, SIGNAL(updatePosition(int)), ui->posLcdNumber_2, SLOT(display(int)));
	connect(this, SIGNAL(updateVelocity(int)), ui->velLcdNumber_2, SLOT(display(int)));
	connect(ui->angluarRadioBtn, SIGNAL(toggled(bool)), this, SLOT(onAngluarRadioBtnToggled(bool)));
	connect(ui->incrementCheckBox, &QCheckBox::toggled, [this](){
		if (ui->incrementCheckBox->isChecked()){
			ui->encoderBitSpinBox->setSuffix("p");
			ui->encoderBitSpinBox->setRange(0,20000);
			ui->encoderBitSpinBox->setValue(10000);
		}
		else{
			ui->encoderBitSpinBox->setSuffix("bits");
			ui->encoderBitSpinBox->setRange(1, 20);
			ui->encoderBitSpinBox->setValue(17);
		}
	});
}

void MotionWidget::initVariables(){
	axisID = 0;	//default axisID is 0
	motionConnected = false;
}

void MotionWidget::onMotionConnected(){
	setEnabled(true);
	ui->axisBox->setEnabled(true);
	motionConnected = true;
}

void MotionWidget::onMotionDisconnected(){
	ui->tabWidget->setEnabled(false);
	ui->axisBox->setEnabled(false);
	setEnabled(false);
	motionConnected = false;
}

void MotionWidget::onAxisComboActivated(int index){
	if (axisID != index && motionConnected){
		char flag;
		MOTION_ERROR err = MotionController::getInstance().readAxisStopped(axisID, &flag);
		if (MOTION_ERROR_NONE == err && !flag){
			err = MotionController::getInstance().stopAxis(axisID);
			if (MOTION_ERROR_NONE == err){
				ui->velMoveBtn->setChecked(false);
				ui->posMoveBtn->setChecked(false);
			}
		}
		err = MotionController::getInstance().readAxisEnabled(axisID, &flag);
		if (MOTION_ERROR_NONE == err && flag){
			err = MotionController::getInstance().disableAxis(axisID);
			if (MOTION_ERROR_NONE == err){
				ui->enableBtn->setText("ENABLE");
			}
		}
		axisID = index;
	}

}

void MotionWidget::onEnableBtnClicked(){
	if (ui->enableBtn->text() == "ENABLE"){
		MOTION_ERROR err = MotionController::getInstance().enableAxis(axisID);
		if (MOTION_ERROR_NONE == err){
			ui->enableBtn->setText("DISABLE");
			ui->tabWidget->setEnabled(true);
			ui->homeBtn->setEnabled(true);
		}
		else{
			QMessageBox::critical(this, 
								  tr("Motion Error"), 
								  QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								  );
			HANDLE_MOTION_ERROR(err);
		}
	}
	else{
		MOTION_ERROR err = MotionController::getInstance().disableAxis(axisID);
		if (MOTION_ERROR_NONE == err){
			ui->enableBtn->setText("ENABLE");
			ui->tabWidget->setEnabled(false);
			ui->homeBtn->setEnabled(false);
		}
		else{
			QMessageBox::critical(this,
								  tr("Motion Error"),
								  QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								 );
			HANDLE_MOTION_ERROR(err);
		}
	}

}

void MotionWidget::onHomeBtnClicked(){
	// encoder range(absolute:17bits,increment:10000 pulses default)
	qint32 encoderRange;
	if (ui->incrementCheckBox->isChecked())
		encoderRange = ui->encoderBitSpinBox->value();
	else
		encoderRange = (1 << ui->encoderBitSpinBox->value());
	// reduction ratio
	qint32 reductionRatio = ui->reductionRatioSpinBox->value();
	// circumference for linear mode is PI*diameter, for angular mode is 360 deg
	double circumference = ui->angluarRadioBtn->isChecked() ? 360 : (ui->diameterDoubleSpinBox->value());
	// only for absolute encoder, for increment encoder is zero
	qint32 offset = ui->incrementCheckBox->isChecked() ? 0 : ui->offsetSpinBox->value();
	// pos convert to motor pulse
	qint32 expPos = ui->offsetSpinBox->value();		//init encoder value
	expPos = expPos * encoderRange / circumference * reductionRatio + offset;
	// vel convert to motor pulse
	qint32 expVel = ui->referredVelSlider->value();
	expVel = expVel * encoderRange / circumference * reductionRatio;
	//
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axisID, expPos, expVel);
	if (MOTION_ERROR_NONE != err){
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		HANDLE_MOTION_ERROR(err);
		return;
	}
	updateTimer->start(100);
}

void MotionWidget::onVelMoveBtnToggled(bool checked){
	if (checked){
		if (ui->posMoveBtn->isChecked()){
			ui->posMoveBtn->setChecked(!checked);
			ui->velMoveBtn->setChecked(!checked);
			QMessageBox::warning(this,
				tr("Motion Warning"),
				QString("Warning: Motion can't be set both vel and pos mode!")
				);
			return;
		}
		// encoder range(absolute:17bits,increment:10000 pulses)
		qint32 encoderRange;											
		if (ui->incrementCheckBox->isChecked())
			encoderRange = ui->encoderBitSpinBox->value();
		else
			encoderRange = (1 << ui->encoderBitSpinBox->value());
		// reduction ratio
		qint32 reductionRatio = ui->reductionRatioSpinBox->value();
		// circumference for linear mode is PI*diameter, for angular mode is 360 deg
		//double circumference = ui->angluarRadioBtn->isChecked() ? 360 : M_PI*ui->diameterDoubleSpinBox->value();
		double circumference = ui->angluarRadioBtn->isChecked() ? 360 : ui->diameterDoubleSpinBox->value();
		// convert to motor pulse
		qint32 expVel = ui->velSlider->value();
		expVel = expVel * encoderRange / circumference * reductionRatio;
		
		MOTION_ERROR err = MotionController::getInstance().moveAxis(axisID, expVel);
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
								  tr("Motion Error"),
								  QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								 );
			HANDLE_MOTION_ERROR(err);
			return;
		}
		updateTimer->start(100);
	}
	else{
		MOTION_ERROR err = MotionController::getInstance().stopAxis(axisID);
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
								tr("Motion Error"),
								QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								);
			HANDLE_MOTION_ERROR(err);
			return;
		}
		//delay 500ms to leave axis velocity drop to zerot99999
		QTimer::singleShot(500, [this](){updateTimer->stop();});
	}
}

void MotionWidget::onPosMoveBtnToggled(bool checked){
	if (checked){
		if (ui->velMoveBtn->isChecked()){
			ui->posMoveBtn->setChecked(!checked);
			ui->velMoveBtn->setChecked(!checked);
			QMessageBox::warning(this,
				tr("Motion Warning"),
				QString("Warning: Motion can't be set both vel and pos mode!")
				);
			return;
		}
		// encoder range(absolute:17bits,increment:10000 pulses default)
		qint32 encoderRange;
		if (ui->incrementCheckBox->isChecked())
			encoderRange = ui->encoderBitSpinBox->value();
		else
			encoderRange = (1 << ui->encoderBitSpinBox->value());
		// reduction ratio
		qint32 reductionRatio = ui->reductionRatioSpinBox->value();
		// circumference for linear mode is PI*diameter, for angular mode is 360 deg
		double circumference = ui->angluarRadioBtn->isChecked() ? 360 : M_PI * ui->diameterDoubleSpinBox->value();
		// only for absolute encoder, for increment encoder is zero
		qint32 offset = ui->incrementCheckBox->isChecked()?0:ui->offsetSpinBox->value();
		// pos convert to motor pulse
		qint32 expPos = ui->posSlider->value();							
		expPos = expPos * encoderRange / circumference * reductionRatio + offset;
		// vel convert to motor pulse
		qint32 expVel = ui->referredVelSlider->value();					
		expVel = expVel * encoderRange / circumference * reductionRatio;
		//
		MOTION_ERROR err;
		err = MotionController::getInstance().p2pAxis(axisID, expPos, expVel);
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
								tr("Motion Error"),
								QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								);
			HANDLE_MOTION_ERROR(err);
			return;
		}
		updateTimer->start(100);
	}
	else{
		MOTION_ERROR err = MotionController::getInstance().stopAxis(axisID);
		if (MOTION_ERROR_NONE != err){
			QMessageBox::critical(this,
								tr("Motion Error"),
								QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
								);
			HANDLE_MOTION_ERROR(err);
			return;
		}
		//delay 500ms to leave axis velocity drop to zero
		QTimer::singleShot(500, [this](){updateTimer->stop(); });
	}
}

void MotionWidget::onUpdateTimerOut(){
	// encoder range(absolute:17bits,increment:10000 pulses default)
	qint32 encoderRange;
	if (ui->incrementCheckBox->isChecked())
		encoderRange = ui->encoderBitSpinBox->value();
	else
		encoderRange = (1 << ui->encoderBitSpinBox->value());
	// reduction ratio
	qint32 reductionRatio = ui->reductionRatioSpinBox->value();
	// circumference for linear mode is PI*diameter, for angular mode is 360 deg
	double circumference = ui->angluarRadioBtn->isChecked() ? 360 : M_PI * ui->diameterDoubleSpinBox->value();
	// only for absolute encoder, for increment encoder is zero
	qint32 offset = ui->incrementCheckBox->isChecked() ? 0 : ui->offsetSpinBox->value();

	qint32 pos, vel;
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axisID, &pos);
	if (MOTION_ERROR_NONE == err)
		emit updatePosition((pos - offset) * circumference / encoderRange / reductionRatio);
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		HANDLE_MOTION_ERROR(err);
	}
	err = MotionController::getInstance().readAxisVelocity(axisID, &vel);
	if (MOTION_ERROR_NONE == err)
		emit updateVelocity(vel * circumference / encoderRange / reductionRatio);
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		HANDLE_MOTION_ERROR(err);
	}
}

void MotionWidget::onAngluarRadioBtnToggled(bool checked){
	auto changeVelModeUI = [this](int range,
						int step, 
						int initValue,
						QString velUnit,
						QString posUnit){

		ui->velSpinBox->setSuffix(velUnit);
		ui->velSpinBox->setRange(-range, range);
		ui->velSpinBox->setSingleStep(step);
		ui->velSpinBox->setValue(initValue);

		ui->velSlider->setRange(-range, range);
		ui->velSlider->setSingleStep(step);
		ui->velSlider->setValue(initValue);

		ui->velLabel->setText(velUnit);
		ui->posLabel->setText(posUnit);
	};
	auto changePosModeUI = [this](int velRange,
						int posRange,
						int step,
						int initValue,
						QString velUnit,
						QString posUnit){

		ui->posSpinBox->setSuffix(posUnit);
		ui->posSpinBox->setRange(-posRange, posRange);
		ui->posSpinBox->setSingleStep(step);
		ui->posSpinBox->setValue(initValue);

		ui->posSlider->setRange(-posRange, posRange);
		ui->posSlider->setSingleStep(step);
		ui->posSlider->setValue(initValue);

		ui->referredVelSpinBox->setSuffix(velUnit);
		ui->referredVelSpinBox->setRange(0, velRange);
		ui->referredVelSpinBox->setSingleStep(step);
		ui->referredVelSpinBox->setValue(velRange / 2);

		ui->referredVelSlider->setRange(0, velRange);
		ui->referredVelSlider->setSingleStep(step);
		ui->referredVelSlider->setValue(velRange / 2);

		ui->velLabel_2->setText(velUnit);
		ui->posLabel_2->setText(posUnit);
	};
	if (!checked){
		// linear velocity 
		int velRange = 100, posRange = 500, step = 1, value = 0;
		QString velUnit("mm/s"), posUnit("mm");
		changeVelModeUI(velRange, step, value, velUnit, posUnit);
		changePosModeUI(velRange, posRange, step, value, velUnit, posUnit);
	}
	else{
		// angular velocity
		int velRange = 18, posRange = 180, step = 1, value = 0;
		QString velUnit("deg/s"), posUnit("deg");
		changeVelModeUI(velRange, step, value, velUnit, posUnit);
		changePosModeUI(velRange, posRange, step, value, velUnit, posUnit);
	}
}
