#include "engine_debug_internal.h"

#include "engine_math.h"

#include <stdio.h>

internal DebugNode *GetNewDebugNode(DebugState *debugState) {
	Assert(debugState->firstFreeNode);
	DebugNode *result = debugState->firstFreeNode;
	debugState->firstFreeNode = result->firstChild;
	result->firstChild = result->nextSibling = 0;
	result->parent = 0;
	return(result);
}

internal void ResetDebugNodes(DebugState *debugState) {
	for (U32 nodeIndex = 0; nodeIndex < debugState->maxFreeNodeCount; ++nodeIndex) {
		DebugNode *node = debugState->nodes + nodeIndex;
		*node = {};
	}
	for (U32 nodeIndex = 0; nodeIndex < debugState->maxFreeNodeCount - 1; ++nodeIndex) {
		DebugNode *node = debugState->nodes + nodeIndex;
		node->firstChild = node + 1;
	}
	debugState->firstFreeNode = debugState->nodes;
	debugState->rootNode = {};
	debugState->rootNode.guid = "ROOT";
}

external void DEBUGInit() {
	Assert(globalDebugMemory);
	DebugState *debugState = (DebugState *)globalDebugMemory->storageBase;
	Assert(debugState);

	debugState->debugMemory = MemoryBlockCreate((U8 *)globalDebugMemory->storageBase + sizeof(DebugState), globalDebugMemory->storageSize - sizeof(DebugState), MemoryFlag::MemoryFlag_None);

	// NOTE(final): Allocate debug nodes
	debugState->maxFreeNodeCount = MAX_DEBUG_EVENT_COUNT / 2;
	debugState->nodes = PushArray(&debugState->debugMemory, DebugNode, debugState->maxFreeNodeCount);
	ResetDebugNodes(debugState);
}

external void DEBUGFrameEnd() {
	Assert(globalDebugTable);
	globalDebugTable->currentEventArrayIndex = !globalDebugTable->currentEventArrayIndex;
	U64 arrayIndex_EventIndex = AtomicExchangeU64(&globalDebugTable->eventArrayIndex_EventIndex, (U64)globalDebugTable->currentEventArrayIndex << 32);
	U32 eventArrayIndex = arrayIndex_EventIndex >> 32;
	Assert(eventArrayIndex <= 1);
	U32 eventCount = arrayIndex_EventIndex & 0xFFFFFFFF;

	Assert(globalDebugMemory);
	DebugState *debugState = (DebugState *)globalDebugMemory->storageBase;
	Assert(debugState);

	ResetDebugNodes(debugState);

	DebugNode *rootNode = &debugState->rootNode;
	DebugNode *curParentNode = rootNode;
	for (U32 eventIndex = 0; eventIndex < eventCount; ++eventIndex) {
		DebugEvent *ev = globalDebugTable->events[eventArrayIndex] + eventIndex;

		if (ev->type == DebugType::DebugType_BeginBlock) {
			// Find last child from parent
			Assert(curParentNode);
			DebugNode *lastChild = curParentNode->firstChild;
			while (lastChild) {
				if (!lastChild->nextSibling) {
					break;
				}
				lastChild = lastChild->nextSibling;
			}

			// Get a new node from the free list
			DebugNode *newNode = GetNewDebugNode(debugState);
			newNode->guid = ev->guid;
			newNode->threadID = ev->threadID;
			newNode->coreIndex = ev->coreIndex;
			newNode->parent = curParentNode;
			newNode->startClock = ev->clock;

			if (!lastChild) {
				// First child
				curParentNode->firstChild = newNode;
			} else {
				// Add another child as a sibling
				lastChild->nextSibling = newNode;
			}

			// Our new node is now the current parent
			curParentNode = newNode;
		} else if (ev->type == DebugType::DebugType_EndBlock) {
			Assert(curParentNode);
			curParentNode->endClock = ev->clock;
			Assert(curParentNode->parent);
			curParentNode = curParentNode->parent;
		}
	}
}

external void DEBUGRender(RenderState *renderState) {
	TIMED_FUNCTION();

	Assert(globalDebugMemory);
	DebugState *debugState = (DebugState *)globalDebugMemory->storageBase;
	Assert(debugState);
}