#pragma once

#include "ref_counted.h"
#include "atomic_flag.h"
#include "scoped_refptr.h"
#include "macros.h"

namespace utils {

	template <typename T> class SupportsWeakPtr;
	template <typename T> class WeakPtr;

	namespace internal {
		// These classes are part of the WeakPtr implementation.
		// DO NOT USE THESE CLASSES DIRECTLY YOURSELF.

		class WeakReference {
		public:
			// Although Flag is bound to a specific SequencedTaskRunner, it may be
			// deleted from another via base::WeakPtr::~WeakPtr().
			class Flag : public RefCountedThreadSafe<Flag> {
			public:
				Flag();

				void Invalidate();
				bool IsValid() const;

				bool MaybeValid() const;

				void DetachFromSequence();

			private:
				friend class RefCountedThreadSafe<Flag>;

				~Flag();

				AtomicFlag invalidated_;
			};

			WeakReference();
			explicit WeakReference(const scoped_refptr<Flag>& flag);
			~WeakReference();

			WeakReference(WeakReference&& other) noexcept;
			WeakReference(const WeakReference& other);
			WeakReference& operator=(WeakReference&& other) noexcept = default;
			WeakReference& operator=(const WeakReference& other) = default;

			bool IsValid() const;
			bool MaybeValid() const;

		private:
			scoped_refptr<const Flag> flag_;
		};

		class WeakReferenceOwner {
		public:
			WeakReferenceOwner();
			~WeakReferenceOwner();

			WeakReference GetRef() const;

			bool HasRefs() const { return !flag_->HasOneRef(); }

			void Invalidate();

		private:
			mutable scoped_refptr<WeakReference::Flag> flag_;
		};

		// This class simplifies the implementation of WeakPtr's type conversion
		// constructor by avoiding the need for a public accessor for ref_.  A
		// WeakPtr<T> cannot access the private members of WeakPtr<U>, so this
		// base class gives us a way to access ref_ in a protected fashion.
		class WeakPtrBase {
		public:
			WeakPtrBase();
			~WeakPtrBase();

			WeakPtrBase(const WeakPtrBase& other) = default;
			WeakPtrBase(WeakPtrBase&& other) noexcept = default;
			WeakPtrBase& operator=(const WeakPtrBase& other) = default;
			WeakPtrBase& operator=(WeakPtrBase&& other) noexcept = default;

			void reset() {
				ref_ = internal::WeakReference();
				ptr_ = 0;
			}

		protected:
			WeakPtrBase(const WeakReference& ref, uintptr_t ptr);

			WeakReference ref_;

			// This pointer is only valid when ref_.is_valid() is true.  Otherwise, its
			// value is undefined (as opposed to nullptr).
			uintptr_t ptr_;
		};

		// This class provides a common implementation of common functions that would
		// otherwise get instantiated separately for each distinct instantiation of
		// SupportsWeakPtr<>.
		class SupportsWeakPtrBase {
		public:
			// A safe static downcast of a WeakPtr<Base> to WeakPtr<Derived>. This
			// conversion will only compile if there is exists a Base which inherits
			// from SupportsWeakPtr<Base>. See base::AsWeakPtr() below for a helper
			// function that makes calling this easier.
			//
			// Precondition: t != nullptr
			template<typename Derived>
			static WeakPtr<Derived> StaticAsWeakPtr(Derived* t) {
				static_assert(
					std::is_base_of<internal::SupportsWeakPtrBase, Derived>::value,
					"AsWeakPtr argument must inherit from SupportsWeakPtr");
				return AsWeakPtrImpl<Derived>(t);
			}

		private:
			// This template function uses type inference to find a Base of Derived
			// which is an instance of SupportsWeakPtr<Base>. We can then safely
			// static_cast the Base* to a Derived*.
			template <typename Derived, typename Base>
			static WeakPtr<Derived> AsWeakPtrImpl(SupportsWeakPtr<Base>* t) {
				WeakPtr<Base> ptr = t->AsWeakPtr();
				return WeakPtr<Derived>(
					ptr.ref_, static_cast<Derived*>(reinterpret_cast<Base*>(ptr.ptr_)));
			}
		};

	}  // namespace internal

	template <typename T> class WeakPtrFactory;

	// The WeakPtr class holds a weak reference to |T*|.
	//
	// This class is designed to be used like a normal pointer.  You should always
	// null-test an object of this class before using it or invoking a method that
	// may result in the underlying object being destroyed.
	//
	// EXAMPLE:
	//
	//   class Foo { ... };
	//   WeakPtr<Foo> foo;
	//   if (foo)
	//     foo->method();
	//
	template <typename T>
	class WeakPtr : public internal::WeakPtrBase {
	public:
		WeakPtr() = default;
		WeakPtr(std::nullptr_t) {}

