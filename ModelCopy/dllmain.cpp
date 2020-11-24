// ModelCopy.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "stdafx.h"
#include "Helper.h"
#include <iostream>


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	auto& helper = *getHelper();

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		helper.attach();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		//helper.detach();
	}
	return TRUE;
}

