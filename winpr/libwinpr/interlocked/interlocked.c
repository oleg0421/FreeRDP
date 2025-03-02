/**
 * WinPR: Windows Portable Runtime
 * Interlocked Singly-Linked Lists
 *
 * Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <winpr/config.h>

#include <winpr/assert.h>
#include <winpr/wlog.h>
#include <winpr/platform.h>
#include <winpr/synch.h>
#include <winpr/handle.h>

#include <winpr/interlocked.h>

/* Singly-Linked List */

#ifndef _WIN32

#include <stdio.h>
#include <stdlib.h>

VOID InitializeSListHead(WINPR_PSLIST_HEADER ListHead)
{
	WINPR_ASSERT(ListHead);
#ifdef _WIN64
	ListHead->s.Alignment = 0;
	ListHead->s.Region = 0;
	ListHead->Header8.Init = 1;
#else
	ListHead->Alignment = 0;
#endif
}

WINPR_PSLIST_ENTRY InterlockedPushEntrySList(WINPR_PSLIST_HEADER ListHead,
                                             WINPR_PSLIST_ENTRY ListEntry)
{
	WINPR_SLIST_HEADER old = { 0 };
	WINPR_SLIST_HEADER newHeader = { 0 };

	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(ListEntry);
#ifdef _WIN64
	newHeader.HeaderX64.NextEntry = (((ULONG_PTR)ListEntry) >> 4);

	while (1)
	{
		old = *ListHead;

		ListEntry->Next = (PSLIST_ENTRY)(((ULONG_PTR)old.HeaderX64.NextEntry) << 4);

		newHeader.HeaderX64.Depth = old.HeaderX64.Depth + 1;
		newHeader.HeaderX64.Sequence = old.HeaderX64.Sequence + 1;

		if (InterlockedCompareExchange64((LONGLONG*)ListHead, newHeader).Alignment, old).Alignment))
		{
			InterlockedCompareExchange64(&((LONGLONG*)ListHead)[1], newHeader).Region,
										 old).Region);
			break;
		}
	}

	return (PSLIST_ENTRY)((ULONG_PTR)old.HeaderX64.NextEntry << 4);
#else
	newHeader.s.Next.Next = ListEntry;

	do
	{
		old = *ListHead;
		ListEntry->Next = old.s.Next.Next;
		newHeader.s.Depth = old.s.Depth + 1;
		newHeader.s.Sequence = old.s.Sequence + 1;
		if (old.Alignment > INT64_MAX)
			return NULL;
		if (newHeader.Alignment > INT64_MAX)
			return NULL;
		if (ListHead->Alignment > INT64_MAX)
			return NULL;
	} while (InterlockedCompareExchange64((LONGLONG*)&ListHead->Alignment,
	                                      (LONGLONG)newHeader.Alignment,
	                                      (LONGLONG)old.Alignment) != (LONGLONG)old.Alignment);

	return old.s.Next.Next;
#endif
}

WINPR_PSLIST_ENTRY InterlockedPushListSListEx(WINPR_ATTR_UNUSED WINPR_PSLIST_HEADER ListHead,
                                              WINPR_ATTR_UNUSED WINPR_PSLIST_ENTRY List,
                                              WINPR_ATTR_UNUSED WINPR_PSLIST_ENTRY ListEnd,
                                              WINPR_ATTR_UNUSED ULONG Count)
{
	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(List);
	WINPR_ASSERT(ListEnd);

	WLog_ERR("TODO", "TODO: implement");
#ifdef _WIN64

#else

#endif
	return NULL;
}

WINPR_PSLIST_ENTRY InterlockedPopEntrySList(WINPR_PSLIST_HEADER ListHead)
{
	WINPR_SLIST_HEADER old = { 0 };
	WINPR_SLIST_HEADER newHeader = { 0 };
	WINPR_PSLIST_ENTRY entry = NULL;

	WINPR_ASSERT(ListHead);

#ifdef _WIN64
	while (1)
	{
		old = *ListHead;

		entry = (PSLIST_ENTRY)(((ULONG_PTR)old.HeaderX64.NextEntry) << 4);

		if (!entry)
			return NULL;

		newHeader.HeaderX64.NextEntry = ((ULONG_PTR)entry->Next) >> 4;
		newHeader.HeaderX64.Depth = old.HeaderX64.Depth - 1;
		newHeader.HeaderX64.Sequence = old.HeaderX64.Sequence - 1;

		if (InterlockedCompareExchange64((LONGLONG*)ListHead, newHeader).Alignment, old).Alignment))
		{
			InterlockedCompareExchange64(&((LONGLONG*)ListHead)[1], newHeader).Region,
										 old).Region);
			break;
		}
	}
#else
	do
	{
		old = *ListHead;

		entry = old.s.Next.Next;

		if (!entry)
			return NULL;

		newHeader.s.Next.Next = entry->Next;
		newHeader.s.Depth = old.s.Depth - 1;
		newHeader.s.Sequence = old.s.Sequence + 1;

		if (old.Alignment > INT64_MAX)
			return NULL;
		if (newHeader.Alignment > INT64_MAX)
			return NULL;
		if (ListHead->Alignment > INT64_MAX)
			return NULL;
	} while (InterlockedCompareExchange64((LONGLONG*)&ListHead->Alignment,
	                                      (LONGLONG)newHeader.Alignment,
	                                      (LONGLONG)old.Alignment) != (LONGLONG)old.Alignment);
