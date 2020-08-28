#include "include\proc.h"
#include <windows.h>
#include "include\system.h"
#include "include\stringex.h"
#include "include\path.h"

namespace utils {

	bool StartProcess(const std::wstring& exePath, const std::wstring& commandline, bool wait/* = false*/)
	{
		if (exePath.empty() && commandline.empty()) return false;

		PROCESS_INFORMATION pi = { 0 };
		STARTUPINFO si = { 0 };
		bool ret = !!CreateProcessW(exePath.c_str(), (LPWSTR)commandline.c_str(), NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		if (ret)
		{
			if (wait) WaitForSingleObject(pi.hProcess, INFINITE);
			if (pi.hThread) CloseHandle(pi.hThread);
			if (pi.hProcess) CloseHandle(pi.hProcess);
		}
		return ret;
	}

	bool StartProcess(const std::string& exePath, const std::string& commandline, bool wait/* = false*/)
	{		
		if (exePath.empty() && commandline.empty()) return false;

		std::wstring wExePath = IsUtf8(exePath) ? Utf8ToWide(exePath) : NativeMBToWide(exePath);
		std::wstring wCommandLine = IsUtf8(commandline) ? Utf8ToWide(commandline) : NativeMBToWide(commandline);
		if (!exePath.empty() && wExePath.empty()) return false;
		if (!commandline.empty() && wCommandLine.empty()) return false;

		return StartProcess(wExePath, wCommandLine, wait);
	}

	std::tuple<bool, bool> IsProcess64Bit(uint32_t processId)
	{
		WinHandle hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, false, processId);
		if (!hProcess)
		{
			return std::make_tuple(false, false);
		}

		BOOL x86 = TRUE;
		if (IsSystem64Bit())
		{
			bool success = !!IsWow64Process(hProcess, &x86);
			if (!success)
			{
				return std::make_tuple(false, false);
			}
		}

		return std::make_tuple(!x86, true);
	}

	std::wstring GetProcessPath()
	{
		std::wstring proc;

		WCHAR szProc[MAX_PATH + 1] = { 0 };
		GetModuleFileNameW(NULL, szProc, MAX_PATH);
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			WCHAR szProcMore[4096] = { 0 };
			GetModuleFileNameW(NULL, szProcMore, 4095);
			if (GetLastError() == ERROR_SUCCESS)
				proc = szProcMore;
		}
		else if (GetLastError() == ERROR_SUCCESS)
			proc = szProc;

		return proc;
	}

	std::wstring GetProcessName()
	{
		std::wstring procPath = GetProcessPath();
		if (procPath.empty()) return procPath;

		return FileName(procPath);
	}

	std::wstring GetProcessDir()
	{
		std::wstring procPath = GetProcessPath();
		if (procPath.empty()) return procPath;

		return FileDir(procPath);
	}
}
