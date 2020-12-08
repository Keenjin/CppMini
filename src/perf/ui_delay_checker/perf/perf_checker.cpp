#include "perf_checker.h"

typedef BOOL(WINAPI *FIsHungAppWindow)(HWND hWnd);

PerfChecker::PerfChecker(HWND hWnd, DWORD hungTimeout)
	: m_hWnd(hWnd)
	, m_hungTimeout(hungTimeout) {
}

PerfChecker::~PerfChecker() {
	if (m_hUser32) {
		FreeLibrary(m_hUser32);
		m_hUser32 = nullptr;
	}
}

bool PerfChecker::IsWinHung() {
	// 优先使用IsHungAppWindow
	if (!IsWindow(m_hWnd)) return false;

	//auto ret = isHungAppWindow(m_hWnd);
	//if (!std::get<0>(ret)) {
		// 不支持的函数，采用SendMessageTimeout，等6s没有反应，就认为卡死
		return IsWinCaton(6000);
	//}
	//return std::get<1>(ret);
}

std::tuple<bool, bool> PerfChecker::isHungAppWindow(HWND hWnd) {
	if (!m_hUser32) {
		m_hUser32 = LoadLibrary(L"User32.dll");
		if (!m_hUser32) return std::make_tuple(false, false);
	}

	FIsHungAppWindow fp = (FIsHungAppWindow)GetProcAddress(m_hUser32, "IsHungAppWindow");
	if (!fp) return std::make_tuple(false, false);

	return std::make_tuple(true, fp(hWnd));
}

bool PerfChecker::IsWinCaton(DWORD dwMillSecond) {
	if (!IsWindow(m_hWnd)) return false;

	LRESULT hr = SendMessageTimeout(m_hWnd, WM_NULL, 0, 0, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, dwMillSecond, NULL);
	if (hr == 0) {
		return ERROR_TIMEOUT == GetLastError();
	}
	return false;
}