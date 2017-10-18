#include <stdint.h>
#include "ecatmc.h"
#include "motioncontroller.h"
#include <windows.h>
using namespace ECAT::Network;

MOTION_ERROR MotionController::setupNetwork(QString ip, quint16 port){
	MOTION_ERROR err = MOTION_ERROR(controller_connect(ip.toLatin1().data(), port));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(controller_connect(ip.toLatin1().data(), port));
	}
	return err;
}

MOTION_ERROR MotionController::takeControl(){
	MOTION_ERROR err = MOTION_ERROR(controller_lock());
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(controller_lock());
	}
	return err;
}

MOTION_ERROR MotionController::releaseControl(){
	MOTION_ERROR err = MOTION_ERROR(controller_unlock());
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(controller_unlock());
	}
	return err;
}

MOTION_ERROR MotionController::enableAxis(int index){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_enable(index));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_enable(index));
	}
	return err;
}

MOTION_ERROR MotionController::disableAxis(int index){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_disable(index));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_disable(index));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisError(int index, qint32* error){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_error_code(index, error));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_error_code(index, error));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisEnabled(int index, char* flag){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_enabled(index, flag));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_enabled(index, flag));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisStopped(int index, char* flag){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_stopped(index, flag));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_stopped(index, flag));
	}
	return err;
}

MOTION_ERROR MotionController::moveAxis(int index, qint32 vel){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_move(index, vel));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_move(index, vel));
	}
	return err;
}

MOTION_ERROR MotionController::stopAxis(int index){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_stop(index));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_stop(index));
	}
	return err;
}

MOTION_ERROR MotionController::waitAxisFinished(int index){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_wait_finished(index));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_wait_finished(index));
	}
	return err;

}

MOTION_ERROR MotionController::p2pAxis(int index, qint32 pos, qint32 vel){
	int times = 0;
	MOTION_ERROR err = MOTION_ERROR_NONE;
	do 
	{
		times++;
		err = MOTION_ERROR(motion_axis_p2p(index, pos, vel));
		if (MOTION_ERROR_NONE == err)
		{
			return MOTION_ERROR_NONE;
		}
		Sleep(50);
		MOTION_ERROR(motion_axis_stop(index));
		Sleep(50);
	} while (times < 5);
	return err;
}

MOTION_ERROR MotionController::homeAxis(int index){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_home(index));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_home(index));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisHomeMethod(int index, qint32* method){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_get_home_method(index, method));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_get_home_method(index, method));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisPostion(int index, qint32* pos){
	MOTION_ERROR err = MOTION_ERROR(motion_axis_position(index, pos));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_position(index, pos));
	}
	return err;
}

MOTION_ERROR MotionController::readAxisVelocity(int index, qint32* vel){

	MOTION_ERROR err = MOTION_ERROR(motion_axis_velocity(index, vel));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(motion_axis_velocity(index, vel));
	}
	return err;
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
	MOTION_ERROR err = MOTION_ERROR(set_digital_output(data, size));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(set_digital_output(data, size));
	}
	return err;
}

MOTION_ERROR MotionController::digitalIOSize(quint32* size){
	MOTION_ERROR err = MOTION_ERROR(digital_output_size(size));
	if (MOTION_ERROR_NONE != err)
	{
		err = MOTION_ERROR(digital_output_size(size));
	}
	return err;
}