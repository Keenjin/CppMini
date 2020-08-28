#include "include/wnd.h"

namespace utils {

	HWND FindWndFirst(const WndMatchInfo& matchWnd)
	{
		/*if ((matchWnd.matchFilter & FILTER_TYPE_TITLE) && !matchWnd.title.empty())
		{
			HWND hWnd = FindWindowW(NULL, matchWnd.title.c_str());
			if (!hWnd || !IsWindow(hWnd))
			{
				return NULL;
			}
			else {
				FindWindowEx(hWnd, hChild)
			}
		}*/

		// todo
		return NULL;
	}

	HWND FindWndFuzzyNearest(const WndMatchInfo& matchWnd)
	{
		// todo
		return NULL;
	}

}