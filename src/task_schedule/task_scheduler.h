#pragma once
#include "task_runner.h"
#include "task_queue.h"

namespace task_schedule {

	class TaskScheduler : public TaskRunner::Delegate {
	public:
		// TaskRunner::Delegate
		virtual void AddTask(Task task) override;
		virtual Task GetNextTask() override;
		virtual bool ScheduleTask(Task task) override;
		virtual void TaskDone(Task task, bool result) override;

		void DisableAdd(bool disable_add_task);
		void CleanTasks();
		uint64_t TasksCount();

	private:
		std::atomic_bool disable_add_task_ = false;
		TaskQueueImpl task_queue;
	};
}
