#include "hotkeys.h"

namespace hotkey {
	PEPROCESS process;

	HANDLE pid;

	PVOID gaf_async_key_state;

	UINT8 key_state_bitmap[256 * 2 / 8]{};

	UINT8 key_state_recent_bitmap[256 / 8]{};

	bool init() {
		pid = memory::get_process_id("csrss.exe", false, 1);
		if (!pid) {
			print("failed to get winlogon pid\n");
			return false;
		}

		if (!NT_SUCCESS(PsLookupProcessByProcessId(pid, &process))) {
			print("failed to get winlogon process\n");
			return false;
		}

		gaf_async_key_state = memory::get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "gafAsyncKeyState");
		if (!gaf_async_key_state) {
			print("failed to get gaf_async_key_state\n");
			return false;
		}
	}

	void clean_up() {
		ObDereferenceObject(process);
	}

	bool is_key_down(UINT8 const vk) {
		key_state_recent_bitmap[vk / 8] &= ~(1 << vk % 8); // works when placing this line here
		return key_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2;
	}

	bool was_key_pressed(UINT8 const vk) { // doesnt work idk
		bool const result = key_state_recent_bitmap[vk / 8] & 1 << vk % 8;
		key_state_recent_bitmap[vk / 8] &= ~(1 << vk % 8);
		return result;
	}

	void update_key_state_bitmap() {
		auto prev_key_state_bitmap = key_state_bitmap;
		SIZE_T size = 0;

		MmCopyVirtualMemory(process,
			gaf_async_key_state,
			PsGetCurrentProcess(), &key_state_bitmap,
			256 * 2 / 8, KernelMode,
			&size);

		for (auto vk = 0u; vk < 256; ++vk) {
			if ((key_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) &&
				!(prev_key_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2))
				key_state_recent_bitmap[vk / 8] |= 1 << vk % 8;
		}
	}
}