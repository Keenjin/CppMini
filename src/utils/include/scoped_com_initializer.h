#pragma once
#include <windows.h>
#include "macros.h"

namespace utils {
	// Serves as a root class for ScopedCOMInitializer and ScopedWinrtInitializer.
	class ScopedWindowsThreadEnvironment {
	public:
		ScopedWindowsThreadEnvironment() {}
		virtual ~ScopedWindowsThreadEnvironment() {}

		virtual bool Succeeded() const = 0;

	private:
		DISALLOW_COPY_AND_ASSIGN(ScopedWindowsThreadEnvironment);
	};

	// Initializes COM in the constructor (STA or MTA), and uninitializes COM in the
	// destructor.
	//
	// WARNING: This should only be used once per thread, ideally scoped to a
	// similar lifetime as the thread itself.  You should not be using this in
	// random utility functions that make COM calls -- instead ensure these
	// functions are running on a COM-supporting thread!
	class ScopedCOMInitializer : public ScopedWindowsThreadEnvironment {
	public:
		// Enum value provided to initialize the thread as an MTA instead of STA.
		enum SelectMTA { kMTA };

		// Constructor for STA initialization.
		ScopedCOMInitializer();

		// Constructor for MTA initialization.
		explicit ScopedCOMInitializer(SelectMTA mta);

		~ScopedCOMInitializer() override;

		// ScopedWindowsThreadEnvironment:
		bool Succeeded() const override;

	private:
		void Initialize(COINIT init);

		HRESULT hr_;

		DISALLOW_COPY_AND_ASSIGN(ScopedCOMInitializer);
	};
}