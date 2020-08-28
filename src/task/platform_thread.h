#pragma once

#include <Windows.h>
#include <stdint.h>
#include <string>
#include "utils/macros.h"

namespace task {

	typedef DWORD PlatformThreadId;
	typedef HANDLE PlatformThreadHandle;

	// Valid values for priority of Thread::Options and SimpleThread::Options, and
	// SetCurrentThreadPriority(), listed in increasing order of importance.
	enum class ThreadPriority : int {
		// Suitable for threads that shouldn't disrupt high priority work.
		BACKGROUND,
		// Default priority level.
		NORMAL,
		// Suitable for threads which generate data for the display (at ~60Hz).
		DISPLAY,
		// Suitable for low-latency, glitch-resistant audio.
		REALTIME_AUDIO,
	};

	class PlatformThread
	{
	public:
		// Implement this interface to run code on a background thread.  Your
		// ThreadMain method will be called on the newly created thread.
		class Delegate
		{
		public:
			virtual void ThreadMain() = 0;
		protected:
			virtual ~Delegate() = default;
		};

		// Gets the current thread id, which may be useful for logging purposes.
		static PlatformThreadId CurrentId();

		// Get the handle representing the current thread. On Windows, this is a
		// pseudo handle constant which will always represent the thread using it and
		// hence should not be shared with other threads nor be used to differentiate
		// the current thread from another.
		static PlatformThreadHandle CurrentHandle();

		// Yield the current thread so another thread can be scheduled.
		static void YieldCurrentThread();

		// µ¥Î»£ºms¡£
		static void Sleep(uint32_t duration);

		// Sets the thread name visible to debuggers/tools. This will try to
		// initialize the context for current thread unless it's a WorkerThread.
		static void SetName(const std::string& name);

		// Gets the thread name, if previously set by SetName.
		//static const char* GetName();

		// Creates a new thread.  The |stack_size| parameter can be 0 to indicate
		// that the default stack size should be used.  Upon success,
		// |*thread_handle| will be assigned a handle to the newly created thread,
		// and |delegate|'s ThreadMain method will be executed on the newly created
		// thread.
		// NOTE: When you are done with the thread handle, you must call Join to
		// release system resources associated with the thread.  You must ensure that
		// the Delegate object outlives the thread.
		static bool Create(size_t stack_size,
			Delegate* delegate,
			PlatformThreadHandle* thread_handle) {
			return CreateWithPriority(stack_size, delegate, thread_handle,
				ThreadPriority::NORMAL);
		}

		// CreateWithPriority() does the same thing as Create() except the priority of
		// the thread is set based on |priority|.
		static bool CreateWithPriority(size_t stack_size, Delegate* delegate,
			PlatformThreadHandle* thread_handle,
			ThreadPriority priority);

		// CreateNonJoinable() does the same thing as Create() except the thread
		// cannot be Join()'d.  Therefore, it also does not output a
		// PlatformThreadHandle.
		static bool CreateNonJoinable(size_t stack_size, Delegate* delegate);

		// CreateNonJoinableWithPriority() does the same thing as CreateNonJoinable()
		// except the priority of the thread is set based on |priority|.
		static bool CreateNonJoinableWithPriority(size_t stack_size,
			Delegate* delegate,
			ThreadPriority priority);

		// Joins with a thread created via the Create function.  This function blocks
		// the caller until the designated thread exits.  This will invalidate
		// |thread_handle|.
		static void Join(PlatformThreadHandle thread_handle);

		// Detaches and releases the thread handle. The thread is no longer joinable
		// and |thread_handle| is invalidated after this call.
		static void Detach(PlatformThreadHandle thread_handle);

		// Toggles the current thread's priority at runtime.
		//
		// A thread may not be able to raise its priority back up after lowering it if
		// the process does not have a proper permission, e.g. CAP_SYS_NICE on Linux.
		// A thread may not be able to lower its priority back down after raising it
		// to REALTIME_AUDIO.
		//
		// This function must not be called from the main thread on Mac. This is to
		// avoid performance regressions (https://crbug.com/601270).
		//
		// Since changing other threads' priority is not permitted in favor of
		// security, this interface is restricted to change only the current thread
		// priority (https://crbug.com/399473).
		static void SetCurrentThreadPriority(ThreadPriority priority);

		static ThreadPriority GetCurrentThreadPriority();

		// Returns the default thread stack size set by chrome. If we do not
		// explicitly set default size then returns 0.
		static size_t GetDefaultThreadStackSize();

	private:
		static void SetCurrentThreadPriorityImpl(ThreadPriority priority);

		DISALLOW_IMPLICIT_CONSTRUCTORS(PlatformThread);
	};
}