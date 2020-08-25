#include "path.h"
#include <windows.h>

namespace utils {

	bool IsPathExist(const std::wstring& path)
	{
		return GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	std::wstring FileDir(const std::wstring& filepath)
	{
		size_t pos1 = filepath.find_last_of(L'\\');
		size_t pos2 = filepath.find_last_of(L'/');

		size_t pos = -1;
		if (pos1 != -1) pos = (pos2 != -1 && pos2 > pos1) ? pos2 : pos1;
		else pos = pos2;

		return (pos > 0) ? filepath.substr(0, pos) : filepath;
	}

	std::wstring FileName(const std::wstring& filepath)
	{
		size_t pos1 = filepath.find_last_of(L'\\');
		size_t pos2 = filepath.find_last_of(L'/');

		size_t pos = -1;
		if (pos1 != -1) pos = (pos2 != -1 && pos2 > pos1) ? pos2 : pos1;
		else pos = pos2;

		return (pos < (filepath.length()-1)) ? filepath.substr(pos + 1, filepath.length() - pos) : filepath;
	}

	std::wstring FileBaseName(const std::wstring& filepath)
	{
		std::wstring name = FileName(filepath);

		size_t pos = name.find_last_of(L'.');
		return pos > 0 ? name.substr(0, pos) : name;
	}

	std::wstring FileExtension(const std::wstring& filepath)
	{
		std::wstring name = FileName(filepath);

		size_t pos = name.find_last_of(L'.');
		return (pos > 0 && pos < name.length() - 1) ? name.substr(pos, filepath.length() - pos + 1) : L"";
	}

	bool EndWith(const std::wstring& input, const std::wstring& end)
	{
		size_t endLen = end.length();
		size_t inputLen = input.length();

		if (inputLen < endLen) return false;
		
		std::wstring inputEnd = input.substr(inputLen - endLen, endLen);
		return inputEnd.compare(end) == 0;
	}

	bool BeginWith(const std::wstring& input, const std::wstring& begin)
	{
		size_t beginLen = begin.length();
		size_t inputLen = input.length();

		if (inputLen < beginLen) return false;

		std::wstring inputBegin = input.substr(0, beginLen);
		return inputBegin.compare(begin) == 0;
	}

	std::wstring& AppendBackslash(std::wstring& input)
	{
		if (EndWith(input, L"\\") || EndWith(input, L"/"))
		{
			return input;
		}

		input.append(L"\\");

		return input;
	}

	std::wstring FilePathJoin(const std::wstring& left, const std::wstring& right)
	{
		std::wstring newPath = left;
		AppendBackslash(newPath);
		newPath += right;
		return newPath;
	}
}
