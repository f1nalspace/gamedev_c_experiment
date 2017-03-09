#pragma once

#include "engine_debug.h"
#include "engine_memory.h"
#include "engine_render.h"

struct DebugNode {
	DebugNode *parent;
	DebugNode *firstChild;
	DebugNode *nextSibling;
	U64 startClock;
	U64 endClock;
	char *guid;
	U16 threadID;
	U16 coreIndex;
};

struct DebugState {
	MemoryBlock debugMemory;
	DebugNode *nodes;
	DebugNode *firstFreeNode;
	U32 maxFreeNodeCount;
	DebugNode rootNode;
};

external void DEBUGInit();
external void DEBUGRender(RenderState *renderState);
external void DEBUGFrameEnd();