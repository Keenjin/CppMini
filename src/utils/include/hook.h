#pragma once

#include <string>

namespace utils {
	// idIsTid表示注入线程。
	bool InjectTarget(const std::wstring& injectExe, const std::wstring& hookDll, uint32_t id, bool idIsTid = true);
}