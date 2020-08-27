#pragma once

#include <string>
#include <Windows.h>

namespace utils {
	// idIsTid表示注入线程。
	bool InjectTarget(const std::wstring& injectExe, const std::wstring& hookDll, uint32_t id, bool idIsTid = true);

	// 通过名字Hook
	bool HookFuncByName(HMODULE hModule, const std::string& funcName, void* newFuncAddr, void* oldFuncAddr = nullptr, bool handAllThreads = true);

	// 通过模糊匹配代码搜索Hook，比较复杂，后面实现
	bool HookFuncByCode(HMODULE hModule, const byte* codeBuffer, size_t codeBufferLen, void* newFuncAddr, void* oldFuncAddr, bool handAllThreads = true);

	bool UnHookFunc(void* newFuncAddr, void* oldFuncAddr);
}
