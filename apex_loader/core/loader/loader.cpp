#include "loader.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>

// todo:replace this with your dll
#define TARGET_DLL_ADDRESS L"C:\\Users\\nickm\\source\\repos\\r6\\x64\\Release\\r6_client.dll"
//#define TARGET_DLL_ADDRESS L"C:\\Users\\nickm\\Desktop\\r6_client.dll"

typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
	PVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
}MANUAL_INJECT, * PMANUAL_INJECT;

DWORD WINAPI LoadDll(PVOID p)
{
	PMANUAL_INJECT ManualInject;

	HMODULE hModule;
	DWORD64 i, Function, count, delta;

	DWORD64* ptr;
	PWORD list;

	PIMAGE_BASE_RELOCATION pIBR;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME pIBN;
	PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

	PDLL_MAIN EntryPoint;

	ManualInject = (PMANUAL_INJECT)p;

	pIBR = ManualInject->BaseRelocation;
	delta = (DWORD64)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

																										  // Relocate the image

	while (pIBR->VirtualAddress)
	{
		if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
		{
			count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			list = (PWORD)(pIBR + 1);

			for (i = 0; i < count; i++)
			{
				if (list[i])
				{
					ptr = (DWORD64*)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
					*ptr += delta;
				}
			}
		}

		pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
	}

	pIID = ManualInject->ImportDirectory;

	// Resolve DLL imports

	while (pIID->Characteristics)
	{
		OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
		FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

		hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

		if (!hModule)
		{
			return FALSE;
		}

		while (OrigFirstThunk->u1.AddressOfData)
		{
			if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				// Import by ordinal

				Function = (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			else
			{
				// Import by name

				pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
				Function = (DWORD64)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

				if (!Function)
				{
					return FALSE;
				}

				FirstThunk->u1.Function = Function;
			}

			OrigFirstThunk++;
			FirstThunk++;
		}

		pIID++;
	}

	if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
		return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
	}

	return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
	return 0;
}


#pragma comment(lib,"ntdll.lib")

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

UCHAR code[] = {
  0x48, 0xB8, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // mov -16 to rax
  0x48, 0x21, 0xC4,                                             // and rsp, rax
  0x48, 0x83, 0xEC, 0x20,                                       // subtract 32 from rsp
  0x48, 0x8b, 0xEC,                                             // mov rbp, rsp
  0x90, 0x90,                                                   // nop nop
  0x48, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,   // mov rcx,CCCCCCCCCCCCCCCC
  0x48, 0xB8, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,   // mov rax,AAAAAAAAAAAAAAAA
  0xFF, 0xD0,                                                   // call rax
  0x90,                                                         // nop
  0x90,                                                         // nop
  0xEB, 0xFC                                                    // JMP to nop
};

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

namespace loader {
	bool load_dll(const wchar_t* process_name, const wchar_t* dll_path) {
		LPBYTE ptr;
		HANDLE hProcess, hThread, hSnap, hFile;
		PVOID mem, mem1;
		DWORD ProcessId, FileSize, read, i;
		PVOID buffer, image;
		BOOLEAN bl;
		PIMAGE_DOS_HEADER pIDH;
		PIMAGE_NT_HEADERS pINH;
		PIMAGE_SECTION_HEADER pISH;
		THREADENTRY32 te32;
		CONTEXT ctx;
		MANUAL_INJECT ManualInject;

		te32.dwSize = sizeof(te32);
		ctx.ContextFlags = CONTEXT_FULL;
		hFile = CreateFile(dll_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); // Open the DLL

		if (hFile == INVALID_HANDLE_VALUE) {
			printf("\nError: Unable to open the DLL (%d)\n", GetLastError());
			return false;
		}

		FileSize = GetFileSize(hFile, NULL);
		buffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!buffer) {
			printf("\nError: Unable to allocate memory for DLL data (%d)\n", GetLastError());

			CloseHandle(hFile);
			return false;
		}

		if (!ReadFile(hFile, buffer, FileSize, &read, NULL)) {
			printf("\nError: Unable to read the DLL (%d)\n", GetLastError());

			VirtualFree(buffer, 0, MEM_RELEASE);
			CloseHandle(hFile);

			return false;
		}

		CloseHandle(hFile);

