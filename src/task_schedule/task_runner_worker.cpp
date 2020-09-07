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

	void TaskRunnerWorker::CleanupTasksImmediately() {
		task_scheduler->DisableAdd(true);
		task_scheduler->CleanTasks();

		// �ȴ����һ���������
		WaitForLastTaskFinish();
	}

	void TaskRunnerWorker::StopAndWaitTasksFinish() {
		task_scheduler->DisableAdd(true);

		// �ȴ����һ���������
		WaitForLastTaskFinish();
	}

	void TaskRunnerWorker::WaitForLastTaskFinish() {
		// ȷ����ǰ�̣߳����������̡߳�����������̣߳�˵��ǰ��Ӧ���Ѿ���������ˣ����л���ִ������
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