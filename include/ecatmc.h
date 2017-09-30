#ifndef ECATMC_API_H
#define ECATMC_API_H
#ifndef ECATMC_API 
#	ifdef WIN32
#		define ECATMC_API extern "C" __declspec(dllimport)
#	endif
#endif

ECATMC_API int controller_connect(const char* addr, int port);
ECATMC_API int controller_lock();
ECATMC_API int controller_unlock();
typedef void* motion_group_t;
typedef int axis_t;


/* API for digitial IO*/
ECATMC_API int digital_input_size(size_t *size);
ECATMC_API int digital_output_size(size_t* size);
ECATMC_API int digital_input(uint8_t* data, size_t size);
ECATMC_API int digital_output(uint8_t* data, size_t size);
ECATMC_API int set_digital_output(uint8_t* data, size_t size);

/* API for motion group */
ECATMC_API int motion_group_error_code(motion_group_t index, int32_t* error, size_t size);
ECATMC_API int motion_group_size(size_t *size);
ECATMC_API int motion_group_list(motion_group_t* data, size_t size);
ECATMC_API int motion_group_list_n(motion_group_t* data, size_t* size);

ECATMC_API int motion_group_halted(motion_group_t group, char* flag);
ECATMC_API int motion_group_stopped(motion_group_t group, char* flag);
ECATMC_API int motion_group_enabled(motion_group_t group, char* flag);
ECATMC_API int motion_group_finished(motion_group_t group, char* flag);
ECATMC_API int motion_group_pvt_busy(motion_group_t group, char* flag);
ECATMC_API int motion_group_wait_finished(motion_group_t group);
ECATMC_API int motion_group_wait_pvt(motion_group_t group);
ECATMC_API int motion_group_position(motion_group_t group, int32_t* pos, size_t n);
ECATMC_API int motion_group_velocity(motion_group_t group, int32_t* vel, size_t n);
ECATMC_API int motion_group_position_n(motion_group_t group, int32_t* pos, size_t* n);
ECATMC_API int motion_group_velocity_n(motion_group_t group, int32_t* vel, size_t* n);

typedef void* motion_pvt_t;
ECATMC_API int motion_pvt_create(motion_pvt_t* pvt, size_t axis_size);
ECATMC_API int motion_pvt_push(motion_pvt_t pvt);
ECATMC_API int motion_pvt_set(motion_pvt_t pvt, size_t axis, int32_t p, int32_t v, int32_t t);
ECATMC_API int motion_pvt_axis_size(motion_pvt_t pvt, size_t * size);
ECATMC_API int motion_pvt_item_size(motion_pvt_t pvt, size_t * size);
ECATMC_API int motion_pvt_clear(motion_pvt_t pvt);

ECATMC_API int motion_group_halt(axis_t index, char flag);
ECATMC_API int motion_group_enable(motion_group_t group);
ECATMC_API int motion_group_disable(motion_group_t group);
ECATMC_API int motion_group_pvt_start(motion_group_t group, motion_pvt_t pvt);
ECATMC_API int motion_group_pvt_push(motion_group_t group, motion_pvt_t pvt);
ECATMC_API int motion_group_stop(motion_group_t group);
ECATMC_API int motion_group_p2p(motion_group_t group, int32_t* pos, int32_t* vel, int32_t n);
ECATMC_API int motion_group_move(motion_group_t intdex, int32_t* vel, int32_t n);
ECATMC_API int motion_group_home(motion_group_t group);

ECATMC_API int motion_group_name(motion_group_t group, char* name, size_t size);
ECATMC_API int motion_group_from_name(const char* name, motion_group_t* group);
ECATMC_API int motion_group_create(axis_t* axis, size_t size, motion_group_t* group, const char* name);
ECATMC_API int motion_group_delete(motion_group_t group);
ECATMC_API int motion_group_axis(motion_group_t group, axis_t* axis, size_t size);
ECATMC_API int motion_group_axis_n(motion_group_t group, axis_t* axis, size_t* size);
ECATMC_API int motion_group_axis_size(motion_group_t group, size_t* size);


