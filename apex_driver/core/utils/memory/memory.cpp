#pragma once
#include "memory.h"

namespace memory {
	PVOID pattern_scan(PVOID base, const char* pattern, const char* mask) {
		auto check_mask = [](const char* base, const char* pattern, const char* mask) -> bool {
			for (; *mask; ++base, ++pattern, ++mask) {
				if ('x' == *mask && *base != *pattern) {
					return false;
				}
			}

			return true;
		};

		auto find_pattern = [&check_mask](PVOID base, int length, const char* pattern, const char* mask) -> PVOID {
			length -= static_cast<int>(strlen(mask));
			for (auto i = 0; i <= length; ++i)
			{
				const auto* data = static_cast<char*>(base);
				const auto* address = &data[i];
				if (check_mask(address, pattern, mask))
					return PVOID(address);
			}

			return nullptr;
		};

		PVOID match = nullptr;

		auto* headers = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<char*>(base) + static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
		auto* sections = IMAGE_FIRST_SECTION(headers);

		for (auto i = 0; i < headers->FileHeader.NumberOfSections; ++i) {
			auto* section = &sections[i];
			if ('EGAP' == *reinterpret_cast<PINT>(section->Name) || memcmp(section->Name, ".text", 5) == 0) {
				match = find_pattern(static_cast<char*>(base) + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
				if (match)
					break;
			}
		}

		return match;
	}

	PVOID get_system_module_base(const char* module_name) {
		ULONG bytes = 0;
		NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, bytes, &bytes);
		if (!bytes)
			return 0;

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x454E4F45); // 'ENON'
		status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);
		if (!NT_SUCCESS(status))
			return 0;

		PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
		PVOID module_base = 0, module_size = 0;

		for (ULONG i = 0; i < modules->NumberOfModules; i++) {
			if (strcmp((char*)module[i].FullPathName, module_name) == 0) {
				module_base = module[i].ImageBase;
				module_size = (PVOID)module[i].ImageSize;
				break;
			}
		}

		if (modules)
			ExFreePoolWithTag(modules, 0);

		if (module_base <= 0)
			return 0;

		return module_base;
	}

	ULONG64 get_module_base_x64(PEPROCESS proc) {
		return (ULONG64)PsGetProcessSectionBaseAddress(proc);
	}

	PVOID get_import_module_base(PEPROCESS target_proc, const char* module_name) {
		auto module_name_unicode = utils::char_to_unicode(module_name);

		PVOID base = 0;
		KeAttachProcess((PKPROCESS)target_proc);

		PPEB peb = PsGetProcessPeb(target_proc);
		if (!peb)
			goto end;

		if (!peb->Ldr || !peb->Ldr->Initialized)
			goto end;

		for (PLIST_ENTRY list = peb->Ldr->ModuleListLoadOrder.Flink;
			list != &peb->Ldr->ModuleListLoadOrder;
			list = list->Flink) {
			PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
			if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
				base = entry->DllBase;
				goto end;
			}
		}

	end:
		KeDetachProcess();
		return base;
	}

	HANDLE get_process_id(const char* process_name, bool print, int index) {
		ULONG buffer_size = 0;
		ZwQuerySystemInformation(SystemProcessInformation, NULL, NULL, &buffer_size);

		auto buffer = ExAllocatePoolWithTag(NonPagedPool, buffer_size, 'mder');
		if (!buffer) {
			print("failed to allocate pool (get_process_id)");
			return 0;
		}

		HANDLE pid = 0;
		auto search_index = 0;
		auto process_name_unicode = utils::char_to_unicode(process_name);
		auto process_info = (PSYSTEM_PROCESS_INFO)buffer;
		if (NT_SUCCESS(ZwQuerySystemInformation(SystemProcessInformation, process_info, buffer_size, NULL))) {
			while (process_info->NextEntryOffset) {
				if (!RtlCompareUnicodeString(&process_name_unicode, &process_info->ImageName, true)) {
					if (search_index == index) {
						if (print)
							print("process name: %wZ | process ID: %d\n", process_info->ImageName, process_info->UniqueProcessId);
						pid = process_info->UniqueProcessId;
						break;
					}
					search_index++;
				}
				process_info = (PSYSTEM_PROCESS_INFO)((BYTE*)process_info + process_info->NextEntryOffset);
			}
		}

		RtlFreeUnicodeString(&process_name_unicode);
		ExFreePoolWithTag(buffer, 'mder');
		return pid;
	}

	PVOID get_system_module_export(const char* module_name, LPCSTR routine_name) {
		PVOID lpModule = memory::get_system_module_base(module_name);

		if (!lpModule)
			return NULL;

		return RtlFindExportedRoutineByName(lpModule, routine_name);
	}

	bool write_to_read_only_memory(void* address, void* buffer, size_t size) {

		PMDL Mdl = IoAllocateMdl(address, size, FALSE, FALSE, NULL);

		if (!Mdl)
			return false;

		// Locking and mapping memory with RW-rights:
		MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
		PVOID Mapping = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
		MmProtectMdlSystemAddress(Mdl, PAGE_READWRITE);

		// Write your buffer to mapping:
		RtlCopyMemory(Mapping, buffer, size);

		// Resources freeing:
		MmUnmapLockedPages(Mapping, Mdl);
		MmUnlockPages(Mdl);
		IoFreeMdl(Mdl);

		return true;
	}

	ULONG virtual_protect(HANDLE pid, PVOID address, ULONG size, ULONG protection) {
		if (!pid || !address || !size || !protection)
			return STATUS_INVALID_PARAMETER;

		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS target_process = nullptr;

		if (!NT_SUCCESS(PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &target_process)))
			return STATUS_NOT_FOUND;

		ULONG protection_old = 0;
		SIZE_T size_t = size;

		KAPC_STATE state;
		KeStackAttachProcess(target_process, &state);

		status = ZwProtectVirtualMemory(NtCurrentProcess(), &address, &size_t, protection, &protection_old);
		print("virtual protect %X", status);

		KeUnstackDetachProcess(&state);

		ObDereferenceObject(target_process);
		return protection_old;
	}

	PVOID virtual_allocate(HANDLE pid, ULONG allocation_type, ULONG protection, ULONG size) {
		PEPROCESS process;
		if (!NT_SUCCESS(PsLookupProcessByProcessId(pid, &process))) {
			print("process lookup failed (virtual_allocate)");
			return false;
		}

		SIZE_T bytes = size;
		PVOID allocation_address = NULL;
		KAPC_STATE apc;
		KeStackAttachProcess(process, &apc);
		auto status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &allocation_address, 0, &bytes, allocation_type, protection);
		KeUnstackDetachProcess(&apc);
		ObfDereferenceObject(process);
		return allocation_address;
	}
}