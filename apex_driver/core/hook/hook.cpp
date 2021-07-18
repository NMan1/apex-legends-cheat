#include "hook.h"
#include "..\utils\memory\memory.h"

// credits to the dude for this nice hook wrapper, forgot his name 

namespace hook {
	uintptr_t address_original = NULL;

	uintptr_t address_handler = NULL;

	unsigned char original[12] = {};

	unsigned char jmp[12] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };

	void init(uintptr_t to_hook, uintptr_t handler) {
		address_original = to_hook;
		address_handler = handler;
		memcpy((void*)original, (void*)to_hook, sizeof(original));
		*(uintptr_t*)(&jmp[2]) = address_handler;
	}	
	
	void set_hook() {
		memory::write_to_read_only_memory((PVOID*)address_original, &jmp, sizeof(jmp));
	}	

	void undo_hook() {
		memory::write_to_read_only_memory((PVOID*)address_original, &original, sizeof(original));
	}
}