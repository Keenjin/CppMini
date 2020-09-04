#pragma once
#include "utils/macros.h"

namespace task_schedule {

	class PlatformThreadHandle {
	public:
		typedef void* Handle;

		constexpr PlatformThreadHandle() : handle_(0) {}

		explicit constexpr PlatformThreadHandle(Handle handle) : handle_(handle) {}

		bool is_equal(const PlatformThreadHandle& other) const {
			return handle_ == other.handle_;
		}

		bool is_null() const {
			return !handle_;
		}

		Handle platform_handle() const {
			return handle_;
		}

	private:
		Handle handle_;
	};

	class PlatformThread {
	public:
		class Delegate {
		public:
			virtual void ThreadMain() = 0;

		protected:
			virtual ~Delegate() = default;
		};

		static bool Create(Delegate* delegate, PlatformThreadHandle* thread_handle);
		static void Join(PlatformThreadHandle thread_handle);
		static void Detach(PlatformThreadHandle thread_handle);

	private:
		DISALLOW_IMPLICIT_CONSTRUCTORS(PlatformThread);
	};
}