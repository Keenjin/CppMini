#include "include\stringex.h"
#include "icu_utf.h"
#include <windows.h>
#include <vector>

namespace utils {

	inline bool IsValidCodepoint(uint32_t code_point) {
		// Excludes the surrogate code points ([0xD800, 0xDFFF]) and
		// codepoints larger than 0x10FFFF (the highest codepoint allowed).
		// Non-characters and unassigned codepoints are allowed.
		return code_point < 0xD800u ||
			(code_point >= 0xE000u && code_point <= 0x10FFFFu);
	}

	inline bool IsValidCharacter(uint32_t code_point) {
		// Excludes non-characters (U+FDD0..U+FDEF, and all codepoints ending in
		// 0xFFFE or 0xFFFF) from the set of valid code points.
		return code_point < 0xD800u || (code_point >= 0xE000u &&
			code_point < 0xFDD0u) || (code_point > 0xFDEFu &&
				code_point <= 0x10FFFFu && (code_point & 0xFFFEu) != 0xFFFEu);
	}

	// Assuming that a pointer is the size of a "machine word", then
	// uintptr_t is an integer type that is also a machine word.
	using MachineWord = uintptr_t;

	inline bool IsMachineWordAligned(const void* pointer) {
		return !(reinterpret_cast<MachineWord>(pointer) & (sizeof(MachineWord) - 1));
	}

#pragma warning(push)
#pragma warning(disable: 4309)
	template <typename CharacterType>
	struct NonASCIIMask;
	template <>
	struct NonASCIIMask<char> {
		static constexpr MachineWord value() {
			return static_cast<MachineWord>(0x8080808080808080ULL);
		}
	};
	template <>
	struct NonASCIIMask<wchar_t> {
		static constexpr MachineWord value() {
			return static_cast<MachineWord>(0xFFFFFF80FFFFFF80ULL);
		}
	};
#pragma warning(pop)

	template <class Char>
	inline bool DoIsStringASCII(const Char* characters, size_t length) {
		if (!length)
			return true;
		constexpr MachineWord non_ascii_bit_mask = NonASCIIMask<Char>::value();
		MachineWord all_char_bits = 0;
		const Char* end = characters + length;

		// Prologue: align the input.
		while (!IsMachineWordAligned(characters) && characters < end)
			all_char_bits |= *characters++;
		if (all_char_bits & non_ascii_bit_mask)
			return false;

		// Compare the values of CPU word size.
		constexpr size_t chars_per_word = sizeof(MachineWord) / sizeof(Char);
		constexpr int batch_count = 16;
		while (characters <= end - batch_count * chars_per_word) {
			all_char_bits = 0;
			for (int i = 0; i < batch_count; ++i) {
				all_char_bits |= *(reinterpret_cast<const MachineWord*>(characters));
				characters += chars_per_word;
			}
			if (all_char_bits & non_ascii_bit_mask)
				return false;
		}

		// Process the remaining words.
		all_char_bits = 0;
		while (characters <= end - chars_per_word) {
			all_char_bits |= *(reinterpret_cast<const MachineWord*>(characters));
			characters += chars_per_word;
		}

		// Process the remaining bytes.
		while (characters < end)
			all_char_bits |= *characters++;

		return !(all_char_bits & non_ascii_bit_mask);
	}

	bool IsUtf8(const std::string& value)
	{
		int32_t len = static_cast<int32_t>(value.size());
		int32_t index = 0;
		while (index < len)
		{
			int32_t code_point;
			CBU8_NEXT(value.c_str(), index, len, code_point);
			if (!IsValidCharacter(code_point))
				return false;
		}
		return true;
	}

	bool IsAscii(const std::string& value)
	{
		return DoIsStringASCII(value.c_str(), value.size());
	}

	bool IsAscii(const std::wstring& value)
	{
		return DoIsStringASCII(value.c_str(), value.size());
	}

	// Do not assert in this function since it is used by the asssertion code!
	std::string SysWideToMultiByte(const std::wstring& wide, uint32_t code_page) {
		int wide_length = static_cast<int>(wide.length());
		if (wide_length == 0)
			return std::string();

		// Compute the length of the buffer we'll need.
		int charcount = WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
			NULL, 0, NULL, NULL);
		if (charcount == 0)
			return std::string();

