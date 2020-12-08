#pragma once
#include <windows.h>
#include <tuple>

class PerfChecker {
public:
	PerfChecker(HWND hWnd, DWORD hungTimeout = 6000);
	~PerfChecker();

	bool IsWinHung();
	bool IsWinCaton(DWORD dwMillSecond);

private:
	std::tuple<bool, bool> isHungAppWindow(HWND hWnd);

	HMODULE m_hUser32 = nullptr;
	HWND m_hWnd = nullptr;
	DWORD m_hungTimeout = 6000;
};