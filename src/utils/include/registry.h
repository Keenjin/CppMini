#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include "macros.h"

namespace utils {

	class RegKey {
	public:
		RegKey();
		explicit RegKey(HKEY key);
		RegKey(HKEY rootkey, const wchar_t* subkey, REGSAM access);
		~RegKey();

		LONG Create(HKEY rootkey, const wchar_t* subkey, REGSAM access);

		LONG CreateWithDisposition(HKEY rootkey,
			const wchar_t* subkey,
			DWORD* disposition,
			REGSAM access);

        // Creates a subkey or open it if it already exists.
        LONG CreateKey(const wchar_t* name, REGSAM access);

        // Opens an existing reg key.
        LONG Open(HKEY rootkey, const wchar_t* subkey, REGSAM access);

        // Opens an existing reg key, given the relative key name.
        LONG OpenKey(const wchar_t* relative_key_name, REGSAM access);

        // Closes this reg key.
        void Close();

        // Replaces the handle of the registry key and takes ownership of the handle.
        void Set(HKEY key);

        // Transfers ownership away from this object.
        HKEY Take();

        // Returns false if this key does not have the specified value, or if an error
        // occurrs while attempting to access it.
        bool HasValue(const wchar_t* value_name) const;

        // Returns the number of values for this key, or 0 if the number cannot be
        // determined.
        DWORD GetValueCount() const;

        // Determines the nth value's name.
        LONG GetValueNameAt(int index, std::wstring* name) const;

        // True while the key is valid.
        bool Valid() const { return key_ != NULL; }

        // Kills a key and everything that lives below it; please be careful when
        // using it.
        LONG DeleteKey(const wchar_t* name);

        // Deletes an empty subkey.  If the subkey has subkeys or values then this
        // will fail.
        LONG DeleteEmptyKey(const wchar_t* name);

        // Deletes a single value within the key.
        LONG DeleteValue(const wchar_t* name);

        // Getters:

        // Reads a REG_DWORD (uint32_t) into |out_value|. If |name| is null or empty,
        // reads the key's default value, if any.
        LONG ReadValueDW(const wchar_t* name, DWORD* out_value) const;

        // Reads a REG_QWORD (int64_t) into |out_value|. If |name| is null or empty,
        // reads the key's default value, if any.
        LONG ReadInt64(const wchar_t* name, int64_t* out_value) const;

        // Reads a string into |out_value|. If |name| is null or empty, reads
        // the key's default value, if any.
        LONG ReadValue(const wchar_t* name, std::wstring* out_value) const;

        // Reads a REG_MULTI_SZ registry field into a vector of strings. Clears
        // |values| initially and adds further strings to the list. Returns
        // ERROR_CANTREAD if type is not REG_MULTI_SZ.
        LONG ReadValues(const wchar_t* name, std::vector<std::wstring>* values);

        // Reads raw data into |data|. If |name| is null or empty, reads the key's
        // default value, if any.
        LONG ReadValue(const wchar_t* name,
            void* data,
            DWORD* dsize,
            DWORD* dtype) const;

        // Setters:

        // Sets an int32_t value.
        LONG WriteValue(const wchar_t* name, DWORD in_value);

        // Sets a string value.
        LONG WriteValue(const wchar_t* name, const wchar_t* in_value);

        // Sets raw data, including type.
        LONG WriteValue(const wchar_t* name,
            const void* data,
            DWORD dsize,
            DWORD dtype);

        HKEY Handle() const { return key_; }

    private:
        // Recursively deletes a key and all of its subkeys.
        static LONG RegDelRecurse(HKEY root_key, const wchar_t* name, REGSAM access);

        HKEY key_;  // The registry key being iterated.
        REGSAM wow64access_;

        DISALLOW_COPY_AND_ASSIGN(RegKey);
	};
}