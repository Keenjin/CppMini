// dllmain.cpp : Defines the entry point for the DLL application.
#include "hook_dll.h"
#include <process.h>
#include <string>

static HINSTANCE dll_inst = NULL;
static HANDLE work_thread = NULL;

// 全局标记位，去重判断
const wchar_t* mutex_id = L"{F3BDF46B-712A-43BD-8B07-D6DDA7B3AD1E}/";
static HANDLE mutex_handle = nullptr;

bool IsProc64Bit()
{
#ifdef _WIN64
	return true;
#else
	BOOL x86 = false;
	bool success = !!IsWow64Process(GetCurrentProcess(), &x86);
	return success && !x86;
#endif
}

bool IsInjectHelper()
{
	wchar_t name[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, name, MAX_PATH);
	std::wstring wname = name;

	// 排除掉inject_helper32.exe和inject_helper64.exe
	size_t pos = wname.find_last_of(L"inject_helper");
	if (pos == -1)
	{
		return false;
	}

	std::wstring injectHelper = L"inject_helper32.exe";
	if (IsProc64Bit())
	{
		injectHelper = L"inject_helper64.exe";
	}

	if (wname.substr(wname.length() - injectHelper.length()).compare(injectHelper) == 0)
	{
		return true;
	}

	return false;
}

bool IsInjected()
{
	DWORD pid = GetCurrentProcessId();
	wchar_t mutexName[50] = { 0 };
	wsprintf(mutexName, L"%s%lu", mutex_id, pid);
	HANDLE hMutex = OpenMutex(SYNCHRONIZE, FALSE, mutexName);
	if (hMutex)
	{
		CloseHandle(hMutex);
		return true;
	}

	mutex_handle = CreateMutex(NULL, FALSE, mutexName);
	return !mutex_handle;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		dll_inst = hModule;

		if (IsInjectHelper())
		{
			return TRUE;
		}

		OutputDebugString(L"hookLyric");

		if (IsInjected())
		{
			// 已经注入过了，就不要重复注入
			return FALSE;
		}

		MessageBox(NULL, L"hookLyric", L"", NULL);

		if (!HookInit())
		{
			return FALSE;
		}

		wchar_t name[MAX_PATH] = { 0 };
		/* this prevents the library from being automatically unloaded
		 * by the next FreeLibrary call */
		GetModuleFileNameW(hModule, name, MAX_PATH);
		LoadLibraryW(name);

		// 启动新线程
		unsigned workTID = 0;
		work_thread = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &workTID);
		if (!work_thread)
		{
			return FALSE;
		}

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		if (!mutex_handle) return TRUE;

		HookUninit();

		if (mutex_handle)
		{
			CloseHandle(mutex_handle);
			mutex_handle = nullptr;
		}
		break;
	}
	}
	return TRUE;
}


// 给SetWindowsHookEx使用的
extern "C" __declspec(dllexport) LRESULT CALLBACK dummy_debug_proc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(0, code, wParam, lParam);
}
