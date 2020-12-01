#include "include\path.h"
#include <assert.h>

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

	bool EndWith(const std::string& input, const std::string& end) {
		size_t endLen = end.length();
		size_t inputLen = input.length();

		if (inputLen < endLen) return false;

		std::string inputEnd = input.substr(inputLen - endLen, endLen);
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

	bool BeginWith(const std::string& input, const std::string& begin)
	{
		size_t beginLen = begin.length();
		size_t inputLen = input.length();

		if (inputLen < beginLen) return false;

		std::string inputBegin = input.substr(0, beginLen);
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

	std::string& AppendBackslash(std::string& input) {
		if (EndWith(input, "\\") || EndWith(input, "/"))
		{
			return input;
		}

		input.append("\\");

		return input;
	}

	std::wstring FilePathJoin(const std::wstring& left, const std::wstring& right)
	{
		std::wstring newPath = left;
		AppendBackslash(newPath);
		newPath += right;
		return newPath;
	}

	std::string FilePathJoin(const std::string& left, const std::string& right)
	{
		std::string newPath = left;
		AppendBackslash(newPath);
		newPath += right;
		return newPath;
	}

	bool GetResourceFromModule(HMODULE module,
		int resource_id,
		LPCTSTR resource_type,
		void** data,
		size_t* length) {
		if (!module)
			return false;

		if (!IS_INTRESOURCE(resource_id)) {
			assert(false);
			return false;
		}

		HRSRC hres_info = FindResource(module, MAKEINTRESOURCE(resource_id),
			resource_type);
		if (NULL == hres_info)
			return false;

		DWORD data_size = SizeofResource(module, hres_info);
		HGLOBAL hres = LoadResource(module, hres_info);
		if (!hres)
			return false;

		void* resource = LockResource(hres);
		if (!resource)
			return false;

		*data = resource;
		*length = static_cast<size_t>(data_size);
		return true;
	}

	bool GetDataResourceFromModule(HMODULE module,
		int resource_id,
		void** data,
		size_t* length) {
		return GetResourceFromModule(module, resource_id, L"BINDATA", data, length);
	}

	// FileVersionInfo

	namespace {

		struct LanguageAndCodePage {
			WORD language;
			WORD code_page;
		};

		// Returns the \VarFileInfo\Translation value extracted from the
		// VS_VERSION_INFO resource in |data|.
		LanguageAndCodePage* GetTranslate(const void* data) {
			static constexpr wchar_t kTranslation[] = L"\\VarFileInfo\\Translation";
			LPVOID translate = nullptr;
			UINT dummy_size = 0;
			if (::VerQueryValue(data, kTranslation, &translate, &dummy_size))
				return static_cast<LanguageAndCodePage*>(translate);
			return nullptr;
		}

		const VS_FIXEDFILEINFO& GetVsFixedFileInfo(const void* data) {
			static constexpr wchar_t kRoot[] = L"\\";
			LPVOID fixed_file_info = nullptr;
			UINT dummy_size = 0;
			assert(::VerQueryValue(data, kRoot, &fixed_file_info, &dummy_size));
			return *static_cast<VS_FIXEDFILEINFO*>(fixed_file_info);
		}

	}  // namespace

	FileVersionInfo::FileVersionInfo(std::vector<uint8_t>&& data,
		WORD language,
		WORD code_page)
		: owned_data_(std::move(data)),
		data_(owned_data_.data()),
		language_(language),
		code_page_(code_page),
		fixed_file_info_(GetVsFixedFileInfo(data_)) {
		assert(!owned_data_.empty());
	}

	FileVersionInfo::FileVersionInfo(void* data,
		WORD language,
		WORD code_page)
		: data_(data),
		language_(language),
		code_page_(code_page),
		fixed_file_info_(GetVsFixedFileInfo(data)) {
		assert(data_);
	}

	// static
	std::unique_ptr<FileVersionInfo> 
		FileVersionInfo::CreateFileVersionInfo(
		const std::wstring& filepath) {
		DWORD dummy;
		const DWORD length = ::GetFileVersionInfoSize(filepath.c_str(), &dummy);
		if (length == 0)
			return nullptr;

		std::vector<uint8_t> data(length, 0);

		if (!::GetFileVersionInfo(filepath.c_str(), dummy, length, data.data()))
			return nullptr;

		const LanguageAndCodePage* translate = GetTranslate(data.data());
		if (!translate)
			return nullptr;

		return std::unique_ptr<FileVersionInfo>(new FileVersionInfo(
			std::move(data), translate->language, translate->code_page));
	}

	// static
	std::unique_ptr<FileVersionInfo>
		FileVersionInfo::CreateFileVersionInfoForModule(HMODULE module) {
		void* data;
		size_t version_info_length;
		const bool has_version_resource = GetResourceFromModule(
			module, VS_VERSION_INFO, RT_VERSION, &data, &version_info_length);
		if (!has_version_resource)
			return nullptr;

		const LanguageAndCodePage* translate = GetTranslate(data);
		if (!translate)
			return nullptr;

		return std::unique_ptr<FileVersionInfo>(
			new FileVersionInfo(data, translate->language, translate->code_page));
	}

	std::wstring FileVersionInfo::company_name() {
		return GetStringValue(L"CompanyName");
	}

	std::wstring FileVersionInfo::company_short_name() {
		return GetStringValue(L"CompanyShortName");
	}

	std::wstring FileVersionInfo::internal_name() {
		return GetStringValue(L"InternalName");
	}

	std::wstring FileVersionInfo::product_name() {
		return GetStringValue(L"ProductName");
	}

	std::wstring FileVersionInfo::product_short_name() {
		return GetStringValue(L"ProductShortName");
	}

	std::wstring FileVersionInfo::product_version() {
		return GetStringValue(L"ProductVersion");
	}

	std::wstring FileVersionInfo::file_description() {
		return GetStringValue(L"FileDescription");
	}

	std::wstring FileVersionInfo::file_version() {
		return GetStringValue(L"FileVersion");
	}

	std::wstring FileVersionInfo::original_filename() {
		return GetStringValue(L"OriginalFilename");
	}

	std::wstring FileVersionInfo::special_build() {
		return GetStringValue(L"SpecialBuild");
	}

	bool FileVersionInfo::GetValue(const wchar_t* name,
		std::wstring& value) const {
		const struct LanguageAndCodePage lang_codepages[] = {
			// Use the language and codepage from the DLL.
			{language_, code_page_},
			// Use the default language and codepage from the DLL.
			{::GetUserDefaultLangID(), code_page_},
			// Use the language from the DLL and Latin codepage (most common).
			{language_, 1252},
			// Use the default language and Latin codepage (most common).
			{::GetUserDefaultLangID(), 1252},
		};

		for (const auto& lang_codepage : lang_codepages) {
			wchar_t sub_block[MAX_PATH];
			_snwprintf_s(sub_block, MAX_PATH, MAX_PATH,
				L"\\StringFileInfo\\%04x%04x\\%ls", lang_codepage.language,
				lang_codepage.code_page, name);
			LPVOID value_ptr = nullptr;
			uint32_t size;
			BOOL r = ::VerQueryValue(data_, sub_block, &value_ptr, &size);
			if (r && value_ptr && size) {
				value.assign(static_cast<wchar_t*>(value_ptr), size - 1);
				return true;
			}
		}
		return false;
	}

	std::wstring FileVersionInfo::GetStringValue(
		const wchar_t* name) const {
		std::wstring str;
		GetValue(name, str);
		return str;
	}

	std::vector<uint32_t> FileVersionInfo::GetFileVersion() const {
		return std::vector<uint32_t>({ HIWORD(fixed_file_info_.dwFileVersionMS),
							  LOWORD(fixed_file_info_.dwFileVersionMS),
							  HIWORD(fixed_file_info_.dwFileVersionLS),
							  LOWORD(fixed_file_info_.dwFileVersionLS) });
	}
}
