#include "CalibrationWidget.h"
#include "InchingWidget.h"
#include "P2PWidget.h"
#include "XYZInchingWidget.h"
#include "XYZP2PWidget.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <glog/logging.h>
#include "network/network.h"
CalibrationWidget::CalibrationWidget(QWidget* parent /* = 0 */, Qt::WindowFlags flags /* = 0 */)
	:ui(new Ui::calibrationPage)
{
	ui->setupUi(this);
	initUi();
	initSignals();
	savePath = "D:/images/1";
	num = 0;
	initVariables();
}

void CalibrationWidget::initUi(){
	/*Big PanTilt UI*/
	panes.push_back(new PaneWidget(ui->bigPanTiltTab, "Inching", ":/icons/inching.png", true, false));
	bigPanInchingWidget = new InchingWidget(panes[0]);
	panes[0]->setWidget(bigPanInchingWidget, true);
	panes[0]->expand();
	ui->bigPanPanesLayout->addWidget(panes[0]);

	panes.push_back(new PaneWidget(ui->bigPanTiltTab, "P2P", ":/icons/inching.png", true, false));
	bigPanP2PWidget = new P2PWidget(panes[1]);
	panes[1]->setWidget(bigPanP2PWidget, true);
	ui->bigPanPanesLayout->addWidget(panes[1]);

	ui->bigPanPanesLayout->setAlignment(Qt::AlignTop);
	ui->bigPanPanesLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	/*Small PanTilt UI*/
	panes.push_back(new PaneWidget(ui->smallPanTiltTab, "Inching", ":/icons/inching.png", true, false));
	smallPanInchingWidget = new InchingWidget(panes[2]);
	panes[2]->setWidget(smallPanInchingWidget, true);
	panes[2]->expand();
	ui->smallPanPanesLayout->addWidget(panes[2]);

	panes.push_back(new PaneWidget(ui->smallPanTiltTab, "P2P", ":/icons/inching.png", true, false));
	smallPanP2PWidget = new P2PWidget(panes[3]);
	panes[3]->setWidget(smallPanP2PWidget, true);
	ui->smallPanPanesLayout->addWidget(panes[3]);

	ui->smallPanPanesLayout->setAlignment(Qt::AlignTop);
	ui->smallPanPanesLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	/*XYZ UI*/
	panes.push_back(new PaneWidget(ui->xyzrTab, "Inching", ":/icons/inching.png", true, false));
	xyzInchingWidget = new XYZInchingWidget(panes[4]);
	panes[4]->setWidget(xyzInchingWidget, true);
	panes[4]->expand();
	ui->xyzPanesLayout->addWidget(panes[4]);

	panes.push_back(new PaneWidget(ui->xyzrTab, "P2P", ":/icons/inching.png", true, false));
	xyzP2PWidget = new XYZP2PWidget(panes[5]);
	panes[5]->setWidget(xyzP2PWidget, true);
	ui->xyzPanesLayout->addWidget(panes[5]);

	ui->xyzPanesLayout->setAlignment(Qt::AlignTop);
	ui->xyzPanesLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
	/*Config Tab*/
	QList<QLineEdit*> lineEdits = ui->configTab->findChildren<QLineEdit*>();
	for (auto edit : lineEdits)
		edit->setValidator(new QIntValidator(edit));
	//Init display Frame
	displayView = new DisplayView(ui->displayFrame);
	ui->displayLayout->addWidget(displayView, 0, 0, 1, 1);
	displayView->show();
	//Switch to the third page
	ui->tabWidget->setCurrentIndex(2);
}

