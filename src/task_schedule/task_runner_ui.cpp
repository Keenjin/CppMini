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

	bool TaskRunnerUI::PostTaskAndReply(OnceClosure task, OnceClosure reply, TaskPriority priority) {
		return true;
	}

	void TaskRunnerUI::OnMainEntry() {

	}

	Task TaskRunnerUI::GetNextTask() {
		return task_scheduler->GetNextTask();
	}

	bool TaskRunnerUI::ScheduleTask(Task task) {
		return task_scheduler->ScheduleTask(task);
	}

	void TaskRunnerUI::TaskDone(Task task, bool result) {

	}
	
	void TaskRunnerUI::OnMainExit() {

	}
}