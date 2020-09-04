#include "task_runner_pool.h"

namespace task_schedule {

	TaskRunnerPool::TaskRunnerPool()
		: thread_pool(new ThreadPool(this))
		, task_scheduler(new TaskScheduler) {
		thread_pool->Start();
	}
	TaskRunnerPool::~TaskRunnerPool() {
		thread_pool->Cleanup();
		thread_pool.reset();
	}

	bool TaskRunnerPool::PostTask(OnceClosure task, TaskPriority priority) {
		// push to queue
		task_scheduler->AddTask(Task(std::move(task), priority));

		// wakeup
		thread_pool->Submit();

		return true;
	}

	bool TaskRunnerPool::PostTaskAndReply(OnceClosure task, OnceClosure reply, TaskPriority priority) {
		return true;
	}

	void TaskRunnerPool::OnMainEntry() {

	}

	Task TaskRunnerPool::GetNextTask() {
		return task_scheduler->GetNextTask();
	}

	bool TaskRunnerPool::ScheduleTask(Task task) {
		return task_scheduler->ScheduleTask(task);
	}

	void TaskRunnerPool::TaskDone(Task task, bool result) {

	}

	void TaskRunnerPool::OnMainExit() {

	}
}