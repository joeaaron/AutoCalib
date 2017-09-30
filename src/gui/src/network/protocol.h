#ifndef _ECAT_NETWORK_PROTOCOL_H_
#define _ECAT_NETWORK_PROTOCOL_H_
#include <QtCore>
//forward declaration
namespace ECAT{
	namespace Network{
		class CMDParser;
	}
}
class CMD
{
public:
	//CMD type
	enum CMD_TYPE : qint16		//to change enum as a quint16 type
	{
		CMD_HEARTBEAT = 0,	//"hello world" test cmd to check communication
		CMD_ACCMD = 1,		//check if cmd is supported 
		CMD_CAMERA = 2,		//cmd about camera
		CMD_LOG = 3,		//cmd print log info
		CMD_REGISTER = 4,	//cmd register camera
		CMD_MODBUS = 5,		//cmd about modbus
		CMD_SHELL = 6,		//cmd shell command
		CMD_TOP_LASER = 7,		//cmd about top laser data
		CMD_BOTTOM_LASER = 8,   //cmd about bottm laser data
		CMD_ALL_LASER = 9,      //cmd about all laser
		CMD_LASERPRO = 14
	};
	//CMD parameter
	enum CMD_PARAMETER : quint32	//to change enum as a quint32 type
	{
		CMD_PARAMETER_NULL = 0,			//no parameter
		CMD_CAMERA_OPEN,				//parameter open camera
		CMD_CAMERA_CLOSE,				//parameter close camera
		CMD_CAMERA_BLACK,				//parameter get a totally black image
		CMD_CAMERA_WHITE,				//parameter get a totally white image
		CMD_CAMERA_TEST,				//parameter get a prescribed image
		CMD_CAMERA_DATA,				//parameter get image data
		CMD_LASER_ON,                   //open all the lasers
		CMD_LASER_OFF,                  //close all the lasers
		CMD_TOP_LASER_ON,
		CMD_BOTTOM_LASER_ON,
		CMD_TOP_FALL_LASER_ON,
		CMD_BOTTOM_FALL_LASER_ON,
		CMD_LASER_DATA,
		CMD_LASER_TEST,
		CMD_LASER_CLEAR,

		CMD_REGISTER_LASER = 0x3005,	//register addr of laser:set 8,laser on,set 0,laser off
		CMD_REGISTER_EXPOSURE = 0x3501,	//register addr of exposure value, range:0 ~ 255
		CMD_REGISTER_GAIN = 0x350b		//register addr of gain value, range:0 ~ 255
	};
	CMD(CMD_TYPE _type, CMD_PARAMETER para) :type(_type){
		switch (type)
		{
		case CMD::CMD_CAMERA:
			initParameter(para, {});
			break;
		case CMD::CMD_TOP_LASER:
			initParameter(para, {});
			break;
		case CMD::CMD_BOTTOM_LASER:
			initParameter(para, {});
			break;
		case CMD::CMD_ALL_LASER:
			initParameter(para, {});
			break;
		case CMD::CMD_REGISTER:
			{
				//value = QByteArray::number(para);
				//for (int i = 0; i != sizeof(para); ++i)
				//	value.prepend((char)((para & (0xFF << (i * 8))) >> (i * 8)));
				QDataStream stream(&value, QIODevice::WriteOnly);
				stream << para;
				length = value.size();
			}
			break;
		case CMD::CMD_HEARTBEAT:
		{
			value = "fuck u";
			length = value.size();
		}
			break;
		case CMD::CMD_LASERPRO:{
			initParameter(para, {});
		}
		default:
			break;
		}
	}

	CMD(CMD_TYPE _type, CMD_PARAMETER para, std::vector<int> subpara) :type(_type){
		switch (type)
		{
		case CMD::CMD_CAMERA:{
			initParameter(para,subpara);
			//QString str;
			//for (auto i : subpara){
			//	str.append(QString::number(i));
			//	str.append(" ");
			//}
			//str.resize(str.size() - 1);
			//value.append(str.toUtf8());
		}	
		break;
		case CMD::CMD_REGISTER:
		{
			//value = QByteArray::number(para);
			//for (int i = 0; i != sizeof(para); ++i)
			//	value.prepend((char)((para & (0xFF << (i * 8))) >> (i * 8)));
			QDataStream stream(&value, QIODevice::WriteOnly);
			stream << para;
			length = value.size();
		}
		break;
		case CMD::CMD_HEARTBEAT:
		{
			value = "fuck u";
			length = value.size();
		}
		break;
		case CMD::CMD_LASERPRO:{
			initParameter(para, subpara);
		}
		default:
			break;
		}
	}
private:
	static QMap<CMD_PARAMETER, QString> createMap(){
		QMap<CMD_PARAMETER, QString> qmap;
		qmap[CMD_PARAMETER_NULL] = QString();
		qmap[CMD_CAMERA_OPEN] = "AC+OPEN";
		qmap[CMD_CAMERA_CLOSE] = "AC+CLOSE";
		qmap[CMD_CAMERA_BLACK] = "AC+BLACK";
		qmap[CMD_CAMERA_WHITE] = "AC+WHITE";
		qmap[CMD_CAMERA_TEST] = "AC+TEST";
		qmap[CMD_CAMERA_DATA] = "AC+DATA";
		qmap[CMD_LASER_ON] = "AC+DEBUG_MODE_ON0";
		qmap[CMD_LASER_OFF] = "AC+DEBUG_MODE_ON1";
		qmap[CMD_TOP_LASER_ON] = "AC+DEBUG_MODE_ON2";
		qmap[CMD_BOTTOM_LASER_ON] = "AC+DEBUG_MODE_ON3";
		qmap[CMD_TOP_FALL_LASER_ON] = "AC+DEBUG_MODE_ON4";
		qmap[CMD_BOTTOM_FALL_LASER_ON] = "AC+DEBUG_MODE_ON5";
		qmap[CMD_LASER_DATA] = "AC+LASER_DATA";
		qmap[CMD_LASER_TEST] = "AC+LASER_TEST";
		qmap[CMD_LASER_CLEAR] = "AC+LASER_CLEAR";
		return qmap;
	}
	void initParameter(CMD_PARAMETER para,std::vector<int> subpara){
		value = parametersMap[para].toUtf8();
		if (!subpara.empty()){
			QString str;
			for (auto i : subpara){
				str.append(QString::number(i));
				str.append(" ");
			}
			str.resize(str.size() - 1);
			value.append(str.toUtf8());
		}
		length = value.size();
	}
	void clearCmd(){
		length = 0;
		value.clear();
	}
	qint64 headerSize() const{
		return sizeof(type) + sizeof(length);
	}
	static CMD_PARAMETER cmdParameter(QString str){
		// return the first key with str, or defaultKey(CMD_PARAMETER(),i.e. CMD_PARAMETER_NULL),
		// if the map contains no item with value value.
		return parametersMap.key(str);
	}
private:
	CMD_TYPE type;
	quint32 length;
	QByteArray value;
	static const QMap<CMD_PARAMETER, QString> parametersMap;
	friend class ECAT::Network::CMDParser;
};
#endif