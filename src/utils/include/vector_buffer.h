#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <type_traits>
#include <utility>

#include "macros.h"

namespace utils {

	namespace internal {

		// Internal implementation detail of base/containers.
		//
		// Implements a vector-like buffer that holds a certain capacity of T. Unlike
		// std::vector, VectorBuffer never constructs or destructs its arguments, and
		// can't change sizes. But it does implement templates to assist in efficient
		// moving and destruction of those items manually.
		//
		// In particular, the destructor function does not iterate over the items if
		// there is no destructor. Moves should be implemented as a memcpy/memmove for
		// trivially copyable objects (POD) otherwise, it should be a std::move if
		// possible, and as a last resort it falls back to a copy. This behavior is
		// similar to std::vector.
		//
		// No special consideration is done for noexcept move constructors since
		// we compile without exceptions.
		//
		// The current API does not support moving overlapping ranges.
		template <typename T>
		class VectorBuffer {
		public:
			constexpr VectorBuffer() = default;

			VectorBuffer(size_t count)
				: buffer_(reinterpret_cast<T*>(
					malloc(sizeof(T) * count))),
				capacity_(count) {
			}
			VectorBuffer(VectorBuffer&& other) noexcept
				: buffer_(other.buffer_), capacity_(other.capacity_) {
				other.buffer_ = nullptr;
				other.capacity_ = 0;
			}

			~VectorBuffer() { free(buffer_); }

			VectorBuffer& operator=(VectorBuffer&& other) {
				free(buffer_);
				buffer_ = other.buffer_;
				capacity_ = other.capacity_;

				other.buffer_ = nullptr;
				other.capacity_ = 0;
				return *this;
			}

			size_t capacity() const { return capacity_; }

			T& operator[](size_t i) {
				// TODO(crbug.com/817982): Some call sites (at least circular_deque.h) are
				// calling this with `i == capacity_` as a way of getting `end()`. Therefore
				// we have to allow this for now (`i <= capacity_`), until we fix those call
				// sites to use real iterators. This comment applies here and to `const T&
				// operator[]`, below.
				assert(i <= capacity_);
				return buffer_[i];
			}

			const T& operator[](size_t i) const {
				assert(i <= capacity_);
				return buffer_[i];
			}

			T* begin() { return buffer_; }
			T* end() { return &buffer_[capacity_]; }

			// DestructRange ------------------------------------------------------------

			// Trivially destructible objects need not have their destructors called.
			template <typename T2 = T,
				typename std::enable_if<std::is_trivially_destructible<T2>::value,
				int>::type = 0>
				void DestructRange(T* begin, T* end) {}

			// Non-trivially destructible objects must have their destructors called
			// individually.
			template <typename T2 = T,
				typename std::enable_if<!std::is_trivially_destructible<T2>::value,
				int>::type = 0>
				void DestructRange(T* begin, T* end) {
				assert(begin <= end);
				while (begin != end) {
					begin->~T();
					begin++;
				}
			}

			// MoveRange ----------------------------------------------------------------
			//
			// The destructor will be called (as necessary) for all moved types. The
			// ranges must not overlap.
			//
			// The parameters and begin and end (one past the last) of the input buffer,
			// and the address of the first element to copy to. There must be sufficient
			// room in the destination for all items in the range [begin, end).

			// Trivially copyable types can use memcpy. trivially copyable implies
			// that there is a trivial destructor as we don't have to call it.
			template <typename T2 = T,
				typename std::enable_if<std::is_trivially_copyable<T2>::value,
				int>::type = 0>
				static void MoveRange(T* from_begin, T* from_end, T* to) {
				assert(!RangesOverlap(from_begin, from_end, to));
				memcpy(
					to, from_begin,
					reinterpret_cast<uintptr_t>(from_end) - reinterpret_cast<uintptr_t>(from_begin));
			}

			// Not trivially copyable, but movable: call the move constructor and
			// destruct the original.
			template <typename T2 = T,
				typename std::enable_if<std::is_move_constructible<T2>::value &&
				!std::is_trivially_copyable<T2>::value,
				int>::type = 0>
				static void MoveRange(T* from_begin, T* from_end, T* to) {
				assert(!RangesOverlap(from_begin, from_end, to));
				while (from_begin != from_end) {
					new (to) T(std::move(*from_begin));
					from_begin->~T();
					from_begin++;
					to++;
				}
			}

			// Not movable, not trivially copyable: call the copy constructor and
			// destruct the original.
			template <typename T2 = T,
				typename std::enable_if<!std::is_move_constructible<T2>::value &&
				!std::is_trivially_copyable<T2>::value,
				int>::type = 0>
				static void MoveRange(T* from_begin, T* from_end, T* to) {
				assert(!RangesOverlap(from_begin, from_end, to));
				while (from_begin != from_end) {
					new (to) T(*from_begin);
					from_begin->~T();
					from_begin++;
					to++;
				}
			}

		private:
			static bool RangesOverlap(const T* from_begin,
				const T* from_end,
				const T* to) {
				const auto from_begin_uintptr = reinterpret_cast<uintptr_t>(from_begin);
				const auto from_end_uintptr = reinterpret_cast<uintptr_t>(from_end);
				const auto to_uintptr = reinterpret_cast<uintptr_t>(to);
				return !(
					to >= from_end ||
					(to_uintptr+ from_end_uintptr - from_begin_uintptr)
					 <= from_begin_uintptr);
			}

			T* buffer_ = nullptr;
			size_t capacity_ = 0;

			DISALLOW_COPY_AND_ASSIGN(VectorBuffer);
		};
	}
}