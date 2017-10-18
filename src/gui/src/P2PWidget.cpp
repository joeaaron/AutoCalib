#include "P2PWidget.h"
#include <QMessageBox>
#include <QFileDialog>


P2PWidget::P2PWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::p2pWidget),
	panTiltPtr(new PanTilt()),
	axesIndex(new QVector<int>(2, -1)),
	axesOffset(new QVector<int>(2, 0)),
	updateTimer(new QTimer(this))
	//formPointptr(new Form())
{
	ui->setupUi(this);
	initUi();
	initSignals();
	initVariables();
}

P2PWidget::~P2PWidget(){}

void P2PWidget::initUi(){
	//ui->controllBox->setEnabled(false);
	ui->xPosSlider->hide();
	ui->zPosSlider->hide();
}

void P2PWidget::initSignals(){
	connect(ui->startBtn, SIGNAL(toggled(bool)), this, SLOT(onStartBtnToggled(bool)));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimerOut()));
}

void P2PWidget::initVariables(){
	panTiltPtr->setAxes(axesIndex);
	//QString filename = QDir::currentPath() + "/counterpoints.xml";
	//if (QFile::exists(filename)){
	//	if (readXml(filename)){
	//		updateLineEdit();
	//	}
	//	else
	//		QMessageBox::warning(
	//		this,
	//		tr("Config Error"),
	//		QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
	//		);
	//}
	//else{
	//	QMessageBox::warning(
	//		this,
	//		tr("Config Error"),
	//		QString("No xml file called \"axis-counterpoint.xml\", \
	//												please select right xml file and save it as \"axis-counterpoint.xml\" in current directory for convenience.")
	//												);
	//	//onReadConfigBtnClicked();
	//}
}

bool P2PWidget::readXml(QString fileName){
	//QFile file(fileName);
	//if (!file.open(QFile::ReadOnly | QFile::Text)){
	//	qDebug() << "Cannot read file" << file.errorString();
	//	return false;
	//}

	//PointSet* pointset = new PointSet;
	//XmlReader xmlReader(pointset);

	//if (!xmlReader.read(&file))
	//	qDebug() << "Parse error in file " << xmlReader.errorString();
	//else
	//{
	//	pointset->print();
	//}		
	return true;
}

void P2PWidget::onMotionConnected(){
	ui->controllBox->setEnabled(true);
}

bool P2PWidget::readXMLFile(QString filename){
	auto axeaNameMap = [](QString axisName) -> DEVICE_AXIS{
		if ("BIGPANTILT_X" == axisName)
			return DEVICE_AXIS::BIGPANTILT_X;
		else if ("BIGPANTILT_Z" == axisName)
			return DEVICE_AXIS::BIGPANTILT_Z;
	};

	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		LOG(ERROR) << "FILE READ ERROR:" << filename.toStdString() << ":" << file.errorString().toStdString();
		return false;
	}

	QXmlStreamReader xmlReader(&file);
	if (xmlReader.readNextStartElement()){
		if ("Config" == xmlReader.name()){
			while (!xmlReader.atEnd()){
				xmlReader.readNext();
				for (auto attr : xmlReader.attributes()){
					if ("P1" == attr.name())
						point_1[axeaNameMap(xmlReader.name().toString())].push_back(attr.value().toInt());
					if ("P2" == attr.name())
						point_2[axeaNameMap(xmlReader.name().toString())].push_back(attr.value().toInt());
					if ("P3" == attr.name())
						point_3[axeaNameMap(xmlReader.name().toString())].push_back(attr.value().toInt());
					if ("P4" == attr.name())
						point_4[axeaNameMap(xmlReader.name().toString())].push_back(attr.value().toInt());
					if ("P5" == attr.name())
						point_5[axeaNameMap(xmlReader.name().toString())].push_back(attr.value().toInt());
				}
			}
		}
		else{
			xmlReader.raiseError(tr("Incorrect file"));
			return false;
		}
	}
	else{
		LOG(WARNING) << "XML EMPTY WARNING:" << filename.toStdString();
		return false;
	}
	return true;
}

void P2PWidget::updatePoints(){

	counterpoints_x.push_back(point_1[DEVICE_AXIS::BIGPANTILT_X].at(0));
	counterpoints_x.push_back(point_2[DEVICE_AXIS::BIGPANTILT_X].at(0));
	counterpoints_x.push_back(point_3[DEVICE_AXIS::BIGPANTILT_X].at(0));
	counterpoints_x.push_back(point_4[DEVICE_AXIS::BIGPANTILT_X].at(0));
	counterpoints_x.push_back(point_5[DEVICE_AXIS::BIGPANTILT_X].at(0));

	counterpoints_z.push_back(point_1[DEVICE_AXIS::BIGPANTILT_Z].at(0));
	counterpoints_z.push_back(point_2[DEVICE_AXIS::BIGPANTILT_Z].at(0));
	counterpoints_z.push_back(point_3[DEVICE_AXIS::BIGPANTILT_Z].at(0));
	counterpoints_z.push_back(point_4[DEVICE_AXIS::BIGPANTILT_Z].at(0));
	counterpoints_z.push_back(point_5[DEVICE_AXIS::BIGPANTILT_Z].at(0));
}

