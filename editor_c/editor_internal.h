#pragma once

#include "engine_math.h"
#include "engine_memory.h"
#include "engine_list.h"

struct TransientState {
	B32 isInitialized;

	MemoryBlock transientMemory;
};

struct Tile : LinkedListItem {
	Vec2i tilePos;
};

enum EditorDrawType {
	EditorDrawType_None,
	EditorDrawType_Add,
	EditorDrawType_Remove,
};

constant U32 EDITOR_MAX_TILE_POOL_CAPACITY = 2048;
constant U32 EDITOR_MAX_TILE_DIMENSION = 128;
constant U32 EDITOR_MAX_TILE_MAP_COUNT = EDITOR_MAX_TILE_DIMENSION * EDITOR_MAX_TILE_DIMENSION;

struct GameState {
	B32 isInitialized;

	MemoryBlock persistentMemory;

	F32 tileSize;
	Vec2i areaTileCount;
	Vec2f areaSize;

	U8 activeDrawType;
	LinkedList<Tile> tilesPool;
	LinkedList<Tile> usedTiles;
	Tile *tilesMap[EDITOR_MAX_TILE_MAP_COUNT];

	Vec2f cameraOffset;
	F32 cameraScale;
	Transform cameraTransform;

	B32 leftMouseDown;
	Vec2f leftMouseStart;

	B32 rightMouseDown;
	Vec2f rightMouseStart;
};