#include "include/registry.h"
#include <assert.h>

namespace utils {

    // Windows only overload of base::WriteInto for std::wstring. See the comment
    // above the cross-platform version in //base/strings/string_util.h for details.
    // TODO(crbug.com/911896): Rename this to WriteInto once base::string16 is
    // std::u16string on all platforms and using the name WriteInto here no longer
    // causes redefinition errors.
    inline wchar_t* WriteIntoW(std::wstring* str, size_t length_with_null) {
        // Note: As of C++11 std::strings are guaranteed to be 0-terminated. Thus it
        // is enough to reserve space for one char less.
        assert(length_with_null >= 1u);
        str->resize(length_with_null - 1);
        return &((*str)[0]);
    }

    // Mask to pull WOW64 access flags out of REGSAM access.
    const REGSAM kWow64AccessMask = KEY_WOW64_32KEY | KEY_WOW64_64KEY;

    // RegKey ----------------------------------------------------------------------

    RegKey::RegKey() : key_(NULL), wow64access_(0) {
    }

    RegKey::RegKey(HKEY key) : key_(key), wow64access_(0) {
    }

    RegKey::RegKey(HKEY rootkey, const wchar_t* subkey, REGSAM access)
        : key_(NULL), wow64access_(0) {
        if (rootkey) {
            if (access & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_CREATE_LINK))
                Create(rootkey, subkey, access);
            else
                Open(rootkey, subkey, access);
        }
        else {
            assert(!subkey);
            wow64access_ = access & kWow64AccessMask;
        }
    }

    RegKey::~RegKey() {
        Close();
    }

    LONG RegKey::Create(HKEY rootkey, const wchar_t* subkey, REGSAM access) {
        DWORD disposition_value;
        return CreateWithDisposition(rootkey, subkey, &disposition_value, access);
    }

    LONG RegKey::CreateWithDisposition(HKEY rootkey,
        const wchar_t* subkey,
        DWORD* disposition,
        REGSAM access) {
        assert(rootkey && subkey && access && disposition);
        HKEY subhkey = NULL;
        LONG result =
            RegCreateKeyEx(rootkey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, access,
                NULL, &subhkey, disposition);
        if (result == ERROR_SUCCESS) {
            Close();
            key_ = subhkey;
            wow64access_ = access & kWow64AccessMask;
        }

        return result;
    }

    LONG RegKey::CreateKey(const wchar_t* name, REGSAM access) {
        assert(name && access);
        // After the application has accessed an alternate registry view using one of
        // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
        // (create, delete, or open) on child registry keys must explicitly use the
        // same flag. Otherwise, there can be unexpected behavior.
        // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
        if ((access & kWow64AccessMask) != wow64access_) {
            assert(false);
            return ERROR_INVALID_PARAMETER;
        }
        HKEY subkey = NULL;
        LONG result = RegCreateKeyEx(key_, name, 0, NULL, REG_OPTION_NON_VOLATILE,
            access, NULL, &subkey, NULL);
        if (result == ERROR_SUCCESS) {
            Close();
            key_ = subkey;
            wow64access_ = access & kWow64AccessMask;
        }

        return result;
    }

    LONG RegKey::Open(HKEY rootkey, const wchar_t* subkey, REGSAM access) {
        assert(rootkey && subkey && access);
        HKEY subhkey = NULL;

        LONG result = RegOpenKeyEx(rootkey, subkey, 0, access, &subhkey);
        if (result == ERROR_SUCCESS) {
            Close();
            key_ = subhkey;
            wow64access_ = access & kWow64AccessMask;
        }

        return result;
    }

    LONG RegKey::OpenKey(const wchar_t* relative_key_name, REGSAM access) {
        assert(relative_key_name && access);
        // After the application has accessed an alternate registry view using one of
        // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
        // (create, delete, or open) on child registry keys must explicitly use the
        // same flag. Otherwise, there can be unexpected behavior.
        // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
        if ((access & kWow64AccessMask) != wow64access_) {
            assert(false);
            return ERROR_INVALID_PARAMETER;
        }
        HKEY subkey = NULL;
        LONG result = RegOpenKeyEx(key_, relative_key_name, 0, access, &subkey);

        // We have to close the current opened key before replacing it with the new
        // one.
        if (result == ERROR_SUCCESS) {
            Close();
            key_ = subkey;
            wow64access_ = access & kWow64AccessMask;
        }
        return result;
    }

    void RegKey::Close() {
        if (key_) {
            ::RegCloseKey(key_);
            key_ = NULL;
            wow64access_ = 0;
        }
    }

    // TODO(wfh): Remove this and other unsafe methods. See http://crbug.com/375400
    void RegKey::Set(HKEY key) {
        if (key_ != key) {
            Close();
            key_ = key;
        }
    }

    HKEY RegKey::Take() {
        assert(wow64access_ == 0u);
        HKEY key = key_;
        key_ = NULL;
        return key;
    }

    bool RegKey::HasValue(const wchar_t* name) const {
        return RegQueryValueEx(key_, name, 0, NULL, NULL, NULL) == ERROR_SUCCESS;
    }

    DWORD RegKey::GetValueCount() const {
        DWORD count = 0;
        LONG result = RegQueryInfoKey(key_, NULL, 0, NULL, NULL, NULL, NULL, &count,
            NULL, NULL, NULL, NULL);
        return (result == ERROR_SUCCESS) ? count : 0;
    }

    LONG RegKey::GetValueNameAt(int index, std::wstring* name) const {
        wchar_t buf[256];
        DWORD bufsize = std::size(buf);
        LONG r = ::RegEnumValue(key_, index, buf, &bufsize, NULL, NULL, NULL, NULL);
        if (r == ERROR_SUCCESS)
            name->assign(buf, bufsize);

        return r;
    }

    LONG RegKey::DeleteKey(const wchar_t* name) {
        assert(key_);
        assert(name);
        HKEY subkey = NULL;

        // Verify the key exists before attempting delete to replicate previous
        // behavior.
        LONG result =
            RegOpenKeyEx(key_, name, 0, READ_CONTROL | wow64access_, &subkey);
        if (result != ERROR_SUCCESS)
            return result;
        RegCloseKey(subkey);

        return RegDelRecurse(key_, name, wow64access_);
    }

    LONG RegKey::DeleteEmptyKey(const wchar_t* name) {
        assert(key_);
        assert(name);

        HKEY target_key = NULL;
        LONG result =
            RegOpenKeyEx(key_, name, 0, KEY_READ | wow64access_, &target_key);

        if (result != ERROR_SUCCESS)
            return result;

        DWORD count = 0;
        result = RegQueryInfoKey(target_key, NULL, 0, NULL, NULL, NULL, NULL, &count,
            NULL, NULL, NULL, NULL);

        RegCloseKey(target_key);

        if (result != ERROR_SUCCESS)
            return result;

        if (count == 0)
            return RegDeleteKeyEx(key_, name, wow64access_, 0);

        return ERROR_DIR_NOT_EMPTY;
    }

    LONG RegKey::DeleteValue(const wchar_t* value_name) {
        assert(key_);
        LONG result = RegDeleteValue(key_, value_name);
        return result;
    }

    LONG RegKey::ReadValueDW(const wchar_t* name, DWORD* out_value) const {
        assert(out_value);
        DWORD type = REG_DWORD;
        DWORD size = sizeof(DWORD);
        DWORD local_value = 0;
        LONG result = ReadValue(name, &local_value, &size, &type);
        if (result == ERROR_SUCCESS) {
            if ((type == REG_DWORD || type == REG_BINARY) && size == sizeof(DWORD))
                *out_value = local_value;
            else
                result = ERROR_CANTREAD;
        }

        return result;
    }

    LONG RegKey::ReadInt64(const wchar_t* name, int64_t* out_value) const {
        assert(out_value);
        DWORD type = REG_QWORD;
        int64_t local_value = 0;
        DWORD size = sizeof(local_value);
        LONG result = ReadValue(name, &local_value, &size, &type);
        if (result == ERROR_SUCCESS) {
            if ((type == REG_QWORD || type == REG_BINARY) &&
                size == sizeof(local_value))
                *out_value = local_value;
            else
                result = ERROR_CANTREAD;
        }

        return result;
    }

    LONG RegKey::ReadValue(const wchar_t* name, std::wstring* out_value) const {
        assert(out_value);
        const size_t kMaxStringLength = 1024;  // This is after expansion.
        // Use the one of the other forms of ReadValue if 1024 is too small for you.
        wchar_t raw_value[kMaxStringLength];
        DWORD type = REG_SZ, size = sizeof(raw_value);
        LONG result = ReadValue(name, raw_value, &size, &type);
        if (result == ERROR_SUCCESS) {
            if (type == REG_SZ) {
                *out_value = raw_value;
            }
            else if (type == REG_EXPAND_SZ) {
                wchar_t expanded[kMaxStringLength];
                size = ExpandEnvironmentStrings(raw_value, expanded, kMaxStringLength);
                // Success: returns the number of wchar_t's copied
                // Fail: buffer too small, returns the size required
                // Fail: other, returns 0
                if (size == 0 || size > kMaxStringLength) {
                    result = ERROR_MORE_DATA;
                }
                else {
                    *out_value = expanded;
                }
            }
            else {
                // Not a string. Oops.
                result = ERROR_CANTREAD;
            }
        }

        return result;
    }

    LONG RegKey::ReadValue(const wchar_t* name,
        void* data,
        DWORD* dsize,
        DWORD* dtype) const {
        LONG result = RegQueryValueEx(key_, name, 0, dtype,
            reinterpret_cast<LPBYTE>(data), dsize);
        return result;
    }

    LONG RegKey::ReadValues(const wchar_t* name,
        std::vector<std::wstring>* values) {
        values->clear();

        DWORD type = REG_MULTI_SZ;
        DWORD size = 0;
        LONG result = ReadValue(name, NULL, &size, &type);
        if (result != ERROR_SUCCESS || size == 0)
            return result;

        if (type != REG_MULTI_SZ)
            return ERROR_CANTREAD;

        std::vector<wchar_t> buffer(size / sizeof(wchar_t));
        result = ReadValue(name, buffer.data(), &size, NULL);
        if (result != ERROR_SUCCESS || size == 0)
            return result;

        // Parse the double-null-terminated list of strings.
        // Note: This code is paranoid to not read outside of |buf|, in the case where
        // it may not be properly terminated.
        auto entry = buffer.cbegin();
        auto buffer_end = buffer.cend();
        while (entry < buffer_end && *entry != '\0') {
            auto entry_end = std::find(entry, buffer_end, '\0');
            values->emplace_back(entry, entry_end);
            entry = entry_end + 1;
        }
        return 0;
    }

    LONG RegKey::WriteValue(const wchar_t* name, DWORD in_value) {
        return WriteValue(
            name, &in_value, static_cast<DWORD>(sizeof(in_value)), REG_DWORD);
    }

    LONG RegKey::WriteValue(const wchar_t* name, const wchar_t* in_value) {
        return WriteValue(
            name, in_value,
            static_cast<DWORD>(sizeof(*in_value) *
                (std::char_traits<wchar_t>::length(in_value) + 1)),
            REG_SZ);
    }

    LONG RegKey::WriteValue(const wchar_t* name,
        const void* data,
        DWORD dsize,
        DWORD dtype) {
        assert(data || !dsize);

        LONG result =
            RegSetValueEx(key_, name, 0, dtype,
                reinterpret_cast<LPBYTE>(const_cast<void*>(data)), dsize);
        return result;
    }

    // static
    LONG RegKey::RegDelRecurse(HKEY root_key, const wchar_t* name, REGSAM access) {
        // First, see if the key can be deleted without having to recurse.
        LONG result = RegDeleteKeyEx(root_key, name, access, 0);
        if (result == ERROR_SUCCESS)
            return result;

        HKEY target_key = NULL;
        result = RegOpenKeyEx(root_key, name, 0, KEY_ENUMERATE_SUB_KEYS | access,
            &target_key);

        if (result == ERROR_FILE_NOT_FOUND)
            return ERROR_SUCCESS;
        if (result != ERROR_SUCCESS)
            return result;

        std::wstring subkey_name(name);

        // Check for an ending slash and add one if it is missing.
        if (!subkey_name.empty() && subkey_name.back() != '\\')
            subkey_name.push_back('\\');

        // Enumerate the keys
        result = ERROR_SUCCESS;
        const DWORD kMaxKeyNameLength = MAX_PATH;
        const size_t base_key_length = subkey_name.length();
        std::wstring key_name;
        while (result == ERROR_SUCCESS) {
            DWORD key_size = kMaxKeyNameLength;
            result =
                RegEnumKeyEx(target_key, 0, WriteIntoW(&key_name, kMaxKeyNameLength),
                    &key_size, NULL, NULL, NULL, NULL);

            if (result != ERROR_SUCCESS)
                break;

            key_name.resize(key_size);
            subkey_name.resize(base_key_length);
            subkey_name += key_name;

            if (RegDelRecurse(root_key, subkey_name.c_str(), access) != ERROR_SUCCESS)
                break;
        }

        RegCloseKey(target_key);

        // Try again to delete the key.
        result = RegDeleteKeyEx(root_key, name, access, 0);

        return result;
    }
}