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
		task_thread->WakeUp();
		return true;
	}

	bool TaskRunnerWorker::PostTaskAndReply(OnceClosure task, OnceClosure reply, TaskPriority priority) {
		return true;
	}

	void TaskRunnerWorker::OnMainEntry(WorkerThread* worker) {

	}

	uint32_t TaskRunnerWorker::GetSleepTimeout() {
		return INFINITE;
	}

	void TaskRunnerWorker::OnMainExit(WorkerThread* worker) {

	}

	Task TaskRunnerWorker::GetNextTask() {
		return task_scheduler->GetNextTask();
	}

	bool TaskRunnerWorker::ScheduleTask(Task task) {
		std::move(task.task_).Run();
		return true;
	}

	void TaskRunnerWorker::TaskDone(Task task, bool result) {

	}
}