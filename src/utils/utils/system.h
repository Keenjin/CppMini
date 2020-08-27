#pragma once
#include <windows.h>
#include <string>

namespace utils {

	bool IsSystem64Bit();

	class WinHandle
	{
	public:
		WinHandle(HANDLE handle = NULL);
		~WinHandle();

		operator HANDLE() const;
		WinHandle& operator = (const HANDLE& handle);
		bool Invalid() const;
		void Close();

	private:
		HANDLE m_handle = NULL;
	};

	enum MatchFilterType {
		FILTER_TYPE_PROCESSNAME = 1,
		FILTER_TYPE_TITLE = FILTER_TYPE_PROCESSNAME << 1,
		FILTER_TYPE_CLASS = FILTER_TYPE_TITLE << 1,
		FILTER_TYPE_WIDTH = FILTER_TYPE_CLASS << 1,
		FILTER_TYPE_HEIGHT = FILTER_TYPE_WIDTH << 1,
		FILTER_TYPE_STYLE = FILTER_TYPE_HEIGHT << 1,
		FILTER_TYPE_EXSTYLE = FILTER_TYPE_STYLE << 1,
		FILTER_TYPE_VISIBLE = FILTER_TYPE_EXSTYLE << 1,
		FILTER_TYPE_TOP = FILTER_TYPE_VISIBLE << 1,
	};

	typedef struct _WndMatchInfo 
	{
		uint32_t matchFilter = FILTER_TYPE_VISIBLE | FILTER_TYPE_PROCESSNAME | FILTER_TYPE_TITLE | FILTER_TYPE_CLASS;

		bool mustTop = false;
		bool mustVisible = true;
		std::wstring processName;
		std::wstring title;
		std::wstring className;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t style = 0;
		uint32_t exStyle = 0;

	}WndMatchInfo, *PWndMatchInfo;

	// 严格匹配，查询第一个命中
	HWND FindWndFirst(const WndMatchInfo& matchWnd);

	// 找最相似的那个
	// 目前打分算法权重：（待调整）
	//		mustTop：44分
	//		mustVisible：44分
	//		processName：13分
	//		title：10分
	//		className：9分
	//		width：1分
	//		height：1分
	//		style：1分
	//		exStyle：1分
	HWND FindWndFuzzyNearest(const WndMatchInfo& matchWnd);
}