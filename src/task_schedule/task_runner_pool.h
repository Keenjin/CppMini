#pragma once
#include "task_runner.h"
#include "thread_pool.h"
#include "task_scheduler.h"

namespace task_schedule {

	class TaskRunnerPool : 
		public TaskRunner, 
		public ThreadPool::Delegate {
	public:
		TaskRunnerPool();
		~TaskRunnerPool();

		// TaskRunner
		virtual bool PostTask(OnceClosure task, TaskPriority priority = TaskPriority::NORMAL) override;
		virtual void CleanupTasksImmediately() override;
		virtual void StopAndWaitTasksFinish() override;

		// MessageWindow::Delegate
		virtual void OnMainEntry() override;
		virtual Task GetNextTask() override;
		virtual bool ScheduleTask(Task task) override;
		virtual void TaskDone(Task task, bool result) override;
		virtual void OnMainExit()  override;

	private:
		std::unique_ptr<ThreadPool> thread_pool;
		std::unique_ptr<TaskScheduler> task_scheduler;
	};
}