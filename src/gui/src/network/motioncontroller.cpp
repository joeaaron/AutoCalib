#include <stdint.h>
#include "ecatmc.h"
#include "motioncontroller.h"
using namespace ECAT::Network;

MOTION_ERROR MotionController::setupNetwork(QString ip, quint16 port){
	return MOTION_ERROR(controller_connect(ip.toLatin1().data(), port));
}

MOTION_ERROR MotionController::takeControl(){
	return MOTION_ERROR(controller_lock());
}

MOTION_ERROR MotionController::releaseControl(){
	return MOTION_ERROR(controller_unlock());
}

MOTION_ERROR MotionController::enableAxis(int index){
	return MOTION_ERROR(motion_axis_enable(index));
}

MOTION_ERROR MotionController::disableAxis(int index){
	return MOTION_ERROR(motion_axis_disable(index));
}

MOTION_ERROR MotionController::readAxisError(int index, qint32* error){
	return MOTION_ERROR(motion_axis_error_code(index,error));
}

MOTION_ERROR MotionController::readAxisEnabled(int index, char* flag){
	return MOTION_ERROR(motion_axis_enabled(index,flag));
}

MOTION_ERROR MotionController::readAxisStopped(int index, char* flag){
	return MOTION_ERROR(motion_axis_stopped(index, flag));
}

MOTION_ERROR MotionController::moveAxis(int index, qint32 vel){
	return MOTION_ERROR(motion_axis_move(index,vel));
}

MOTION_ERROR MotionController::stopAxis(int index){
	return MOTION_ERROR(motion_axis_stop(index));
}

MOTION_ERROR MotionController::waitAxisFinished(int index){
	return MOTION_ERROR(motion_axis_wait_finished(index));
}

MOTION_ERROR MotionController::p2pAxis(int index, qint32 pos, qint32 vel){
	return MOTION_ERROR(motion_axis_p2p(index, pos, vel));
}

MOTION_ERROR MotionController::homeAxis(int index){
	return MOTION_ERROR(motion_axis_home(index));
}

MOTION_ERROR MotionController::readAxisHomeMethod(int index, qint32* method){
	return MOTION_ERROR(motion_axis_get_home_method(index, method));
}

MOTION_ERROR MotionController::readAxisPostion(int index, qint32* pos){
	return MOTION_ERROR(motion_axis_position(index, pos));
}

MOTION_ERROR MotionController::readAxisVelocity(int index, qint32* vel){
	return MOTION_ERROR(motion_axis_velocity(index,vel));
}

MOTION_ERROR MotionController::createGroup(int *index, size_t size, void** group, const char* name){
	return MOTION_ERROR(motion_group_create(index, size, group, name));
}

MOTION_ERROR MotionController::enableGroup(void** group){
	return MOTION_ERROR(motion_group_enable(group));
}

MOTION_ERROR MotionController::homeGroup(void** group){
	return MOTION_ERROR(motion_group_home(group));
}

MOTION_ERROR MotionController::waitGroupFinished(void** group){
	return MOTION_ERROR(motion_group_wait_finished(group));
}

MOTION_ERROR MotionController::disableGroup(void** group){
	return MOTION_ERROR(motion_group_disable(group));
}

MOTION_ERROR MotionController::deleteGroup(void** group){
	return MOTION_ERROR(motion_group_delete(group));
}

MOTION_ERROR MotionController::p2pMotion(void** group, qint32* pos, qint32* vel, qint32 size){
	return MOTION_ERROR(motion_group_p2p(group, pos, vel, size));
}

MOTION_ERROR MotionController::setDigitalIO(quint8 *data, quint32 size){
	return MOTION_ERROR(set_digital_output(data, size));
}

MOTION_ERROR MotionController::digitalIOSize(quint32* size){
	return MOTION_ERROR(digital_output_size(size));
}