#include "platform_thread.h"
#include "utils/stringex.h"
#include <assert.h>

namespace task {

	// static
	void PlatformThread::SetCurrentThreadPriority(ThreadPriority priority) {
		SetCurrentThreadPriorityImpl(priority);
	}

	// static
	PlatformThreadId PlatformThread::CurrentId() {
		return ::GetCurrentThreadId();
	}

	// static
	PlatformThreadHandle PlatformThread::CurrentHandle() {
		return PlatformThreadHandle(::GetCurrentThread());
	}

	// static
	void PlatformThread::YieldCurrentThread() {
		::Sleep(0);
	}

	// static
	void PlatformThread::Sleep(uint32_t duration) {
		::Sleep(duration);
	}

	// The information on how to set the thread name comes from
	// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
	const DWORD kVCThreadNameException = 0x406D1388;

	typedef struct tagTHREADNAME_INFO {
		DWORD dwType;  // Must be 0x1000.
		LPCSTR szName;  // Pointer to name (in user addr space).
		DWORD dwThreadID;  // Thread ID (-1=caller thread).
		DWORD dwFlags;  // Reserved for future use, must be zero.
	} THREADNAME_INFO;

	// The SetThreadDescription API was brought in version 1607 of Windows 10.
	typedef HRESULT(WINAPI* SetThreadDescription)(HANDLE hThread,
		PCWSTR lpThreadDescription);

	// This function has try handling, so it is separated out of its caller.
	void SetNameInternal(PlatformThreadId thread_id, const char* name) {
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name;
		info.dwThreadID = thread_id;
		info.dwFlags = 0;

		__try {
			RaiseException(kVCThreadNameException, 0, sizeof(info) / sizeof(DWORD),
				reinterpret_cast<DWORD_PTR*>(&info));
		}
		__except (EXCEPTION_CONTINUE_EXECUTION) {
		}
	}

	// static
	void PlatformThread::SetName(const std::string& name) {

		// The SetThreadDescription API works even if no debugger is attached.
		static auto set_thread_description_func =
			reinterpret_cast<SetThreadDescription>(::GetProcAddress(
				::GetModuleHandle(L"Kernel32.dll"), "SetThreadDescription"));
		if (set_thread_description_func) {
			set_thread_description_func(::GetCurrentThread(),
				utils::Utf8ToWide(name).c_str());
		}

		// The debugger needs to be around to catch the name in the exception.  If
		// there isn't a debugger, we are just needlessly throwing an exception.
		if (!::IsDebuggerPresent())
			return;

		SetNameInternal(CurrentId(), name.c_str());
	}

	// static
	bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate* delegate,
		PlatformThreadHandle* thread_handle,
		ThreadPriority priority) {
		assert(thread_handle);
		return CreateThreadInternal(stack_size, delegate, thread_handle, priority);
	}

	// static
	bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate* delegate) {
		return CreateNonJoinableWithPriority(stack_size, delegate,
			ThreadPriority::NORMAL);
	}

	// static
	bool PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
		Delegate* delegate,
		ThreadPriority priority) {
		return CreateThreadInternal(stack_size, delegate, nullptr /* non-joinable */,
			priority);
	}

	// static
	void PlatformThread::Join(PlatformThreadHandle thread_handle) {
		assert(thread_handle);

		//DWORD thread_id = 0;
		//thread_id = ::GetThreadId(thread_handle);
		//DWORD last_error = 0;
		//if (!thread_id)
		//	last_error = ::GetLastError();

		//// Record information about the exiting thread in case joining hangs.
		//base::debug::Alias(&thread_id);
		//base::debug::Alias(&last_error);

		//// Record the event that this thread is blocking upon (for hang diagnosis).
		//base::debug::ScopedThreadJoinActivity thread_activity(&thread_handle);

		//base::internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
		//	base::BlockingType::MAY_BLOCK);

		// Wait for the thread to exit.  It should already have terminated but make
		// sure this assumption is valid.
		assert(WAIT_OBJECT_0 == WaitForSingleObject(thread_handle, INFINITE));
		CloseHandle(thread_handle);
	}

	// static
	void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
		CloseHandle(thread_handle);
	}

	namespace internal {

		void AssertMemoryPriority(HANDLE thread, int memory_priority) {
#ifdef _DEBUG
			static const auto get_thread_information_fn =
				reinterpret_cast<decltype(&::GetThreadInformation)>(::GetProcAddress(
					::GetModuleHandle(L"Kernel32.dll"), "GetThreadInformation"));

			if (!get_thread_information_fn) {
				DCHECK_EQ(win::GetVersion(), win::Version::WIN7);
				return;
			}

			MEMORY_PRIORITY_INFORMATION memory_priority_information = {};
			DCHECK(get_thread_information_fn(thread, ::ThreadMemoryPriority,
				&memory_priority_information,
				sizeof(memory_priority_information)));

			DCHECK_EQ(memory_priority,
				static_cast<int>(memory_priority_information.MemoryPriority));
#endif
		}

	}  // namespace internal

	// static
	void PlatformThread::SetCurrentThreadPriorityImpl(ThreadPriority priority) {
		const bool use_thread_mode_background = 
			(priority == ThreadPriority::BACKGROUND);

		PlatformThreadHandle thread_handle =
			PlatformThread::CurrentHandle();

		if (use_thread_mode_background && priority != ThreadPriority::BACKGROUND) {
			// Exit background mode if the new priority is not BACKGROUND. This is a
			// no-op if not in background mode.
			::SetThreadPriority(thread_handle, THREAD_MODE_BACKGROUND_END);
			internal::AssertMemoryPriority(thread_handle, MEMORY_PRIORITY_NORMAL);
		}

		int desired_priority = THREAD_PRIORITY_ERROR_RETURN;
		switch (priority) {
		case ThreadPriority::BACKGROUND:
			desired_priority = use_thread_mode_background
				? THREAD_MODE_BACKGROUND_BEGIN
				: THREAD_PRIORITY_LOWEST;
			break;
		case ThreadPriority::NORMAL:
			desired_priority = THREAD_PRIORITY_NORMAL;
			break;
		case ThreadPriority::DISPLAY:
			desired_priority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case ThreadPriority::REALTIME_AUDIO:
			desired_priority = THREAD_PRIORITY_TIME_CRITICAL;
			break;
		default:
			NOTREACHED() << "Unknown priority.";
			break;
		}
		DCHECK_NE(desired_priority, THREAD_PRIORITY_ERROR_RETURN);

#if DCHECK_IS_ON()
		const BOOL success =
#endif
			::SetThreadPriority(thread_handle, desired_priority);
		DPLOG_IF(ERROR, !success) << "Failed to set thread priority to "
			<< desired_priority;

		if (use_thread_mode_background && priority == ThreadPriority::BACKGROUND) {
			// In a background process, THREAD_MODE_BACKGROUND_BEGIN lowers the memory
			// and I/O priorities but not the CPU priority (kernel bug?). Use
			// THREAD_PRIORITY_LOWEST to also lower the CPU priority.
			// https://crbug.com/901483
			if (GetCurrentThreadPriority() != ThreadPriority::BACKGROUND) {
				::SetThreadPriority(thread_handle, THREAD_PRIORITY_LOWEST);
				// Make sure that using THREAD_PRIORITY_LOWEST didn't affect the memory
				// priority set by THREAD_MODE_BACKGROUND_BEGIN. There is no practical
				// way to verify the I/O priority.
				internal::AssertMemoryPriority(thread_handle, MEMORY_PRIORITY_VERY_LOW);
			}
		}

		DCHECK_EQ(GetCurrentThreadPriority(), priority);
	}
}