		pIDH = (PIMAGE_DOS_HEADER)buffer;
		if (pIDH->e_magic != IMAGE_DOS_SIGNATURE) {
			printf("\nError: Invalid executable image.\n");

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		pINH = (PIMAGE_NT_HEADERS)((LPBYTE)buffer + pIDH->e_lfanew);
		if (pINH->Signature != IMAGE_NT_SIGNATURE) {
			printf("\nError: Invalid PE header.\n");

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		if (!(pINH->FileHeader.Characteristics & IMAGE_FILE_DLL)) {
			printf("\nError: The image is not DLL.\n");

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		RtlAdjustPrivilege(20, TRUE, FALSE, &bl);

		ProcessId = FindProcessId(process_name);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
		if (!hProcess) {
			printf("\nError: Unable to open target process handle (%d)\n", GetLastError());
			return false;
		}

		image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the DLL
		if (!image) {
			printf("\nError: Unable to allocate memory for the DLL (%d)\n", GetLastError());

			VirtualFree(buffer, 0, MEM_RELEASE);
			CloseHandle(hProcess);

			return false;
		}

		if (!WriteProcessMemory(hProcess, image, buffer, pINH->OptionalHeader.SizeOfHeaders, NULL)) {
			printf("\nError: Unable to copy headers to target process (%d)\n", GetLastError());

			VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
			CloseHandle(hProcess);

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);
		for (i = 0; i < pINH->FileHeader.NumberOfSections; i++) {
			WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)buffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
		}
		mem1 = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code

		if (!mem1) {
			printf("\nError: Unable to allocate memory for the loader code (%d)\n", GetLastError());

			VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
			CloseHandle(hProcess);

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		memset(&ManualInject, 0, sizeof(MANUAL_INJECT));

		ManualInject.ImageBase = image;
		ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
		ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		ManualInject.fnLoadLibraryA = LoadLibraryA;
		ManualInject.fnGetProcAddress = GetProcAddress;

		if (!WriteProcessMemory(hProcess, mem1, &ManualInject, sizeof(MANUAL_INJECT), NULL))
			std::cout << "Error " << std::hex << GetLastError() << std::endl;

		if (!WriteProcessMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem1 + 1), LoadDll, 4096 - sizeof(MANUAL_INJECT), NULL))
			std::cout << "Error " << std::hex << GetLastError() << std::endl;
		hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		Thread32First(hSnap, &te32);
		while (Thread32Next(hSnap, &te32)) {
			if (te32.th32OwnerProcessID == ProcessId) {
				break;
			}
		}

		CloseHandle(hSnap);

		mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!mem) {
			printf("\nError: Unable to allocate memory in target process (%d)", GetLastError());

			CloseHandle(hProcess);
			return false;
		}

		hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
		if (!hThread) {
			printf("\nError: Unable to open target thread handle (%d)\n", GetLastError());

			VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		SuspendThread(hThread);
		GetThreadContext(hThread, &ctx);

		buffer = VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		ptr = (LPBYTE)buffer;
		ZeroMemory(buffer, 65536);
		memcpy(buffer, code, sizeof(code));

		for (BYTE* ptr = (LPBYTE)buffer; ptr < ((LPBYTE)buffer + 300); ptr++) {
			DWORD64 address = *(DWORD64*)ptr;
			if (address == 0xCCCCCCCCCCCCCCCC) {
				*(DWORD64*)ptr = (DWORD64)mem1;
			}

			if (address == 0xAAAAAAAAAAAAAAAA) {
				*(DWORD64*)ptr = (DWORD64)((PMANUAL_INJECT)mem1 + 1);
			}
		}

		if (!WriteProcessMemory(hProcess, mem, buffer, sizeof(code), NULL)) {
			printf("\nError: Unable to write shellcode into target process (%d)\n", GetLastError());

			VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
			ResumeThread(hThread);

			CloseHandle(hThread);
			CloseHandle(hProcess);

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		ctx.Rip = (DWORD64)mem;
		if (!SetThreadContext(hThread, &ctx)) {
			printf("\nError: Unable to hijack target thread (%d)\n", GetLastError());

			VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
			ResumeThread(hThread);

			CloseHandle(hThread);
			CloseHandle(hProcess);

			VirtualFree(buffer, 0, MEM_RELEASE);
			return false;
		}

		ResumeThread(hThread);
		CloseHandle(hThread);
		CloseHandle(hProcess);
		VirtualFree(buffer, 0, MEM_RELEASE);
	}
}
