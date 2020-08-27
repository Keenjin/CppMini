#pragma once

#include <Windows.h>

bool HookInit();
void HookUninit();

unsigned __stdcall WorkThread(void * pParam);

typedef BOOL(WINAPI *FuncUpdateLayeredWindow)(
	HWND          hWnd,
	HDC           hdcDst,
	POINT         *pptDst,
	SIZE          *psize,
	HDC           hdcSrc,
	POINT         *pptSrc,
	COLORREF      crKey,
	BLENDFUNCTION *pblend,
	DWORD         dwFlags
	);

typedef BOOL(WINAPI *FuncUpdateLayeredWindowIndirect)(
	_In_       HWND                    hwnd,
	_In_ const UPDATELAYEREDWINDOWINFO *pULWInfo
	);

typedef struct _LyricShareMem {
	BITMAPINFO bmi = { 0 };
	int width = 0;
	int height = 0;
}LyricShareMem;

class HookLyric
{
public:
	bool Init();
	void UnInit();

	void Capture(HDC hdc, LONG cx, LONG cy);

public:
	FuncUpdateLayeredWindow m_oldUpdateLayerdWindow = nullptr;
	FuncUpdateLayeredWindowIndirect m_oldUpdateLayerdWindowIndirect = nullptr;

	HANDLE m_hMap = nullptr;
	BYTE* m_pBuffer = nullptr;
};