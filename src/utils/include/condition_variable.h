#pragma once
#include "lock.h"
#include <stdint.h>
#include <assert.h>
#include "macros.h"

namespace utils {

	class ConditionVariable {
	public:
		// Construct a cv for use with ONLY one user lock.
		explicit ConditionVariable(Lock* user_lock);

		~ConditionVariable();

		// Wait() releases the caller's critical section atomically as it starts to
		// sleep, and the reacquires it when it is signaled. The wait functions are
		// susceptible to spurious wakeups. (See usage note 1 for more details.)
		void Wait();
		void TimedWait(const uint32_t& max_time);

		// Broadcast() revives all waiting threads. (See usage note 2 for more
		// details.)
		void Broadcast();
		// Signal() revives one waiting thread.
		void Signal();

		// Declares that this ConditionVariable will only ever be used by a thread
		// that is idle at the bottom of its stack and waiting for work (in
		// particular, it is not synchronously waiting on this ConditionVariable
		// before resuming ongoing work). This is useful to avoid telling
		// base-internals that this thread is "blocked" when it's merely idle and
		// ready to do work. As such, this is only expected to be used by thread and
		// thread pool impls.
		void declare_only_used_while_idle() { waiting_is_blocking_ = false; }

	private:

		internal::CHROME_CONDITION_VARIABLE cv_;
		internal::CHROME_SRWLOCK* const srwlock_;

		// Whether a thread invoking Wait() on this ConditionalVariable should be
		// considered blocked as opposed to idle (and potentially replaced if part of
		// a pool).
		bool waiting_is_blocking_ = true;

		DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
	};

}