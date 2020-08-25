#include "hook.h"
#include "process.h"
#include "path.h"
#include "stringex.h"

namespace utils {
	bool InjectTarget(const std::wstring& injectExe, const std::wstring& hookDll, uint32_t id, bool idIsTid)
	{
		if (!IsPathExist(injectExe) || !IsPathExist(hookDll))
		{
			return false;
		}

		return StartProcess(injectExe, Format(L"\"%s\" \"%s\" %lu %lu", injectExe.c_str(), hookDll.c_str(), idIsTid ? 1 : 0, id));
	}
}