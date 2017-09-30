#ifndef _ECAT_NETWORK_DEVICE_H_
#define _ECAT_NETWORK_DEVICE_H_
#include <QtCore>
#include <memory>
#include "motioncontroller.h"
using namespace std;
namespace ECAT{
	namespace Network{
		enum class DEVICE_AXIS{
			BIGPANTILT_X,
			BIGPANTILT_Z,
			SMALLPANTILT_X,
			SMALLPANTILT_Z,
			XYZ_X,
			XYZ_Y,
			XYZ_Z,
			XYZ_R
		};

		class Device{
		public:
			Device(QVector<int> index = QVector<int>(2, -1));
			virtual void setAxes(shared_ptr<QVector<int>>);
			virtual bool enable();
			virtual bool disable();
			virtual bool enabled() const;
			//virtual bool groupEnable();
			//virtual bool groupDisable();
		protected:
			shared_ptr<QVector<int>> axesIndex;	/*[0]:axisX,[1]:axisZ*/
			shared_ptr<QVector<void**>> groupIndex;
		};

		class PanTilt :public Device{
		public:
			PanTilt(QVector<int> index = QVector<int>(2, -1));
			bool pitch(qint32 vel);
			bool yaw(qint32 vel);
			bool stopPitch();
			bool stopYaw();
			bool readPitchVel(qint32* vel);
			bool readYawVel(qint32* vel);
			bool readPitchPos(qint32* pos);
			bool readYawPos(qint32* pos);
			bool pitchP2P(qint32 pos,qint32 refVel);
			bool yawP2P(qint32 pos, qint32 refVel);
			bool waitFinished(int axisIndex);
		};
		 
		class XYZ :public Device{
		public:
			XYZ(QVector<int> index = QVector<int>(4, -1));
			bool home(int axisindex);
			bool waitFinished(int axisIndex);
			bool moveX(qint32 vel);
			bool moveY(qint32 vel);
			bool moveZ(qint32 vel);
			bool moveR(qint32 vel);
			bool stopX();
			bool stopY();
			bool stopZ();
			bool stopR();
			bool readXPos(qint32* pos);
			bool readYPos(qint32* pos);
			bool readZPos(qint32* pos);
			bool readRPos(qint32* pos);
			bool readXVel(qint32* vel);
			bool readYVel(qint32* vel);
			bool readZVel(qint32* vel);
			bool readRVel(qint32* vel);
			bool moveXP2P(qint32 pos, qint32 refVel);
			bool moveYP2P(qint32 pos, qint32 refVel);
			bool moveZP2P(qint32 pos, qint32 refVel);
			bool moveRP2P(qint32 pos, qint32 refVel);
		};
	}

}
#endif