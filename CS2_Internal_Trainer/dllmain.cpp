#include "Windows.h"
#include "CheatManager.h"
#include "GamePointers.h"
#include "ConsoleManager.h"

DWORD WINAPI MainThread(HMODULE hModule)
{
	GamePointers gamePointers;

	// Initialize game pointers used by the cheat
	gamePointers.InitializePointers();

	if (gamePointers.GetArePointersInit())
	{
		const bool bInGamePtrInit{ CheatManager::Start() };

		gamePointers.Set_bPointersInit(bInGamePtrInit);
	}

	// Pointer initialization error
	if (!gamePointers.GetArePointersInit())
	{
		ConsoleManager::PrintErrorPtrInit(gamePointers.GetPointersState());

		while (!GetAsyncKeyState(VK_DELETE) & 1) Sleep(5);
	}

	ConsoleManager::DestroyConsole();

	FreeLibraryAndExitThread(hModule, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		HANDLE hMainThread{ CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&MainThread, hModule, NULL, nullptr) };

		if (hMainThread)
			CloseHandle(hMainThread);
	}

	return TRUE;
}