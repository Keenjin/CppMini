#include "task_runner_ui.h"

namespace task_schedule {

	TaskRunnerUI::TaskRunnerUI(const std::wstring& name/* = L""*/)
		: msg_window(new MessageWindow(this))
		, task_scheduler(new TaskScheduler) {
		msg_window->Create(name.empty() ? L"{EC1BC36F-C715-4D45-B18E-B5DA872C414F}" : name);
	}
	TaskRunnerUI::~TaskRunnerUI() {
		msg_window.reset();
	}

	bool TaskRunnerUI::PostTask(OnceClosure task, TaskPriority priority) {
		// push to queue
		task_scheduler->AddTask(Task(std::move(task), priority));

		// wakeup
		msg_window->Submit();

		return true;
	}

	void TaskRunnerUI::CleanupTasksImmediately() {
		task_scheduler->DisableAdd(true);
		task_scheduler->CleanTasks();

		WaitForLastTaskFinish();
	}

	void TaskRunnerUI::StopAndWaitTasksFinish() {
		task_scheduler->DisableAdd(true);

		WaitForLastTaskFinish();
	}

	void TaskRunnerUI::WaitForLastTaskFinish() {
		// 确保当前线程，并非任务线程。如果是任务线程，则无脑清空队列，这里理论上不应该出现这种等待
		if (msg_window->ThreadId() == GetCurrentThreadId()) {
			task_scheduler->CleanTasks();
			return;
		}

		last_task_event.Reset();
		msg_window->Submit();
		last_task_event.Wait();
	}

	void TaskRunnerUI::OnMainEntry() {

	}

	Task TaskRunnerUI::GetNextTask() {
		Task next_task = std::move(task_scheduler->GetNextTask());
		if (!next_task.IsValid())
			last_task_event.Signal();
		return std::move(next_task);
	}

	bool TaskRunnerUI::ScheduleTask(Task task) {
		return task_scheduler->ScheduleTask(task);
	}

	void TaskRunnerUI::TaskDone(Task task, bool result) {

	}
	
	void TaskRunnerUI::OnMainExit() {

	}
}