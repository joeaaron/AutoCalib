#ifndef _ECAT_MOTIONCONTROLLER_H_
#define _ECAT_MOTIONCONTROLLER_H_
#include <QtCore>
#include <glog/logging.h>
namespace ECAT{
	namespace Network{
		// Error Code for MotionController
		enum MOTION_ERROR:int
		{
			MOTION_ERROR_BUSY = 170,												// axis is occupied by other group
			MOTION_ERROR_NONE = 0,													// return without error
			MOTION_ERROR_ARG = -1,													// pass wrong argument to function
			MOTION_ERROR_DEVICE = -3,												// unsupported command at current state
			MOTION_ERROR_HALTED = -4,												// at emergency stop state
			MOTION_ERROR_COE = -5,													// parameter set unsuccessfully
			MOTION_ERROR_NOTLATCHED = -6,											// no latched data
			MOTION_ERROR_NAME = -7,													// wrong name
			MOTION_ERROR_MOREDATA = -9,												// more data to read
			MOTION_ERROR_CONNCTION = -10,											// failed to connect to rpc server
			MOTION_ERROR_AUTHORITY = -1000											// no authority
		};
	/*
		\brief A macro function to log motion error
		\param[in] err: detailed MOTION_ERROR
		\param[out] error record in logger files
	*/
#define HANDLE_MOTION_ERROR(err) \
	switch (err) \
	{ \
	case ECAT::Network::MOTION_ERROR_BUSY: \
		LOG(ERROR)<<"MOTION_ERROR_BUSY: Axis is occupied by other group \
or current program hasn't connected to controller runtime."; \
		break; \
	case ECAT::Network::MOTION_ERROR_ARG: \
		LOG(ERROR)<<"MOTION_ERROR_ARG: Pass wrong argument to function."; \
		break; \
	case ECAT::Network::MOTION_ERROR_DEVICE: \
		LOG(ERROR)<<"MOTION_ERROR_DEVICE: Unsupported command at current state."; \
		break; \
	case ECAT::Network::MOTION_ERROR_HALTED: \
		LOG(ERROR)<<"MOTION_ERROR_HALTED: At emergency stop state."; \
		break; \
	case ECAT::Network::MOTION_ERROR_COE: \
		LOG(ERROR)<<"MOTION_ERROR_COE: Parameter set unsuccessfully."; \
		break; \
	case ECAT::Network::MOTION_ERROR_NOTLATCHED: \
		LOG(ERROR)<<"MOTION_ERROR_NOTLATCHED: No latched data."; \
		break; \
	case ECAT::Network::MOTION_ERROR_NAME:\
		LOG(ERROR)<<"MOTION_ERROR_NAME: Wrong name."; \
		break; \
	case ECAT::Network::MOTION_ERROR_MOREDATA: \
		LOG(ERROR)<<"MOTION_ERROR_MOREDATA: More data to read."; \
		break; \
	case ECAT::Network::MOTION_ERROR_CONNCTION: \
		LOG(ERROR)<<"MOTION_ERROR_CONNCTION: More data to read."; \
		break; \
	case ECAT::Network::MOTION_ERROR_AUTHORITY:	\
		LOG(ERROR)<<"MOTION_ERROR_AUTHORITY: No authority."; \
		break; \
	default: \
		break; \
	} \
		// MotionController class is designed as a singleton
		class MotionController:public QObject{
			Q_OBJECT
		private:
			MotionController(){}													// forbid constructor make sure only one instance
			MotionController(const MotionController&);								// forbid copy constructor make sure only one instance
			MotionController& operator=(const MotionController&);					// forbid operator= make sure only one instance
		public:
			static MotionController& getInstance(){
				static MotionController instance;
				return instance;
			}

			/* \brief	Setup the controller network environment
			   \return	MOTION_ERROR_BUSY: has connected to a controller runtime
						MOTION_ERROR_NONE: success
						MOTION_ERROR_CONNCTION: failure 
			*/
			MOTION_ERROR setupNetwork(QString ip, quint16 port);

			/* \brief	Take control of controller runtime
			   \return	MOTION_ERROR_BUSY: control taken by other process
						MOTION_ERROR_NONE: success
			*/
			MOTION_ERROR takeControl();

			/* \brief	Release control to controller runtime
			   \return	MOTION_ERROR_BUSY: current program hasn't connected to controller runtime
						MOTION_ERROR_NONE: success \
						MOTION_ERROR_AUTHORITY: failed to release control due to occupation by other process
			*/
			MOTION_ERROR releaseControl();
			
			/*	\brief	Enable axis
				\param[in]	index:Index of axis
				\return	MOTION_ERROR_BUSY: current axis has been used by group
						MOTION_ERROR_NONE: success
						MOTION_MOTION_ARG: wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR enableAxis(int index);

			/*	\brief	Enable axis
				\param[in]	index:Index of axis
				\return	MOTION_ERROR_BUSY: current axis has been used by group
						MOTION_ERROR_NONE: success
						MOTION_MOTION_ARG: wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR disableAxis(int index);

			/*
				\brief Velocity Motion Mode:control the axis moving at fixed velocity.
				\param[in] index:Index of axis	
				\param[in] vel:expected velocity
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_DEVICE:disable the axis
						MOTION_ERROR_HALTED:at emergency stop state
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR moveAxis(int index,qint32 vel);

			/*
				\brief Home Mode Position:control the axis to the zero position.
					   <b>this function should not be called until home method is set.</b>
				\param[in] index:Index of axis
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_DEVICE:disable the axis
						MOTION_ERROR_HALTED:at emergency stop state
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR homeAxis(int index);

			/*
				\brief Wait for the completion of last command,this function is blocking.If any error, function return immmediately

			*/
			MOTION_ERROR waitAxisFinished(int index);

