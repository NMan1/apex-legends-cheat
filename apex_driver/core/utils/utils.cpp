#include "utils.h"
#include "imports.h"

namespace utils {
	UNICODE_STRING char_to_unicode(const char* convert) {
		ANSI_STRING ansi = {};
		UNICODE_STRING uni = {};
		RtlInitAnsiString(&ansi, convert);
		if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE))) {
			RtlFreeAnsiString(&ansi);
		}
		return uni;
	}

	LIST_ENTRY old{};

	// pair this with function relink_threads when leaving hook or else bsod. This is an alternatvie to the funciton below, but I diiodnt do this one becaue its suspisocus with a process wiht no
	// theeads (not possible) and this was just a lazy implemneation
    bool unlink_threadlist_head(HANDLE process_id) {
		if (!process_id) {
			print("invalid process id\n");
			return false;
		}

		PEPROCESS process = nullptr;
		if (NT_SUCCESS(PsLookupProcessByProcessId(process_id, &process))) {
			LIST_ENTRY* thread_list_head = (LIST_ENTRY*)make_ptr(process, 0x5e0);
			if (!thread_list_head) {
				print("failed to get thread list head");
				ObDereferenceObject(process);
				return false;
			}

			old.Blink = thread_list_head->Blink;
			old.Flink = thread_list_head->Flink;
			thread_list_head->Blink = thread_list_head;
			thread_list_head->Flink = thread_list_head;

			print("removed all threads from linked list\n");
			ObDereferenceObject(process);
			return true;
		}
		
		return false;
    }

	bool unlink_thread(HANDLE process_id, HANDLE thread_id) {
		PEPROCESS process = nullptr;
		if (NT_SUCCESS(PsLookupProcessByProcessId(process_id, &process))) {
			PETHREAD thread = nullptr;
			if (NT_SUCCESS(PsLookupThreadByThreadId(thread_id, &thread))) {
				LIST_ENTRY* thread_list_head = (LIST_ENTRY*)make_ptr(process, 0x5e0);
				if (!thread_list_head) {
					print("failed to get thread list head");
					ObDereferenceObject(process);
					return false;
				}

				PLIST_ENTRY list = thread_list_head;
				while ((list = list->Flink) != thread_list_head) { // thxs russian man on unkwn
					PETHREAD_META_2004 p_entry = CONTAINING_RECORD(list, ETHREAD_META_2004, ThreadListEntry);
					UINT64 current_tid = (UINT64)PsGetThreadId((PETHREAD)p_entry);
					if (current_tid == (UINT64)thread_id) {
						PETHREAD_META_2004 p_previous_entry = CONTAINING_RECORD(list->Blink, ETHREAD_META_2004, ThreadListEntry);
						PETHREAD_META_2004 p_next_entry = CONTAINING_RECORD(list->Flink, ETHREAD_META_2004, ThreadListEntry);

						p_previous_entry->ThreadListEntry.Flink = list->Flink;
						p_next_entry->ThreadListEntry.Blink = list->Blink;

						list->Blink = list;
						list->Flink = list;

						print("removed thread[%d] from linked list\n", thread_id);
						ObDereferenceObject(process);
						ObDereferenceObject(thread);
						return true;
					}

				}

				ObDereferenceObject(process);
				ObDereferenceObject(thread);
				return false;
			}
		}
	}

	bool relink_threads(HANDLE process_id) {
		if (!process_id) {
			print("invalid process id\n");
			return false;
		}

		PEPROCESS process = nullptr;
		if (NT_SUCCESS(PsLookupProcessByProcessId(process_id, &process))) {
			LIST_ENTRY* thread_list_head = (LIST_ENTRY*)make_ptr(process, 0x5e0);
			if (!thread_list_head) {
				print("failed to get thread list head");
				ObDereferenceObject(process);
				return false;
			}

			thread_list_head->Blink = old.Blink;
			thread_list_head->Flink = old.Flink;

			print("restored threads to linked list\n");
			ObDereferenceObject(process);

			return true;
		}
		
		return false;
	}
}
