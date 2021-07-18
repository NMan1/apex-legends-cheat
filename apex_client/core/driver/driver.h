#pragma once
#include <Windows.h>
#include <mutex>
#include <iostream>
#include "../sdk/sdk.h"

typedef struct init_t {
	DWORD		usermode_process_id;
	DWORD		usermode_thread_id;
};

namespace driver {
	static void call_hook(init_t* argument) {
		while (!argument->usermode_thread_id) {};

		//wchar_t buffer[64]{};
		//wsprintf(buffer, L"id: %d", argument->usermode_thread_id);

		//MessageBox(NULL, buffer, L"Notify", MB_OK);

		void* control_function = GetProcAddress(LoadLibrary(L"win32u.dll"), "NtOpenCompositionSurfaceSectionInfo");
		if (!control_function) {
			MessageBox(NULL, L"failed to get control_function address", L"error", MB_OK);
			return;
		}

		auto possible_return = static_cast<uint64_t(__stdcall*)(init_t*)>(control_function)(argument);
		if (possible_return == 0xC0000001L) { // STATUS_UNSUCCESSFUL
			MessageBox(NULL, L"driver failed", L"error", MB_OK);
			return;
		}
	}
}