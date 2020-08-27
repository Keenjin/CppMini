// test_inject_helper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "hook.h"
#include "proc.h"
#include <Windows.h>
#include <assert.h>
#include "path.h"
#include <string>

HWND InputLyricWnd()
{
	std::cout << "===============================================================" << std::endl;
	std::cout << "Use Spy++ to Capture a Lyric Wnd, then type input a Wnd Handle." << std::endl;
	std::cout << "Wnd Handle(exp: 110aec9): ";
	std::string wnd;
	std::cin >> wnd;
	char* str;
	HWND hWnd = (HWND)strtol(wnd.c_str(), &str, 16);
	if (!IsWindow(hWnd))
	{
		std::cout << "[error] Invalid Wnd..." << std::endl;
		return InputLyricWnd();
	}
	return hWnd;
}

int main()
{
	std::wstring dir = utils::GetProcessDir();

	// 注入的QQ音乐，查询窗口规则：exename=[qqmusic.exe], className=[TXGuiFoundation], style=[-1777467392]
	// 注入到酷狗音乐，查询窗口规则：exename=[kugou.exe], className=[kugou_ui], style=[-1811939328]

	while (1)
	{
		HWND hQQMusic = InputLyricWnd();

		DWORD pid = 0;
		DWORD tid = GetWindowThreadProcessId(hQQMusic, &pid);

		auto targetIs64 = utils::IsProcess64Bit(pid);
		std::wstring injectHelperExe = L"inject_helper32.exe";
		std::wstring hookDllExe = L"hookLyric32.dll";
		if (std::get<0>(targetIs64))
		{
			injectHelperExe = L"inject_helper64.exe";
			hookDllExe = L"hookLyric64.dll";
		}

		std::wstring injectHelper = utils::FilePathJoin(dir, injectHelperExe);
		std::wstring hookDll = utils::FilePathJoin(dir, hookDllExe);
		bool success = utils::InjectTarget(injectHelper, hookDll, tid);

		if (success) std::cout << "inject success." << std::endl;
		else std::cout << "inject failed." << std::endl;
	}
}
