#ifndef _ECAT_NETWORK_H_
#define _ECAT_NETWORK_H_
#include "config.h"
#include <QtNetwork>
#include <QtCore>
#include "protocol.h"

#ifdef NETWORK_EXPORT
#	define NETWORK_API __declspec(dllexport)
#else
#	define NETWORK_API 
#endif

namespace ECAT{
	namespace Network{
		class CMDParser :public QObject{
			Q_OBJECT
		private:
			CMDParser();												// forbid constructor make sure only one instance
			CMDParser(const CMDParser&);								// forbid copy constructor make sure only one instance
			CMDParser& operator=(const CMDParser&);						// forbid operator= make sure only one instance

			bool readSocketHeader(QTcpSocket* pSock, CMD* pCmd);
			bool readSocketData(QTcpSocket* pSock, CMD* pCmd);
			void parseCmd(CMD* pCmd);
			void sendCmd(CMD& cmd);
		public:
			static CMDParser& getInstance(){
				static CMDParser instance;
				return instance;
			}
			~CMDParser();																
			void setupNetwork(QString ip, quint16 port);								//setup the network environment
			void exitNetwork();															//disconnected and exit network environment
			bool adbCommand(QString program, QStringList arguments, QString& output);	//adb command to connect host to devices
			bool adbCommand(QString cmdstr, QString& output);							//adb command to connect host to devices
			void openCam();																//open camera
			void closeCam();														    //close camera
			//void requestImage();														//get image from sense
			void requestImage(std::vector<int> vec = std::vector<int>());
			void requestLaser(std::vector<int> vec = std::vector<int>());
			void triggerCamera(int cameraID);											//trigger camera with ID
			void setGainValue(int val);													//set gain value of camera
			void setExposureValue(int val);												//set exposure value of camera
			void getGainValue();														//get gain value of camera
			void getExposureValue();													//get exposure value of camera
			void setLaserStatus(int val);                                               //open or close the laser
			void heartbeat();
			void openLaser();
			void closeLaser();
			void openTopLaser();
			void openBottomLaser();
			void openFallTopLaser();
			void openFallBottomLaser();
			void topLaserTest();
			void topLaserClear();
			void topLaserGet();
			void bottomLaserTest();
			void bottomLaserClear();
			void bottomLaserGet();
			void allLaserGet();
		signals:
			void connected();
			void disconnected();
			void socketerror(QString);
			void camOpened(bool);
			void camClosed(bool);
			void recvImage(quint16/*camera*/, quint16/*width*/, quint16/*height*/, QByteArray/*imageData*/);
			void recvAckString(QString/*ackString*/);
			void cameraTriggered(bool /*flag*/);
			void rigesterError();
			void gainValue(quint32 /*value*/);
			void exposureValue(quint32 /*value*/);
			void laserValue(quint32 /*value*/);
			void updateLaserData(short*, int);
			private slots:
			void onHandleSocketError(QAbstractSocket::SocketError);
			void onReadyRead();
			void onRecvAckString(QString);
			
		private:
			QTcpSocket *tcpSocket;
			CMD* cmd;
			bool bReadHeader;
			QByteArray buffer;
		};
	}
}
#endif