void CalibrationWidget::initSignals(){
	QList<QComboBox*> comBoxes = ui->configTab->findChildren<QComboBox*>();
	for (auto box : comBoxes)
		connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onComBoxIndexChanged(int)));

	QList<QLineEdit*> lineEdits = ui->configTab->findChildren<QLineEdit*>();
	for (auto edit : lineEdits)
		connect(edit,SIGNAL(textChanged(const QString)),this,SLOT(onLineEditTextChanged(const QString)));

	connect(ui->saveConfigBtn, SIGNAL(clicked()),this,SLOT(onSaveConfigBtnClicked()));
	connect(ui->readConfigBtn,SIGNAL(clicked()),this,SLOT(onReadConfigBtnClicked()));

	connect(this, SIGNAL(motionConnected()), bigPanInchingWidget, SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionConnected()), bigPanP2PWidget, SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionConnected()), smallPanInchingWidget,SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionConnected()), smallPanP2PWidget, SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionConnected()), xyzInchingWidget, SLOT(onMotionConnected()));
	connect(this, SIGNAL(motionConnected()), xyzP2PWidget, SLOT(onMotionConnected()));

	connect(ui->displayBtn, SIGNAL(clicked()), this, SLOT(onDisplayBtnClicked()));
	connect(ui->saveBtn, SIGNAL(clicked()),this, SLOT(onSaveBtnClicked()));
	connect(ui->camCombo, SIGNAL(activated(int)), this, SLOT(onCamComboActivated(int)));
	connect(ui->videoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onVideoCheckBoxStateChanged(int)));
	connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onVideoCheckBox2StateChanged(int)));
	connect(&CMDParser::getInstance(), SIGNAL(recvImage(quint16, quint16, quint16, QByteArray)), 
			this, SLOT(onRecvImage(quint16, quint16, quint16, QByteArray)));
}

