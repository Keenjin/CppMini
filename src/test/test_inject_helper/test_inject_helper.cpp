// test_inject_helper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "hook.h"
#include "process.h"

int main()
{
	std::wstring dir = utils::GetProcessDir();

	// 注入的QQ音乐，查询窗口规则：exename=[qqmusic.exe], className=[TXGuiFoundation], style=[-1777467392]
	// 注入到酷狗音乐，查询窗口规则：exename=[kugou.exe], className=[kugou_ui], style=[-1811939328]

	

	bool targetIs64 = utils::IsProcess64Bit()
	std::wstring utils::FilePathJoin(dir, L"");
    utils::InjectTarget()
}
