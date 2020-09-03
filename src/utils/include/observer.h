#pragma once

#include "macros.h"
#include "weak_ptr.h"
#include "linked_list.h"
#include <vector>
#include "stl_util.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
//
// OVERVIEW:
//
//   A list of observers. Unlike a standard vector or list, this container can
//   be modified during iteration without invalidating the iterator. So, it
//   safely handles the case of an observer removing itself or other observers
//   from the list while observers are being notified.
//
//
// WARNING:
//
//   ObserverList is not thread-compatible. Iterating on the same ObserverList
//   simultaneously in different threads is not safe, even when the ObserverList
//   itself is not modified.
//
//   For a thread-safe observer list, see ObserverListThreadSafe.
//
//
// TYPICAL USAGE:
//
//   class MyWidget {
//    public:
//     ...
//
//     class Observer : public base::CheckedObserver {
//      public:
//       virtual void OnFoo(MyWidget* w) = 0;
//       virtual void OnBar(MyWidget* w, int x, int y) = 0;
//     };
//
//     void AddObserver(Observer* obs) {
//       observers_.AddObserver(obs);
//     }
//
//     void RemoveObserver(const Observer* obs) {
//       observers_.RemoveObserver(obs);
//     }
//
//     void NotifyFoo() {
//       for (Observer& obs : observers_)
//         obs.OnFoo(this);
//     }
//
//     void NotifyBar(int x, int y) {
//       for (Observer& obs : observers_)
//         obs.OnBar(this, x, y);
//     }
//
//    private:
//     utils::ObserverList<Observer> observers_;
//   };
//
//
///////////////////////////////////////////////////////////////////////////////

namespace utils {

	// Enumeration of which observers are notified by ObserverList.
	enum class ObserverListPolicy {
		// Specifies that any observers added during notification are notified.
		// This is the default policy if no policy is provided to the constructor.
		ALL,

		// Specifies that observers added while sending out notification are not
		// notified.
		EXISTING_ONLY,
	};

	namespace internal {
		class CheckedObserverAdapter;
	}

	// A CheckedObserver serves as a base class for an observer interface designed
	// to be used with base::ObserverList. It helps detect potential use-after-free
	// issues that can occur when observers fail to remove themselves from an
	// observer list upon destruction.
	//
	// A CheckedObserver will CHECK() if an ObserverList iteration is attempted over
	// a destroyed Observer.
	//
	// Note that a CheckedObserver subclass must be deleted on the same thread as
	// the ObserverList(s) it is added to. This is DCHECK()ed via WeakPtr.
	class CheckedObserver {
	public:
		CheckedObserver();

	protected:
		virtual ~CheckedObserver();

		// Returns whether |this| is in any ObserverList. Subclasses can CHECK() this
		// in their destructor to obtain a nicer stacktrace.
		bool IsInObserverList() const;

	private:
		friend class internal::CheckedObserverAdapter;

		// Must be mutable to allow ObserverList<const Foo>.
		mutable WeakPtrFactory<CheckedObserver> factory_{ this };

		DISALLOW_COPY_AND_ASSIGN(CheckedObserver);
	};

	namespace internal {

		// Adapter for putting raw pointers into an ObserverList<Foo>::Unchecked.
		class UncheckedObserverAdapter {
		public:
			explicit UncheckedObserverAdapter(const void* observer)
				: ptr_(const_cast<void*>(observer)) {}
			UncheckedObserverAdapter(UncheckedObserverAdapter&& other) = default;
			UncheckedObserverAdapter& operator=(UncheckedObserverAdapter&& other) =
				default;

			void MarkForRemoval() { ptr_ = nullptr; }

			bool IsMarkedForRemoval() const { return !ptr_; }
			bool IsEqual(const void* rhs) const { return ptr_ == rhs; }

			template <class ObserverType>
			static ObserverType* Get(const UncheckedObserverAdapter& adapter) {
				static_assert(
					!std::is_base_of<CheckedObserver, ObserverType>::value,
					"CheckedObserver classes must not use ObserverList<T>::Unchecked.");
				return static_cast<ObserverType*>(adapter.ptr_);
			}

		private:
			void* ptr_;

			DISALLOW_COPY_AND_ASSIGN(UncheckedObserverAdapter);
		};

		// Adapter for CheckedObserver types so that they can use the same syntax as a
		// raw pointer when stored in the std::vector of observers in an ObserverList.
		// It wraps a WeakPtr<CheckedObserver> and allows a "null" pointer via
		// destruction to be distinguished from an observer marked for deferred removal
		// whilst an iteration is in progress.
		class CheckedObserverAdapter {
		public:
			explicit CheckedObserverAdapter(const CheckedObserver* observer);

			// Move-only construction and assignment is required to store this in STL
			// types.
			CheckedObserverAdapter(CheckedObserverAdapter&& other);
			CheckedObserverAdapter& operator=(CheckedObserverAdapter&& other);
			~CheckedObserverAdapter();