void CalibrationWidget::initVariables(){
	bVideo = false;	//default mode is capturing image
	cameraID = 0;	//default cameraID is 0
	saveImgNum = 12;
	//read default axis-driver config xml file
	QString filename = QDir::currentPath() + "/axis-driver.xml";
	if (QFile::exists(filename)){
		if (readXMLFile(filename)){
			if (checkAxesMap()){
				updateComBox();
				updateLineEdit();
			}
			else
				QMessageBox::warning(
				this,
				tr("Config Warning"),
				QString("Axes share same drivers, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
				);
		}
		else
			QMessageBox::warning(
			this,
			tr("Config Error"),
			QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	else{
		QMessageBox::warning(
			this,
			tr("Config Error"),
			QString("No xml file called \"axis-driver.xml\", \
			please select right xml file and save it as \"axis-driver.xml\" in current directory for convenience.")
			);
		onReadConfigBtnClicked();
	}
}

CalibrationWidget::~CalibrationWidget(){}

void CalibrationWidget::onComBoxIndexChanged(int index){
	auto updateInchingWidgetAxis = [](QWidget* widget,DEVICE_AXIS axis,int index){
		InchingWidget* inchingWidget = dynamic_cast<InchingWidget*>(widget);
		if (nullptr != inchingWidget)
			inchingWidget->updateAxisIndex(axis, index);
	};
	auto updateP2PWidgetAxis = [](QWidget* widget, DEVICE_AXIS axis, int index){
		P2PWidget* p2pWidget = dynamic_cast<P2PWidget*>(widget);
		if (nullptr != p2pWidget)
			p2pWidget->updateAxisIndex(axis, index);
	};
	auto updateXYZInchingWidgetAxis = [](QWidget* widget, DEVICE_AXIS axis, int index){
		XYZInchingWidget* inchingWidget = dynamic_cast<XYZInchingWidget*>(widget);
		if (nullptr != inchingWidget)
			inchingWidget->updateAxisIndex(axis, index);
	};
	auto updateXYZP2PWidgetAxis = [](QWidget* widget, DEVICE_AXIS axis, int index){
		XYZP2PWidget* p2pWidget = dynamic_cast<XYZP2PWidget*>(widget);
		if (nullptr != p2pWidget)
			p2pWidget->updateAxisIndex(axis, index);
	};

	QObject* sender = QObject::sender();
	if (sender == ui->bigPanXComBox){
		deviceAxesMap[DEVICE_AXIS::BIGPANTILT_X] = index;
		updateInchingWidgetAxis(bigPanInchingWidget, DEVICE_AXIS::BIGPANTILT_X, index);
		updateP2PWidgetAxis(bigPanP2PWidget, DEVICE_AXIS::BIGPANTILT_X, index);
	}
	else if (sender == ui->bigPanZComBox){
		deviceAxesMap[DEVICE_AXIS::BIGPANTILT_Z] = index;
		updateInchingWidgetAxis(bigPanInchingWidget, DEVICE_AXIS::BIGPANTILT_Z, index);
		updateP2PWidgetAxis(bigPanP2PWidget, DEVICE_AXIS::BIGPANTILT_Z, index);
	}
	else if (sender == ui->smallPanTiltXComBox){
		deviceAxesMap[DEVICE_AXIS::SMALLPANTILT_X] = index;
		updateInchingWidgetAxis(smallPanInchingWidget, DEVICE_AXIS::SMALLPANTILT_X, index);
		updateP2PWidgetAxis(smallPanP2PWidget, DEVICE_AXIS::SMALLPANTILT_X, index);
	}
	else if (sender == ui->smallPanTiltZComBox){
		deviceAxesMap[DEVICE_AXIS::SMALLPANTILT_Z] = index;
		updateInchingWidgetAxis(smallPanInchingWidget, DEVICE_AXIS::SMALLPANTILT_Z, index);
		updateP2PWidgetAxis(smallPanP2PWidget, DEVICE_AXIS::SMALLPANTILT_Z, index);
	}
	else if (sender == ui->xyzXComBox){
		deviceAxesMap[DEVICE_AXIS::XYZ_X] = index;
		updateXYZInchingWidgetAxis(xyzInchingWidget, DEVICE_AXIS::XYZ_X, index);
		updateXYZP2PWidgetAxis(xyzP2PWidget, DEVICE_AXIS::XYZ_X, index);
	}
	else if (sender == ui->xyzYComBox){
		deviceAxesMap[DEVICE_AXIS::XYZ_Y] = index;
		updateXYZInchingWidgetAxis(xyzInchingWidget, DEVICE_AXIS::XYZ_Y, index);
		updateXYZP2PWidgetAxis(xyzP2PWidget, DEVICE_AXIS::XYZ_Y, index);
	}
	else if (sender == ui->xyzZComBox){
		deviceAxesMap[DEVICE_AXIS::XYZ_Z] = index;
		updateXYZInchingWidgetAxis(xyzInchingWidget, DEVICE_AXIS::XYZ_Z, index);
		updateXYZP2PWidgetAxis(xyzP2PWidget, DEVICE_AXIS::XYZ_Z, index);
	}
	else if (sender == ui->xyzRComBox){
		deviceAxesMap[DEVICE_AXIS::XYZ_R] = index;
		updateXYZInchingWidgetAxis(xyzInchingWidget, DEVICE_AXIS::XYZ_R, index);
		updateXYZP2PWidgetAxis(xyzP2PWidget, DEVICE_AXIS::XYZ_R, index);
	}
	else{
		QMessageBox::critical(
			this,
			tr("Config Error"),
			QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		LOG(ERROR) << "DEVICE AXIS CONFIG ERROR.";
	}
}

void CalibrationWidget::onLineEditTextChanged(const QString text){
	auto updateInchingWidgetOffset = [](QWidget* widget, DEVICE_AXIS axis, int offset){
		InchingWidget* inchingWidget = dynamic_cast<InchingWidget*>(widget);
		if (nullptr != inchingWidget)
			inchingWidget->updateAxisOffset(axis, offset);
	};
	auto updateP2PWidgetOffset = [](QWidget* widget, DEVICE_AXIS axis, int offset){
		P2PWidget* p2pWidget = dynamic_cast<P2PWidget*>(widget);
		if (nullptr != p2pWidget)
			p2pWidget->updateAxisOffset(axis, offset);
	};
	QObject* sender = QObject::sender();
	if (sender == ui->bigPanXOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::BIGPANTILT_X] = text.toInt();
		updateInchingWidgetOffset(bigPanInchingWidget,DEVICE_AXIS::BIGPANTILT_X,text.toInt());
		updateP2PWidgetOffset(bigPanP2PWidget, DEVICE_AXIS::BIGPANTILT_X, text.toInt());
	}
	else if (sender == ui->bigPanZOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::BIGPANTILT_Z] = text.toInt();
		updateInchingWidgetOffset(bigPanInchingWidget, DEVICE_AXIS::BIGPANTILT_Z, text.toInt());
		updateP2PWidgetOffset(bigPanP2PWidget, DEVICE_AXIS::BIGPANTILT_Z, text.toInt());
	}
	else if (sender == ui->smallPanXOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::SMALLPANTILT_X] = text.toInt();
	}
	else if (sender == ui->smallPanZOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::SMALLPANTILT_Z] = text.toInt();
	}
	else if (sender == ui->xyzXOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::XYZ_X] = text.toInt();
	}
	else if (sender == ui->xyzYOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::XYZ_Y] = text.toInt();
	}
	else if (sender == ui->xyzZOffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::XYZ_Z] = text.toInt();
	}
	else if (sender == ui->xyzROffsetLineEdit){
		axisOffsetMap[DEVICE_AXIS::XYZ_R] = text.toInt();
	}
	else{
		QMessageBox::critical(
			this,
			tr("Config Error"),
			QString("Error occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
		LOG(ERROR) << "DEVICE AXIS CONFIG ERROR.";
	}
}

void CalibrationWidget::onSaveConfigBtnClicked(){
	if (checkAxesMap()){
		QString filename = QFileDialog::getSaveFileName(this,
			tr("Save File"),
			".",
			tr("Xml files (*.xml)")
			);
		if (filename.isEmpty())
			filename = "axis-driver.xml";
		saveXMLFile(filename);
	}
	else
		QMessageBox::warning(
		this,
		tr("Config Warning"),
		QString("Axes share same drivers, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
		);
}

void CalibrationWidget::saveXMLFile(QString filename){
	//map enum DEVICE_AXIS to QString
	auto axesNameMap = [](DEVICE_AXIS axis) -> QString{
		switch (axis)
		{
		case DEVICE_AXIS::BIGPANTILT_X:
			return QString("BIGPANTILT_X");
		case DEVICE_AXIS::BIGPANTILT_Z:
			return QString("BIGPANTILT_Z");
		case DEVICE_AXIS::SMALLPANTILT_X:
			return QString("SMALLPANTILT_X");
		case DEVICE_AXIS::SMALLPANTILT_Z:
			return QString("SMALLPANTILT_Z");
		case DEVICE_AXIS::XYZ_X:
			return QString("XYZ_X");
		case DEVICE_AXIS::XYZ_Y:
			return QString("XYZ_Y");
		case DEVICE_AXIS::XYZ_Z:
			return QString("XYZ_Z");
		case DEVICE_AXIS::XYZ_R:
			return QString("XYZ_R");
		default:
			return QString("");
		}
	};
	QFile file(filename);
	file.open(QFile::WriteOnly);

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);

	xmlWriter.writeStartDocument();	// start doc
	xmlWriter.writeStartElement("Config"); // start Config

	xmlWriter.writeAttribute("name", "Calibration");

	QList<DEVICE_AXIS> devices = deviceAxesMap.keys();
	for (auto device : devices){
		xmlWriter.writeStartElement(axesNameMap(device));
		xmlWriter.writeAttribute("Axis", QString::number(deviceAxesMap[device]));
		xmlWriter.writeAttribute("Offset", QString::number(axisOffsetMap[device]));
		xmlWriter.writeEndElement();
	}
	xmlWriter.writeEndElement();	// end Config
	xmlWriter.writeEndDocument();	// end doc

	file.close();
}

void CalibrationWidget::onReadConfigBtnClicked(){
	QString filename = QFileDialog::getOpenFileName(this, 
													tr("Config File"), 
													".", 
													tr("Xml files (*.xml)")
													);
	if (readXMLFile(filename)){
		if (checkAxesMap()){
			updateComBox();
			updateLineEdit();
		}
		else
			QMessageBox::warning(
			this,
			tr("Config Warning"),
			QString("Axes share same drivers, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
			);
	}
	else
		QMessageBox::warning(
		this,
		tr("Config Error"),
		QString("Can't read xml file, occurred at line number %1 in function %2 in %3 file.").arg(__LINE__).arg(__FUNCTION__).arg(__FILE__)
		);
}

bool CalibrationWidget::readXMLFile(QString filename){
	auto axeaNameMap = [](QString axisName) -> DEVICE_AXIS{
		if ("BIGPANTILT_X" == axisName)
			return DEVICE_AXIS::BIGPANTILT_X;
		else if ("BIGPANTILT_Z" == axisName)
			return DEVICE_AXIS::BIGPANTILT_Z;
		else if ("SMALLPANTILT_X" == axisName)
			return DEVICE_AXIS::SMALLPANTILT_X;
		else if ("SMALLPANTILT_Z" == axisName)
			return DEVICE_AXIS::SMALLPANTILT_Z;
		else if ("XYZ_X" == axisName)
			return DEVICE_AXIS::XYZ_X;
		else if ("XYZ_Y" == axisName)
			return DEVICE_AXIS::XYZ_Y;
		else if ("XYZ_Z" == axisName)
			return DEVICE_AXIS::XYZ_Z;
		else
			return DEVICE_AXIS::XYZ_R;
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
					if ("Axis" == attr.name())
						deviceAxesMap[axeaNameMap(xmlReader.name().toString())] = attr.value().toInt();
					if ("Offset" == attr.name())
						axisOffsetMap[axeaNameMap(xmlReader.name().toString())] = attr.value().toInt();
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

void CalibrationWidget::updateComBox(){
	ui->bigPanXComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::BIGPANTILT_X]);
	ui->bigPanZComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::BIGPANTILT_Z]);
	ui->smallPanTiltXComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::SMALLPANTILT_X]);
	ui->smallPanTiltZComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::SMALLPANTILT_Z]);
	ui->xyzXComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::XYZ_X]);
	ui->xyzYComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::XYZ_Y]);
	ui->xyzZComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::XYZ_Z]);
	ui->xyzRComBox->setCurrentIndex(deviceAxesMap[DEVICE_AXIS::XYZ_R]);
}

