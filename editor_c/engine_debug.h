#pragma once

#include "engine_types.h"
#include "engine_intrinsics.h"

#ifdef _DEBUG
#define DEBUG_ENABLED 1
#else
#undef DEBUG_ENABLED
#endif

enum DebugType {
	DebugType_Unknown,

	DebugType_FrameMarker,
	DebugType_BeginBlock,
	DebugType_EndBlock,

	DebugType_Count,
};

struct DebugEvent {
	U8 type;
	U64 clock;
	char *guid;
	U16 threadID;
	U16 coreIndex;
	F32 value;
};

constant U32 MAX_DEBUG_EVENT_COUNT = 16 * 65536;
struct DebugTable {
	U32 currentEventArrayIndex;
	volatile U64 eventArrayIndex_EventIndex;
	DebugEvent events[2][MAX_DEBUG_EVENT_COUNT];
};

struct DebugMemory {
	void *storageBase;
	memory_size storageSize;
};

extern DebugTable *globalDebugTable;
extern DebugMemory *globalDebugMemory;

#if DEBUG_ENABLED
#define UniqueFileCounterString__(a, b, c, d) a "|" #b "|" #c "|" d
#define UniqueFileCounterString_(a, b, c, d) UniqueFileCounterString__(a, b, c, d)
#define DEBUG_NAME(name) UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__, name)

inline void RecordDebugEvent(DebugType type, char* guid, F32 value = 0.0f) {
	Assert(globalDebugTable);
	U64 arrayIndex_EventIndex = AtomicAddU64(&globalDebugTable->eventArrayIndex_EventIndex, 1);
	U32 eventIndex = arrayIndex_EventIndex & 0xFFFFFFFF;
	Assert(eventIndex < ArrayCount(globalDebugTable->events[0]));
	DebugEvent *ev = globalDebugTable->events[arrayIndex_EventIndex >> 32] + eventIndex;
	ev->clock = __rdtsc();
	ev->type = (U8)type;
	ev->coreIndex = 0;
	ev->threadID = (U16)GetThreadID();
	ev->guid = guid;
	ev->value = value;
}

#define FRAME_MARKER(secondsElapsed) \
	{ RecordDebugEvent(DebugType_FrameMarker, DEBUG_NAME("Frame Marker"), secondsElapsed); }  

#define TIMED_BLOCK__(guid, counter, ...) TimedBlock timedBlock_##counter(guid, ## __VA_ARGS__)
#define TIMED_BLOCK_(guid, counter, ...) TIMED_BLOCK__(guid, counter, ## __VA_ARGS__)
#define TIMED_BLOCK(name, ...) TIMED_BLOCK_(DEBUG_NAME(name), __COUNTER__, ## __VA_ARGS__)
#define TIMED_FUNCTION(...) TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), ## __VA_ARGS__)

#define BEGIN_BLOCK_(guid) {RecordDebugEvent(DebugType_BeginBlock, guid);}
#define END_BLOCK_() {RecordDebugEvent(DebugType_EndBlock, DEBUG_NAME("::END_BLOCK::"));}

#define BEGIN_BLOCK(name) BEGIN_BLOCK_(DEBUG_NAME(name))
#define END_BLOCK() END_BLOCK_()

struct TimedBlock
{
	TimedBlock(char *guid, U32 hitCountInit = 1)
	{
		BEGIN_BLOCK_(guid);
	}

	~TimedBlock()
	{
		END_BLOCK_();
	}
};
#else
#define TIMED_BLOCK(...) 
#define TIMED_FUNCTION(...) 
#define BEGIN_BLOCK(...)
#define END_BLOCK(...)
#define FRAME_MARKER(...)
#endif