/* API for single axis */
ECATMC_API int motion_axis_error_code(axis_t index, int32_t* error);
ECATMC_API int motion_axis_stopped(axis_t index, char* flag);
ECATMC_API int motion_axis_enabled(axis_t index, char* flag);
ECATMC_API int motion_axis_halted(axis_t index, char* flag);
ECATMC_API int motion_axis_wait_finished(axis_t index);
ECATMC_API int motion_axis_finished(axis_t index, char* flag);
ECATMC_API int motion_axis_position(axis_t index, int32_t* p);
ECATMC_API int motion_axis_velocity(axis_t index, int32_t* v);

ECATMC_API int motion_axis_halt(axis_t index, char flag);
ECATMC_API int motion_axis_enable(axis_t index);
ECATMC_API int motion_axis_disable(axis_t index);
ECATMC_API int motion_axis_stop(axis_t index);
ECATMC_API int motion_axis_p2p(axis_t index, int32_t p, int32_t vel);
ECATMC_API int motion_axis_move(axis_t intdex, int32_t vel);
ECATMC_API int motion_axis_home(axis_t index);
ECATMC_API int motion_axis_name(axis_t axis, char* name, size_t size);
ECATMC_API int motion_axis_from_name(const char* name, axis_t* axis);
ECATMC_API int motion_axis_set_acc(axis_t index, int32_t acc, int32_t dec);
ECATMC_API int motion_axis_get_acc(axis_t index, int32_t* acc, int32_t* dec);
ECATMC_API int motion_axis_set_home_method(axis_t index, int32_t method);
ECATMC_API int motion_axis_get_home_method(axis_t index, int32_t* method);
ECATMC_API int motion_axis_set_home_vel(axis_t index, int32_t vel1, int32_t vel2);
ECATMC_API int motion_axis_get_home_vel(axis_t index, int32_t* vel1, int32_t* vel2);
ECATMC_API int motion_axis_set_home_acc(axis_t index, int32_t acc);
ECATMC_API int motion_axis_get_home_acc(axis_t index, int32_t* acc);
typedef struct  
{
	uint16_t idx;
	int32_t pos;
} axis_latched_pos_t;
ECATMC_API int motion_axis_latched_position_n(axis_t index, axis_latched_pos_t* pos, size_t* size);
ECATMC_API int motion_axis_latched_has_position(axis_t index, char* flag);
ECATMC_API int motion_axis_latched_top(axis_t index, axis_latched_pos_t* pos);

/* API for Robotics motion */
typedef int robot_t;
typedef void* robot_path_t;
typedef struct
{
	double x;
	double y;
	double z;
	double roll;
	double pitch;
	double yaw;
} robot_pose_t;
ECATMC_API int robot_enabled(robot_t robot, char* flag);
ECATMC_API int robot_stopped(robot_t robot, char* flag);
ECATMC_API int robot_wait_finished(robot_t robot);
ECATMC_API int robot_finished(robot_t robot, char* flag);
ECATMC_API int robot_position(robot_t robot, ::robot_pose_t* pose);
ECATMC_API int robot_velocity(robot_t robot, ::robot_pose_t* vel);
ECATMC_API int robot_joint_position(robot_t robot, int32_t* pos, size_t n);
ECATMC_API int robot_joint_velocity(robot_t robot, int32_t* vel, size_t n);

ECATMC_API int robot_enable(robot_t robot);
ECATMC_API int robot_disable(robot_t robot);
ECATMC_API int robot_stop(robot_t robot);
ECATMC_API int robot_p2p(robot_t robot, ::robot_pose_t pose, double vel);
ECATMC_API int robot_joint_p2p(robot_t robot, int32_t* pos, int32_t* vel, size_t n);
ECATMC_API int robot_home(robot_t robot);
ECATMC_API int robot_start_path(robot_t robot, robot_path_t path, int32_t mode);

ECATMC_API int robot_name(robot_t robot, char* name, size_t size);
ECATMC_API int robot_from_name(const char* name, robot_t* robot);

ECATMC_API int robot_path_create(robot_t robot, robot_path_t* path);
ECATMC_API int robot_path_push_line(robot_t robot, robot_path_t path, ::robot_pose_t start, ::robot_pose_t end, double smooth, double vel);
ECATMC_API int robot_path_push_circle(robot_t robot, robot_path_t path, ::robot_pose_t start, ::robot_pose_t via, ::robot_pose_t end, double smooth, double vel);
ECATMC_API int robot_path_clear(robot_t robot, robot_path_t path);


#endif