void P2PWidget::onReadConfig(){
	QString filename = QFileDialog::getOpenFileName(this,
		tr("Config File"),
		".",
		tr("Xml files (*.xml)")
		);
	if (readXMLFile(filename))
		updatePoints();
	else
		QMessageBox::warning(
		this,
		tr("Config Error"),
		QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
		);
}

void P2PWidget::onStartBtnToggled(bool checked){
	if (checked){
		//onReadConfig();
		if (!panTiltPtr->enabled()){
				if (!panTiltPtr->enable()){
				QMessageBox::critical(this,
					tr("Motion Error"),
					QString("Pantilt is not enabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
					);
				return;
			}
		}
		updateTimer->start(100);
		//
		/*for (int i = 0; i < counterpoints_x.size(); ++i)
		{*/
			//qint32 expPitchPos = counterpoints_x.at(i) * (1 << 17) / 360 * 100 + axesOffset->at(0);
			//qint32 expYawPos = counterpoints_z.at(i)* (1 << 17) / 360 * 100 + axesOffset->at(1);

			//qint32 expPitchPos = ui->xPosSlider->value() * (1 << 17) / 360 * 100 + axesOffset->at(0);
			qint32 expPitchPos = ui->xPosSpinBox->value() * (1 << 17) / 360 * 40 ;
			//qint32 expYawPos = ui->zPosSlider->value()* (1 << 17) / 360 * 100 + axesOffset->at(1);
			qint32 expYawPos = ui->zPosSpinBox->value() * (1 << 17) / 360 * 40;
			//qint32 expVel = ui->refVelSlider->value()* (1 << 17) / 360 * 100;
			qint32 expVel = ui->refVelSlider->value() *  (1 << 17) / 360 * 40;
			if (ui->axisXCheckBox->isChecked()){
				if (!panTiltPtr->pitchP2P(expPitchPos, expVel)){
					QMessageBox::critical(this, 
						tr("Motion Error"),
						QString("Pantilt pitchP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
						);
					return;
				}
			}
			panTiltPtr->waitFinished(axesIndex->at(0));

			if (ui->axisZCheckBox->isChecked())
				if (!panTiltPtr->yawP2P(expYawPos, expVel)){
					QMessageBox::critical(this,
						tr("Motion Error"),
						QString("Pantilt yawP2P is not successful at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
						);
					return;
				}
			panTiltPtr->waitFinished(axesIndex->at(1));
		
	}
	else{
		if (!panTiltPtr->disable()){
			QMessageBox::critical(this,
				tr("Motion Error"),
				QString("Pantilt is not disabled at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else{
			//delay 500ms to leave axis velocity drop to zero
			QTimer::singleShot(500, [this](){updateTimer->stop(); });
		}
	}
	
}

void P2PWidget::sleep(unsigned int msec)
{
	QTime reachTime = QTime::currentTime().addMSecs(msec);

	while (QTime::currentTime() < reachTime)
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void P2PWidget::updateAxisIndex(DEVICE_AXIS axis, int index){
	if (DEVICE_AXIS::BIGPANTILT_X == axis || DEVICE_AXIS::SMALLPANTILT_X == axis)
		(*axesIndex)[0] = index;
	else if (DEVICE_AXIS::BIGPANTILT_Z == axis || DEVICE_AXIS::SMALLPANTILT_Z == axis)
		(*axesIndex)[1] = index;
	else
		return;
}

void P2PWidget::updateAxisOffset(DEVICE_AXIS axis, int offset){
	if (DEVICE_AXIS::BIGPANTILT_X == axis || DEVICE_AXIS::SMALLPANTILT_X == axis)
		(*axesOffset)[0] = offset;
	else if (DEVICE_AXIS::BIGPANTILT_Z == axis || DEVICE_AXIS::SMALLPANTILT_Z == axis)
		(*axesOffset)[1] = offset;
	else
		return;
}

void P2PWidget::onUpdateTimerOut(){
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
		pos = (pos - (*axesOffset)[0]) * 360 / (1 << 17) / 100;
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
		pos = (pos - (*axesOffset)[1]) * 360 / (1 << 17) / 100;
		ui->zPosLcdNum->display(pos);
	}
	else{
		QMessageBox::critical(this,
			tr("Motion Error"),
			QString("Read yaw position has errors at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
}