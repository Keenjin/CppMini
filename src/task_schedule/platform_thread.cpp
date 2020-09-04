#include "platform_thread.h"
#include <windows.h>
#include <assert.h>

namespace task_schedule {

	struct ThreadParams {
		PlatformThread::Delegate* delegate = nullptr;
	};

	DWORD __stdcall ThreadFunc(void* params) {
		ThreadParams* thread_params = static_cast<ThreadParams*>(params);
		PlatformThread::Delegate* delegate = thread_params->delegate;
		delete thread_params;
		delegate->ThreadMain();
		return 0;
	}

	bool CreateThreadInternal(PlatformThread::Delegate* delegate, PlatformThreadHandle* out_thread_handle) {
		ThreadParams* params = new ThreadParams;
		params->delegate = delegate;

		void* thread_handle = ::CreateThread(nullptr, 0, ThreadFunc, params, 0, nullptr);
		if (!thread_handle) {
			DWORD last_error = ::GetLastError();
			switch (last_error){
			case ERROR_NOT_ENOUGH_MEMORY:
			case ERROR_OUTOFMEMORY:
			case ERROR_COMMITMENT_LIMIT:
				break;
			}

			delete params;
			return false;
		}

		if (out_thread_handle)
			*out_thread_handle = PlatformThreadHandle(thread_handle);
		else
			CloseHandle(thread_handle);

		return true;
	}

	bool PlatformThread::Create(Delegate* delegate, PlatformThreadHandle* thread_handle) {
		return CreateThreadInternal(delegate, thread_handle);
	}

	void PlatformThread::Join(PlatformThreadHandle thread_handle) {
		assert(WAIT_OBJECT_0 == WaitForSingleObject(thread_handle.platform_handle(), INFINITE));
		CloseHandle(thread_handle.platform_handle());
	}

	void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
		CloseHandle(thread_handle.platform_handle());
	}
}