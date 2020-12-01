#include "include\hook.h"
#include "include\proc.h"
#include "include\path.h"
#include "include\stringex.h"
#include "detour/detours.h"
#include "include\system.h"
#include <TlHelp32.h>
#include <set>
#include <tuple>

namespace utils {
	bool InjectTarget(const std::wstring& injectExe, const std::wstring& hookDll, uint32_t id, bool idIsTid)
	{
		if (!IsPathExist(injectExe) || !IsPathExist(hookDll))
		{
			return false;
		}

		return StartProcess(injectExe, Format(L"\"%s\" \"%s\" %lu %lu", injectExe.c_str(), hookDll.c_str(), idIsTid ? 1 : 0, id));
	}

	// 如果只是hang当前线程，不需要返回句柄，否则，需要返回
	bool DetourUpdateThreads(bool hangAllThread, std::vector<WinHandle>& closeDeffer)
	{
		if (!hangAllThread)
		{
			return NO_ERROR == DetourUpdateThread(GetCurrentThread());
		}

		bool success = false;

		int loopTime = 10;
		WinHandle hSnap;
		std::set<DWORD> tids;
		do 
		{
			hSnap.Close();
			hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (hSnap.Invalid())
			{
				success = false;
				break;
			}

			success = true;

			DWORD dwPID = GetCurrentProcessId();
			DWORD dwTID = GetCurrentThreadId();

			bool havNew = false;
			THREADENTRY32 te = { 0 };
			te.dwSize = sizeof(te);
			BOOL ret = Thread32First(hSnap, &te);
			while (ret)
			{
				// 遍历线程，依次执行
				auto iter = tids.find(te.th32ThreadID);
				if (te.th32OwnerProcessID == dwPID &&
					te.th32ThreadID == dwTID &&
					iter == tids.end())
				{
					havNew = true;
					tids.insert(te.th32ThreadID);
					HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
					if (hThread)
					{
						closeDeffer.emplace_back(hThread);
						if (NO_ERROR != DetourUpdateThread(hThread))
						{
							success = false;
							break;
						}
					}
				}
				ret = Thread32Next(hSnap, &te);
			}

			if (!success) break;
			if (!havNew) break;

			--loopTime;

		} while (loopTime > 0);

		return success;
	}

	bool DetourAttachFunc(void** ppPointer, void* pDetour, bool hangAllThread)
	{
		LONG errorCode = DetourTransactionBegin();
		if (errorCode != NO_ERROR) return false;

		bool success = false;
		do 
		{
			std::vector<WinHandle> closeDeffer;
			success = DetourUpdateThreads(hangAllThread, closeDeffer);
			if (!success) break;

			errorCode = DetourAttach(ppPointer, pDetour);
			if (errorCode != NO_ERROR) break;

			success = true;

		} while (false);

		DetourTransactionCommit();
		return success;
	}

	bool DetourDetachFunc(void** ppPointer, void* pDetour)
	{
		LONG errorCode = DetourTransactionBegin();
		if (errorCode != NO_ERROR) return false;

		bool success = false;
		do 
		{
			errorCode = DetourUpdateThread(GetCurrentThread());
			if (errorCode != NO_ERROR) break;

			errorCode = DetourDetach(ppPointer, pDetour);
			if (errorCode != NO_ERROR) break;

			success = true;

		} while (false);

		DetourTransactionCommit();
		return success;
	}

	bool HookFuncByName(HMODULE hModule, const std::string& funcName, void* newFuncAddr, void* oldFuncAddr, bool handAllThreads)
	{
		if (!hModule || funcName.empty() || !newFuncAddr) return false;

		void* dstFunc = GetProcAddress(hModule, funcName.c_str());
		if (!dstFunc) return false;

		bool ret = DetourAttachFunc(&dstFunc, newFuncAddr, handAllThreads);
		if (oldFuncAddr) *(DWORD*)oldFuncAddr = (DWORD)dstFunc;
		return ret;
	}

	// 通过模糊匹配代码搜索Hook，比较复杂，后面实现
	bool HookFuncByCode(HMODULE hModule, const byte* codeBuffer, size_t codeBufferLen, void* newFuncAddr, void* oldFuncAddr, bool handAllThreads)
	{
		if (newFuncAddr == nullptr) return false;

		// todo
		return false;
	}

	bool UnHookFunc(void* newFuncAddr, void* oldFuncAddr)
	{
		if (!newFuncAddr || !oldFuncAddr) return false;

		return DetourDetachFunc(&oldFuncAddr, newFuncAddr);
	}
}
