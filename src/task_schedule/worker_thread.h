#pragma once
#include "utils/ref_counted.h"
#include "platform_thread.h"
#include "utils/lock.h"
#include "utils/atomic_flag.h"
#include "utils/waitable_event.h"
#include "task.h"

namespace task_schedule {

	class WorkerThread : 
		public utils::RefCountedThreadSafe<WorkerThread>,
		public PlatformThread::Delegate {
	public:
		class Delegate {
		public:
			virtual ~Delegate() = default;

			virtual void OnMainEntry(WorkerThread* worker) = 0;
			virtual Task GetNextTask() = 0;
			virtual bool ScheduleTask(Task task) = 0;
			virtual void TaskDone(Task task, bool result) = 0;
			virtual uint32_t GetSleepTimeout() = 0;
			virtual void WaitForWork(utils::WaitableEvent* wake_up_event);
			virtual void OnMainExit(WorkerThread* worker) {}
		};

		WorkerThread(WorkerThread::Delegate* delegate);

		bool Start();
		void Submit();
		void Cleanup();

	private:
		friend class utils::RefCountedThreadSafe<WorkerThread>;

		~WorkerThread() override;

		bool ShouldExit() const;

		// PlatforThread::Delegate
		void ThreadMain() override;

		void RunWorker();

		std::unique_ptr<WorkerThread::Delegate> delegate_;

		mutable utils::Lock thread_lock;
		PlatformThreadHandle thread_handle;
		utils::AtomicFlag should_exit;
		utils::WaitableEvent wake_up_event;

		DISALLOW_COPY_AND_ASSIGN(WorkerThread);
	};
}