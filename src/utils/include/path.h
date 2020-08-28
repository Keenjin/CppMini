#pragma once
#include <string>

namespace utils {

	bool IsPathExist(const std::wstring& path);
	
	std::wstring FileDir(const std::wstring& filepath);
	std::wstring FileName(const std::wstring& filepath);
	std::wstring FileBaseName(const std::wstring& filepath);
	std::wstring FileExtension(const std::wstring& filepath);

	bool EndWith(const std::wstring& input, const std::wstring& end);
	bool BeginWith(const std::wstring& input, const std::wstring& begin);

	std::wstring& AppendBackslash(std::wstring& input);
	std::wstring FilePathJoin(const std::wstring& left, const std::wstring& right);
}
