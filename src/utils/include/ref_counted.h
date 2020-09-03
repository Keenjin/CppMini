#pragma once

#include "macros.h"
#include "scoped_refptr.h"
#include "atomic_ref_count.h"

namespace utils {

	namespace internal {
		class RefCountedBase {
		public:
			bool HasOneRef() const { return ref_count_ == 1; }
			bool HasAtLeastOneRef() const { return ref_count_ >= 1; }

		protected:
			explicit RefCountedBase(StartRefCountFromZeroTag) {
			}

			explicit RefCountedBase(StartRefCountFromOneTag) : ref_count_(1) {
			}

			~RefCountedBase() {
			}

			void AddRef() const {
				// TODO(maruel): Add back once it doesn't assert 500 times/sec.
				// Current thread books the critical section "AddRelease"
				// without release it.
				// DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);

				AddRefImpl();
			}

			// Returns true if the object should self-delete.
			bool Release() const {
				ReleaseImpl();

				// TODO(maruel): Add back once it doesn't assert 500 times/sec.
				// Current thread books the critical section "AddRelease"
				// without release it.
				// DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);

				return ref_count_ == 0;
			}

			// Returns true if it is safe to read or write the object, from a thread
			// safety standpoint. Should be DCHECK'd from the methods of RefCounted
			// classes if there is a danger of objects being shared across threads.
			//
			// This produces fewer false positives than adding a separate SequenceChecker
			// into the subclass, because it automatically detaches from the sequence when
			// the reference count is 1 (and never fails if there is only one reference).
			//
			// This means unlike a separate SequenceChecker, it will permit a singly
			// referenced object to be passed between threads (not holding a reference on
			// the sending thread), but will trap if the sending thread holds onto a
			// reference, or if the object is accessed from multiple threads
			// simultaneously.
			bool IsOnValidSequence() const {
				return true;
			}

		private:
			template <typename U>
			friend scoped_refptr<U> AdoptRef(U*);

			void Adopted() const {
			}

			void AddRefImpl() const { ++ref_count_; }
			void ReleaseImpl() const { --ref_count_; }

			mutable uint32_t ref_count_ = 0;
			static_assert(std::is_unsigned<decltype(ref_count_)>::value,
				"ref_count_ must be an unsigned type.");

			DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
		};

		class RefCountedThreadSafeBase {
		public:
			bool HasOneRef() const;
			bool HasAtLeastOneRef() const;

		protected:
			explicit constexpr RefCountedThreadSafeBase(StartRefCountFromZeroTag) {}
			explicit constexpr RefCountedThreadSafeBase(StartRefCountFromOneTag)
				: ref_count_(1) {
			}

			~RefCountedThreadSafeBase() = default;

			// Release and AddRef are suitable for inlining on X86 because they generate
			// very small code sequences. On other platforms (ARM), it causes a size
			// regression and is probably not worth it.
			// Returns true if the object should self-delete.
			bool Release() const;
			void AddRef() const;
			void AddRefWithCheck() const;

		private:
			template <typename U>
			friend scoped_refptr<U> AdoptRef(U*);

			void Adopted() const {
			}

			inline void AddRefImpl() const {
				ref_count_.Increment();
			}

			inline void AddRefWithCheckImpl() const {
				assert(ref_count_.Increment() > 0);
			}

			inline bool ReleaseImpl() const {
				if (!ref_count_.Decrement()) {
					return true;
				}
				return false;
			}

			mutable AtomicRefCount ref_count_{ 0 };

			DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
		};
	}

	//
	// A base class for reference counted classes.  Otherwise, known as a cheap
	// knock-off of WebKit's RefCounted<T> class.  To use this, just extend your
	// class from it like so:
	//
	//   class MyFoo : public base::RefCounted<MyFoo> {
	//    ...
	//    private:
	//     friend class base::RefCounted<MyFoo>;
	//     ~MyFoo();
	//   };
	//
	// You should always make your destructor non-public, to avoid any code deleting
	// the object accidently while there are references to it.
	//
	//
	// The ref count manipulation to RefCounted is NOT thread safe and has DCHECKs
	// to trap unsafe cross thread usage. A subclass instance of RefCounted can be
	// passed to another execution sequence only when its ref count is 1. If the ref
	// count is more than 1, the RefCounted class verifies the ref updates are made
	// on the same execution sequence as the previous ones. The subclass can also
	// manually call IsOnValidSequence to trap other non-thread-safe accesses; see
	// the documentation for that method.
	//
	//
	// The reference count starts from zero by default, and we intended to migrate
	// to start-from-one ref count. Put REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE() to
	// the ref counted class to opt-in.
	//
	// If an object has start-from-one ref count, the first scoped_refptr need to be
	// created by base::AdoptRef() or base::MakeRefCounted(). We can use
	// base::MakeRefCounted() to create create both type of ref counted object.
	//
	// The motivations to use start-from-one ref count are:
	//  - Start-from-one ref count doesn't need the ref count increment for the
	//    first reference.
	//  - It can detect an invalid object acquisition for a being-deleted object
	//    that has zero ref count. That tends to happen on custom deleter that
	//    delays the deletion.
	//    TODO(tzik): Implement invalid acquisition detection.
	//  - Behavior parity to Blink's WTF::RefCounted, whose count starts from one.
	//    And start-from-one ref count is a step to merge WTF::RefCounted into
	//    base::RefCounted.
	//
#define REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE()             \
  static constexpr ::utils::internal::StartRefCountFromOneTag \
      kRefCountPreference = ::utils::internal::kStartRefCountFromOneTag

