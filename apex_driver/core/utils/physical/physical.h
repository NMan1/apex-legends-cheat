#pragma once
#include "..\imports.h"

namespace physical {
	NTSTATUS read(PEPROCESS pProcess, PVOID Address, PVOID AllocatedBuffer, SIZE_T size, SIZE_T* read);

	NTSTATUS write(PEPROCESS pProcess, PVOID Address, PVOID AllocatedBuffer, SIZE_T size, SIZE_T* written);
}