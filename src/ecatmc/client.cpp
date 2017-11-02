#include <iostream>
#include <stdint.h>
#include "serialport.h"
#include <string>
#include <sstream>
#include <Windows.h>
#include <thread>
using namespace std;
#ifdef WIN32
#	define ECATMC_API extern "C" __declspec(dllexport)
#endif
#include "ecatmc.h"
#define DEBUG_INFO(...) //printf

#define SERIAL_SIZE 4
CSerialPort serial_port[SERIAL_SIZE];
string cmd[SERIAL_SIZE * 2];
int32_t ret[SERIAL_SIZE * 2] = { 0 };
int32_t pos[SERIAL_SIZE * 2] = { 0 };
int32_t vel[SERIAL_SIZE * 2] = { 0 };
int32_t enabled[SERIAL_SIZE * 2] = { -1 };
int32_t stopped[SERIAL_SIZE * 2] = { -1 };
int32_t finished[SERIAL_SIZE * 2] = { -1 };

void clear_serial_port(int serial_index)
{
	serial_port[serial_index].Enter_Critical();
	bool is_empty = serial_port[serial_index].fifo.is_empty();
	if (!is_empty)
	{
		serial_port[serial_index].fifo.reset();
		serial_port[serial_index].Leave_Critical();
	}
	else
	{
		serial_port[serial_index].Leave_Critical();
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
int send_cmd(int serial_index, string cmd)
{
	int sended_size = 0;
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
string get_status(int serial_index)
{
	stringstream ss;
	string status;
	int times = 0;
	do
	{
		times++;
		status = status + serial_read_data(serial_index);
		ss << serial_index * 2 + 3;
		int char_index = status.find_last_of(ss.str() + "->");
		if (-1 != char_index)
		{
			return status;
		}
		ss.str("");
		ss << serial_index * 2 + 1 + 3;
		char_index = status.find_last_of(ss.str() + "->");
		if (-1 != char_index)
		{
			return status;
		}
	} while ((status != "" && times < 1000) || (status == "" && times < 100));
	return "";
}
int select_axis(int serial_index, int index)
{
	//Ñ¡ÖÐÖá
	string cmd_str;
	stringstream ss;

	ss << index + 3;
	cmd_str = "\\" + ss.str();
	clear_serial_port(serial_index);
	send_cmd(serial_index, cmd_str);
	string status = "";
	int times = 0;
	do
	{
		times++;
		status = get_status(serial_index);
		if (-1 != status.find(ss.str() + "->"))
		{
			cmd_str = "";
			clear_serial_port(serial_index);
			send_cmd(serial_index, cmd_str);
			times = 0;
			break;
		}
	} while (times < 100);
	if (times != 0)
		return -1;
	else
	{
		do
		{
			times++;
			status = get_status(serial_index);
			if (-1 != status.find(ss.str() + "->"))
			{
				return 0;
			}
		} while (times < 100);
	}
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

bool serial_stop = false;
void serial_update(int serial_index)
{
	int axis_index = 0;
	string axis_cmd = "";
	int32_t str_index = 0;
	while (!serial_stop)
	{
		Sleep(20);
		for (axis_index = serial_index * 2; axis_index < serial_index * 2 + 2; axis_index ++)
		{
			if (0 == select_axis(serial_index, axis_index))
			{
				if (cmd[axis_index] != "")
				{
					send_cmd(serial_index, cmd[axis_index]);
					Sleep(20);
					axis_cmd = get_status(serial_index);
					str_index = axis_cmd.find(cmd[axis_index]);
					if (str_index != -1)
					{
						cmd[axis_index] = "";
						ret[axis_index] = 0;
					}
					else
					{
						if (ret[axis_index] > 1)
						{
							ret[axis_index]--;
						}
						else if (ret[axis_index] == 1)
						{
							cmd[axis_index] = "";
							ret[axis_index] = -1;
						}
					}
				}
			}
		}
		for (axis_index = serial_index * 2; axis_index < serial_index * 2 + 2; axis_index++)
		{
			if (0 == select_axis(serial_index, axis_index))
			{
				string ret_str = "";
				stringstream ss;
				send_cmd(serial_index, "pfb");
				Sleep(1);
				ret_str = get_status(serial_index);
				str_index = ret_str.find("pfb");
				if (str_index != -1)
				{
					if (str_index + 3 < ret_str.size())
						ret_str = ret_str.substr(str_index + 3, ret_str.size());
					else
						ret_str = "";
					str_index = ret_str.find_first_not_of("\r\n");
					if (str_index != -1)
					{
						ret_str = ret_str.substr(str_index, ret_str.size());
						str_index = ret_str.find(".00");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						if (ret_str != "")
						{
							ss.str("");
							ss.clear();
							ss << ret_str;
							ss >> pos[axis_index];
						}
					}
				}

				send_cmd(serial_index, "v");
				Sleep(1);
				ret_str = get_status(serial_index);
				str_index = ret_str.find("v");
				if (str_index != -1)
				{
					if (str_index + 1 < ret_str.size())
						ret_str = ret_str.substr(str_index + 1, ret_str.size());
					else
						ret_str = "";
					str_index = ret_str.find_first_not_of("\r\n");
					if (str_index != -1)
					{
						ret_str = ret_str.substr(str_index, ret_str.size());
						str_index = ret_str.find(" ");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						if (ret_str != "")
						{
							ss.str("");
							ss.clear();
							ss << ret_str;
							double vel_counts = 0;
							ss >> vel_counts;
							vel[axis_index] = (int32_t)(vel_counts * 131072.0 / 60);
						}
					}
				}

				send_cmd(serial_index, "active");
				Sleep(1);
				ret_str = get_status(serial_index);
				str_index = ret_str.find("active");
				if (str_index != -1)
				{
					if (str_index + 6 < ret_str.size())
						ret_str = ret_str.substr(str_index + 6, ret_str.size());
					else
						ret_str = "";
					str_index = ret_str.find_first_not_of("\r\n");
					if (str_index != -1)
					{
						ret_str = ret_str.substr(str_index, ret_str.size());
						str_index = ret_str.find("\r\n");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						str_index = ret_str.find("<");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						ss.str("");
						ss.clear();
						ss << ret_str;
						int is_enable;
						ss >> is_enable;
						if (is_enable == 0 || is_enable == 1)
						{
							enabled[axis_index] = is_enable;
						}
					}
				}

				send_cmd(serial_index, "stopped");
				Sleep(1);
				ret_str = get_status(serial_index);
				str_index = ret_str.find("stopped");
				if (str_index != -1)
				{
					if (str_index + 7 < ret_str.size())
						ret_str = ret_str.substr(str_index + 7, ret_str.size());
					else
						ret_str = "";
					str_index = ret_str.find_first_not_of("\r\n");
					if (str_index != -1)
					{
						ret_str = ret_str.substr(str_index, ret_str.size());
						str_index = ret_str.find("\r\n");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						str_index = ret_str.find("<");
						if (str_index != -1)
						{
							ret_str = ret_str.substr(0, str_index);
						}
						ss.str("");
						ss.clear();
						ss << ret_str;
						int is_stopped;
						ss >> is_stopped;
						if (is_stopped == -1 || is_stopped == 0 || is_stopped == 1 || is_stopped == 2)
						{
							if (is_stopped == 0)
							{
								stopped[axis_index] = 0;
							}
							else
							{
								stopped[axis_index] = 1;
							}

							if (is_stopped == 2)
							{
								finished[axis_index] = 1;
							}
							else
							{
								finished[axis_index] = 0;
							}
						}
					}
				}
				
				select_axis(serial_index, axis_index);
			}
		}
	}
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
	int32_t index = 3;
	for (int i = 0; i < 5; i++)
	{
		ret[index] = 5;
		cmd[index] = "out " + int_to_str(i * 2 + 8) + " " + int_to_str(data[i]);
		while (ret[index] > 0)
		{
			Sleep(1);
		}
		if (ret[index] = -1)
		{
			continue;
		}
		else
		{
			return 0;
		}
		Sleep(10);
	}
	return -1;
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
	if (stopped[index] != 0 && stopped[index] != 1)
	{
		return -1;
	}
	else
	{
		*flag = stopped[index];
		return 0;
	}
}
int motion_axis_enabled(axis_t index, char* flag)
{
	if (enabled[index] != 0 && enabled[index] != 1)
	{
		return -1;
	}
	else
	{
		*flag = enabled[index];
		return 0;
	}
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
	if (finished[index] != 0 && finished[index] != 1)
	{
		return -1;
	}
	else
	{
		*flag = finished[index];
		return 0;
	}
}
int motion_axis_position(axis_t index, int32_t* p)
{
	*p = pos[index];
	return 0;
}
int motion_axis_velocity(axis_t index, int32_t* v)
{
	*v = vel[index];
	return 0;
}
int motion_axis_halted(axis_t index, char* flag)
{
	return 0;
}
int motion_axis_enable(axis_t index)
{
	for (int i = 0; i < 5; i++)
	{
		ret[index] = 5;
		cmd[index] = "en";
		while (ret[index] > 0)
		{
			Sleep(1);
		}
		if (ret[index] == -1)
		{
			continue;
		}
		else
		{
			char flag = 0;
			ret[index] = motion_axis_enabled(index, &flag);
			if (0 == ret[index])
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
			else
			{
				continue;
			}
		}
		Sleep(10);
	}
	return -1;
}
int motion_axis_disable(axis_t index)
{
	for (int i = 0; i < 5; i++)
	{
		ret[index] = 5;
		cmd[index] = "k";
		while (ret[index] > 0)
		{
			Sleep(1);
		}
		if (ret[index] == -1)
		{
			continue;
		}
		else
		{
			char flag = 0;
			ret[index] = motion_axis_enabled(index, &flag);
			if (0 == ret[index])
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
			else
			{
				continue;
			}
		}
		Sleep(10);
	}
	return -1;
}
int motion_axis_stop(axis_t index)
{
	for (int i = 0; i < 5; i++)
	{
		ret[index] = 5;
		cmd[index] = "stop";
		while (ret[index] > 0)
		{
			Sleep(1);
		}
		if (ret[index] == -1)
		{
			continue;
		}
		else
		{
			char flag = 0;
			ret[index] = motion_axis_stopped(index, &flag);
			if (0 == ret[index])
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
			else
			{
				continue;
			}
		}
		Sleep(10);
	}
	return -1;
}
int motion_axis_p2p(axis_t index, int32_t p, int32_t vel)
{
	for (int i = 0; i < 5; i++)
	{
		ret[index] = 5;
		cmd[index] = "moveabs " + int_to_str(p) + " " + double_to_str(vel * 1.0 / 131072 * 60);
		while (ret[index] > 0)
		{
			Sleep(1);
		}
		if (ret[index] == -1)
		{
			continue;
		}
		else
		{
			char flag = 0;
			ret[index] = motion_axis_stopped(index, &flag);
			if (0 != ret[index])
			{
				continue;
			}
			else
			{
				if (flag == 1)
				{
					ret[index] = motion_axis_finished(index, &flag);
					if (0 != ret[index])
					{
						continue;
					}
					if (flag != 0)
					{
						int32_t pos_cur = 0;
						ret[index] = motion_axis_position(index, &pos_cur);
						if (0 != ret[index])
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
		}
		Sleep(10);
	}
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
		std::thread thrd(serial_update, i);
		thrd.detach();
	}
	return 0;
}
int controller_unlock()
{
	serial_stop = true;
	Sleep(100);
	for (int i = 0; i < SERIAL_SIZE; i++)
	{
		serial_port[i].CloseListenTread();
		serial_port[i].fifo.reset();
	}
	return 0;
}

