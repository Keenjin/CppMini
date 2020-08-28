#include "hook_dll.h"
#include "utils/hook.h"

static std::unique_ptr<HookLyric> hook_lyric;

bool HookInit()
{
	if (!hook_lyric)
	{
		hook_lyric = std::make_unique<HookLyric>();
		return hook_lyric->Init();
	}
	return true;
}

void HookUninit()
{
	if (hook_lyric)
	{
		hook_lyric->UnInit();
		hook_lyric.reset();
	}
}

unsigned WINAPI WorkThread(void * pParam)
{
	// todo
	return 0;
}

BOOL WINAPI HookUpdateLayeredWindow(
	HWND          hWnd,
	HDC           hdcDst,
	POINT         *pptDst,
	SIZE          *psize,
	HDC           hdcSrc,
	POINT         *pptSrc,
	COLORREF      crKey,
	BLENDFUNCTION *pblend,
	DWORD         dwFlags
)
{
	if (hook_lyric)
	{
		hook_lyric->Capture(hdcSrc, psize->cx, psize->cy);
		return hook_lyric->m_oldUpdateLayerdWindow(hWnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
	}
	return UpdateLayeredWindow(hWnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
}

BOOL WINAPI HookUpdateLayeredWindowIndirect(
	_In_       HWND                    hwnd,
	_In_ const UPDATELAYEREDWINDOWINFO *pULWInfo
)
{
	if (hook_lyric)
	{
		hook_lyric->Capture(pULWInfo->hdcSrc, pULWInfo->psize->cx, pULWInfo->psize->cy);
		return hook_lyric->m_oldUpdateLayerdWindowIndirect(hwnd, pULWInfo);
	}
	return UpdateLayeredWindowIndirect(hwnd, pULWInfo);
}

bool HookLyric::Init()
{
	// 共享内存
	m_hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, L"LyricShareMem");
	m_pBuffer = (BYTE*)MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 1024 * 1024);

	// Hook一下api，然后执行操作
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	bool success = true;
	success &= utils::HookFuncByName(hUser32, "UpdateLayeredWindow", HookUpdateLayeredWindow, &m_oldUpdateLayerdWindow, false);
	success &= utils::HookFuncByName(hUser32, "UpdateLayeredWindowIndirect", HookUpdateLayeredWindowIndirect, &m_oldUpdateLayerdWindowIndirect, false);
	return success;
}

void HookLyric::UnInit()
{
	if (m_pBuffer)
	{
		UnmapViewOfFile(m_pBuffer);
		m_pBuffer = nullptr;
	}

	if (m_hMap)
	{
		CloseHandle(m_hMap);
		m_hMap = nullptr;
	}
	

	utils::UnHookFunc(HookUpdateLayeredWindow, m_oldUpdateLayerdWindow);
	utils::UnHookFunc(HookUpdateLayeredWindowIndirect, m_oldUpdateLayerdWindowIndirect);
}

void HookLyric::Capture(HDC hdc, LONG cx, LONG cy)
{
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, cx, cy);
	SelectObject(hMemDC, hBitmap);

	::BitBlt(hMemDC, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

	// 往共享内存中存数据
	BITMAP bm;
	if (GetObject(hBitmap, sizeof(BITMAP), &bm))
	{
		BITMAPINFO bi = {0};
		bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
		
		if (GetDIBits(hMemDC, hBitmap, 0, 0, NULL, &bi, DIB_RGB_COLORS))
		{
			//BYTE* buffer = new BYTE[bi.bmiHeader.biSizeImage];
			bi.bmiHeader.biCompression = BI_RGB;

			LyricShareMem* lyricMem = (LyricShareMem*)m_pBuffer;
			if (/*buffer && */GetDIBits(hMemDC, hBitmap, 0, bi.bmiHeader.biHeight, (LPVOID)(lyricMem + sizeof(LyricShareMem)), &bi, DIB_RGB_COLORS))
			{
				lyricMem->bmi = bi;
				lyricMem->width = cx;
				lyricMem->height = cy;
				//memcpy(lyricMem + sizeof(LyricShareMem), buffer, bi.bmiHeader.biSizeImage);
			}

			//if (buffer) delete[] buffer;
		}
	}

	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
}

