#pragma once

#include "engine_math.h"
#include "engine_memory.h"

struct TransientState {
	B32 isInitialized;

	MemoryBlock transientMemory;
};

struct EditorState {
	B32 isInitialized;

	MemoryBlock persistentMemory;

	F32 tileSize;
	Vec2i areaTileCount;
	Vec2f areaSize;

	Vec2f cameraOffset;
	F32 cameraScale;
	Transform cameraTransform;

	B32 leftMouseDown;
	Vec2f leftMouseStart;

	B32 rightMouseDown;
	Vec2f rightMouseStart;
};