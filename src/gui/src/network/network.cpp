#include "config.h"
#include "network.h"
#include <cassert>

using namespace ECAT::Network;
//init static member of class CMD in protocol.h
const QMap<CMD::CMD_PARAMETER, QString> CMD::parametersMap = CMD::createMap();

NETWORK_API CMDParser::CMDParser() 
{
	if (!cmd)
	{
		cmd = new CMD(CMD::CMD_CAMERA, CMD::CMD_CAMERA_OPEN);
		cmd->clearCmd();
	}
	tcpSocket = new QTcpSocket();
}

NETWORK_API CMDParser::~CMDParser(){
	if (tcpSocket)
		delete tcpSocket;
}

NETWORK_API void CMDParser::setupNetwork(QString ip, quint16 port){
	assert(tcpSocket);
	//immediately closes the socket, discarding any pending data in the write buffer.
	tcpSocket->abort();
	//process connected() signal
	//connect(tcpSocket, SIGNAL(connected()), this, SIGNAL(connected()));
	connect(tcpSocket, &QTcpSocket::connected, this, [=](){
		tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
		emit this->connected();
	});
	//process connect error
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onHandleSocketError(QAbstractSocket::SocketError)));
	//process disconnect success
	connect(tcpSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	//process data coming on socket
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	//process received an acknowledge string
	connect(this, SIGNAL(recvAckString(QString)), this, SLOT(onRecvAckString(QString)));
	tcpSocket->connectToHost(ip, port);
	bReadHeader = false;	// tcpSocket has not read data header by default
}

NETWORK_API bool CMDParser::adbCommand(QString program, QStringList arguments, QString& output){
	QProcess process(this);
	process.start(program, arguments);
	if (!process.waitForStarted())
		return false;
	if (!process.waitForFinished())
		return false;
	QByteArray result = process.readAll();
	output = result;
	return true;
}

NETWORK_API bool CMDParser::adbCommand(QString cmdstr, QString& output){
	QProcess process(this);
	process.start(cmdstr);
	//process.start("adb forward tcp:6000 tcp:6000");
	if (!process.waitForStarted())
		return false;
	if (!process.waitForFinished())
		return false;
	QByteArray result = process.readAll();
	output = result;
	return true;
}

NETWORK_API bool CMDParser::adbCommand(QString cmdstr){
	QProcess process(this);
	process.start(cmdstr);
	//process.start("adb forward tcp:6000 tcp:6000");
	if (!process.waitForStarted())
		return false;
	if (!process.waitForFinished())
		return false;

	return true;
}

NETWORK_API void CMDParser::exitNetwork(){
	assert(tcpSocket);
	tcpSocket->disconnectFromHost();
}

NETWORK_API void CMDParser::sendCmd(CMD& cmd){
	//cmd serialization:T(type--2Bytes)--L(length--4Bytes)--V(value)
	QByteArray block;
	QDataStream serializedCmd(&block, QIODevice::WriteOnly);
	serializedCmd << (quint16)cmd.type << cmd.length;
	serializedCmd.writeRawData(cmd.value, cmd.length);
	//send to sensor device
	tcpSocket->write(block);
}

NETWORK_API void CMDParser::openCam(){
	//define open camera cmd
	CMD openCmd(CMD::CMD_CAMERA, CMD::CMD_CAMERA_OPEN);
	sendCmd(openCmd);
}

NETWORK_API void CMDParser::closeCam(){
	//define close camera cmd
	CMD closeCmd(CMD::CMD_CAMERA, CMD::CMD_CAMERA_CLOSE);
	sendCmd(closeCmd);
}

NETWORK_API void CMDParser::openLaser(){
	//define open laser cmd
	CMD openLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_LASER_ON);
	sendCmd(openLaserCmd);
}

NETWORK_API void CMDParser::closeLaser(){
	//define close laser cmd
	CMD closeLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_LASER_OFF);
 	sendCmd(closeLaserCmd);
}

NETWORK_API void CMDParser::topLaserTest(){
	//define close laser cmd
	CMD tLaserTestCmd(CMD::CMD_TOP_LASER, CMD::CMD_LASER_TEST);
	sendCmd(tLaserTestCmd);
}

