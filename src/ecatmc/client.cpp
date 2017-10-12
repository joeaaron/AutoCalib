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

CSerialPort serial_port;

void clear_serial_port()
{
	serial_port.Enter_Critical();
	bool is_empty = serial_port.fifo.is_empty();
	if (!is_empty)
	{
		serial_port.fifo.reset();
		serial_port.Leave_Critical();
	}
	else
	{
		serial_port.Leave_Critical();
		return;
	}
}
string serial_read_data()
{
	string data = "";
	serial_port.Enter_Critical();
	bool is_empty = serial_port.fifo.is_empty();
	serial_port.Leave_Critical();
	while (!is_empty)
	{
		char c = 0;
		serial_port.Enter_Critical();
		serial_port.fifo.out(&c, 1);
		data = data + c;
		is_empty = serial_port.fifo.is_empty();
		serial_port.Leave_Critical();
	}
	return data;
}
int select_axis(int index)
{
	clear_serial_port();
	//—°÷–÷·
	string cmd;
	stringstream ss;

	ss << index + 3;
	cmd = "\\" + ss.str();

	serial_port.WriteData((unsigned char *)cmd.c_str(), cmd.size());
	unsigned char c = 0x0D;
	serial_port.WriteData(&c, 1);
	string status = "";
	int times = 0;
	do
	{
		times++;
		status = status + serial_read_data();
		if (-1 != status.find(ss.str() + "->"))
		{
			return 0;
		}
	} while (times < 100);
	return -1;
}
int send_cmd(string cmd)
{
	clear_serial_port();
	cmd = cmd + "\r";
	serial_port.WriteData((unsigned char *)cmd.c_str(), cmd.size());
	return 0;
}
string get_status(int index)
{
	stringstream ss;
	ss << index + 3;

	string status;
	int times = 0;
	do
	{
		times++;
		status = status + serial_read_data();
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
	sprintf(buff, "%.3f", num);
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
	int i = (size == 0 ? -1 : 0);
	while (i < (int64_t)size)
	{
		times = 0;
		char flag = 0;
		do
		{
			times++;
			ret = send_cmd("out " + int_to_str(i * 2 + 4) + " " + int_to_str(data[i == -1 ? 0 : i]));
			if (ret != 0)
			{
				continue;
			}
			Sleep(50);
		} while (times < 5);
		i++;
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
		ret = send_cmd("stopped");
		if (ret != 0)
		{
			continue;
		}
		Sleep(50);
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
		ret = send_cmd("active");
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
	return 0;
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
		ret = send_cmd("stopped");
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
		ret = send_cmd("pfb");
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
	} while (times < 100);
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
		ret = send_cmd("v");
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
			*v = (int32_t)(vel * 131072);
			return 0;
		}
		else
		{
			continue;
		}
	} while (times < 100);
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
		send_cmd("en");
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
	} while (times < 100);
	if (flag == 0)
	{
		return -1;
	}
	else
	{
		if (index == 2 || index == 4 || index == 5)
		{
			do
			{
				flag = 0;
				int ret = motion_axis_enabled(2, &flag);
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
						return 0;
					}
				}
			} while (1);

			do
			{
				flag = 0;
				int ret = motion_axis_enabled(4, &flag);
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
						return 0;
					}
				}
			} while (1);

			do
			{
				flag = 0;
				int ret = motion_axis_enabled(5, &flag);
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
						return 0;
					}
				}
			} while (1);

			uint8_t motor_break[1] = { 1 };
			ret = set_digital_output(motor_break, 0);
			if (ret != 0)
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
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

	if (index == 2 || index == 4 || index == 5)
	{
		uint8_t motor_break[1] = { 0 };
		do
		{
			ret = set_digital_output(motor_break, 0);
		} while (ret != 0);
	}

	times = 0;
	char flag = 1;
	do
	{
		times++;
		send_cmd("k");
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
		send_cmd("stop");
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
		send_cmd("moveabs " + int_to_str(p) + " " + double_to_str(vel * 1.0 / 131072));
		flag = 0;
		Sleep(50);
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
					continue;
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
	} while (times < 100);
	return -1;
}
int motion_axis_home(axis_t index)
{
	return 0;
}
int motion_axis_move(axis_t index, int32_t vel)
{

	if (vel > 0)
		return motion_axis_p2p(index, 214748364, vel);
	else
		return motion_axis_p2p(index, -214748364, -vel);
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
	if (serial_port.InitPort(port, 115200, 'N'))
		return 0;
	else
		return -1;
}
int controller_lock()
{
	serial_port.Enter_Critical();
	serial_port.fifo.free();
	serial_port.fifo.init(256);
	serial_port.Leave_Critical();
	serial_port.OpenListenThread();
	return 0;
}
int controller_unlock()
{
	serial_port.CloseListenTread();
	serial_port.fifo.reset();
	return 0;
}