			void MarkForRemoval() {
				assert(weak_ptr_);
				weak_ptr_ = nullptr;
			}

			bool IsMarkedForRemoval() const {
				// If |weak_ptr_| was invalidated then this attempt to iterate over the
				// pointer is a UAF. Tip: If it's unclear where the `delete` occurred, try
				// adding CHECK(!IsInObserverList()) to the ~CheckedObserver() (destructor)
				// override. However, note that this is not always a bug: a destroyed
				// observer can exist in an ObserverList so long as nothing iterates over
				// the ObserverList before the list itself is destroyed.
				assert(!weak_ptr_.WasInvalidated());
				return weak_ptr_ == nullptr;
			}

			bool IsEqual(const CheckedObserver* rhs) const {
				// Note that inside an iteration, ObserverList::HasObserver() may call this
				// and |weak_ptr_| may be null due to a deferred removal, which is fine.
				return weak_ptr_.get() == rhs;
			}

			template <class ObserverType>
			static ObserverType* Get(const CheckedObserverAdapter& adapter) {
				static_assert(
					std::is_base_of<CheckedObserver, ObserverType>::value,
					"Observers should inherit from base::CheckedObserver. "
					"Use ObserverList<T>::Unchecked to observe with raw pointers.");
				assert(adapter.weak_ptr_);
				return static_cast<ObserverType*>(adapter.weak_ptr_.get());
			}

		private:
			WeakPtr<CheckedObserver> weak_ptr_;

			DISALLOW_COPY_AND_ASSIGN(CheckedObserverAdapter);
		};

		// Wraps a pointer in a stack-allocated, base::LinkNode. The node is
		// automatically removed from the linked list upon destruction (of the node, not
		// the pointer). Nodes are detached from the list via Invalidate() in the
		// destructor of ObserverList. This invalidates all WeakLinkNodes. There is no
		// threading support.
		template <class ObserverList>
		class WeakLinkNode : public utils::LinkNode<WeakLinkNode<ObserverList>> {
		public:
			WeakLinkNode() = default;
			explicit WeakLinkNode(ObserverList* list) { SetList(list); }

			~WeakLinkNode() { Invalidate(); }

			bool IsOnlyRemainingNode() const {
				return list_ &&
					list_->live_iterators_.head() == list_->live_iterators_.tail();
			}

			void SetList(ObserverList* list) {
				assert(!list_);
				assert(list);
				list_ = list;
				list_->live_iterators_.Append(this);
			}

			void Invalidate() {
				if (list_) {
					list_ = nullptr;
					this->RemoveFromList();
				}
			}

			ObserverList* get() const {
				return list_;
			}
			ObserverList* operator->() const { return get(); }
			explicit operator bool() const { return get(); }

		private:
			ObserverList* list_ = nullptr;

			DISALLOW_COPY_AND_ASSIGN(WeakLinkNode);
		};

	}  // namespace internal

	// When check_empty is true, assert that the list is empty on destruction.
	// When allow_reentrancy is false, iterating throught the list while already in
	// the iteration loop will result in DCHECK failure.
	// TODO(oshima): Change the default to non reentrant. https://crbug.com/812109
	template <class ObserverType,
		bool check_empty = false,
		bool allow_reentrancy = true,
		class ObserverStorageType = internal::CheckedObserverAdapter>
	class ObserverList {
	public:
		// Allow declaring an ObserverList<...>::Unchecked that replaces the default
		// ObserverStorageType to use raw pointers. This is required to support legacy
		// observers that do not inherit from CheckedObserver. The majority of new
		// code should not use this, but it may be suited for performance-critical
		// situations to avoid overheads of a CHECK(). Note the type can't be chosen
		// based on ObserverType's definition because ObserverLists are often declared
		// in headers using a forward-declare of ObserverType.
		using Unchecked = ObserverList<ObserverType,
			check_empty,
			allow_reentrancy,
			internal::UncheckedObserverAdapter>;

		// An iterator class that can be used to access the list of observers.
		class Iter {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = ObserverType;
			using difference_type = ptrdiff_t;
			using pointer = ObserverType * ;
			using reference = ObserverType & ;

			Iter() : index_(0), max_index_(0) {}

			explicit Iter(const ObserverList* list)
				: list_(const_cast<ObserverList*>(list)),
				index_(0),
				max_index_(list->policy_ == ObserverListPolicy::ALL
					? std::numeric_limits<size_t>::max()
					: list->observers_.size()) {
				assert(list);
				assert(allow_reentrancy || list_.IsOnlyRemainingNode());
				EnsureValidIndex();
			}

			~Iter() {
				if (list_.IsOnlyRemainingNode())
					list_->Compact();
			}

			Iter(const Iter& other)
				: index_(other.index_), max_index_(other.max_index_) {
				if (other.list_)
					list_.SetList(other.list_.get());
			}

