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

	private:
		TaskQueueImpl task_queue;
	};
}
