#include "task_runner_pool.h"
#include "include/post_task.h"

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

	void TaskRunnerPool::CleanupTasksImmediately() {
		// 停止接收新任务
		task_scheduler->DisableAdd(true);
		// 清空未进行任务
		task_scheduler->CleanTasks();
		// 如果在线程池内部，使用Join，是非常危险的，这一类接口只能在线程池线程的外部调用
		thread_pool->Join();
	}

	void TaskRunnerPool::StopAndWaitTasksFinish() {
		// 停止接收新任务
		task_scheduler->DisableAdd(true);
		thread_pool->Join();
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