			Iter& operator=(const Iter& other) {
				if (&other == this)
					return *this;

				if (list_.IsOnlyRemainingNode())
					list_->Compact();

				list_.Invalidate();
				if (other.list_)
					list_.SetList(other.list_.get());

				index_ = other.index_;
				max_index_ = other.max_index_;
				return *this;
			}

			bool operator==(const Iter& other) const {
				return (is_end() && other.is_end()) ||
					(list_.get() == other.list_.get() && index_ == other.index_);
			}

			bool operator!=(const Iter& other) const { return !(*this == other); }

			Iter& operator++() {
				if (list_) {
					++index_;
					EnsureValidIndex();
				}
				return *this;
			}

			Iter operator++(int) {
				Iter it(*this);
				++(*this);
				return it;
			}

			ObserverType* operator->() const {
				ObserverType* const current = GetCurrent();
				assert(current);
				return current;
			}

			ObserverType& operator*() const {
				ObserverType* const current = GetCurrent();
				assert(current);
				return *current;
			}

		private:
			friend class ObserverListTestBase;

			ObserverType* GetCurrent() const {
				assert(list_);
				assert(index_ < clamped_max_index());
				return ObserverStorageType::template Get<ObserverType>(
					list_->observers_[index_]);
			}

			void EnsureValidIndex() {
				assert(list_);
				const size_t max_index = clamped_max_index();
				while (index_ < max_index &&
					list_->observers_[index_].IsMarkedForRemoval()) {
					++index_;
				}
			}

			size_t clamped_max_index() const {
				return std::min(max_index_, list_->observers_.size());
			}

			bool is_end() const { return !list_ || index_ == clamped_max_index(); }

			// Lightweight weak pointer to the ObserverList.
			internal::WeakLinkNode<ObserverList> list_;

			// When initially constructed and each time the iterator is incremented,
			// |index_| is guaranteed to point to a non-null index if the iterator
			// has not reached the end of the ObserverList.
			size_t index_;
			size_t max_index_;
		};

		using iterator = Iter;
		using const_iterator = Iter;
		using value_type = ObserverType;

		const_iterator begin() const {
			// An optimization: do not involve weak pointers for empty list.
			return observers_.empty() ? const_iterator() : const_iterator(this);
		}

		const_iterator end() const { return const_iterator(); }

		explicit ObserverList(ObserverListPolicy policy = ObserverListPolicy::ALL)
			: policy_(policy) {
		}

		~ObserverList() {
			while (!live_iterators_.empty())
				live_iterators_.head()->value()->Invalidate();
			if (check_empty) {
				Compact();
				assert(observers_.empty());
			}
		}

		// Add an observer to this list. An observer should not be added to the same
		// list more than once.
		//
		// Precondition: obs != nullptr
		// Precondition: !HasObserver(obs)
		void AddObserver(ObserverType* obs) {
			assert(obs);
			if (HasObserver(obs)) {
				assert(false && "Observers can only be added once!");
				return;
			}
			observers_.emplace_back(ObserverStorageType(obs));
		}

		// Removes the given observer from this list. Does nothing if this observer is
		// not in this list.
		void RemoveObserver(const ObserverType* obs) {
			assert(obs);
			const auto it =
				std::find_if(observers_.begin(), observers_.end(),
					[obs](const auto& o) { return o.IsEqual(obs); });
			if (it == observers_.end())
				return;

			if (live_iterators_.empty()) {
				observers_.erase(it);
			}
			else {
				it->MarkForRemoval();
			}
		}

		// Determine whether a particular observer is in the list.
		bool HasObserver(const ObserverType* obs) const {
			// Client code passing null could be confused by the treatment of observers
			// removed mid-iteration. TODO(https://crbug.com/876588): This should
			// probably DCHECK, but some client code currently does pass null.
			if (obs == nullptr)
				return false;
			return std::find_if(observers_.begin(), observers_.end(),
				[obs](const auto& o) { return o.IsEqual(obs); }) !=
				observers_.end();
		}

		// Removes all the observers from this list.
		void Clear() {
			if (live_iterators_.empty()) {
				observers_.clear();
			}
			else {
				for (auto& observer : observers_)
					observer.MarkForRemoval();
			}
		}

		bool might_have_observers() const { return !observers_.empty(); }

	private:
		friend class internal::WeakLinkNode<ObserverList>;

		// Compacts list of observers by removing those marked for removal.
		void Compact() {
			// Detach whenever the last iterator is destroyed. Detaching is safe because
			// Compact() is only ever called when the last iterator is destroyed.

			EraseIf(observers_, [](const auto& o) { return o.IsMarkedForRemoval(); });
		}

		std::vector<ObserverStorageType> observers_;

		LinkedList<internal::WeakLinkNode<ObserverList>> live_iterators_;

		const ObserverListPolicy policy_;

		DISALLOW_COPY_AND_ASSIGN(ObserverList);
	};

	template <class ObserverType, bool check_empty = false>
	using ReentrantObserverList = ObserverList<ObserverType, check_empty, true>;

}