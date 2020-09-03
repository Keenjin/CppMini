#pragma once

#include <atomic>
#include "macros.h"

namespace utils {
	// AtomicSequenceNumber is a thread safe increasing sequence number generator.
	// Its constructor doesn't emit a static initializer, so it's safe to use as a
	// global variable or static member.
	class AtomicSequenceNumber {
	public:
		constexpr AtomicSequenceNumber() = default;

		// Returns an increasing sequence number starts from 0 for each call.
		// This function can be called from any thread without data race.
		inline int GetNext() { return seq_.fetch_add(1, std::memory_order_relaxed); }

	private:
		std::atomic_int seq_{ 0 };

		DISALLOW_COPY_AND_ASSIGN(AtomicSequenceNumber);
	};
}