#pragma once
#include <stdint.h>

namespace task_schedule {

	enum class TaskThreadType : int {
		UI = 0,
		Background,
	};

	enum class TaskPriority : int8_t {
		LOWEST,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST
	};

}