	template <class T, typename Traits>
	class RefCounted;

	template <typename T>
	struct DefaultRefCountedTraits {
		static void Destruct(const T* x) {
			RefCounted<T, DefaultRefCountedTraits>::DeleteInternal(x);
		}
	};

	template <class T, typename Traits = DefaultRefCountedTraits<T>>
	class RefCounted : public internal::RefCountedBase {
	public:
		static constexpr internal::StartRefCountFromZeroTag kRefCountPreference =
			internal::kStartRefCountFromZeroTag;

		RefCounted() : internal::RefCountedBase(T::kRefCountPreference) {}

		void AddRef() const {
			internal::RefCountedBase::AddRef();
		}

		void Release() const {
			if (subtle::RefCountedBase::Release()) {
				// Prune the code paths which the static analyzer may take to simulate
				// object destruction. Use-after-free errors aren't possible given the
				// lifetime guarantees of the refcounting system.
				Traits::Destruct(static_cast<const T*>(this));
			}
		}

	protected:
		~RefCounted() = default;

	private:
		friend struct DefaultRefCountedTraits<T>;
		template <typename U>
		static void DeleteInternal(const U* x) {
			delete x;
		}

		DISALLOW_COPY_AND_ASSIGN(RefCounted);
	};

	// Forward declaration.
	template <class T, typename Traits> class RefCountedThreadSafe;

	// Default traits for RefCountedThreadSafe<T>.  Deletes the object when its ref
	// count reaches 0.  Overload to delete it on a different thread etc.
	template<typename T>
	struct DefaultRefCountedThreadSafeTraits {
		static void Destruct(const T* x) {
			// Delete through RefCountedThreadSafe to make child classes only need to be
			// friend with RefCountedThreadSafe instead of this struct, which is an
			// implementation detail.
			RefCountedThreadSafe<T,
				DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
		}
	};

	//
	// A thread-safe variant of RefCounted<T>
	//
	//   class MyFoo : public base::RefCountedThreadSafe<MyFoo> {
	//    ...
	//   };
	//
	// If you're using the default trait, then you should add compile time
	// asserts that no one else is deleting your object.  i.e.
	//    private:
	//     friend class base::RefCountedThreadSafe<MyFoo>;
	//     ~MyFoo();
	//
	// We can use REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE() with RefCountedThreadSafe
	// too. See the comment above the RefCounted definition for details.
	template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
	class RefCountedThreadSafe : public internal::RefCountedThreadSafeBase {
	public:
		static constexpr internal::StartRefCountFromZeroTag kRefCountPreference =
			internal::kStartRefCountFromZeroTag;

		explicit RefCountedThreadSafe()
			: internal::RefCountedThreadSafeBase(T::kRefCountPreference) {}

		void AddRef() const { AddRefImpl(T::kRefCountPreference); }

		void Release() const {
			if (internal::RefCountedThreadSafeBase::Release()) {
				Traits::Destruct(static_cast<const T*>(this));
			}
		}

	protected:
		~RefCountedThreadSafe() = default;

	private:
		friend struct DefaultRefCountedThreadSafeTraits<T>;
		template <typename U>
		static void DeleteInternal(const U* x) {
			delete x;
		}

		void AddRefImpl(internal::StartRefCountFromZeroTag) const {
			internal::RefCountedThreadSafeBase::AddRef();
		}

		void AddRefImpl(internal::StartRefCountFromOneTag) const {
			internal::RefCountedThreadSafeBase::AddRefWithCheck();
		}

		DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafe);
	};

	//
	// A thread-safe wrapper for some piece of data so we can place other
	// things in scoped_refptrs<>.
	//
	template<typename T>
	class RefCountedData
		: public utils::RefCountedThreadSafe< utils::RefCountedData<T> > {
	public:
		RefCountedData() : data() {}
		RefCountedData(const T& in_value) : data(in_value) {}
		RefCountedData(T&& in_value) : data(std::move(in_value)) {}

		T data;

	private:
		friend class utils::RefCountedThreadSafe<utils::RefCountedData<T> >;
		~RefCountedData() = default;
	};

	template <typename T>
	bool operator==(const RefCountedData<T>& lhs, const RefCountedData<T>& rhs) {
		return lhs.data == rhs.data;
	}

	template <typename T>
	bool operator!=(const RefCountedData<T>& lhs, const RefCountedData<T>& rhs) {
		return !(lhs == rhs);
	}
}