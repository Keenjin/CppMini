#pragma once
#include <stdint.h>

namespace task_schedule {

	enum class TaskThreadType : int8_t {
		UI = 0,
		Background,
		Pool,

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