void CalibrationWidget::updateLineEdit(){
	ui->bigPanXOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::BIGPANTILT_X]));
	ui->bigPanZOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::BIGPANTILT_Z]));
	ui->smallPanXOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::SMALLPANTILT_X]));
	ui->smallPanZOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::SMALLPANTILT_Z]));
	ui->xyzXOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::XYZ_X]));
	ui->xyzYOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::XYZ_Y]));
	ui->xyzZOffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::XYZ_Z]));
	ui->xyzROffsetLineEdit->setText(QString::number(axisOffsetMap[DEVICE_AXIS::XYZ_R]));
}

bool CalibrationWidget::checkAxesMap(){
	// make sure no duplicates in axis map 
	QList<int> list = deviceAxesMap.values();
	QSet<int> set = list.toSet();	//remove duplicates in list
	return (list.size() == set.size());
}

void CalibrationWidget::onDisplayBtnClicked(){
	//ui->displayBtn->setEnabled(false);
	CMDParser::getInstance().requestImage({cameraID});
}

void CalibrationWidget::onCamComboActivated(int index){
	cameraID = index;

	if (1 == ((cameraID + 1) % 4))
	{
		savePath = "D:/images/1";
	}
	else if (2 == ((cameraID + 1) % 4))
	{
		savePath = "D:/images/2";
	}
	else if (3 == ((cameraID + 1) % 4))
	{
		savePath = "D:/images/3";
	}
	else
		savePath = "D:/images/4";
	/*DirContext *saveImg = new DirContext(cameraID + 1);
	saveImg->getSavePath();*/
}

