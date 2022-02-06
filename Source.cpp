#include <Windows.h>

extern "C" __declspec(dllexport) // compile as C
BOOL APIENTRY Dllmain(HMODULE hMod, DWORD reason, LPVOID reserved) // Define entry point
{
	__debugbreak(); // Breakpoint interrupt
	switch (reason)
	{
	case DLL_PROCESS_ATTACH: // When the process is created
	case DLL_THREAD_ATTACH: // Anytime a thread is created
	case DLL_PROCESS_DETACH: // When the process is shutting down
	case DLL_THREAD_DETACH: // When a thread ends
		break;

	}

	return TRUE;
}