#endif
	return entry;
}

WINPR_PSLIST_ENTRY InterlockedFlushSList(WINPR_PSLIST_HEADER ListHead)
{
	WINPR_SLIST_HEADER old = { 0 };
	WINPR_SLIST_HEADER newHeader = { 0 };

	WINPR_ASSERT(ListHead);
	if (!QueryDepthSList(ListHead))
		return NULL;

#ifdef _WIN64
	newHeader).Alignment = 0;
	newHeader).Region = 0;
	newHeader.HeaderX64.HeaderType = 1;

	while (1)
	{
		old = *ListHead;
		newHeader.HeaderX64.Sequence = old.HeaderX64.Sequence + 1;

		if (InterlockedCompareExchange64((LONGLONG*)ListHead, newHeader).Alignment, old).Alignment))
		{
			InterlockedCompareExchange64(&((LONGLONG*)ListHead)[1], newHeader).Region,
										 old).Region);
			break;
		}
	}

	return (PSLIST_ENTRY)(((ULONG_PTR)old.HeaderX64.NextEntry) << 4);
#else
	newHeader.Alignment = 0;

	do
	{
		old = *ListHead;
		newHeader.s.Sequence = old.s.Sequence + 1;

		if (old.Alignment > INT64_MAX)
			return NULL;
		if (newHeader.Alignment > INT64_MAX)
			return NULL;
		if (ListHead->Alignment > INT64_MAX)
			return NULL;
	} while (InterlockedCompareExchange64((LONGLONG*)&ListHead->Alignment,
	                                      (LONGLONG)newHeader.Alignment,
	                                      (LONGLONG)old.Alignment) != (LONGLONG)old.Alignment);

	return old.s.Next.Next;
#endif
}

USHORT QueryDepthSList(WINPR_PSLIST_HEADER ListHead)
{
	WINPR_ASSERT(ListHead);

#ifdef _WIN64
	return ListHead->HeaderX64.Depth;
#else
	return ListHead->s.Depth;
#endif
}

