#include "task_scheduler.h"

namespace task_schedule {

	void TaskScheduler::AddTask(Task task) {
		if (disable_add_task_)
			return;
		task_queue.Push(std::move(task));
	}

	Task TaskScheduler::GetNextTask() {
		return task_queue.Pop();
	}

	bool TaskScheduler::ScheduleTask(Task task) {
		std::move(task.task_).Run();
		return true;
	}

	void TaskScheduler::TaskDone(Task task, bool result) {

	}

	void TaskScheduler::DisableAdd(bool disable_add_task) {
		std::atomic_exchange(&disable_add_task_, disable_add_task);
	}

	void TaskScheduler::CleanTasks() {
		task_queue.Empty();
	}
}
