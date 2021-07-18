#pragma once
#include <ntifs.h>

#define print(f_, ...) DbgPrintEx(0, 0, (f_), ##__VA_ARGS__)

#define make_ptr( ptr, val ) ( (uintptr_t)ptr + (uintptr_t)val )

#define RELATIVE(wait) (-(wait))

#define NANOSECONDS(nanos) \
(((signed __int64)(nanos)) / 100L)

#define MICROSECONDS(micros) \
(((signed __int64)(micros)) * NANOSECONDS(1000L))

#define MILLISECONDS(milli) \
(((signed __int64)(milli)) * MICROSECONDS(1000L))

#define SECONDS(seconds) \
(((signed __int64)(seconds)) * MILLISECONDS(1000L))

namespace utils {
	UNICODE_STRING char_to_unicode(const char* convert);

	void random_text(char* text, const int length);

	bool unlink_threadlist_head(HANDLE process_id);

	bool unlink_thread(HANDLE process_id, HANDLE thread_id);

	bool relink_threads(HANDLE process_id);
}