NETWORK_API void CMDParser::topLaserClear(){
	//define close laser cmd
	CMD tLaserClearCmd(CMD::CMD_TOP_LASER, CMD::CMD_LASER_CLEAR);
	sendCmd(tLaserClearCmd);
}

NETWORK_API void CMDParser::topLaserGet(){
	//define close laser cmd
	CMD tLaserGetCmd(CMD::CMD_TOP_LASER, CMD::CMD_LASER_DATA);
	sendCmd(tLaserGetCmd);
}

NETWORK_API void CMDParser::bottomLaserTest(){
	//define close laser cmd
	CMD bLaserTestCmd(CMD::CMD_BOTTOM_LASER, CMD::CMD_LASER_TEST);
	sendCmd(bLaserTestCmd);
}

NETWORK_API void CMDParser::bottomLaserClear(){
	//define close laser cmd
	CMD bLaserClearCmd(CMD::CMD_BOTTOM_LASER, CMD::CMD_LASER_CLEAR);
	sendCmd(bLaserClearCmd);
}

NETWORK_API void CMDParser::bottomLaserGet(){
	//define close laser cmd
	CMD bLaserGetCmd(CMD::CMD_BOTTOM_LASER, CMD::CMD_LASER_DATA);
	sendCmd(bLaserGetCmd);
}

NETWORK_API void CMDParser::allLaserGet(){
	//define close laser cmd
	CMD laserGetCmd(CMD::CMD_ALL_LASER, CMD::CMD_LASER_DATA);
	sendCmd(laserGetCmd);
}

NETWORK_API void CMDParser::openTopLaser(){
	//define close laser cmd
	CMD openTopLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_TOP_LASER_ON);
	sendCmd(openTopLaserCmd);
}

NETWORK_API void CMDParser::openBottomLaser(){
	//define close laser cmd
	CMD openBottomLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_BOTTOM_LASER_ON);
	sendCmd(openBottomLaserCmd);
}

NETWORK_API void CMDParser::openFallTopLaser(){
	//define close laser cmd
	CMD openFallTopLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_TOP_FALL_LASER_ON);
	sendCmd(openFallTopLaserCmd);
}

NETWORK_API void CMDParser::openFallBottomLaser(){
	//define close laser cmd
	CMD openFallBottomLaserCmd(CMD::CMD_LASERPRO, CMD::CMD_BOTTOM_FALL_LASER_ON);
	sendCmd(openFallBottomLaserCmd);
}

NETWORK_API void CMDParser::requestImage(std::vector<int> vec /* = std::vector<int>() */){
	//define request image cmd
	CMD reqImageCmd(CMD::CMD_CAMERA, CMD::CMD_CAMERA_DATA);
	if (!vec.empty())
		reqImageCmd = CMD(CMD::CMD_CAMERA, CMD::CMD_CAMERA_DATA,vec);
	qDebug() << reqImageCmd.value << endl;
	sendCmd(reqImageCmd);
}

NETWORK_API void CMDParser::requestLaser(std::vector<int> vec /* = std::vector<int>() */){
	//define request image cmd
	CMD reqLaserCmd(CMD::CMD_TOP_LASER, CMD::CMD_LASER_DATA);
	if (!vec.empty())
		reqLaserCmd = CMD(CMD::CMD_TOP_LASER, CMD::CMD_LASER_DATA, vec);
	qDebug() << reqLaserCmd.value << endl;
	sendCmd(reqLaserCmd);
}

NETWORK_API void CMDParser::triggerCamera(int cameraID){
	QFile file("trig_config.txt");
	QStringList strlist;
	//strlist[0]:CAM1 and CAM3,strlist[1]:CAM2 and CAM4,strlist[2]:ALLCAM
	strlist << "2 0 2 0 0 1 2 3" << "0 2 0 2 0 1 2 3" << "2 2 2 2 0 1 2 3";
	if (file.open(QIODevice::ReadWrite))
	{
		QTextStream stream(&file);
		if (cameraID == 4)
			stream << strlist[2] << endl;
		else
			stream << strlist[cameraID % 2] << endl;
	}
	file.close();
	//send trigger cmd by adbCommand
	QString triggerCmd("adb push trig_config.txt /data/cowa_cam_config"),output;
	bool ret = adbCommand(triggerCmd, output);
	emit cameraTriggered(ret);
}

