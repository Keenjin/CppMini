// hook_dll_demo.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "hook_dll_demo.h"


// This is an example of an exported variable
HOOKDLLDEMO_API int nhookdlldemo=0;

// This is an example of an exported function.
HOOKDLLDEMO_API int fnhookdlldemo(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
Chookdlldemo::Chookdlldemo()
{
    return;
}
