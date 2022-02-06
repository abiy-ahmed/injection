#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>

HMODULE GetModuleHandleAEx(HANDLE Process, PSTR ModuleName)
{
	HMODULE mods[0x1000];
	DWORD needed;
	CHAR mod_path[MAX_PATH];
	PCHAR mod_name;

	if (EnumProcessModules(Process, mods, sizeof(mods), &needed)) { // Returns an array of HMODULEs
		for (int ii = 0; ii < needed / sizeof(mods[0]); ii++) {
			if (GetModuleFileNameExA(Process, mods[ii], mod_path, sizeof(mod_path))) { // Ask for the name of the module at the address
				mod_name = strrchr((char*)mod_path, '\\');
				if (_stricmp(mod_name + 1, ModuleName) == 0) { // See if the name of the module is the same as the passed ModuleName argument
					return mods[ii];
				}
			}
			else {
				printf("Failed to query module name: %d\n", GetLastError());
			}
		}
	}
	else {
		printf("Failed to enumerate modules: %d\n", GetLastError());
	}
	return NULL; // Module was not found
}

int main(void)
{
	DWORD pid = 4232;
	HANDLE process = NULL;
	ULONG_PTR ll_offset; //LoadLibrary offset
	HMODULE remote_k32 = NULL;
	PVOID remote_buffer = NULL;
	CHAR dllname[MAX_PATH] = "C:\\Users\\User\\source\\repos\\InjectionFun\\x64\\Debug\\InjectionFun.dll";
	HANDLE remote_thread = NULL;

	process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
	if (!process) {
		printf("Failed to open process: %d\n", GetLastError());
		return -1;
	}

	ll_offset = (ULONG_PTR)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA") - (ULONG_PTR)GetModuleHandleA("kernel32.dll"); // Calculates how far into kernel32 the process address is

	remote_k32 = GetModuleHandleAEx(process, (PSTR)"kernel32.dll");
	if (!remote_k32) {
		printf("Failed to find module: %d\n", GetLastError());
		CloseHandle(process);
		return -1;
	}
	
	remote_buffer = VirtualAllocEx(process, NULL, sizeof(dllname), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!remote_buffer) {
		printf("Failed to allocate remote buffer: %d\n", GetLastError());
		CloseHandle(process);
		return -1;
	}

	WriteProcessMemory(process, remote_buffer, dllname, sizeof(dllname), NULL);

	remote_thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)(ll_offset + remote_k32), remote_buffer, 0, NULL); // The basic unit of execution in Windows is a thread, so we need a thread to execute
	if (!remote_thread) {
		printf("Failed to create remote thread: %d\n", GetLastError());
		VirtualFreeEx(process, remote_buffer, 0, MEM_RELEASE);
		CloseHandle(process);
		return -1;
	}
	
	WaitForSingleObject(remote_thread, INFINITE);
	return 0;
}