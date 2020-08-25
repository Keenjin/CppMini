#pragma once

#include <string>
#include <tuple>

namespace utils {

	// �򿪽���
	bool StartProcess(const std::wstring& exePath, const std::wstring& commandline, bool wait = false);
	bool StartProcess(const std::string& exePath, const std::string& commandline, bool wait = false);

	// �ж�Ŀ����̣�����ֵfirst��ʾ�Ƿ�64��second��ʾ�Ƿ�����쳣
	std::tuple<bool, bool> IsProcess64Bit(uint32_t processId);

	std::wstring GetProcessPath();
	std::wstring GetProcessName();
	std::wstring GetProcessDir();
}
