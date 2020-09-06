#pragma once
#include "lock_impl.h"

namespace utils {

	// A convenient wrapper for an OS specific critical section.  The only real
// intelligence in this class is in debug mode for the support for the
// AssertAcquired() method.
	class Lock {
	public:
		// Optimized wrapper implementation
		Lock() : lock_() {}
		~Lock() {}

		// TODO(lukasza): https://crbug.com/831825: Add EXCLUSIVE_LOCK_FUNCTION
		// annotation to Acquire method and similar annotations to Release, Try and
		// AssertAcquired methods (here and in the #else branch).
		void Acquire() { lock_.Lock(); }
		void Release() { lock_.Unlock(); }

		// If the lock is not held, take it and return true. If the lock is already
		// held by another thread, immediately return false. This must not be called
		// by a thread already holding the lock (what happens is undefined and an
		// assertion may fail).
		bool Try() { return lock_.Try(); }

		// Null implementation if not debug.
		void AssertAcquired() const {}

		// Whether Lock mitigates priority inversion when used from different thread
		// priorities.
		static bool HandlesMultipleThreadPriorities() {
			// Windows mitigates priority inversion by randomly boosting the priority of
			// ready threads.
			// https://msdn.microsoft.com/library/windows/desktop/ms684831.aspx
			return true;
		}

		// Both Windows and POSIX implementations of ConditionVariable need to be
		// able to see our lock and tweak our debugging counters, as they release and
		// acquire locks inside of their condition variable APIs.
		friend class ConditionVariable;

	private:

		// Platform specific underlying lock implementation.
		internal::LockImpl lock_;

		DISALLOW_COPY_AND_ASSIGN(Lock);
	};

	// A helper class that acquires the given Lock while the AutoLock is in scope.
	using AutoLock = internal::BasicAutoLock<Lock>;

	// AutoUnlock is a helper that will Release() the |lock| argument in the
	// constructor, and re-Acquire() it in the destructor.
	using AutoUnlock = internal::BasicAutoUnlock<Lock>;

	// Like AutoLock but is a no-op when the provided Lock* is null. Inspired from
	// absl::MutexLockMaybe. Use this instead of base::Optional<base::AutoLock> to
	// get around -Wthread-safety-analysis warnings for conditional locking.
	using AutoLockMaybe = internal::BasicAutoLockMaybe<Lock>;

	// Like AutoLock but permits Release() of its mutex before destruction.
	// Release() may be called at most once. Inspired from
	// absl::ReleasableMutexLock. Use this instead of base::Optional<base::AutoLock>
	// to get around -Wthread-safety-analysis warnings for AutoLocks that are
	// explicitly released early (prefer proper scoping to this).
	using ReleasableAutoLock = internal::BasicReleasableAutoLock<Lock>;
}