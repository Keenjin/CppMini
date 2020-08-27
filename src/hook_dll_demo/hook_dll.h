#pragma once

#include <Windows.h>

bool HookInit();
void HookUninit();

unsigned __stdcall WorkThread(void * pParam);