LONG InterlockedIncrement(LONG volatile* Addend)
{
	WINPR_ASSERT(Addend);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_add_and_fetch(Addend, 1);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

LONG InterlockedDecrement(LONG volatile* Addend)
{
	WINPR_ASSERT(Addend);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_sub_and_fetch(Addend, 1);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

LONG InterlockedExchange(LONG volatile* Target, LONG Value)
{
	WINPR_ASSERT(Target);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_val_compare_and_swap(Target, *Target, Value);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

LONG InterlockedExchangeAdd(LONG volatile* Addend, LONG Value)
{
	WINPR_ASSERT(Addend);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_fetch_and_add(Addend, Value);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

LONG InterlockedCompareExchange(LONG volatile* Destination, LONG Exchange, LONG Comperand)
{
	WINPR_ASSERT(Destination);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

PVOID InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID Exchange,
                                        PVOID Comperand)
{
	WINPR_ASSERT(Destination);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

#endif /* _WIN32 */

#if defined(_WIN32) && !defined(WINPR_INTERLOCKED_COMPARE_EXCHANGE64)

/* InterlockedCompareExchange64 already defined */

#elif defined(_WIN32) && defined(WINPR_INTERLOCKED_COMPARE_EXCHANGE64)

static volatile HANDLE mutex = NULL;

BOOL static_mutex_lock(volatile HANDLE* static_mutex)
{
	if (*static_mutex == NULL)
	{
		HANDLE handle;

		if (!(handle = CreateMutex(NULL, FALSE, NULL)))
			return FALSE;

		if (InterlockedCompareExchangePointer((PVOID*)static_mutex, (PVOID)handle, NULL) != NULL)
			(void)CloseHandle(handle);
	}

	return (WaitForSingleObject(*static_mutex, INFINITE) == WAIT_OBJECT_0);
}

LONGLONG InterlockedCompareExchange64(LONGLONG volatile* Destination, LONGLONG Exchange,
                                      LONGLONG Comperand)
{
	LONGLONG previousValue = 0;
	BOOL locked = static_mutex_lock(&mutex);

	previousValue = *Destination;

	if (*Destination == Comperand)
		*Destination = Exchange;

	if (locked)
		(void)ReleaseMutex(mutex);
	else
		(void)fprintf(stderr,
		              "WARNING: InterlockedCompareExchange64 operation might have failed\n");

	return previousValue;
}

#elif (defined(ANDROID) && ANDROID) || \
    (defined(__GNUC__) && !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8))

#include <pthread.h>

static pthread_mutex_t mutex;

LONGLONG InterlockedCompareExchange64(LONGLONG volatile* Destination, LONGLONG Exchange,
                                      LONGLONG Comperand)
{
	LONGLONG previousValue = 0;

	pthread_mutex_lock(&mutex);

	previousValue = *Destination;

	if (*Destination == Comperand)
		*Destination = Exchange;

	pthread_mutex_unlock(&mutex);

	return previousValue;
}

#else

LONGLONG InterlockedCompareExchange64(LONGLONG volatile* Destination, LONGLONG Exchange,
                                      LONGLONG Comperand)
{
	WINPR_ASSERT(Destination);

#if defined(__GNUC__) || defined(__clang__)
	WINPR_PRAGMA_DIAG_PUSH
	WINPR_PRAGMA_DIAG_IGNORED_ATOMIC_SEQ_CST
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
	WINPR_PRAGMA_DIAG_POP
#else
	return 0;
#endif
}

#endif

/* Doubly-Linked List */

/**
 * Kernel-Mode Basics: Windows Linked Lists:
 * http://www.osronline.com/article.cfm?article=499
 *
 * Singly and Doubly Linked Lists:
 * http://msdn.microsoft.com/en-us/library/windows/hardware/ff563802/
 */

VOID InitializeListHead(WINPR_PLIST_ENTRY ListHead)
{
	WINPR_ASSERT(ListHead);
	ListHead->Flink = ListHead->Blink = ListHead;
}

BOOL IsListEmpty(const WINPR_LIST_ENTRY* ListHead)
{
	WINPR_ASSERT(ListHead);
	return (BOOL)(ListHead->Flink == ListHead);
}

BOOL RemoveEntryList(WINPR_PLIST_ENTRY Entry)
{
	WINPR_ASSERT(Entry);
	WINPR_PLIST_ENTRY OldFlink = Entry->Flink;
	WINPR_ASSERT(OldFlink);

	WINPR_PLIST_ENTRY OldBlink = Entry->Blink;
	WINPR_ASSERT(OldBlink);

	OldFlink->Blink = OldBlink;
	OldBlink->Flink = OldFlink;

	return (BOOL)(OldFlink == OldBlink);
}

VOID InsertHeadList(WINPR_PLIST_ENTRY ListHead, WINPR_PLIST_ENTRY Entry)
{
	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(Entry);

	WINPR_PLIST_ENTRY OldFlink = ListHead->Flink;
	WINPR_ASSERT(OldFlink);

	Entry->Flink = OldFlink;
	Entry->Blink = ListHead;
	OldFlink->Blink = Entry;
	ListHead->Flink = Entry;
}

WINPR_PLIST_ENTRY RemoveHeadList(WINPR_PLIST_ENTRY ListHead)
{
	WINPR_ASSERT(ListHead);

	WINPR_PLIST_ENTRY Entry = ListHead->Flink;
	WINPR_ASSERT(Entry);

	WINPR_PLIST_ENTRY Flink = Entry->Flink;
	WINPR_ASSERT(Flink);

	ListHead->Flink = Flink;
	Flink->Blink = ListHead;

	return Entry;
}

VOID InsertTailList(WINPR_PLIST_ENTRY ListHead, WINPR_PLIST_ENTRY Entry)
{
	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(Entry);

	WINPR_PLIST_ENTRY OldBlink = ListHead->Blink;
	WINPR_ASSERT(OldBlink);

	Entry->Flink = ListHead;
	Entry->Blink = OldBlink;
	OldBlink->Flink = Entry;
	ListHead->Blink = Entry;
}

WINPR_PLIST_ENTRY RemoveTailList(WINPR_PLIST_ENTRY ListHead)
{
	WINPR_ASSERT(ListHead);

	WINPR_PLIST_ENTRY Entry = ListHead->Blink;
	WINPR_ASSERT(Entry);

	WINPR_PLIST_ENTRY Blink = Entry->Blink;
	WINPR_ASSERT(Blink);

	ListHead->Blink = Blink;
	Blink->Flink = ListHead;

	return Entry;
}

VOID AppendTailList(WINPR_PLIST_ENTRY ListHead, WINPR_PLIST_ENTRY ListToAppend)
{
	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(ListToAppend);

	WINPR_PLIST_ENTRY ListEnd = ListHead->Blink;

	ListHead->Blink->Flink = ListToAppend;
	ListHead->Blink = ListToAppend->Blink;
	ListToAppend->Blink->Flink = ListHead;
	ListToAppend->Blink = ListEnd;
}

VOID PushEntryList(WINPR_PSINGLE_LIST_ENTRY ListHead, WINPR_PSINGLE_LIST_ENTRY Entry)
{
	WINPR_ASSERT(ListHead);
	WINPR_ASSERT(Entry);

	Entry->Next = ListHead->Next;
	ListHead->Next = Entry;
}

WINPR_PSINGLE_LIST_ENTRY PopEntryList(WINPR_PSINGLE_LIST_ENTRY ListHead)
{
	WINPR_ASSERT(ListHead);
	WINPR_PSINGLE_LIST_ENTRY FirstEntry = ListHead->Next;

	if (FirstEntry != NULL)
		ListHead->Next = FirstEntry->Next;

	return FirstEntry;
}
