#include <iostream>
#include <stdint.h>
#include "serialport.h"
#include <string>
#include <sstream>
#include <Windows.h>
using namespace std;
#ifdef WIN32
#	define ECATMC_API extern "C" __declspec(dllexport)
#endif
#include "ecatmc.h"
#define DEBUG_INFO(...) //printf

#define SERIAL_SIZE 2
CSerialPort serial_port[SERIAL_SIZE];

void clear_serial_port(int index)
{
	serial_port[index].Enter_Critical();
	bool is_empty = serial_port[index].fifo.is_empty();
	if (!is_empty)
	{
		serial_port[index].fifo.reset();
		serial_port[index].Leave_Critical();
	}
	else
	{
		serial_port[index].Leave_Critical();
		return;
	}
}
string serial_read_data(int index)
{
	string data = "";
	serial_port[index].Enter_Critical();
	bool is_empty = serial_port[index].fifo.is_empty();
	serial_port[index].Leave_Critical();
	while (!is_empty)
	{
		char c = 0;
		serial_port[index].Enter_Critical();
		serial_port[index].fifo.out(&c, 1);
		data = data + c;
		is_empty = serial_port[index].fifo.is_empty();
		serial_port[index].Leave_Critical();
	}
	return data;
}
int select_axis(int index)
{
	int serial_index = 0;
	if (index > 3)
	{
		serial_index = 1;
	}
	clear_serial_port(serial_index);
	//—°÷–÷·
	string cmd;
	stringstream ss;

	ss << index + 3;
	cmd = "\\" + ss.str() + "\r";

	serial_port[serial_index].WriteData((unsigned char *)cmd.c_str(), cmd.size());
	string status = "";
	int times = 0;
	do
	{
		times++;
		status = status + serial_read_data(serial_index);
		if (-1 != status.find(ss.str() + "->"))
		{
			cmd = "\r";
			serial_port[serial_index].WriteData((unsigned char *)cmd.c_str(), cmd.size());
			return 0;
		}
	} while (times < 100);
	return -1;
}
int send_cmd(int axis_index, string cmd)
{
	int serial_index = 0;
	int sended_size = 0;
	if (axis_index > 3)
	{
		serial_index = 1;
	}
	clear_serial_port(serial_index);
	cmd = cmd + "\r";
	while (sended_size < cmd.size())
	{
		int size = ((cmd.size() - sended_size) > 5) ? 5 : (cmd.size() - sended_size);
		serial_port[serial_index].WriteData((unsigned char *)cmd.c_str() + sended_size, size);
		sended_size += size;
		Sleep(20);
	}
	
	return 0;
}
string get_status(int index)
{
	int serial_index = 0;
	if (index > 3)
	{
		serial_index = 1;
	}
	stringstream ss;
	ss << index + 3;

	string status;
	int times = 0;
	do
	{
		times++;
		status = status + serial_read_data(serial_index);
		int index = status.find(ss.str() + "->");
		if (-1 != index)
		{
			return status.substr(0, index);
		}
	} while ((status != "" && times < 1000) || (status == "" && times < 100));
	return "";
}
string int_to_str(int32_t num)
{
	stringstream ss;
	ss << num;
	return ss.str();
}
string double_to_str(double num)
{
	char buff[64];
	sprintf(buff, "%.2f", num);
	return std::string(buff);
}
int digital_input_size(size_t *size)
{
	return 0;
}
int digital_output_size(size_t* size)
{
	return 0;
}
int digital_input(uint8_t* data, size_t size)
{
	return 0;
}
int digital_output(uint8_t* data, size_t size)
{
	return 0;
}
int set_digital_output(uint8_t* data, size_t size)
{
	int times = 0;
	int ret = 0;
	int is_enable;
	do
	{
		times++;
		ret == select_axis(3);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	Sleep(100);
	for (int i = 0; i < (int64_t)size; i++)
	{
		times = 0;
		char flag = 0;
		do
		{  
			times++;
			ret = send_cmd(2, "out " + int_to_str(i * 2 + 8) + " " + int_to_str(data[i]));
			if (ret != 0)
			{
				continue;
			}
			Sleep(50);
		} while (times < 5);
	}
	return 0;
}


int motion_group_error_code(motion_group_t group, int32_t* error, size_t size)
{
	return 0;
}
int motion_group_size(size_t *size)
{
	return 0;
}
int motion_group_stopped(motion_group_t group, char* flag)
{
	return 0;
}
int motion_group_enabled(motion_group_t group, char* flag)
{
	return 0;
}
int motion_group_finished(motion_group_t group, char* flag)
{
	return 0;
}
int motion_group_pvt_busy(motion_group_t group, char* flag)
{
	return 0;
}
int motion_group_wait_finished(motion_group_t group)
{
	return 0;
}
int motion_group_wait_pvt(motion_group_t group)
{
	return 0;
}
int motion_group_position(motion_group_t group, int32_t* pos, size_t n)
{
	return 0;
}
int motion_group_velocity(motion_group_t group, int32_t* vel, size_t n)
{
	return 0;
}
int motion_group_position_n(motion_group_t group, int32_t* pos, size_t* n)
{
	return 0;
}
int motion_group_velocity_n(motion_group_t group, int32_t* vel, size_t* n)
{
	return 0;
}
int motion_group_halted(motion_group_t group, char* flag)
{
	return 0;
}

int motion_pvt_create(motion_pvt_t* pvt, size_t axis_size)
{
	return 0;
}
int motion_pvt_push(motion_pvt_t _pvt)
{
	return 0;
}
int motion_pvt_set(motion_pvt_t _pvt, size_t axis, int32_t p, int32_t v, int32_t t)
{
	return 0;
}
int motion_pvt_axis_size(motion_pvt_t _pvt, size_t * size)
{
	return 0;
}
int motion_pvt_item_size(motion_pvt_t _pvt, size_t * size)
{
	return 0;
}
int motion_pvt_clear(motion_pvt_t _pvt)
{
	return 0;
}

int motion_group_halt(axis_t group, char flag)
{
	return 0;
}
int motion_group_enable(motion_group_t group)
{
	return 0;
}
int motion_group_disable(motion_group_t group)
{
	return 0;
}
int motion_group_pvt_start(motion_group_t group, motion_pvt_t _pvt)
{
	return 0;
}
int motion_group_pvt_push(motion_group_t group, motion_pvt_t _pvt)
{
	return 0;
}
int motion_group_stop(motion_group_t group)
{
	return 0;
}
int motion_group_home(motion_group_t group)
{
	return 0;
}
int motion_group_p2p(motion_group_t group, int32_t* pos, int32_t* vel, int32_t n)
{
	return 0;
}
int motion_group_move(motion_group_t group, int32_t* vel, int32_t n)
{
	return 0;
}

int motion_group_name(motion_group_t group, char* name, size_t size)
{
	return 0;
}
int motion_group_from_name(const char* name, motion_group_t* group)
{
	return 0;
}
int motion_group_list(motion_group_t* data, size_t size)
{
	return 0;
}
int motion_group_list_n(motion_group_t* data, size_t* size)
{
	return 0;
}
int motion_group_create(axis_t* axis, size_t size, motion_group_t* group, const char* name)
{
	return 0;
}
int motion_group_delete(motion_group_t group)
{
	return 0;
}
int motion_group_axis(motion_group_t group, axis_t* axis, size_t size)
{
	return 0;
}
int motion_group_axis_n(motion_group_t group, axis_t* axis, size_t* size)
{
	return 0;
}
int motion_group_axis_size(motion_group_t group, size_t* size)
{
	return 0;
}

int motion_axis_error_code(axis_t index, int32_t* error)
{
	return 0;
}
int motion_axis_stopped(axis_t index, char* flag)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	do
	{
		times++;
		ret = send_cmd(index, "stopped");
		if (ret != 0)
		{
			continue;
		}
		Sleep(100);
		string status = get_status(index);
		stringstream ss;
		int _index = status.find_first_not_of("\r\n");
		if (_index == -1)
		{
			continue;
		}
		status = status.substr(_index, status.size());
		_index = status.find("\r\n");
		if (_index != -1)
		{
			status = status.substr(0, _index);
		}
		ss << status;
		int is_stopped;
		ss >> is_stopped;
		if (is_stopped == -1 || is_stopped == 0 || is_stopped == 1 || is_stopped == 2)
		{
			if (is_stopped == 0)
			{
				*flag = 0;
			}
			else
			{
				*flag = 1;
			}
			return 0;
		}
	} while (times < 5);
	return -1;
}
int motion_axis_enabled(axis_t index, char* flag)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	do
	{
		times++;
		ret = send_cmd(index, "active");
		if (ret != 0)
		{
			continue;
		}
		string status = get_status(index);
		stringstream ss;
		int _index = status.find_first_not_of("\r\n");
		if (_index == -1)
		{
			continue;
		}
		status = status.substr(_index, status.size());
		_index = status.find("\r\n");
		if (_index != -1)
		{
			status = status.substr(0, _index);
		}
		ss << status;
		int is_enable;
		ss >> is_enable;
		if (is_enable == 0 || is_enable == 1)
		{
			*flag = is_enable;
			return 0;
		}
	} while (times < 100);
	return -1;
}
int motion_axis_wait_finished(axis_t index)
{
	while (1)
	{
		Sleep(5);
		char flag;
		int ret = motion_axis_finished(index, &flag);
		if (0 != ret)
		{
			continue;
		}
		if (flag != 1)
		{
			continue;
		}
		else
		{
			return 0;
		}
	}
}
int motion_axis_finished(axis_t index, char* flag)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	do
	{
		times++;
		ret = send_cmd(index,"stopped");
		if (ret != 0)
		{
			continue;
		}
		string status = get_status(index);
		stringstream ss;
		int _index = status.find_first_not_of("\r\n");
		if (_index == -1)
		{
			continue;
		}
		status = status.substr(_index, status.size());
		_index = status.find("\r\n");
		if (_index != -1)
		{
			status = status.substr(0, _index);
		}
		ss << status;
		int is_stopped;
		ss >> is_stopped;
		if (is_stopped == -1 || is_stopped == 0 || is_stopped == 1 || is_stopped == 2)
		{
			if (is_stopped == 2)
			{
				*flag = 1;
			}
			else
			{
				*flag = 0;
			}
			return 0;
		}
	} while (times < 100);
	return -1;
}
int motion_axis_position(axis_t index, int32_t* p)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	do
	{
		times++;
		ret = send_cmd(index,"pfb");
		if (ret != 0)
		{
			continue;
		}
		string status = get_status(index);
		stringstream ss;
		int _index = status.find_first_not_of("\r\n");
		if (_index == -1)
		{
			continue;
		}
		status = status.substr(_index, status.size());
		_index = status.find(".00");
		if (_index != -1)
		{
			status = status.substr(0, _index);
		}
		if (status != "")
		{
			ss << status;
			ss >> *p;
			return 0;
		}
		else
		{
			continue;
		}
		Sleep(50);
	} while (times < 5);
	return -1;
}
int motion_axis_velocity(axis_t index, int32_t* v)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	do
	{
		times++;
		ret = send_cmd(index,"v");
		if (ret != 0)
		{
			continue;
		}
		string status = get_status(index);
		stringstream ss;
		int _index = status.find_first_not_of("\r\n");
		if (_index == -1)
		{
			continue;
		}
		status = status.substr(_index, status.size());
		_index = status.find(" ");
		if (_index != -1)
		{
			status = status.substr(0, _index);
		}
		if (status != "")
		{
			ss << status;
			double vel = 0;
			ss >> vel;
			*v = (int32_t)(vel * 131072.0 / 60);
			return 0;
		}
		else
		{
			continue;
		}
		Sleep(50);
	} while (times < 5);
	return -1;
}
int motion_axis_halted(axis_t index, char* flag)
{
	return 0;
}
int motion_axis_enable(axis_t index)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	char flag = 0;
	do
	{
		times++;
		send_cmd(index,"en");
		flag = 0;
		int ret = motion_axis_enabled(index, &flag);
		if (0 != ret)
		{
			continue;
		}
		else
		{
			if (flag == 1)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		Sleep(50);
	} while (times < 5);
	if (flag == 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
int motion_axis_disable(axis_t index)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}

	times = 0;
	char flag = 1;
	do
	{
		times++;
		send_cmd(index, "k");
		flag = 1;
		int ret = motion_axis_enabled(index, &flag);
		if (0 != ret)
		{
			continue;
		}
		else
		{
			if (flag == 0)
			{
				return 0;
			}
			else
			{
				continue;
			}
		}
	} while (times < 100);
	if (flag == 1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
int motion_axis_stop(axis_t index)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	char flag = 0;
	do
	{
		times++;
		send_cmd(index,"stop");
		flag = 0;
		int ret = motion_axis_stopped(index, &flag);
		if (0 != ret)
		{
			continue;
		}
		else
		{
			if (flag == 1)
			{
				return 0;
			}
			else
			{
				continue;
			}
		}
	} while (times < 100);
	if (flag == 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
int motion_axis_p2p(axis_t index, int32_t p, int32_t vel)
{
	int times = 0;
	int ret = 0;
	do
	{
		times++;
		ret == select_axis(index);
		if (ret == 0)
		{
			break;
		}
	} while (times < 100);

	if (ret == -1)
	{
		return -1;
	}
	times = 0;
	char flag = 0;
	do
	{
		times++;
		char buffer[256];
		send_cmd(index, "moveabs " + int_to_str(p) + " " + double_to_str(vel * 1.0 / 131072 * 60));
		flag = 0;
		Sleep(100);
		int ret = motion_axis_stopped(index, &flag);
		if (0 != ret)
		{
			continue;
		}
		else
		{
			if (flag == 1)
			{
				ret = motion_axis_finished(index, &flag);
				if (0 != ret)
				{
					continue;
				}
				if (flag != 0)
				{
					int32_t pos_cur = 0;
					ret = motion_axis_position(index, &pos_cur);
					if (0 != ret)
					{
						continue;
					}
					if (abs(pos_cur - p) < 500)
					{
						return 0;
					}
					else
					{
						continue;
					}
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
	} while (times < 5);
	return -1;
}
int motion_axis_home(axis_t index)
{
	return 0;
}
int motion_axis_move(axis_t index, int32_t vel)
{

	if (vel > 0)
		return motion_axis_p2p(index, 2147483640, vel);
	else
		return motion_axis_p2p(index, -2147483640, -vel);
}
int motion_axis_halt(axis_t index, char flag)
{
	return 0;
}

int motion_axis_name(axis_t axis, char* name, size_t size)
{
	return 0;
}
int motion_axis_from_name(const char* name, axis_t* axis)
{
	return 0;
}
int motion_axis_set_acc(axis_t index, int32_t acc, int32_t dec)
{
	return 0;
}
int motion_axis_get_acc(axis_t index, int32_t *acc, int32_t *dec)
{
	return 0;
}
int motion_axis_set_home_method(axis_t index, int32_t method)
{
	return 0;
}
int motion_axis_get_home_method(axis_t index, int32_t* method)
{
	return 0;
}
int motion_axis_set_home_vel(axis_t index, int32_t vel1, int32_t vel2)
{
	return 0;
}
int motion_axis_get_home_vel(axis_t index, int32_t* vel1, int32_t* vel2)
{
	return 0;
}
int motion_axis_set_home_acc(axis_t index, int32_t acc)
{
	return 0;
}
int motion_axis_get_home_acc(axis_t index, int32_t* acc)
{
	return 0;
}

int motion_axis_latched_position_n(axis_t index, axis_latched_pos_t* pos, size_t* size)
{
	return 0;
}
int motion_axis_latched_has_position(axis_t index, char* flag)
{
	return 0;
}
int motion_axis_latched_top(axis_t index, axis_latched_pos_t* pos)
{
	return 0;
}


int robot_enabled(robot_t robot, char* flag)
{
	return 0;
}
int robot_stopped(robot_t robot, char* flag)
{
	return 0;
}
int robot_wait_finished(robot_t robot)
{
	return 0;
}
int robot_finished(robot_t robot, char* flag)
{
	return 0;
}
int robot_position(robot_t robot, ::robot_pose_t* pose)
{
	return 0;
}
int robot_velocity(robot_t robot, ::robot_pose_t* vel)
{
	return 0;
}
int robot_joint_position(robot_t robot, int32_t* pos, size_t n)
{
	return 0;
}
int robot_joint_velocity(robot_t robot, int32_t* vel, size_t n)
{
	return 0;
}

int robot_enable(robot_t robot)
{
	return 0;
}
int robot_disable(robot_t robot)
{
	return 0;
}
int robot_stop(robot_t robot)
{
	return 0;
}
int robot_p2p(robot_t robot, ::robot_pose_t pose, double vel)
{
	return 0;
}
int robot_joint_p2p(robot_t robot, int32_t* pos, int32_t* vel, size_t n)
{
	return 0;
}
int robot_home(robot_t robot)
{
	return 0;
}
int robot_start_path(robot_t robot, robot_path_t path, int32_t mode)
{
	return 0;
}

int robot_name(robot_t robot, char* name, size_t size)
{
	return 0;
}
int robot_from_name(const char* name, robot_t* robot)
{
	return 0;
}

int robot_path_create(robot_t robot, robot_path_t* path)
{
	return 0;
}
int robot_path_push_line(robot_t robot, robot_path_t path, ::robot_pose_t start, ::robot_pose_t end, double smooth, double vel)
{
	return 0;
}
int robot_path_push_circle(robot_t robot, robot_path_t path, ::robot_pose_t start, ::robot_pose_t via, ::robot_pose_t end, double smooth, double vel)
{
	return 0;
}
int robot_path_clear(robot_t robot, robot_path_t path)
{
	return 0;
}


void controller_exit(void)
{
	return;
}
int controller_connect(const char* addr, int port)
{
	for (int i = 0; i < SERIAL_SIZE; i++)
	{
		if (serial_port[i].InitPort(port+i, 115200, 'N'))
			continue;
		else
			return -1;
	}
	return 0;
}
int controller_lock()
{
	for (int i = 0; i < SERIAL_SIZE; i++)
	{
		serial_port[i].Enter_Critical();
		serial_port[i].fifo.free();
		serial_port[i].fifo.init(256);
		serial_port[i].Leave_Critical();
		serial_port[i].OpenListenThread();
	}
	return 0;
}
int controller_unlock()
{
	for (int i = 0; i < SERIAL_SIZE; i++)
	{
		serial_port[i].CloseListenTread();
		serial_port[i].fifo.reset();
	}
	return 0;
}

