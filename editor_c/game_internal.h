#pragma once

#include "engine_math.h"
#include "engine_memory.h"
#include "engine_list.h"
#include "engine_physics.h"

struct TransientState {
	B32 isInitialized;

	MemoryBlock transientMemory;
};

struct Tile : LinkedListItem {
	Vec2i tilePos;
	Body *body;
};
StaticAlignmentAssert(Tile);

enum EditorDrawType {
	EditorDrawType_None,
	EditorDrawType_Add,
	EditorDrawType_Remove,
};

constant U32 EDITOR_MAX_TILE_POOL_CAPACITY = 2048;
constant U32 EDITOR_MAX_TILE_DIMENSION = 256;
constant U32 EDITOR_MAX_TILE_MAP_COUNT = EDITOR_MAX_TILE_DIMENSION * EDITOR_MAX_TILE_DIMENSION;

struct Camera {
	Vec2f offset;
	F32 scale;
	Transform transform;
};

struct EditorState {
	EditorDrawType activeDrawType;
	Tile *tilesBase;
	LinkedList<Tile> tilesPool;
	LinkedList<Tile> usedTiles;
	Tile *tilesMap[EDITOR_MAX_TILE_MAP_COUNT];

	Camera camera;

	B32 leftMouseDown;
	Vec2f leftMouseStart;

	B32 rightMouseDown;
	Vec2f rightMouseStart;
};

struct GameState {
	B32 isInitialized;

	MemoryBlock persistentMemory;

	Vec2f tileSize;
	Vec2i areaTileCount;
	Vec2f areaSize;

	Camera camera;

	EditorState editor;

	B32 editorActive;

	Physics physics;

	Body *playerBody;
};