NETWORK_API void CMDParser::setGainValue(int val){
	//define set register cmd
	CMD setGainCmd(CMD::CMD_REGISTER, CMD::CMD_REGISTER_GAIN);
	QByteArray valarray;
	QDataStream stream(&valarray, QIODevice::WriteOnly);
	stream << (quint32)val;
	setGainCmd.value.append(valarray);
	setGainCmd.length = setGainCmd.value.size();
	sendCmd(setGainCmd);
}

NETWORK_API void CMDParser::heartbeat(){
	//define set helloworld cmd
	CMD heartbeatCmd(CMD::CMD_HEARTBEAT,CMD::CMD_PARAMETER_NULL);
	sendCmd(heartbeatCmd);
}

NETWORK_API void CMDParser::getGainValue(){
	//define get register cmd
	CMD getGainCmd(CMD::CMD_REGISTER, CMD::CMD_REGISTER_GAIN);
	sendCmd(getGainCmd);
}

NETWORK_API void CMDParser::setLaserStatus(int val)
{
	//define laser register cmd
	CMD setLaserCmd(CMD::CMD_REGISTER, CMD::CMD_REGISTER_LASER);
	QByteArray valarray;
	QDataStream stream(&valarray, QIODevice::WriteOnly);
	stream << (quint32)val;
	setLaserCmd.value.append(valarray);
	setLaserCmd.length = setLaserCmd.value.size();
	sendCmd(setLaserCmd);

}

NETWORK_API void CMDParser::setExposureValue(int val){
	//define set register cmd
	CMD setExpCmd(CMD::CMD_REGISTER, CMD::CMD_REGISTER_EXPOSURE);
	QByteArray valarray;
	QDataStream stream(&valarray, QIODevice::WriteOnly);
	stream << (quint32)val;
	setExpCmd.value.append(valarray);
	setExpCmd.length = setExpCmd.value.size();
	sendCmd(setExpCmd);
}

NETWORK_API void CMDParser::getExposureValue(){
	//define get register cmd
	CMD getExpCmd(CMD::CMD_REGISTER, CMD::CMD_REGISTER_EXPOSURE);
	sendCmd(getExpCmd);
}

NETWORK_API void CMDParser::onReadyRead(){
	//read socket data header
#if 0
	if (!bReadHeader)
		bReadHeader = readSocketHeader(tcpSocket, cmd);
	if (!bReadHeader) return;	// hasn't received header yet
	//header complete, then read socket data
	if (!readSocketData(tcpSocket, cmd))
		return;
	//parse received data
	parseCmd(cmd);
	//reset data container for next CMD
	bReadHeader = false;
	cmd->clearCmd();
#endif
#if 1
	//get all bytes of tcp and store in a buffer
	qint64 n = tcpSocket->bytesAvailable();
	if (tcpSocket->bytesAvailable())
	{
		buffer.append(tcpSocket->read(tcpSocket->bytesAvailable()));
	}
	//parse the data package in the buffer
	QDataStream io(buffer);
	while (buffer.size())
	{
		qint16 type;
		quint32 length;
		if (cmd->length == 0)
		{
			if (buffer.size() < cmd->headerSize()) return;
			io >> type;
			io >> length;
			cmd->type = CMD::CMD_TYPE(type);
			cmd->length = length;
			buffer.remove(0, cmd->headerSize());
		}

		//
		if (buffer.size() < cmd->length)
		{
			return;
		}
		cmd->value.resize(cmd->length);
		io.readRawData(cmd->value.data(), cmd->length);
		buffer.remove(0, cmd->length);
		// process data
		parseCmd(cmd);
		cmd->clearCmd();
	}

#endif
}

