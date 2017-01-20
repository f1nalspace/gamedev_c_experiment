#include "editor.h"

#include "editor_internal.h"

internal void InitGame(GameState *gameState) {
	// NOTE(final): Define number of tiles to fit on screen with 100% scale
	gameState->tileSize = 1.0f;
	gameState->areaTileCount = V2i(16, 10);
	gameState->areaSize = V2((F32)gameState->areaTileCount.x, (F32)gameState->areaTileCount.y) * gameState->tileSize;

	// NOTE(final): Define our initial camera parameters
	gameState->cameraOffset = V2(0, 0);
	gameState->cameraScale = 1.0f;

	Tile *tilesForPool = PushArray(&gameState->persistentMemory, Tile, EDITOR_MAX_TILE_POOL_CAPACITY);
	gameState->tilesPool.Init();
	for (U32 tileIndex = 0; tileIndex < EDITOR_MAX_TILE_POOL_CAPACITY; ++tileIndex) {
		Tile *tile = tilesForPool + tileIndex;
		gameState->tilesPool.PushBack(tile);
	}
	gameState->usedTiles.Init();
}

internal Vec2f GetMouseWorld(GameState *gameState, InputState *inputState) {
	Vec2f mousePos = (inputState->mouse.mousePos - gameState->cameraOffset * gameState->cameraScale) * (1.0f / gameState->cameraScale);
	return(mousePos);
}

inline U32 EditorTileComputeIndex(S32 tileX, S32 tileY) {
	U32 remappedX = tileX + (EDITOR_MAX_TILE_DIMENSION - 1);
	U32 remappedY = tileY + (EDITOR_MAX_TILE_DIMENSION - 1);
	S32 fullWidth = EDITOR_MAX_TILE_DIMENSION * 2;
	U32 tileIndex = remappedY * fullWidth + remappedX;
	return(tileIndex);
}

inline Tile *EditorTileGet(GameState *gameState, S32 tileX, S32 tileY) {
	Tile *tile = 0;
	U32 tileIndex = EditorTileComputeIndex(tileX, tileY);
	if (gameState->tilesMap[tileIndex]) {
		tile = gameState->tilesPool.PopBack();
	}
	return(tile);
}

internal void EditorTileAdd(GameState *gameState, S32 tileX, S32 tileY) {
	Assert((tileX >= -(EDITOR_MAX_TILE_DIMENSION - 1) || tileX <= (EDITOR_MAX_TILE_DIMENSION - 1)));
	Assert((tileY >= -(EDITOR_MAX_TILE_DIMENSION - 1) || tileY <= (EDITOR_MAX_TILE_DIMENSION - 1)));
	U32 tileIndex = EditorTileComputeIndex(tileX, tileY);
	if (!gameState->tilesMap[tileIndex]) {
		Tile *tile = gameState->tilesPool.PopBack();
		*tile = {};
		tile->tilePos = V2i(tileX, tileY);
		gameState->usedTiles.PushBack(tile);
		gameState->tilesMap[tileIndex] = tile;
	}
}

internal void EditorTileRemove(GameState *gameState, S32 tileX, S32 tileY) {
	Assert((tileX >= -(EDITOR_MAX_TILE_DIMENSION - 1) || tileX <= (EDITOR_MAX_TILE_DIMENSION - 1)));
	Assert((tileY >= -(EDITOR_MAX_TILE_DIMENSION - 1) || tileY <= (EDITOR_MAX_TILE_DIMENSION - 1)));
	U32 tileIndex = EditorTileComputeIndex(tileX, tileY);
	if (gameState->tilesMap[tileIndex]) {
		Tile *tile = gameState->tilesMap[tileIndex];
		gameState->tilesMap[tileIndex] = 0;
		gameState->usedTiles.Remove(tile);
		*tile = {};
		gameState->tilesPool.PushBack(tile);
	}
}

