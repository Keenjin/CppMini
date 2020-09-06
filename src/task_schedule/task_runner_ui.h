#pragma once
#include "task_runner.h"
#include "message_window.h"
#include "task_scheduler.h"
#include "include/task.h"

namespace task_schedule {

	class TaskRunnerUI : 
		public TaskRunner,
		public MessageWindow::Delegate {
	public:
		TaskRunnerUI(const std::wstring& name = L"");
		~TaskRunnerUI();

		// TaskRunner
		virtual bool PostTask(OnceClosure task, TaskPriority priority) override;
		virtual bool PostTaskAndReply(OnceClosure task, OnceClosure reply, TaskPriority priority) override;

		// MessageWindow::Delegate
		virtual void OnMainEntry() override;
		virtual Task GetNextTask() override;
		virtual bool ScheduleTask(Task task) override;
		virtual void TaskDone(Task task, bool result) override;
		virtual void OnMainExit()  override;

	private:
		std::unique_ptr<MessageWindow>	msg_window;
		std::unique_ptr<TaskScheduler> task_scheduler;
	};
}