bool CMDParser::readSocketHeader(QTcpSocket* pSock, CMD* pCmd){
	int ret = pSock->bytesAvailable();
	int size = pCmd->headerSize();
	if (pSock->bytesAvailable() < pCmd->headerSize())	//header is not completed
		return false;
	qint16 type; quint32 length;
	pSock->read((char *)&(type), sizeof(qint16));
	pSock->read((char *)&(length), sizeof(quint32));
	//On CPU architectures where the host byte order is little-endian (such as x86)
	//return the byte order swapped; otherwise it will return src unmodified.
	pCmd->type = CMD::CMD_TYPE(qFromBigEndian(type));
	pCmd->length = qFromBigEndian(length);
	return true;
}

bool CMDParser::readSocketData(QTcpSocket* pSock, CMD* pCmd){
	if (pSock->bytesAvailable() < pCmd->length)
		return false;
	// data is completed
	pCmd->value = pSock->read(pCmd->length);
	return true;
}

void CMDParser::parseCmd(CMD* pCmd){
	switch (pCmd->type)
	{
	case CMD::CMD_HEARTBEAT:
		{
			qDebug() << QString(pCmd->value) << endl;
		}
		break;
	case CMD::CMD_CAMERA:
		{
			QDataStream serializedCmd(pCmd->value);
			quint16 cameraID;
			serializedCmd >> cameraID;
			//if 0 <= cameraID <= 11, value is an image,otherwise is a acknowledge string
			if (cameraID >= 0 && cameraID <= 11){
				//value:camera_id(2Bytes),image_width(2Bytes),image_height(2Bytes),image_data(the rest) 
				quint16 width, height;
				QByteArray imageData;
				serializedCmd >> width >> height;
				imageData = pCmd->value.remove(0, 6);
				qDebug() << imageData;
				emit recvImage(cameraID, width, height, imageData);
			}
			else{
				QString str(pCmd->value); //acknowledge string
				emit recvAckString(str);
			}
		}
		break;
	case CMD::CMD_TOP_LASER:
		{
			//qDebug() << buffer;
			QByteArray laserData;
			laserData = pCmd->value.remove(0, 6);
			unsigned char *value = (uchar*)laserData.data();
			qDebug() << value;

			int length = pCmd->length;
			if (length % 4 != 0)
				break;

			emit updateLaserData((short*)value, length / 4);
		}
		break;
	case CMD::CMD_BOTTOM_LASER:
		{
			QByteArray laserData;
			laserData = pCmd->value.remove(0, 6);
			unsigned char *value = (uchar*)laserData.data();
			qDebug() << value;

			int length = pCmd->length;
			if (length % 4 != 0)
				break;

			emit updateLaserData((short*)value, length / 4);
		}
		break;
	case CMD::CMD_REGISTER:
		{
			if (pCmd->length < 4)
				emit rigesterError();
			if (pCmd->length >= 8){
				//get register cmd returned
				QDataStream serializedCmd(pCmd->value);
				quint32 addr, data;
				serializedCmd >> addr >> data;
				switch (addr)
				{
				case CMD::CMD_REGISTER_EXPOSURE:
					emit exposureValue(data);
					break;
				case CMD::CMD_REGISTER_GAIN:
					emit gainValue(data);
					break;
				case CMD::CMD_REGISTER_LASER:
					emit laserValue(data);
					break;
				default:
					break;
				}
			}
				
		}
		break;
	default:
		break;
	}
}

void CMDParser::onRecvAckString(QString ackStr){
	if (ackStr.contains(':')){
		QStringList strList = ackStr.split(':');
		CMD::CMD_PARAMETER para = CMD::cmdParameter(strList[0]);
		QString status = strList[1].remove(' ');
		switch (para)
		{
		case CMD::CMD_PARAMETER_NULL:
			break;
		case CMD::CMD_CAMERA_OPEN:
			{
				//status can only be "OK" or "ERROR"
				bool ret = (status == "OK") ? true : false;
				emit camOpened(ret);
			}
			break;
		case CMD::CMD_CAMERA_CLOSE:
			{
				//status can only be "OK" or "ERROR"
				bool ret = (status == "OK") ? true : false;
				emit camClosed(ret);
			}
			break;
		default:
			break;
		}
	}
}

void CMDParser::onHandleSocketError(QAbstractSocket::SocketError socketError){
	emit socketerror(tcpSocket->errorString());
}


