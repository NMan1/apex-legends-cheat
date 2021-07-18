#pragma once
#include <ntdef.h>
#include <ntimage.h>
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntstrsafe.h>

typedef struct _MM_UNLOADED_DRIVER
{
	UNICODE_STRING     Name;
	PVOID             ModuleStart;
	PVOID             ModuleEnd;
	ULONG64         UnloadTime;
} MM_UNLOADED_DRIVER, * PMM_UNLOADED_DRIVER;
typedef struct _PIDDBCACHE_ENTRY
{
	LIST_ENTRY        List;
	UNICODE_STRING    DriverName;
	ULONG            TimeDateStamp;
	NTSTATUS        LoadStatus;
	char            _0x0028[16];
} PIDDBCACHE_ENTRY, * PPIDDBCACHE_ENTRY;

typedef struct _SYSTEM_MODULEE
{
	ULONG_PTR Reserved[2];
	PVOID     Base;
	ULONG     Size;
	ULONG     Flags;
	USHORT    Index;
	USHORT    Unknown;
	USHORT    LoadCount;
	USHORT    ModuleNameOffset;
	CHAR      ImageName[256];
} SYSTEM_MODULEE, * PSYSTEM_MODULEE;
typedef struct _SYSTEM_MODULE_INFORMATIONN
{
	ULONG_PTR     ModuleCount;
	SYSTEM_MODULEE Modules[1];
} SYSTEM_MODULE_INFORMATIONN, * PSYSTEM_MODULE_INFORMATIONN;

typedef struct _ETHREAD_META_2004
{
	char pad_0x0008[0x448]; //0x0008
	PVOID StartAddress; //0x0450 
	char pad_0x0458[0x20]; //0x0458
	_CLIENT_ID Cid; //0x0478 
	char pad_0x0480[0x50]; //0x0480
	PVOID Win32StartAddress; //0x04D0 
	char pad_0x04D8[0x10]; //0x04D8
	struct _LIST_ENTRY ThreadListEntry; //0x04E8 
} ETHREAD_META_2004, * PETHREAD_META_2004; /* size: 0x0898 */

typedef struct PiDDBCacheEntry {
	LIST_ENTRY		List;
	UNICODE_STRING	DriverName;
	ULONG			TimeDateStamp;
	NTSTATUS		LoadStatus;
	char			_0x0028[16];
}PIDCacheobj;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation = 0x0B
} SYSTEM_INFORMATION_CLASS,
* PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESS_INFO {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
}SYSTEM_PROCESS_INFO, * PSYSTEM_PROCESS_INFO;

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void); // not exported

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, * PPEB;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;  // in bytes
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;  // LDR_*
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
	//    PVOID			LoadedImports;
	//    // seems they are exist only on XP !!! PVOID
	//    EntryPointActivationContext;	// -same-
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _STOR_SCSI_IDENTITY {
	char Space[0x8]; // +0x008 SerialNumber     : _STRING
	STRING SerialNumber;
} STOR_SCSI_IDENTITY, * PSTOR_SCSI_IDENTITY;

typedef struct _TELEMETRY_UNIT_EXTENSION {
	/*
		+0x000 Flags            : <anonymous-tag>
		+0x000 DeviceHealthEventsLogged : Pos 0, 1 Bit
		+0x000 FailedFirstSMARTCommand : Pos 1, 1 Bit
		+0x000 FailedFirstDeviceStatisticsLogCommand : Pos 2, 1 Bit
		+0x000 FailedFirstNvmeCloudSSDCommand : Pos 3, 1 Bit
		+0x000 SmartPredictFailure : Pos 4, 1 Bit
		+0x000 Reserved         : Pos 5, 27 Bits
	 */
	int SmartMask;
} TELEMETRY_UNIT_EXTENSION, * PTELEMETRY_UNIT_EXTENSION;

// lkd> dt storport!_RAID_UNIT_EXTENSION -b
typedef struct _RAID_UNIT_EXTENSION {
	union
	{
		struct
		{
			char Space[0x68]; // +0x068 Identity         : _STOR_SCSI_IDENTITY
			STOR_SCSI_IDENTITY Identity;
		} _Identity;

		struct
		{
			char Space[0x7c8]; // +0x7c8 TelemetryExtension : _TELEMETRY_UNIT_EXTENSION
			TELEMETRY_UNIT_EXTENSION Telemetry;
		} _Smart;
	};
} RAID_UNIT_EXTENSION, * PRAID_UNIT_EXTENSION;


extern "C" NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID Base);

typedef __int64(__fastcall* RaidUnitRegisterInterfaces)(PRAID_UNIT_EXTENSION a1);

typedef NTSTATUS(__fastcall* DiskEnableDisableFailurePrediction)(void* a1, bool a2);

extern "C" __declspec(dllimport) NTSTATUS NTAPI ZwProtectVirtualMemory
(
	HANDLE ProcessHandle,
	PVOID * BaseAddress,
	PSIZE_T ProtectSize,
	ULONG NewProtect,
	PULONG OldProtect
);

extern "C" NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(_In_ PVOID ImageBase, _In_ PCCH RoutineName);

extern "C" NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);

extern "C" NTKERNELAPI PPEB PsGetProcessPeb(IN PEPROCESS Process);

extern "C" NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID Base);

extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

extern "C" NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
	PUNICODE_STRING ObjectName,
	ULONG Attributes,
	PACCESS_STATE AccessState,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	PVOID ParseContext OPTIONAL,
	PVOID* Object
);

extern "C" NTKERNELAPI
NTSTATUS
NTAPI
PsGetContextThread(
	__in PETHREAD Thread,
	__inout PCONTEXT ThreadContext,
	__in KPROCESSOR_MODE Mode
);

extern "C" NTKERNELAPI
NTSTATUS
NTAPI
PsSetContextThread(
	__in PETHREAD Thread,
	 __in PCONTEXT ThreadContext,
	 __in KPROCESSOR_MODE PreviousMode
	);

typedef HBRUSH(*gdi_select_brush_t)(_In_ HDC hdc, _In_ HBRUSH hbr);

typedef BOOL(*pal_blt_t)(_In_ HDC, _In_ int x, _In_ int y, _In_ int w, _In_ int h, _In_ DWORD);

typedef HDC(*nt_user_get_dc_t)(HWND hwnd);

typedef HBRUSH(*nt_gdi_create_solid_brush_t)(_In_ COLORREF color, _In_opt_ HBRUSH hbr);

typedef int (*release_dc_t)(HDC hdc);

typedef BOOL(*delete_object_app_t)(HANDLE handle);

typedef void(*gre_ext_text_out_w_internal_t)(_In_ HDC dc, int left, int top, UINT64, UINT64, LPCWSTR text, UINT32 textSize, UINT64, UINT64, UINT64);