void CalibrationWidget::onVideoCheckBoxStateChanged(int state){
	bVideo = (bool)state;
	if (Qt::Unchecked == state)
	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;
		MotionController::getInstance().setDigitalIO(data, 4);
	}
	else
	{
		saveImgNum = 12;
		data[0] = 1;
		data[1] = 1;
		data[2] = 0;
		data[3] = 1;
		MotionController::getInstance().setDigitalIO(data, 4);
	}
		 
}

void CalibrationWidget::onVideoCheckBox2StateChanged(int state){

	if (Qt::Unchecked == state)
	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;
		MotionController::getInstance().setDigitalIO(data, 4);
	}
	else
	{
		saveImgNum = 10;
		data[0] = 1;
		data[1] = 0;
		data[2] = 1;
		data[3] = 1;
		MotionController::getInstance().setDigitalIO(data, 4);
	}

}

void CalibrationWidget::onRecvImage(quint16 camera, quint16 width, quint16 height, QByteArray imageData){
	QImage image = QImage((uchar*)imageData.data(), width, height, QImage::Format_Indexed8);
	if (image.isNull())
		return;
	else{
		//if (cameraID >= 4)
		//	return;
		if (camera == cameraID){
			imageToSave = image.copy();
			displayView->showImage(image);
			/*if (bVideo)
				CMDParser::getInstance().requestImage();*/
		}

		/*if (!ui->videoCheckBox->isChecked())
			ui->displayBtn->setEnabled(true);*/
	}

}

void CalibrationWidget::onSaveBtnClicked(){
	//QPixmap pixmap = QPixmap::fromImage(imageToSave.mirrored(true, true));
	QPixmap pixmap = QPixmap::fromImage(imageToSave);
	QString tmp;
	tmp.sprintf("%s/x%d", savePath.toLatin1().data(), ++num);
	QString imagePath = QString(tmp) + ".bmp";
	pixmap.save(imagePath);
	if (num == saveImgNum)
	{
		num = 0;
	}
	
}


