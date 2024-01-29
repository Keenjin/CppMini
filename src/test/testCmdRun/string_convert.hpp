#pragma once
#include <string>
#include <map>
#include <codecvt>
#include <system_error>
#include <cuchar>
#include <iostream>

namespace string_convert {
	// 自定义错误码
	enum class string_convert_error : int {
		ok,
		partial,
		error,
		noconv
	};

	class string_convert_error_category : public std::error_category {
	public:
		const char* name() const noexcept override {
			return "string_convert";
		}

		std::string message(int ev) const override {
			switch (static_cast<string_convert_error>(ev)) {
			case string_convert_error::ok:
				return "string convert sucess.";
			case string_convert_error::partial:
				return "only partial string convert finish";
			case string_convert_error::error:
				return "string convert runtime error";
			case string_convert_error::noconv:
				return "no convert because not supportted";
			default:
				return "unknown error";
			}
		}
	};

	static const string_convert_error_category ERROR_CATEGORY;

	std::error_code make_error_code(string_convert_error err) {
		return { static_cast<int>(err), ERROR_CATEGORY };
	}

	// 字符集
	enum class Charset : int {
		ASCII,
		C,
		GB2312,
		GBK,
		GB18030,
		BIG5,
		UTF8,
		UTF16,
		UTF32
	};

	static std::map<Charset, std::string> CHARSET_AND_NAMES = {
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WIN)
		/***************************************************
		 *	Reference:
		 *		http://msdn.microsoft.com/en-us/library/dd317756(VS.85).aspx
		 *
		 ****************************************************/
		{ Charset::ASCII, "" },					//	ASCII(1250)
		{ Charset::C, "C" },					//	C(0)
		{ Charset::GB2312, ".20936" },			//	GB2312
		{ Charset::GBK, ".936" },				//	GBK
		{ Charset::GB18030, ".54936" },			//	GB18030
		{ Charset::BIG5, ".950" },				//	BIG5
		{ Charset::UTF8, ".65001" },			//	UTF8
		{ Charset::UTF16, ".1200" },			//	UTF16 Little-endian(X86)
		{ Charset::UTF32, ".12000" },			//	UTF32 Little-endian(X86)
#else
		/***************************************************
		 *	Reference:
		 *		http://www.iana.org/assignments/character-sets
		 *		http://stdcxx.apache.org/doc/stdlibref/codecvt-byname.html
		 *		http://gcc.gnu.org/onlinedocs/libstdc++/manual/codecvt.html
		 ****************************************************/
		{ Charset::ASCII, "" },					//	ASCII
		{ Charset::C, "C" },					//	C
		{ Charset::GB2312, "zh_CN.GB2312" },	//	GB2312
		{ Charset::GBK, "zh_CN.GBK" },			//	GBK
		{ Charset::GB18030, "zh_CN.GB18030" },	//	GB18030
		{ Charset::BIG5, "zh_TW.BIG5" },		//	BIG5
		{ Charset::UTF8, "zh_CN.UTF-8" },		//	UTF8
		{ Charset::UTF16, "zh_CN.UTF-16LE" },	//	UTF16 Little-endian(X86)
		{ Charset::UTF32, "zh_CN.UTF-32LE" },	//	UTF32 Little-endian(X86)
#endif
	};

	static std::locale make_locale(const Charset code) {
		try {
#if defined(_WIN32) || defined(_WIN64) || defined(OS_WIN)
			switch (code) {
			case Charset::UTF8:
				return std::locale(std::locale::classic(), new std::codecvt<char8_t, char8_t, std::mbstate_t>);
			case Charset::UTF16:
				return std::locale(std::locale::classic(), new std::codecvt<char16_t, char8_t, std::mbstate_t>);
			case Charset::UTF32:
				return std::locale(std::locale::classic(), new std::codecvt<char32_t, char8_t, std::mbstate_t>);
			default:
				return std::locale(CHARSET_AND_NAMES[code]);
			}
#else
			return std::locale(CHARSET_AND_NAMES[code]);
#endif
		}
		catch (std::runtime_error& err) {
			std::cout << err.what() << std::endl;
			throw err;
		}
	}

	const static std::locale locale_ascii = make_locale(Charset::ASCII);
	const static std::locale locale_c = make_locale(Charset::C);
	const static std::locale locale_gb2312 = make_locale(Charset::GB2312);
	const static std::locale locale_gbk = make_locale(Charset::GBK);
	const static std::locale locale_big5 = make_locale(Charset::BIG5);
	const static std::locale locale_utf8 = make_locale(Charset::UTF8);
	const static std::locale locale_utf16 = make_locale(Charset::UTF16);
	const static std::locale locale_utf32 = make_locale(Charset::UTF32);

#if defined(_WIN32) || defined(_WIN64) || defined(OS_WIN)
	// Utf16 as Wide
	std::wstring Utf16AsWide(const std::u16string& src) {
		return std::wstring(src.data(), src.data() + src.size());
	}

	// Wide as Utf16
	std::u16string WideAsUtf16(const std::wstring& src) {
		return std::u16string(src.data(), src.data() + src.size());
	}
#else
	// Utf32 as Wide
	std::wstring Utf32AsWide(const std::u32string& src) {
		return std::wstring(src.data(), src.data() + src.size());
	}

	// Wide as Utf32
	std::u32string WideAsUtf32(const std::wstring& src) {
		return std::u32string(src.data(), src.data() + src.size());
	}
