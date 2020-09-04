#include "task_scheduler.h"

namespace task_schedule {

	void TaskScheduler::AddTask(Task task) {
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
}
