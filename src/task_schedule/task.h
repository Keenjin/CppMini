#pragma once
#include "utils/macros.h"
#include <queue>
#include "utils/container.h"
#include "bind.h"
#include "task_def.h"

namespace task_schedule {

	class Task {
	public:
		Task() = default;
		Task(Task&& other) = default;
		Task(const Task& other);

		Task(OnceClosure task, TaskPriority priority = TaskPriority::NORMAL);

		bool IsValid() { return !task_.is_null(); }

		Task& operator=(Task&& other);

		bool operator<(const Task& other) const;

		mutable OnceClosure task_;
		int sequence_num = 0;

		TaskPriority priority_ = TaskPriority::NORMAL;
	};

	using TaskQueue = utils::queue<Task>;
	using PriorityTaskQueue = std::priority_queue<Task>;
}
