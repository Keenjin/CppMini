#pragma once
#include <string>
#include <memory>
#include <vector>
#include "macros.h"
#include <windows.h>

struct tagVS_FIXEDFILEINFO;
typedef tagVS_FIXEDFILEINFO VS_FIXEDFILEINFO;

namespace utils {

	bool IsPathExist(const std::wstring& path);
	
	std::wstring FileDir(const std::wstring& filepath);
	std::wstring FileName(const std::wstring& filepath);
	std::wstring FileBaseName(const std::wstring& filepath);
	std::wstring FileExtension(const std::wstring& filepath);

	bool EndWith(const std::wstring& input, const std::wstring& end);
	bool EndWith(const std::string& input, const std::string& end);
	bool BeginWith(const std::wstring& input, const std::wstring& begin);
	bool BeginWith(const std::string& input, const std::string& begin);

	std::wstring& AppendBackslash(std::wstring& input);
	std::string& AppendBackslash(std::string& input);
	std::wstring FilePathJoin(const std::wstring& left, const std::wstring& right);
	std::string FilePathJoin(const std::string& left, const std::string& right);

    // Function for getting a data resource of the specified |resource_type| from
    // a dll.  Some resources are optional, especially in unit tests, so this
    // returns false but doesn't raise an error if the resource can't be loaded.
    bool GetResourceFromModule(HMODULE module,
        int resource_id,
        LPCTSTR resource_type,
        void** data,
        size_t* length);

    // Function for getting a data resource (BINDATA) from a dll.  Some
    // resources are optional, especially in unit tests, so this returns false
    // but doesn't raise an error if the resource can't be loaded.
    bool GetDataResourceFromModule(HMODULE module,
        int resource_id,
        void** data,
        size_t* length);

	class FileVersionInfo {
	public:

        // Creates a FileVersionInfo for the specified path. Returns nullptr if
        // something goes wrong (typically the file does not exit or cannot be
        // opened).
		static std::unique_ptr<FileVersionInfo> CreateFileVersionInfo(
			const std::wstring& filepath
		);

        // Creates a FileVersionInfo for the specified module. Returns nullptr in
        // case of error.
        static std::unique_ptr<FileVersionInfo> CreateFileVersionInfoForModule(
            HMODULE module);

        // Accessors to the different version properties.
        // Returns an empty string if the property is not found.
        std::wstring company_name();
        std::wstring company_short_name();
        std::wstring product_name();
        std::wstring product_short_name();
        std::wstring internal_name();
        std::wstring product_version();
        std::wstring special_build();
        std::wstring original_filename();
        std::wstring file_description();
        std::wstring file_version();

        // Lets you access other properties not covered above. |value| is only
        // modified if GetValue() returns true.
        bool GetValue(const wchar_t* name, std::wstring& value) const;

        // Similar to GetValue but returns a string16 (empty string if the property
        // does not exist).
        std::wstring GetStringValue(const wchar_t* name) const;

        // Get file version number in dotted version format.
        std::vector<uint32_t> GetFileVersion() const;

    private:

        FileVersionInfo(std::vector<uint8_t>&& data,
            WORD language,
            WORD code_page);
        FileVersionInfo(void* data,
            WORD language,
            WORD code_page);

        const std::vector<uint8_t> owned_data_;
        const void* const data_;
        const WORD language_;
        const WORD code_page_;

        // This is a reference for a portion of |data_|.
        const VS_FIXEDFILEINFO& fixed_file_info_;

        DISALLOW_COPY_AND_ASSIGN(FileVersionInfo);
	};
}