external void GameUpdateAndRender(AppState *appState, RenderState *renderState, InputState *inputState) {
	GameState *gameState = (GameState *)appState->persistentStorageBase;
	TransientState *tranState = (TransientState *)appState->transientStorageBase;

	if (!tranState->isInitialized) {
		// NOTE(final): Initialize transient state
		*tranState = {};
		tranState->transientMemory = MemoryBlockCreate((U8 *)appState->transientStorageBase + sizeof(*tranState), appState->transientStorageSize - sizeof(*tranState), MemoryFlag::MemoryFlag_None);
		tranState->isInitialized = true;
	}

	if (!gameState->isInitialized) {
		// NOTE(final): Initialize editor state
		*gameState = {};
		gameState->persistentMemory = MemoryBlockCreate((U8 *)appState->persistentStorageBase + sizeof(GameState), appState->persistentStorageSize - sizeof(GameState), MemoryFlag::MemoryFlag_None);
		InitGame(gameState);
		gameState->isInitialized = true;

		// NOTE(final): Set area dimension and aspect ratio - This will never change
		renderState->areaSize = gameState->areaSize;
		renderState->aspectRatio = renderState->areaSize.w / renderState->areaSize.h;
	}

	// NOTE(final): Calculate target viewport based on area size, screen dimension and aspect ratio
	Vec2i *viewportSize = &renderState->viewportSize;
	Vec2i *viewportOffset = &renderState->viewportOffset;
	renderState->areaScale = (F32)renderState->screenSize.w / renderState->areaSize.w;
	renderState->viewportOffset = V2i();
	viewportSize->w = renderState->screenSize.w;
	viewportSize->h = (U32)(renderState->screenSize.w / renderState->aspectRatio);
	if (viewportSize->h > renderState->screenSize.h) {
		viewportSize->h = renderState->screenSize.h;
		viewportSize->w = (U32)(viewportSize->h * renderState->aspectRatio);
		renderState->areaScale = (F32)viewportSize->w / renderState->areaSize.w;
	}
	renderState->viewportOffset.x = (renderState->screenSize.w - viewportSize->w) / 2;
	renderState->viewportOffset.y = (renderState->screenSize.h - viewportSize->h) / 2;

	gameState->cameraTransform = TransformMake(gameState->cameraOffset, 0.0f, gameState->cameraScale);

	RenderPushClear(renderState, V4(0, 0, 0, 1));

	Vec2f verts[4] = {
		V2(gameState->areaSize.x, gameState->areaSize.y) * 0.5f,
		V2(-gameState->areaSize.x, gameState->areaSize.y) * 0.5f,
		V2(-gameState->areaSize.x, -gameState->areaSize.y) * 0.5f,
		V2(gameState->areaSize.x, -gameState->areaSize.y) * 0.5f,
	};
	Transform linesTransform = gameState->cameraTransform;
	RenderPushLines(renderState, linesTransform, ArrayCount(verts), verts, true, V4(1, 0, 1, 1), 2.0f);

	Vec2f mousePos = GetMouseWorld(gameState, inputState);

	enum PenState {
		PenState_None = 0,

		PenState_Started,
		PenState_Changed,
	};

	constant U32 MOUSEFLAG_PAN = 1;
	constant U32 MOUSEFLAG_ZOOM = 2;
	constant U32 MOUSEFLAG_DRAW = 4;

	U32 mouseInteractionFlags = 0;
	Vec2f panDelta = V2();
	F32 zoomDelta = 0;
	PenState penState = PenState::PenState_None;

	if (InputButtonIsDown(inputState->mouse.buttons[MouseButton::MouseButton_Right])) {
		if (!gameState->rightMouseDown) {
			gameState->rightMouseDown = true;
			gameState->rightMouseStart = inputState->mouse.mousePos;
		} else {
			Vec2f delta = inputState->mouse.mousePos - gameState->rightMouseStart;
			panDelta = delta;
			mouseInteractionFlags |= MOUSEFLAG_PAN;
			gameState->rightMouseStart = inputState->mouse.mousePos;
		}
	} else {
		if (gameState->rightMouseDown) {
			gameState->rightMouseDown = false;
		}
	}

	if (InputButtonIsDown(inputState->mouse.buttons[MouseButton::MouseButton_Left])) {
		mouseInteractionFlags |= MOUSEFLAG_DRAW;
		if (!gameState->leftMouseDown) {
			gameState->leftMouseDown = true;
			gameState->leftMouseStart = inputState->mouse.mousePos;
			penState = PenState::PenState_Started;
		} else {
			penState = PenState::PenState_Changed;
			gameState->leftMouseStart = inputState->mouse.mousePos;
		}
	} else {
		if (gameState->leftMouseDown) {
			gameState->leftMouseDown = false;
		}
	}

	if (Abs(inputState->mouse.wheelDelta) > 0) {
		mouseInteractionFlags |= MOUSEFLAG_ZOOM;
		zoomDelta = inputState->mouse.wheelDelta;
	}

	// NOTE(final): Move camera offset while holding down the mouse right button
	if (mouseInteractionFlags & MOUSEFLAG_PAN) {
		gameState->cameraOffset += panDelta * (1.0f / gameState->cameraScale);
	}
	// NOTE(final): Inc/Decrease the camera scale by the mouse wheel delta
	if (mouseInteractionFlags & MOUSEFLAG_ZOOM) {
		F32 zoomIncrease = 0.1f;
		gameState->cameraScale = ScalarClamp(gameState->cameraScale + inputState->mouse.wheelDelta * zoomIncrease, 0.1f, 10.0f);
	}

	F32 tileSize = gameState->tileSize;

	Vec2i mousePosInTiles = V2i(FloorF32ToS32(mousePos.x / tileSize), FloorF32ToS32(mousePos.y / tileSize));
	Vec2f tileDimension = V2(tileSize, tileSize);
	Vec2f mouseTilePos = Vec2Hadamard(V2((F32)mousePosInTiles.x, (F32)mousePosInTiles.y), tileDimension) + tileDimension * 0.5f;

	// NOTE(final): Draw tiles
	if (mouseInteractionFlags & MOUSEFLAG_DRAW) {
		if (penState == PenState::PenState_Started) {
			Tile *existingTile = EditorTileGet(gameState, mousePosInTiles.x, mousePosInTiles.y);
			gameState->activeDrawType = existingTile ? EditorDrawType::EditorDrawType_Remove : EditorDrawType::EditorDrawType_Add;
		} else {
			if (gameState->activeDrawType == EditorDrawType::EditorDrawType_Add) {
				EditorTileAdd(gameState, mousePosInTiles.x, mousePosInTiles.y);
			} else if (gameState->activeDrawType == EditorDrawType::EditorDrawType_Remove) {
				EditorTileRemove(gameState, mousePosInTiles.x, mousePosInTiles.y);
			}
		}
	} else {
		gameState->activeDrawType = EditorDrawType::EditorDrawType_None;
	}

	Vec2f tileBounds[4] = {
		V2(tileSize, tileSize) * 0.5f,
		V2(-tileSize, tileSize) * 0.5f,
		V2(-tileSize, -tileSize) * 0.5f,
		V2(tileSize, -tileSize) * 0.5f,
	};

	// NOTE(final): Render grid
	// It should render the necessary lines to fill out the scaled/panned game area only
	Vec2f scaledHalfGridSize = gameState->areaSize * 0.5f * (1.0f / gameState->cameraScale);

	Vec4f gridLineColor = V4(0.55f, 0.55f, 0.55f, 1);
	F32 gridLineWidth = 1.0f;
	U32 halfLineCountX = FloorF32ToS32(scaledHalfGridSize.x / tileSize) + 1;
	U32 halfLineCountY = FloorF32ToS32(scaledHalfGridSize.y / tileSize) + 1;

	Vec2f gridSize = V2(halfLineCountX * tileSize, halfLineCountY * tileSize) * 2.0f;
	Vec2i panOffsetInTiles = V2i(FloorF32ToS32(gameState->cameraOffset.x / tileSize), FloorF32ToS32(gameState->cameraOffset.y / tileSize));
	Vec2f panOffset = -V2(panOffsetInTiles.x * tileSize, panOffsetInTiles.y * tileSize);
	Vec2f gridOffset = -gridSize * 0.5f + panOffset;
	Transform gridTransform = TransformMult(TransformMakeTranslation(gridOffset), gameState->cameraTransform);

	U32 lineCountX = halfLineCountX * 2;
	U32 lineCountY = halfLineCountY * 2;

	// NOTE(final): Draw used tiles
	for (Tile *tile = (Tile *)gameState->usedTiles.next; tile != (Tile *)&gameState->usedTiles; tile = (Tile *)tile->next) {
		Vec2f tilePos = V2((F32)tile->tilePos.x, (F32)tile->tilePos.y) * tileSize + V2(0.5f) * tileSize;
		Transform tileTransform = TransformMult(TransformMakeTranslation(tilePos), gameState->cameraTransform);
		RenderPushPolygon(renderState, tileTransform, 4, tileBounds);
	}

	TemporaryMemory tempMemory = TemporaryMemoryBegin(&tranState->transientMemory);
	Vec2f *gridLinePoints = PushArray(&tranState->transientMemory, Vec2f, Max(lineCountX, lineCountY) * 2);

	// Horizontal lines
	for (U32 verticalLineIndex = 0; verticalLineIndex < lineCountY; ++verticalLineIndex) {
		F32 yPos = verticalLineIndex * tileSize;
		gridLinePoints[verticalLineIndex * 2 + 0] = V2(gridSize.x, yPos);
		gridLinePoints[verticalLineIndex * 2 + 1] = V2(0, yPos);
	}
	U32 lineVertexCount = lineCountY * 2;
	U32 lineBatchCount = lineVertexCount / MAX_RENDER_LINES_VERTEX_COUNT + 1;
	for (U32 lineVertexStartIndex = 0; lineVertexStartIndex < lineVertexCount; lineVertexStartIndex += MAX_RENDER_LINES_VERTEX_COUNT) {
		U32 vertexCount = Min((lineVertexCount - lineVertexStartIndex), MAX_RENDER_LINES_VERTEX_COUNT);
		RenderPushLines(renderState, gridTransform, vertexCount, gridLinePoints + lineVertexStartIndex, false, gridLineColor, gridLineWidth);
	}

	// Vertical lines
	for (U32 horizontalLineIndex = 0; horizontalLineIndex < lineCountX; ++horizontalLineIndex) {
		F32 xPos = horizontalLineIndex * tileSize;
		gridLinePoints[horizontalLineIndex * 2 + 0] = V2(xPos, gridSize.y);
		gridLinePoints[horizontalLineIndex * 2 + 1] = V2(xPos, 0);
	}
	lineVertexCount = lineCountX * 2;
	lineBatchCount = lineVertexCount / MAX_RENDER_LINES_VERTEX_COUNT + 1;
	for (U32 lineVertexStartIndex = 0; lineVertexStartIndex < lineVertexCount; lineVertexStartIndex += MAX_RENDER_LINES_VERTEX_COUNT) {
		U32 vertexCount = Min((lineVertexCount - lineVertexStartIndex), MAX_RENDER_LINES_VERTEX_COUNT);
		RenderPushLines(renderState, gridTransform, vertexCount, gridLinePoints + lineVertexStartIndex, false, gridLineColor, gridLineWidth);
	}

	TemporaryMemoryEnd(&tempMemory);

	// NOTE(final): Draw mouse tile
	Transform mouseTileTransform = TransformMult(TransformMakeTranslation(mouseTilePos), gameState->cameraTransform);
	RenderPushLines(renderState, mouseTileTransform, ArrayCount(tileBounds), tileBounds, true, V4(1, 1, 0, 1));
}