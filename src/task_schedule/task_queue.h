#pragma once
#include "task.h"
#include "utils/lock.h"

namespace task_schedule {

	class TaskQueueImpl {
	public:
		bool Push(Task&& task);
		Task Pop();
		void Empty();

	private:
		utils::Lock lock_queue;
		PriorityTaskQueue task_queue;
	};
}
