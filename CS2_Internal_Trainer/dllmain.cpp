#include <Windows.h>
#include "CheatManager.h"
#include "GamePointers.h"
#include "ConsoleCheatMenu.h"

DWORD WINAPI MainThread(HMODULE hModule)
{
	if (!(GamePointers::ArePointersInit() && CheatManager::InitHook()))
	{
		ConsoleCheatMenu::PrintErrorPtrInit(GamePointers::pointersState);

		while (!(GetAsyncKeyState(VK_DELETE) & 1)) Sleep(5);
	}

	ConsoleCheatMenu::DestroyConsole();
	FreeLibraryAndExitThread(hModule, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		HANDLE hMainThread{ CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&MainThread, hModule, NULL, nullptr) };

		if (hMainThread) CloseHandle(hMainThread);
	}

	return TRUE;
}