		std::string mb;
		mb.resize(charcount);
		WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
			&mb[0], charcount, NULL, NULL);

		return mb;
	}

	// Do not assert in this function since it is used by the asssertion code!
	std::wstring SysMultiByteToWide(const std::string& mb, uint32_t code_page) {
		if (mb.empty())
			return std::wstring();

		int mb_length = static_cast<int>(mb.length());
		// Compute the length of the buffer.
		int charcount = MultiByteToWideChar(code_page, 0,
			mb.data(), mb_length, NULL, 0);
		if (charcount == 0)
			return std::wstring();

		std::wstring wide;
		wide.resize(charcount);
		MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

		return wide;
	}

	std::string MakeSureUtf8(const std::string& value) {
		if (IsUtf8(value))
			return value;

		return WideToUtf8(NativeMBToWide(value));
	}

	std::string WideToUtf8(const std::wstring& value)
	{
		return SysWideToMultiByte(value, CP_UTF8);
	}

	std::wstring Utf8ToWide(const std::string& value)
	{
		return SysMultiByteToWide(value, CP_UTF8);
	}

	std::string WideToNativeMB(const std::wstring& value)
	{
		return SysWideToMultiByte(value, CP_ACP);
	}

	std::wstring NativeMBToWide(const std::string& value)
	{
		return SysMultiByteToWide(value, CP_ACP);
	}

	// C++14 implementation of C++17's std::size():
	// http://en.cppreference.com/w/cpp/iterator/size
	template <typename Container>
	constexpr auto size(const Container& c) -> decltype(c.size()) {
		return c.size();
	}

	template <typename T, size_t N>
	constexpr size_t size(const T(&array)[N]) noexcept {
		return N;
	}

	inline int vsnprintf(char* buffer, size_t size,
		const char* format, va_list arguments) {
		int length = vsnprintf_s(buffer, size, size - 1, format, arguments);
		if (length < 0)
			return _vscprintf(format, arguments);
		return length;
	}

	inline int vswprintf(wchar_t* buffer, size_t size,
		const wchar_t* format, va_list arguments) {
		//DCHECK(IsWprintfFormatPortable(format));

		int length = _vsnwprintf_s(buffer, size, size - 1, format, arguments);
		if (length < 0)
			return _vscwprintf(format, arguments);
		return length;
	}

	// Overloaded wrappers around vsnprintf and vswprintf. The buf_size parameter
	// is the size of the buffer. These return the number of characters in the
	// formatted string excluding the NUL terminator. If the buffer is not
	// large enough to accommodate the formatted string without truncation, they
	// return the number of characters that would be in the fully-formatted string
	// (vsnprintf, and vswprintf on Windows), or -1 (vswprintf on POSIX platforms).
	inline int vsnprintfT(char* buffer,
		size_t buf_size,
		const char* format,
		va_list argptr) {
		return vsnprintf(buffer, buf_size, format, argptr);
	}

	inline int vsnprintfT(wchar_t* buffer,
		size_t buf_size,
		const wchar_t* format,
		va_list argptr) {
		return vswprintf(buffer, buf_size, format, argptr);
	}

	// Templatized backend for StringPrintF/StringAppendF. This does not finalize
