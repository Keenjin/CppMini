// test_inject_helper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "hook.h"
#include "process.h"

int main()
{
	std::wstring dir = utils::GetProcessDir();

	// ע���QQ���֣���ѯ���ڹ���exename=[qqmusic.exe], className=[TXGuiFoundation], style=[-1777467392]
	// ע�뵽�ṷ���֣���ѯ���ڹ���exename=[kugou.exe], className=[kugou_ui], style=[-1811939328]

	

	bool targetIs64 = utils::IsProcess64Bit()
	std::wstring utils::FilePathJoin(dir, L"");
    utils::InjectTarget()
}
