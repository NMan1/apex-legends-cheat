#include <Windows.h>
#include "driver/driver.h"

init_t init{};

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reserved) {
	if (reason_for_call == DLL_PROCESS_ATTACH) {
		init.usermode_process_id = GetCurrentProcessId();
		init.usermode_thread_id = GetThreadId(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)driver::call_hook, &init, 0, 0));
	}
}