#include "device.h"
using namespace ECAT::Network;


Device::Device(QVector<int> index /* = QVector<int>(2 , -1)*/){
	axesIndex = make_shared<QVector<int>>(index);
}

void Device::setAxes(shared_ptr<QVector<int>> index){
	axesIndex = index;
}

bool Device::enable(){
	MOTION_ERROR err;
	for (auto index : (*axesIndex)){
		err = MotionController::getInstance().enableAxis(index);
		if (MOTION_ERROR_NONE != err){
			HANDLE_MOTION_ERROR(err);
			return false;
		}
	}
	return true;
}

bool Device::disable(){
	MOTION_ERROR err;
	for (auto index : (*axesIndex)){
		err = MotionController::getInstance().disableAxis(index);
		if (MOTION_ERROR_NONE != err){
			HANDLE_MOTION_ERROR(err);
			return false;
		}
	}
	return true;
}

bool Device::enabled() const{
	MOTION_ERROR err;
	char flag;
	for (auto index : (*axesIndex)){
		err = MotionController::getInstance().readAxisEnabled(index, &flag);
		if (MOTION_ERROR_NONE != err){
			HANDLE_MOTION_ERROR(err);
			return false;
		}
		if (!flag) return false;
	}
	return true;
}


PanTilt::PanTilt(QVector<int> index /* = QVector<int>(2 , -1)*/)
	:Device(index)
{

}

/*pitch is motion which rotation around axis X(axesIndex[0])*/
bool PanTilt::pitch(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(0), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::stopPitch(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(0));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::pitchP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(0),pos,refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::readPitchVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(0),vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::readPitchPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(0), pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}
/*yaw is motion which rotation around axis Z(axesIndex[1])*/
bool PanTilt::yaw(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(1), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::stopYaw(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(1));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::yawP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(1), pos, refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::readYawVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(1),vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::readYawPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(1), pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool PanTilt::waitFinished(int axesIndex){
	MOTION_ERROR err = MotionController::getInstance().waitAxisFinished(axesIndex);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}


XYZ::XYZ(QVector<int> index /* = QVector<int>(4 *, -1)*/)
	:Device(index)
{

}

bool XYZ::home(int axisindex){
	MOTION_ERROR err = MotionController::getInstance().homeAxis(axisindex);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::waitFinished(int axesIndex){
	MOTION_ERROR err = MotionController::getInstance().waitAxisFinished(axesIndex);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

/*moveX is motion which move along axis X(axesIndex[0])*/
bool XYZ::moveX(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(0), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::stopX(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(0));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::moveXP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(0), pos, refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readXPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(0),pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readXVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(0), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

/*moveY is motion which move along axis Y(axesIndex[1])*/
bool XYZ::moveY(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(1), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::stopY(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(1));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::moveYP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(1), pos, refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readYPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(1), pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readYVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(1), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

/*moveZ is motion which move along axis Z(axesIndex[2])*/
bool XYZ::moveZ(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(2), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::stopZ(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(2));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::moveZP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(2), pos, refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readZPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(2), pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readZVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(2), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

/*moveR is motion which rotation around axis Z(axesIndex[3])*/
bool XYZ::moveR(qint32 vel){
	MOTION_ERROR err = MotionController::getInstance().moveAxis(axesIndex->at(3), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::stopR(){
	MOTION_ERROR err = MotionController::getInstance().stopAxis(axesIndex->at(3));
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::moveRP2P(qint32 pos, qint32 refVel){
	MOTION_ERROR err = MotionController::getInstance().p2pAxis(axesIndex->at(3), pos, refVel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readRPos(qint32* pos){
	MOTION_ERROR err = MotionController::getInstance().readAxisPostion(axesIndex->at(3), pos);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}

bool XYZ::readRVel(qint32* vel){
	MOTION_ERROR err = MotionController::getInstance().readAxisVelocity(axesIndex->at(3), vel);
	if (MOTION_ERROR_NONE != err){
		HANDLE_MOTION_ERROR(err);
		return false;
	}
	else
		return true;
}
