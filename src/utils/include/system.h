#pragma once
#include <windows.h>

namespace utils {

	bool IsSystem64Bit();

	class WinHandle
	{
	public:
		WinHandle(HANDLE handle = NULL);
		~WinHandle();

		inline HANDLE operator()() const;
		inline WinHandle& operator = (const HANDLE& handle);

	public:
		HANDLE m_handle = NULL;
	};
}