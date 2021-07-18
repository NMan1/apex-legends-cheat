#pragma once
#include <ntdef.h>

namespace hook {
	extern uintptr_t address_original;

	extern uintptr_t address_handler;

	extern unsigned char original[12];

	extern unsigned char jmp[12];

	void init(uintptr_t to_hook, uintptr_t handler);

	void set_hook();

	void undo_hook();
}