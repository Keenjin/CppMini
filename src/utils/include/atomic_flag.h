#pragma once

#include <stdint.h>
#include <atomic>
#include "macros.h"

namespace utils {

	// A flag that can safely be set from one thread and read from other threads.
	//
	// This class IS NOT intended for synchronization between threads.
	class AtomicFlag {
	public:
		AtomicFlag();
		~AtomicFlag();

		// Set the flag. Must always be called from the same sequence.
		void Set();

		// Returns true iff the flag was set. If this returns true, the current thread
		// is guaranteed to be synchronized with all memory operations on the sequence
		// which invoked Set() up until at least the first call to Set() on it.
		bool IsSet() const {
			// Inline here: this has a measurable performance impact on base::WeakPtr.
			return flag_.load(std::memory_order_acquire) != 0;
		}

		// Resets the flag. Be careful when using this: callers might not expect
		// IsSet() to return false after returning true once.
		void UnsafeResetForTesting();

	private:
		std::atomic<uint_fast8_t> flag_{ 0 };

		DISALLOW_COPY_AND_ASSIGN(AtomicFlag);
	};
}
