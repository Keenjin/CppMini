#include "worker_thread.h"

namespace task_schedule {

	WorkerThread::WorkerThread(WorkerThread::Delegate* delegate) 
		: delegate_(delegate) {

	}

	void WorkerThread::Delegate::WaitForWork(utils::WaitableEvent* wake_up_event) {
		const uint32_t sleep_time = GetSleepTimeout();
		if (sleep_time == -1) {
			wake_up_event->Wait();
		}
		else {
			wake_up_event->TimedWait(sleep_time);
		}
	}

	bool WorkerThread::Start() {
		utils::AutoLock auto_lock(thread_lock);

		if (should_exit.IsSet())
			return true;

		PlatformThread::Create(this, &thread_handle);
		if (thread_handle.is_null()) {
			return false;
		}

		return true;
	}

	void WorkerThread::WakeUp() {
		wake_up_event.Signal();
	}

	void WorkerThread::Cleanup() {
		should_exit.Set();
		wake_up_event.Signal();
	}

	bool WorkerThread::ShouldExit() const {
		return should_exit.IsSet();
	}

	WorkerThread::~WorkerThread() {
		utils::AutoLock auto_lock(thread_lock);

		if (!thread_handle.is_null()) {
			PlatformThread::Detach(thread_handle);
		}
	}

	void WorkerThread::ThreadMain() {
		RunWorker();
	}

	void WorkerThread::RunWorker() {
		delegate_->OnMainEntry(this);
		delegate_->WaitForWork(&wake_up_event);
		while (!ShouldExit()) {
			Task task = delegate_->GetNextTask();
			if (!task.IsValid()) {
				if (ShouldExit())
					break;

				delegate_->WaitForWork(&wake_up_event);
				continue;
			}

			delegate_->TaskDone(task, delegate_->ScheduleTask(task));

			wake_up_event.Reset();
		}
		delegate_->OnMainExit(this);
	}
}