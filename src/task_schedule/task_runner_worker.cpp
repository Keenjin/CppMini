#include "task_runner_worker.h"

namespace task_schedule {

	TaskRunnerWorker::TaskRunnerWorker()
		: task_scheduler(new TaskScheduler), task_thread(utils::MakeRefCounted<WorkerThread>(this)) {
		task_thread->Start();
	}

	TaskRunnerWorker::~TaskRunnerWorker() {
		task_thread->Cleanup();
	}

	bool TaskRunnerWorker::PostTask(OnceClosure task, TaskPriority priority) {
		task_scheduler->AddTask(Task(std::move(task), priority));
		task_thread->Submit();
		return true;
	}

	void TaskRunnerWorker::CleanupTasksImmediately(bool disableForever) {
		task_scheduler->DisableAdd(true);
		task_scheduler->CleanTasks();

		// 等待最后一个任务完成
		WaitForLastTaskFinish();

		if (!disableForever) task_scheduler->DisableAdd(false);
	}

	void TaskRunnerWorker::StopAndWaitTasksFinish() {
		task_scheduler->DisableAdd(true);

		// 等待最后一个任务完成
		WaitForLastTaskFinish();
	}

	void TaskRunnerWorker::WaitForLastTaskFinish() {
		// 确保当前线程，并非任务线程。如果是任务线程，说明前面应该已经清空任务了，才有机会执行这里
		if (task_thread->ThreadId() == GetCurrentThreadId()) {
			task_scheduler->CleanTasks();
			return;
		}

		last_task_event.Reset();
		task_thread->Submit();
		last_task_event.Wait();
	}

	void TaskRunnerWorker::OnMainEntry(WorkerThread* worker) {

	}

	uint32_t TaskRunnerWorker::GetSleepTimeout() {
		return INFINITE;
	}

	void TaskRunnerWorker::OnMainExit(WorkerThread* worker) {

	}

	Task TaskRunnerWorker::GetNextTask() {
		Task next_task = std::move(task_scheduler->GetNextTask());
		if (!next_task.IsValid())
			last_task_event.Signal();
		return std::move(next_task);
	}

	bool TaskRunnerWorker::ScheduleTask(Task task) {
		std::move(task.task_).Run();
		return true;
	}

	void TaskRunnerWorker::TaskDone(Task task, bool result) {

	}
}