#include "utils\memory\memory.h"
#include "utils\imports.h"
#include "hook\hook.h"
#include "renderer\renderer.h"
#include "utils\color.h"
#include "utils\hotkeys\hotkeys.h"
#include "sdk\sdk.h"
#include "features\features.h"

typedef struct init_t {
	DWORD		usermode_process_id;
	DWORD		usermode_thread_id;
};

LARGE_INTEGER Timeout;

NTSTATUS hook_handler(PVOID called_param) {
	hook::undo_hook();

	print("hook init called\n");
	
	init_t* init = (init_t*)called_param;
	if (!init)
		return STATUS_UNSUCCESSFUL;

	if (!utils::unlink_thread((HANDLE)init->usermode_process_id, (HANDLE)init->usermode_thread_id)) {
		print("failed to unlink thread");
		return STATUS_UNSUCCESSFUL;
	}

	if (!render::init()) {
		print("failed to initalize render functions\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (!hotkey::init()) {
		print("failed to initalize hotkey functions\n");
		return STATUS_UNSUCCESSFUL;
	}

	bool esp = true, rcs = false;
	while (true) {
		hotkey::update_key_state_bitmap();
		if (hotkey::is_key_down(VK_END)) {
			break;
		}

		// search for game
		if (!sdk::pid) {
			sdk::pid = memory::get_process_id("r5apex.exe");
			if (sdk::pid) {
				if (NT_SUCCESS(PsLookupProcessByProcessId(sdk::pid, &sdk::process))) {
					sdk::module_base = memory::get_module_base_x64(sdk::process);
					print("base : %p\n", sdk::module_base);
					print("pid : %d\n", sdk::pid);
				}
			}

			Timeout.QuadPart = RELATIVE(SECONDS(20));
			KeDelayExecutionThread(KernelMode, FALSE, &Timeout);
		}

		// when game is found
		if (sdk::pid) {
			auto local_player = sdk::get_local_player();
			if (local_player && sdk::is_player(local_player)) {
				if (esp)
					features::esp(local_player);

				if (rcs && hotkey::is_key_down(VK_LBUTTON))
					features::rcs(local_player);
			}
		}

		// settings
		if (hotkey::is_key_down(VK_F1))
			esp = !esp;
		else if (hotkey::is_key_down(VK_F2))
			rcs = !rcs;
	}

	hotkey::clean_up();

	if (sdk::process)
		ObDereferenceObject(sdk::process);

	print("leaving hook\n\n");
	return STATUS_SUCCESS;
}

void real_entry() {
	print("\nreal entry called.\n");

	auto exported_func = memory::get_system_module_export("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", "NtOpenCompositionSurfaceSectionInfo");
	hook::init((uintptr_t)exported_func, (uintptr_t)hook_handler);
	hook::set_hook();
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT  driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);
	return STATUS_SUCCESS;
}