			/*
				\brief Position Motion Mode:control the axis moving to expected position at at fixed velocity.
					   <b>this function should not be called until homeAxis() is called</b>
				\param[in] index:Index of axis
				\param[in] pos:expected position
				\param[in] vel:expected velocity
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_DEVICE:disable the axis
						MOTION_ERROR_HALTED:at emergency stop state
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR p2pAxis(int index, qint32 pos, qint32 vel);

			/*
				\brief Stop the motion of axis
				\param[in] index:Index of axis
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_DEVICE:the axis is disabled
						MOTION_ERROR_HALTED:at emergency stop state
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR stopAxis(int index);

			/*
				\brief	Read error code of axis
				\param[in]	index:Index of axis
				\param[out] error:Error code of axis driver: error code represented by hexadecimal number,
							where started by 0xF is warning information, otherwise is error(which will disable axis  automatically)
				\return	MOTION_ERROR_BUSY: current axis has been used by group
						MOTION_ERROR_NONE: success
						MOTION_MOTION_ARG: wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisError(int index, qint32* error);

			/*
				\brief	Read the availability of axis
				\param[in]	index:Index of axis
				\param[out]	flag: If flag != 0, axis is enabled,otherwise disabled
				\return MOTION_ERROR_BUSY: current axis has been used by group
						MOTION_ERROR_NONE: success
						MOTION_MOTION_ARG: wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisEnabled(int index, char* flag);

			/*
				\brief Read the motion of axis.
				\param[in] index:Index of axis
				\param[out] flag: If flag != 0, axis is stopped,otherwise moving
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_DEVICE:disable the axis
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisStopped(int index, char* flag);

			/*
				\brief Read axis home method.
				\param[in] index:Index of axis
				\param[out] method:Method of homing
				\return MOTION_ERROR_NONE:success
						MOTION_ERROR_COE:failed to read method,try again.If still failed, restart the whole motion system
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisHomeMethod(int index, qint32* method);

			/*
				\brief Read axis position
				\param[in] index:Index of axis
				\param[out] pos:Position of axis
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisPostion(int index, qint32* pos);

			/*
				\brief Read axis velocity
				\param[in] index:Index of axis
				\param[out] vel:Velocity of axis
				\return MOTION_ERROR_BUSY:current axis has been used by group
						MOTION_ERROR_NONE:success
						MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR readAxisVelocity(int index, qint32* vel);

			/*
				\brief Create motion group
				\param[in] index:Index list of motion group axis
				\param[in] size:Num of motion group axis
				\param[in] name:Name of motion group
				\param[out] group:Index of motion group
				\return  MOTION_ERROR_BUSY:current axis has been used by group
				         MOTION_ERROR_NONE:success
						 MOTION_MOTION_NAME:already occupied by other motion_groups
						 MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						 MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR MotionController::createGroup(int *index, size_t size, void** group, const char* name);

			/*
				\brief Enable group
				\param[in] group:Index of motion group
				\return MOTION_ERROR_NONE:success
				        MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR MotionController::enableGroup(void** group);

			/*
				\brief Home Mode Position:control the group to the zero position.
				       <b>this function should not be called until home method is set.</b>
				\param[in] group:Index of motion group
				\return MOTION_ERROR_NONE:success
				        MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR MotionController::homeGroup(void** group);

			/*
			     \brief Wait for the completion of last command,this function is blocking.If any error, function return immmediately
			*/
			MOTION_ERROR MotionController::waitGroupFinished(void** group);

			/*
				\brief Disable group
				\param[in] group:Index of motion group
				\return MOTION_ERROR_NONE:success
				        MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR MotionController::disableGroup(void** group);

			/*
				\brief Delete group
				\param[in] group:Index of motion group
				\return MOTION_ERROR_NONE:success
				        MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/
			MOTION_ERROR MotionController::deleteGroup(void** group);

			/*
				\brief Delete group
				\param[in] group:Index of motion group
				\param[in] pos:Expected position
				\param[in] vel:Expected velocity
				\param[in] size:Num of motion group axis
				\return MOTION_ERROR_NONE:success
				        MOTION_MOTION_ARG:wrong parameter
						MOTION_ERROR_CONNCTION: can't connect to rpc runtime
						MOTION_ERROR_AUTHORITY: no authority
			*/ 
			MOTION_ERROR MotionController::p2pMotion(void** group, qint32* pos, qint32* vel, qint32 size);

			/*
				\brief Set digital I/O output status
				\param[in] data:Digital I/O output instruction
				\param[in] size:The number of I/O output
				\return MOTION_ERROR_NONE:success
				MOTION_MOTION_ARG:wrong parameter
				MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR MotionController::setDigitalIO(quint8 *data, quint32 size);

			/*
			\brief Read digital I/O output num
			\param[in] size:The number of I/O output
			\return MOTION_ERROR_NONE:success
			MOTION_ERROR_CONNCTION: can't connect to rpc runtime
			*/
			MOTION_ERROR MotionController::digitalIOSize(quint32* size);
		};
	}
}
#endif