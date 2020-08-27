#pragma once

#include <string>
#include <Windows.h>

namespace utils {
	// idIsTid��ʾע���̡߳�
	bool InjectTarget(const std::wstring& injectExe, const std::wstring& hookDll, uint32_t id, bool idIsTid = true);

	// ͨ������Hook
	bool HookFuncByName(HMODULE hModule, const std::string& funcName, void* newFuncAddr, void* oldFuncAddr = nullptr, bool handAllThreads = true);

	// ͨ��ģ��ƥ���������Hook���Ƚϸ��ӣ�����ʵ��
	bool HookFuncByCode(HMODULE hModule, const byte* codeBuffer, size_t codeBufferLen, void* newFuncAddr, void* oldFuncAddr, bool handAllThreads = true);

	bool UnHookFunc(void* newFuncAddr, void* oldFuncAddr);
}
