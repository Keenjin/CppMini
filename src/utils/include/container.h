#pragma once

#include "circular_deque.h"
#include <stack>
#include <queue>

namespace utils {
	// Provides a definition of base::stack that's like std::stack but uses a
	// base::circular_deque instead of std::deque. Since std::stack is just a
	// wrapper for an underlying type, we can just provide a typedef for it that
	// defaults to the base circular_deque.
	template <class T, class Container = circular_deque<T>>
	using stack = std::stack<T, Container>;

	// Provides a definition of base::queue that's like std::queue but uses a
	// base::circular_deque instead of std::deque. Since std::queue is just a
	// wrapper for an underlying type, we can just provide a typedef for it that
	// defaults to the base circular_deque.
	template <class T, class Container = circular_deque<T>>
	using queue = std::queue<T, Container>;
}
