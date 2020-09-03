#pragma once
#include <windows.h>
#include "macros.h"
#include "system.h"
#include <assert.h>

namespace utils {
	// A WaitableEvent can be a useful thread synchronization tool when you want to
	// allow one thread to wait for another thread to finish some work. For
	// non-Windows systems, this can only be used from within a single address
	// space.
	//
	// Use a WaitableEvent when you would otherwise use a Lock+ConditionVariable to
	// protect a simple boolean value.  However, if you find yourself using a
	// WaitableEvent in conjunction with a Lock to wait for a more complex state
	// change (e.g., for an item to be added to a queue), then you should probably
	// be using a ConditionVariable instead of a WaitableEvent.
	//
	// NOTE: On Windows, this class provides a subset of the functionality afforded
	// by a Windows event object.  This is intentional.  If you are writing Windows
	// specific code and you need other features of a Windows event, then you might
	// be better off just using an Windows event directly.
	class WaitableEvent {
	public:
		// Indicates whether a WaitableEvent should automatically reset the event
		// state after a single waiting thread has been released or remain signaled
		// until Reset() is manually invoked.
		enum class ResetPolicy { MANUAL, AUTOMATIC };

		// Indicates whether a new WaitableEvent should start in a signaled state or
		// not.
		enum class InitialState { SIGNALED, NOT_SIGNALED };

		// Constructs a WaitableEvent with policy and initial state as detailed in
		// the above enums.
		WaitableEvent(ResetPolicy reset_policy = ResetPolicy::MANUAL,
			InitialState initial_state = InitialState::NOT_SIGNALED);

		// Create a WaitableEvent from an Event HANDLE which has already been
		// created. This objects takes ownership of the HANDLE and will close it when
		// deleted.
		explicit WaitableEvent(WinHandle event_handle);

		~WaitableEvent();

		// Put the event in the un-signaled state.
		void Reset();

		// Put the event in the signaled state.  Causing any thread blocked on Wait
		// to be woken up.
		void Signal();

		// Returns true if the event is in the signaled state, else false.  If this
		// is not a manual reset event, then this test will cause a reset.
		bool IsSignaled();

		// Wait indefinitely for the event to be signaled. Wait's return "happens
		// after" |Signal| has completed. This means that it's safe for a
		// WaitableEvent to synchronise its own destruction, like this:
		//
		//   WaitableEvent *e = new WaitableEvent;
		//   SendToOtherThread(e);
		//   e->Wait();
		//   delete e;
		void Wait();

		// Wait up until wait_delta has passed for the event to be signaled
		// (real-time; ignores time overrides).  Returns true if the event was
		// signaled. Handles spurious wakeups and guarantees that |wait_delta| will
		// have elapsed if this returns false.
		//
		// TimedWait can synchronise its own destruction like |Wait|.
		bool TimedWait(const uint32_t& wait_delta);

		HANDLE handle() const { return handle_; }

		// Declares that this WaitableEvent will only ever be used by a thread that is
		// idle at the bottom of its stack and waiting for work (in particular, it is
		// not synchronously waiting on this event before resuming ongoing work). This
		// is useful to avoid telling base-internals that this thread is "blocked"
		// when it's merely idle and ready to do work. As such, this is only expected
		// to be used by thread and thread pool impls.
		void declare_only_used_while_idle() { waiting_is_blocking_ = false; }

		// Wait, synchronously, on multiple events.
		//   waitables: an array of WaitableEvent pointers
		//   count: the number of elements in @waitables
		//
		// returns: the index of a WaitableEvent which has been signaled.
		//
		// You MUST NOT delete any of the WaitableEvent objects while this wait is
		// happening, however WaitMany's return "happens after" the |Signal| call
		// that caused it has completed, like |Wait|.
		//
		// If more than one WaitableEvent is signaled to unblock WaitMany, the lowest
		// index among them is returned.
		static size_t WaitMany(WaitableEvent** waitables, size_t count);

	private:

		WinHandle handle_;

		// Whether a thread invoking Wait() on this WaitableEvent should be considered
		// blocked as opposed to idle (and potentially replaced if part of a pool).
		bool waiting_is_blocking_ = true;

		DISALLOW_COPY_AND_ASSIGN(WaitableEvent);
	};
}