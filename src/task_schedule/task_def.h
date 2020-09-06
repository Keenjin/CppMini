#pragma once
#include <stdint.h>

namespace task_schedule {

	enum class TaskThreadType : int8_t {
		UI = 0,			// ����ִ����UI�߳�
		Background,		// ����ִ���ڽ����ڹ̶��ĺ�̨�߳�
		Pool,			// ����ִ����һ���̳߳��У��̸߳����������������CPU��������̬���㣬������Χ��1~255
		// Target,			// ����ִ��������Ŀ���̣߳���Ҫ����Ŀ���̵߳�id��������Զ��л���Ŀ���߳��첽ִ��  -- todo

		Max
	};

	enum class TaskPriority : int8_t {
		LOWEST = 0,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST
	};

}