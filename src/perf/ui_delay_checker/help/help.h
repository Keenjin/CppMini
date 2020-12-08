#pragma once
#include <afxwin.h>
#include <vector>
#include <ctype.h>
#include <string>

// ������ȡ�����б�
struct ProcInfo {
	std::wstring name;
	std::wstring path;
	int32_t pid = -1;
	int32_t parrentPid = -1;
	std::vector<int32_t> tids;
};
bool EnumProcs(std::vector<ProcInfo>& procs);

struct WndInfo {
	HWND hWnd = nullptr;
	std::wstring title;
	std::wstring className;
};
bool EnumWnd(int32_t tid, std::vector<WndInfo>& wnds, bool onlyTop = true);