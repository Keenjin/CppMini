#pragma once
#include "worker_thread.h"
#include "task_runner.h"
#include "task_scheduler.h"
#include "worker_thread.h"

namespace task_schedule {

	class TaskRunnerWorker : 
		public TaskRunner, 
		public WorkerThread::Delegate {
	public:
		TaskRunnerWorker();
		~TaskRunnerWorker();

		// TaskRunner
		virtual bool PostTask(OnceClosure task, TaskPriority priority) override;
		virtual bool PostTaskAndReply(OnceClosure task, OnceClosure reply, TaskPriority priority) override;

		// WorkerThread::Delegate
		virtual void OnMainEntry(WorkerThread* worker) override;
		virtual Task GetNextTask() override;
		virtual bool ScheduleTask(Task task) override;
		virtual void TaskDone(Task task, bool result) override;
		virtual uint32_t GetSleepTimeout() override;
		virtual void OnMainExit(WorkerThread* worker) override;

	private:
		std::unique_ptr<TaskScheduler> task_scheduler;
		utils::scoped_refptr<WorkerThread> task_thread;
	};
}