		// Allow conversion from U to T provided U "is a" T. Note that this
		// is separate from the (implicit) copy and move constructors.
		template <typename U>
		WeakPtr(const WeakPtr<U>& other) : WeakPtrBase(other) {
			// Need to cast from U* to T* to do pointer adjustment in case of multiple
			// inheritance. This also enforces the "U is a T" rule.
			T* t = reinterpret_cast<U*>(other.ptr_);
			ptr_ = reinterpret_cast<uintptr_t>(t);
		}
		template <typename U>
		WeakPtr(WeakPtr<U>&& other) noexcept : WeakPtrBase(std::move(other)) {
			// Need to cast from U* to T* to do pointer adjustment in case of multiple
			// inheritance. This also enforces the "U is a T" rule.
			T* t = reinterpret_cast<U*>(other.ptr_);
			ptr_ = reinterpret_cast<uintptr_t>(t);
		}

		T* get() const {
			return ref_.IsValid() ? reinterpret_cast<T*>(ptr_) : nullptr;
		}

		T& operator*() const {
			assert(get() != nullptr);
			return *get();
		}
		T* operator->() const {
			assert(get() != nullptr);
			return get();
		}

		// Allow conditionals to test validity, e.g. if (weak_ptr) {...};
		explicit operator bool() const { return get() != nullptr; }

		// Returns false if the WeakPtr is confirmed to be invalid. This call is safe
		// to make from any thread, e.g. to optimize away unnecessary work, but
		// operator bool() must always be called, on the correct sequence, before
		// actually using the pointer.
		//
		// Warning: as with any object, this call is only thread-safe if the WeakPtr
		// instance isn't being re-assigned or reset() racily with this call.
		bool MaybeValid() const { return ref_.MaybeValid(); }

		// Returns whether the object |this| points to has been invalidated. This can
		// be used to distinguish a WeakPtr to a destroyed object from one that has
		// been explicitly set to null.
		bool WasInvalidated() const { return ptr_ && !ref_.IsValid(); }

	private:
		friend class internal::SupportsWeakPtrBase;
		template <typename U> friend class WeakPtr;
		friend class SupportsWeakPtr<T>;
		friend class WeakPtrFactory<T>;

		WeakPtr(const internal::WeakReference& ref, T* ptr)
			: WeakPtrBase(ref, reinterpret_cast<uintptr_t>(ptr)) {}
	};

	// Allow callers to compare WeakPtrs against nullptr to test validity.
	template <class T>
	bool operator!=(const WeakPtr<T>& weak_ptr, std::nullptr_t) {
		return !(weak_ptr == nullptr);
	}
	template <class T>
	bool operator!=(std::nullptr_t, const WeakPtr<T>& weak_ptr) {
		return weak_ptr != nullptr;
	}
	template <class T>
	bool operator==(const WeakPtr<T>& weak_ptr, std::nullptr_t) {
		return weak_ptr.get() == nullptr;
	}
	template <class T>
	bool operator==(std::nullptr_t, const WeakPtr<T>& weak_ptr) {
		return weak_ptr == nullptr;
	}

	namespace internal {
		class WeakPtrFactoryBase {
		protected:
			WeakPtrFactoryBase(uintptr_t ptr);
			~WeakPtrFactoryBase();
			internal::WeakReferenceOwner weak_reference_owner_;
			uintptr_t ptr_;
		};
	}  // namespace internal

	// A class may be composed of a WeakPtrFactory and thereby
	// control how it exposes weak pointers to itself.  This is helpful if you only
	// need weak pointers within the implementation of a class.  This class is also
	// useful when working with primitive types.  For example, you could have a
	// WeakPtrFactory<bool> that is used to pass around a weak reference to a bool.
	template <class T>
	class WeakPtrFactory : public internal::WeakPtrFactoryBase {
	public:
		explicit WeakPtrFactory(T* ptr)
			: WeakPtrFactoryBase(reinterpret_cast<uintptr_t>(ptr)) {}

		~WeakPtrFactory() = default;

		WeakPtr<T> GetWeakPtr() {
			return WeakPtr<T>(weak_reference_owner_.GetRef(),
				reinterpret_cast<T*>(ptr_));
		}

		// Call this method to invalidate all existing weak pointers.
		void InvalidateWeakPtrs() {
			assert(ptr_);
			weak_reference_owner_.Invalidate();
		}

		// Call this method to determine if any weak pointers exist.
		bool HasWeakPtrs() const {
			assert(ptr_);
			return weak_reference_owner_.HasRefs();
		}

	private:
		DISALLOW_IMPLICIT_CONSTRUCTORS(WeakPtrFactory);
	};
}