#include "system.h"

namespace utils {

	bool IsSystem64Bit()
	{
#ifdef _WIN64
		return true;
#else
		BOOL x86 = false;
		bool success = !!IsWow64Process(GetCurrentProcess(), &x86);
		return success && !!x86;
#endif
	}

	WinHandle::WinHandle(HANDLE handle/* = NULL*/)
		: m_handle(handle)
	{

	}

	WinHandle::~WinHandle()
	{
		if (m_handle)
		{
			CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	inline HANDLE WinHandle::operator()() const
	{
		return m_handle;
	}

	inline WinHandle& WinHandle::operator = (const HANDLE& handle)
	{
		m_handle = handle;
		return *this;
	}

	HWND FindWndFirst(const WndMatchInfo& matchWnd)
	{
		/*if ((matchWnd.matchFilter & FILTER_TYPE_TITLE) && !matchWnd.title.empty())
		{
			HWND hWnd = FindWindowW(NULL, matchWnd.title.c_str());
			if (!hWnd || !IsWindow(hWnd))
			{
				return NULL;
			}
			else {
				FindWindowEx(hWnd, hChild)
			}
		}*/

		// todo
		return NULL;
	}

	HWND FindWndFuzzyNearest(const WndMatchInfo& matchWnd)
	{
		// todo
		return NULL;
	}
}
