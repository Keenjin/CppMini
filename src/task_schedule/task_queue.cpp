#include "task_queue.h"

namespace task_schedule {

	bool TaskQueueImpl::Push(Task&& task) {
		utils::AutoLock lock(lock_queue);
		task_queue.push(std::move(task));
		return true;
	}

	Task TaskQueueImpl::Pop() {
		utils::AutoLock lock(lock_queue);
		
		if (!task_queue.empty())
		{
			auto task = std::move(task_queue.top());
			task_queue.pop();
			return task;
		}
		
		return Task();
	}

	void TaskQueueImpl::Empty() {
		utils::AutoLock lock(lock_queue);
		while (!task_queue.empty())
			task_queue.pop();
	}

	uint64_t TaskQueueImpl::Count() {
		utils::AutoLock lock(lock_queue);
		return task_queue.size();
	}
}
