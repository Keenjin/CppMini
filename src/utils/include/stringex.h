#pragma once
#include <string>
#include <tuple>
#include <vector>

namespace utils {

	bool IsUtf8(const std::string& value);
	bool IsAscii(const std::string& value);
	bool IsAscii(const std::wstring& value);

	std::string WideToUtf8(const std::wstring& value);
	std::wstring Utf8ToWide(const std::string& value);
	std::string WideToNativeMB(const std::wstring& value);
	std::wstring NativeMBToWide(const std::string& value);

	std::string Format(const char* format, ...);
	std::wstring Format(const wchar_t* format, ...);

	enum TrimType {
		TRIM_TYPE_NONE = 0,
		TRIM_TYPE_LEFT = 1,
		TRIM_TYPE_RIGHT = TRIM_TYPE_LEFT << 1,
		TRIM_TYPE_ALL = TRIM_TYPE_LEFT | TRIM_TYPE_RIGHT
	};
	std::string TrimString(const std::string& input, const std::string& trimChars, TrimType trimType);
	std::wstring TrimString(const std::wstring& input, const std::wstring& trimChars, TrimType trimType);

	std::vector<std::string> Split(const std::string& input, const std::string& separator, bool trimWhitespace = true);
	std::vector<std::string> Split(const std::string& input, char separator, bool trimWhitespace = true);
	std::vector<std::wstring> Split(const std::wstring& input, const std::wstring& separator, bool trimWhitespace = true);
	std::vector<std::wstring> Split(const std::wstring& input, wchar_t separator, bool trimWhitespace = true);
}