#endif

	// Utf8 as Narrow
	std::string Utf8AsNarrow(const std::u8string& src) {
		return std::string(src.data(), src.data() + src.size());
	}

	// Narrow as Utf8
	std::u8string NarrowAsUtf8(const std::string& src) {
		return std::u8string(src.data(), src.data() + src.size());
	}

	// Wide to Narrow
	std::tuple<std::string, std::error_code> WideToNarrow(const std::wstring& src, const std::locale& loc = locale_c) {
		const std::codecvt<wchar_t, char, std::mbstate_t>& cvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		std::string out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const wchar_t* from_next = nullptr;
		char* to_next = nullptr;
		auto result = cvt.out(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::string(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Narrow to Wide
	std::tuple<std::wstring, std::error_code> NarrowToWide(const std::string& src, const std::locale& loc = locale_c) {
		const std::codecvt<wchar_t, char, std::mbstate_t>& cvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		std::wstring out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const char* from_next = nullptr;
		wchar_t* to_next = nullptr;
		auto result = cvt.in(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::wstring(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Utf8 to Utf16
	std::tuple<std::u16string, std::error_code> Utf8ToUtf16(const std::u8string& src, const std::locale& loc = locale_utf16) {
		const std::codecvt<char16_t, char8_t, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char16_t, char8_t, std::mbstate_t>>(loc);
		std::u16string out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const char8_t* from_next = nullptr;
		char16_t* to_next = nullptr;
		auto result = cvt.in(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::u16string(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Utf16 to Utf8
	std::tuple<std::u8string, std::error_code> Utf16ToUtf8(const std::u16string& src, const std::locale& loc = locale_utf16) {
		const std::codecvt<char16_t, char8_t, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char16_t, char8_t, std::mbstate_t>>(loc);
		std::u8string out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const char16_t* from_next = nullptr;
		char8_t* to_next = nullptr;
		auto result = cvt.out(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::u8string(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Utf32 to Utf8
	std::tuple<std::u8string, std::error_code> Utf32ToUtf8(const std::u32string& src, const std::locale& loc = locale_utf32) {
		const std::codecvt<char32_t, char8_t, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char32_t, char8_t, std::mbstate_t>>(loc);
		std::u8string out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const char32_t* from_next = nullptr;
		char8_t* to_next = nullptr;
		auto result = cvt.out(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::u8string(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Utf8 to Utf32
	std::tuple<std::u32string, std::error_code> Utf8ToUtf32(const std::u8string& src, const std::locale& loc = locale_utf32) {
		const std::codecvt<char32_t, char8_t, std::mbstate_t>& cvt = std::use_facet<std::codecvt<char32_t, char8_t, std::mbstate_t>>(loc);
		std::u32string out(cvt.max_length() * (src.size() + 1), 0);

		std::mbstate_t state;
		const char8_t* from_next = nullptr;
		char32_t* to_next = nullptr;
		auto result = cvt.in(state, src.data(), src.data() + src.size(), from_next, out.data(), out.data() + out.size(), to_next);
		return { std::u32string(out.data(), to_next), make_error_code(static_cast<string_convert_error>(result)) };
	}

	// Utf16 to Utf32
	std::tuple<std::u32string, std::error_code> Utf16ToUtf32(const std::u16string& src, const std::locale& loc = locale_utf32) {
		// Utf16 to Utf8
		auto [temp, err] = Utf16ToUtf8(src);
		if (err) return { U"", err };

		// Utf8 to Utf32
		return Utf8ToUtf32(temp, loc);
	}

	// Utf32 to Utf16
	std::tuple<std::u16string, std::error_code> Utf32ToUtf16(const std::u32string& src, const std::locale& loc = locale_utf32) {
		// Utf32 to Utf8
		auto [temp, err] = Utf32ToUtf8(src);
		if (err) return { u"", err };

		// Utf8 to Utf16
		return Utf8ToUtf16(temp, loc);
	}

	// GBK to Utf8
	std::tuple<std::u8string, std::error_code> GBKToUtf8(const std::u16string& src, const std::locale& loc = locale_gbk) {
		return Utf16ToUtf8(src, loc);
	}

	// Utf8 to GBK
	std::tuple<std::u16string, std::error_code> Utf8ToGBK(const std::u8string& src, const std::locale& loc = locale_gbk) {
		return Utf8ToUtf16(src, loc);
	}

#if defined(_WIN32) || defined(_WIN64) || defined(OS_WIN)
	// GBK to Utf8
	std::tuple<std::string, std::error_code> GBKToUtf8(const std::wstring& src, const std::locale& loc = locale_gbk) {
		auto [ret, err] = Utf16ToUtf8(WideAsUtf16(src), loc);
		if (err) return { "", err};

		return { Utf8AsNarrow(ret), err };
	}

	// Utf8 to GBK
	std::tuple<std::wstring, std::error_code> Utf8ToGBK(const std::string& src, const std::locale& loc = locale_gbk) {
		auto [ret, err] = Utf8ToUtf16(NarrowAsUtf8(src), loc);
		if (err) return { L"", err };

		return { Utf16AsWide(ret), err };
	}
#else
	// 待校验
	// GBK to Utf8
	std::tuple<std::string, std::error_code> GBKToUtf8(const std::wstring& src, const std::locale& loc = locale_gbk) {
		auto [ret, err] = Utf32ToUtf8(WideAsUtf32(src), loc);
		if (err) return { "", err };

		return { Utf8AsNarrow(ret), err };
	}

	// Utf8 to GBK
	std::tuple<std::wstring, std::error_code> Utf8ToGBK(const std::string& src, const std::locale& loc = locale_gbk) {
		auto [ret, err] = Utf8ToUtf32(NarrowAsUtf8(src), loc);
		if (err) return { L"", err };

		return { Utf32AsWide(ret), err };
	}
#endif
};
