#pragma once
#include <windows.h>
#include "include/task.h"

namespace task_schedule {

	class ThreadPool {
	public:
		class Delegate {
		public:
			virtual void OnMainEntry() = 0;
			virtual Task GetNextTask() = 0;
			virtual bool ScheduleTask(Task task) = 0;
			virtual void TaskDone(Task task, bool result) = 0;
			virtual void OnMainExit() = 0;

		protected:
			virtual ~Delegate() = default;
		};

		ThreadPool(ThreadPool::Delegate* delegate);
		~ThreadPool();

		bool Start();
		void Cleanup();
		void Join();
		void Submit();

		void ThreadMain();

	private:
		ThreadPool::Delegate* delegate_ = nullptr;

		// Thread pool object that |work_| gets executed on.
		PTP_POOL pool_ = nullptr;

		// Callback environment. |pool_| is associated with |environment_| so that
		// work objects using this environment run on |pool_|.
		TP_CALLBACK_ENVIRON environment_ = {};

		// Work object that executes RunNextTaskSource. It has a pointer to the
		// current |ThreadGroupNativeWin| and a pointer to |environment_| bound
		// to it.
		PTP_WORK work_ = nullptr;
	};
}