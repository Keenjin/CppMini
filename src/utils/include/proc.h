#pragma once

#include <string>
#include <tuple>

namespace utils {

	// 打开进程
	bool StartProcess(const std::wstring& exePath, const std::wstring& commandline, bool wait = false);
	bool StartProcess(const std::string& exePath, const std::string& commandline, bool wait = false);

	// 判断目标进程，返回值first表示是否64，second表示是否存在异常
	std::tuple<bool, bool> IsProcess64Bit(uint32_t processId);

	std::wstring GetProcessPath();
	std::wstring GetProcessName();
	std::wstring GetProcessDir();
}