// the va_list, the caller is expected to do that.
	template <class CharT>
	static void StringAppendVT(std::basic_string<CharT>* dst,
		const CharT* format,
		va_list ap) {
		// First try with a small fixed size buffer.
		// This buffer size should be kept in sync with StringUtilTest.GrowBoundary
		// and StringUtilTest.StringPrintfBounds.
		CharT stack_buf[1024];

		va_list ap_copy;
		va_copy(ap_copy, ap);

		//base::internal::ScopedClearLastError last_error;
		int result = vsnprintfT(stack_buf, size(stack_buf), format, ap_copy);
		va_end(ap_copy);

		if (result >= 0 && result < static_cast<int>(size(stack_buf))) {
			// It fit.
			dst->append(stack_buf, result);
			return;
		}

		// Repeatedly increase buffer size until it fits.
		int mem_length = size(stack_buf);
		while (true) {
			if (result < 0) {
#if defined(OS_WIN)
				// On Windows, vsnprintfT always returns the number of characters in a
				// fully-formatted string, so if we reach this point, something else is
				// wrong and no amount of buffer-doubling is going to fix it.
				return;
#else
				if (errno != 0 && errno != EOVERFLOW)
					return;
				// Try doubling the buffer size.
				mem_length *= 2;
#endif
			}
			else {
				// We need exactly "result + 1" characters.
				mem_length = result + 1;
			}

			if (mem_length > 32 * 1024 * 1024) {
				// That should be plenty, don't try anything larger.  This protects
				// against huge allocations when using vsnprintfT implementations that
				// return -1 for reasons other than overflow without setting errno.
				//DLOG(WARNING) << "Unable to printf the requested string due to size.";
				return;
			}

			std::vector<CharT> mem_buf(mem_length);

			// NOTE: You can only use a va_list once.  Since we're in a while loop, we
			// need to make a new copy each time so we don't use up the original.
			va_copy(ap_copy, ap);
			result = vsnprintfT(&mem_buf[0], mem_length, format, ap_copy);
			va_end(ap_copy);

			if ((result >= 0) && (result < mem_length)) {
				// It fit.
				dst->append(&mem_buf[0], result);
				return;
			}
		}
	}

	void StringAppendV(std::string* dst, const char* format, va_list ap) {
		StringAppendVT(dst, format, ap);
	}

	void StringAppendV(std::wstring* dst, const wchar_t* format, va_list ap) {
		StringAppendVT(dst, format, ap);
	}

	std::string Format(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		std::string result;
		StringAppendV(&result, format, ap);
		va_end(ap);
		return result;
	}

	std::wstring Format(const wchar_t* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		std::wstring result;
		StringAppendV(&result, format, ap);
		va_end(ap);
		return result;
	}

	template <typename Str>
	Str TrimStringT(const Str& input,
		const Str& trim_chars,
		TrimType positions) {
		// Find the edges of leading/trailing whitespace as desired. Need to use
		// a StringPiece version of input to be able to call find* on it with the
		// StringPiece version of trim_chars (normally the trim_chars will be a
		// constant so avoid making a copy).
		const size_t last_char = input.length() - 1;
		const size_t first_good_char =
			(positions & TRIM_TYPE_LEFT) ? input.find_first_not_of(trim_chars) : 0;
		const size_t last_good_char = (positions & TRIM_TYPE_RIGHT)
			? input.find_last_not_of(trim_chars)
			: last_char;

		// When the string was all trimmed, report that we stripped off characters
		// from whichever position the caller was interested in. For empty input, we
		// stripped no characters, but we still need to clear |output|.
		if (input.empty() || first_good_char == Str::npos ||
			last_good_char == Str::npos) {
			bool input_was_empty = input.empty();  // in case output == &input
			return Str();
		}

		// Trim.
		Str output;
		output.assign(input.data() + first_good_char,
			last_good_char - first_good_char + 1);

		return output;
	}

	std::string TrimString(const std::string& input, const std::string& trimChars, TrimType trimType)
	{
		return TrimStringT(input, trimChars, trimType);
	}

	std::wstring TrimString(const std::wstring& input, const std::wstring& trimChars, TrimType trimType)
	{
		return TrimStringT(input, trimChars, trimType);
	}

	template<typename Str>
	Str WhitespaceForType();

	template<> std::string WhitespaceForType<std::string>() {
		return " ";
	}
	template<> std::wstring WhitespaceForType<std::wstring>() {
		return L" ";
	}

	// General string splitter template. Can take 8- or 16-bit input, can produce
	// the corresponding string or StringPiece output.
	template <typename OutputStringType, typename DelimiterType>
	static std::vector<OutputStringType> SplitStringT(
		const OutputStringType& str,
		const DelimiterType& delimiter,
		bool trimWhitespace) {
		std::vector<OutputStringType> result;
		if (str.empty())
			return result;

		size_t start = 0;
		while (start != OutputStringType::npos) {
			size_t end = str.find_first_of(delimiter, start);

			OutputStringType piece;
			if (end == OutputStringType::npos) {
				piece = str.substr(start);
				start = OutputStringType::npos;
			}
			else {
				piece = str.substr(start, end - start);
				start = end + 1;
			}

			if (trimWhitespace)
				piece = TrimString(piece, WhitespaceForType<OutputStringType>(), TRIM_TYPE_ALL);
		}
		return result;
	}

	std::vector<std::string> Split(const std::string& input, const std::string& separator, bool trimWhitespace/* = true*/)
	{
		return SplitStringT(input, separator, trimWhitespace);
	}

	std::vector<std::string> Split(const std::string& input, char separator, bool trimWhitespace/* = true*/)
	{
		return SplitStringT(input, separator, trimWhitespace);
	}

	std::vector<std::wstring> Split(const std::wstring& input, const std::wstring& separator, bool trimWhitespace/* = true*/)
	{
		return SplitStringT(input, separator, trimWhitespace);
	}

	std::vector<std::wstring> Split(const std::wstring& input, wchar_t separator, bool trimWhitespace/* = true*/)
	{
		return SplitStringT(input, separator, trimWhitespace);
	}
}
