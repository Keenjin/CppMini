#pragma once
#include <string>
#include "utils/macros.h"
#include <windows.h>
#include "utils/weak_ptr.h"
#include <windows.h>
#include "task.h"

namespace task_schedule {

	class MessageWindow {
	public:
		enum { kMessageCommunition = WM_USER + 1 };
		class WindowClass;

		class Delegate {
		public:
			virtual ~Delegate() = default;

			virtual void OnMainEntry() = 0;
			virtual Task GetNextTask() = 0;
			virtual bool ScheduleTask(Task task) = 0;
			virtual void TaskDone(Task task, bool result) = 0;
			virtual void OnMainExit() = 0;
		};

		MessageWindow(MessageWindow::Delegate* delegate);
		~MessageWindow();

		bool Create(const std::wstring& window_name);
		HWND GetWnd() { return window_; }
		void Submit();

		LRESULT OnMsgProc(
			UINT message,
			WPARAM wparam,
			LPARAM lparam,
			bool* handle_self);

	private:

		HWND window_ = nullptr;
		MessageWindow::Delegate* delegate_ = nullptr;

		DISALLOW_COPY_AND_ASSIGN(MessageWindow);
	};
}
