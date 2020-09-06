// WARNING: Thread local storage is a bit tricky to get right. Please make sure
// that this is really the proper solution for what you're trying to achieve.
// Don't prematurely optimize, most likely you can just use a Lock.
//
// These classes implement a wrapper around ThreadLocalStorage::Slot. On
// construction, they will allocate a TLS slot, and free the TLS slot on
// destruction. No memory management (creation or destruction) is handled. This
// means for uses of ThreadLocalPointer, you must correctly manage the memory
// yourself, these classes will not destroy the pointer for you. There are no
// at-thread-exit actions taken by these classes.
//
// ThreadLocalPointer<Type> wraps a Type*. It performs no creation or
// destruction, so memory management must be handled elsewhere. The first call
// to Get() on a thread will return NULL. You can update the pointer with a call
// to Set().
//
// ThreadLocalBoolean wraps a bool. It will default to false if it has never
// been set otherwise with Set().
//
// Thread Safety: An instance of ThreadLocalStorage is completely thread safe
// once it has been created. If you want to dynamically create an instance, you
// must of course properly deal with safety and race conditions.
//
// In Android, the system TLS is limited.
//
// Example usage:
//   // My class is logically attached to a single thread. We cache a pointer
//   // on the thread it was created on, so we can implement current().
//   MyClass::MyClass() {
//     DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() == NULL);
//     Singleton<ThreadLocalPointer<MyClass> >::get()->Set(this);
//   }
//
//   MyClass::~MyClass() {
//     DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() != NULL);
//     Singleton<ThreadLocalPointer<MyClass> >::get()->Set(NULL);
//   }
//
//   // Return the current MyClass associated with the calling thread, can be
//   // NULL if there isn't a MyClass associated.
//   MyClass* MyClass::current() {
//     return Singleton<ThreadLocalPointer<MyClass> >::get()->Get();
//   }

#pragma once
#include "macros.h"
#include "thread_local_storage.h"
#include <memory>

namespace utils {

	template <typename T>
	class ThreadLocalPointer {
	public:
		ThreadLocalPointer() = default;
		~ThreadLocalPointer() = default;

		T* Get() const { return static_cast<T*>(slot_.Get()); }

		void Set(T* ptr) {
			slot_.Set(const_cast<void*>(static_cast<const void*>(ptr)));
		}

	private:
		ThreadLocalStorage::Slot slot_;

		DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer<T>);
	};

	template <typename T>
	class ThreadLocalOwnedPointer {
	public:
		ThreadLocalOwnedPointer() = default;

		~ThreadLocalOwnedPointer() {
			// Assume that this thread is the only one with potential state left. This
			// is verified in ~CheckedThreadLocalOwnedPointer().
			Set(nullptr);
		}

		T* Get() const { return static_cast<T*>(slot_.Get()); }

		void Set(std::unique_ptr<T> ptr) {
			delete Get();
			slot_.Set(const_cast<void*>(static_cast<const void*>(ptr.release())));
		}

	private:
		static void DeleteTlsPtr(void* ptr) { delete static_cast<T*>(ptr); }

		ThreadLocalStorage::Slot slot_{ &DeleteTlsPtr };

		DISALLOW_COPY_AND_ASSIGN(ThreadLocalOwnedPointer<T>);
	};

	class ThreadLocalBoolean {
	public:
		ThreadLocalBoolean() = default;
		~ThreadLocalBoolean() = default;

		bool Get() const { return tlp_.Get() != nullptr; }

		void Set(bool val) { tlp_.Set(val ? this : nullptr); }

	private:
		ThreadLocalPointer<void> tlp_;

		DISALLOW_COPY_AND_ASSIGN(ThreadLocalBoolean);
	};
}