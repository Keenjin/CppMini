#include "thread_pool.h"
#include <assert.h>

namespace task_schedule {

	void CALLBACK ThreadPoolFunc(PTP_CALLBACK_INSTANCE callback_instance,
			void* thread_group_windows_impl,
			PTP_WORK) {
		auto* thread_group =
			static_cast<ThreadPool*>(thread_group_windows_impl);
		thread_group->ThreadMain();
	}

	ThreadPool::ThreadPool(ThreadPool::Delegate* delegate) 
		: delegate_(delegate) {

	}

	ThreadPool::~ThreadPool() {
		Cleanup();
	}

	bool ThreadPool::Start() {
		::InitializeThreadpoolEnvironment(&environment_);

		delegate_->OnMainEntry();

		pool_ = ::CreateThreadpool(nullptr);
		assert(pool_);
		::SetThreadpoolThreadMinimum(pool_, 1);
		::SetThreadpoolThreadMaximum(pool_, 256);

		work_ = ::CreateThreadpoolWork(&ThreadPoolFunc, this, &environment_);
		assert(work_);
		::SetThreadpoolCallbackPool(&environment_, pool_);

		return pool_ && work_;
	}

	void ThreadPool::Cleanup() {
		if (delegate_) {
			delegate_->OnMainExit();
			delegate_ = nullptr;
		}
		
		::DestroyThreadpoolEnvironment(&environment_);
		if (work_) {
			::CloseThreadpoolWork(work_);
			work_ = nullptr;
		}
		if (pool_) {
			::CloseThreadpool(pool_);
			pool_ = nullptr;
		}
	}

	void ThreadPool::Join() {
		::WaitForThreadpoolWorkCallbacks(work_, true);
	}

	void ThreadPool::Submit() {
		::SubmitThreadpoolWork(work_);
	}

	void ThreadPool::ThreadMain() {
		Task task = delegate_->GetNextTask();
		delegate_->TaskDone(task, delegate_->ScheduleTask(task));
	}
}