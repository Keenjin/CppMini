#pragma once
#include <stdint.h>

namespace task_schedule {

	enum class TaskThreadType : int8_t {
		Invalid = -1,
		UI = 0,			// 任务被执行在UI线程
		Background,		// 任务被执行在进程内固定的后台线程
		Pool,			// 任务被执行在一个线程池中，线程个数会根据任务量和CPU空闲量动态计算，个数范围：1~255
		// Target,			// 任务被执行在任意目标线程，需要给定目标线程的id，任务会自动切换到目标线程异步执行  -- todo

		Max
	};

	enum class TaskPriority : int8_t {
		LOWEST = 0,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST
	};

	enum { kThreadInvalidId = 0 };
}