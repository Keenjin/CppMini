#pragma once
#include "task_runner.h"
#include "message_window.h"
#include "task_scheduler.h"
#include "include/task.h"
#include "utils/waitable_event.h"

namespace task_schedule {

	class TaskRunnerUI : 
		public TaskRunner,
		public MessageWindow::Delegate {
	public:
		TaskRunnerUI(const std::wstring& name = L"");
		~TaskRunnerUI();

		// TaskRunner
		virtual bool PostTask(OnceClosure task, TaskPriority priority = TaskPriority::NORMAL) override;
		virtual void CleanupTasksImmediately() override;
		virtual void StopAndWaitTasksFinish() override;
		virtual uint32_t ThreadId() override {
			return msg_window->ThreadId();
		}

		// MessageWindow::Delegate
		virtual void OnMainEntry() override;
		virtual Task GetNextTask() override;
		virtual bool ScheduleTask(Task task) override;
		virtual void TaskDone(Task task, bool result) override;
		virtual void OnMainExit()  override;

	private:
		void WaitForLastTaskFinish();

		std::unique_ptr<MessageWindow>	msg_window;
		std::unique_ptr<TaskScheduler> task_scheduler;

		utils::WaitableEvent last_task_event;
	};
}
