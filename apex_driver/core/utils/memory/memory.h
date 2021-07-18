#pragma once
#include "..\imports.h"
#include "..\utils.h"
#include "..\physical\physical.h"

namespace memory {
	PVOID pattern_scan(PVOID base, const char* pattern, const char* mask);

	PVOID get_system_module_base(const char* module_name);

	ULONG64 get_module_base_x64(PEPROCESS proc);

	PVOID get_import_module_base(PEPROCESS target_proc, const char* module_name);

	HANDLE get_process_id(const char* process_name, bool print = false, int index = 0);

	PVOID get_system_module_export(const char* module_name, LPCSTR routine_name);

	bool write_to_read_only_memory(void* address, void* buffer, size_t size);

	template <typename type>
	type read(PEPROCESS proccess, uintptr_t address, bool phsical_memeory=true) {
		type buffer{};
		if (!proccess || !address || (ULONGLONG)address > 0x7fffffffffff)
			return buffer;

		if (!phsical_memeory) {
			SIZE_T bytes = 0;
			MmCopyVirtualMemory(proccess, (PVOID)address, PsGetCurrentProcess(), &buffer, sizeof(type), KernelMode, &bytes);
			return buffer;
		}
		else {
			SIZE_T out_size{};
			if (NT_SUCCESS(physical::read(proccess, (PVOID)address, &buffer, sizeof(type), &out_size)))
				return buffer;
			else 
				return {};
		}
	}

	template <typename type>
	bool write(PEPROCESS proccess, uintptr_t address, type value, bool phsical_memeory = true) {
		if (!proccess || !address || (ULONGLONG)address > 0x7fffffffffff)
			return false;

		if (!phsical_memeory) {
			SIZE_T bytes = 0;
			return MmCopyVirtualMemory(PsGetCurrentProcess(), &value, proccess, (PVOID)address, sizeof(value), KernelMode, &bytes) == STATUS_SUCCESS;
		}
		else {
			SIZE_T out_size{};
			return physical::write(proccess, (PVOID)address, &value, sizeof(value), &out_size) == STATUS_SUCCESS;
		}
	}

	ULONG virtual_protect(HANDLE pid, PVOID address, ULONG size, ULONG protection);

	PVOID virtual_allocate(HANDLE pid, ULONG allocation_type, ULONG protection, ULONG size);
}