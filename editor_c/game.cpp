#include "game.h"

#include "game_internal.h"

internal void GameInit(GameState *gameState) {
	// NOTE(final): Define number of tiles to fit on screen with 100% scale
	gameState->tileSize = V2(1.0f, 1.0f);
	gameState->areaTileCount = V2i(16, 10);
	gameState->areaSize = Vec2Hadamard(V2((F32)gameState->areaTileCount.x, (F32)gameState->areaTileCount.y), gameState->tileSize);

	gameState->camera.offset = V2(0, 0);
	gameState->camera.scale = 1.0f;
	gameState->editor.camera.offset = V2(0, 0);
	gameState->editor.camera.scale = 1.0f;

	gameState->editorActive = true;

	// NOTE(final): Initialize editor tiles
	gameState->editor.tilesBase = PushArray(&gameState->persistentMemory, Tile, EDITOR_MAX_TILE_POOL_CAPACITY);
	gameState->editor.tilesPool.Init();
	for (U32 tileIndex = 0; tileIndex < EDITOR_MAX_TILE_POOL_CAPACITY; ++tileIndex) {
		Tile *tile = gameState->editor.tilesBase + tileIndex;
		gameState->editor.tilesPool.PushBack(tile);
	}
	gameState->editor.usedTiles.Init();

	// NOTE(final): Init physics system
	memory_size physicsMemorySize = MegaBytes(32);
	gameState->physics.physicsMemory = MemoryBlockCreateFrom(&gameState->persistentMemory, physicsMemorySize);
	PhysicsInit(&gameState->physics, V2(0, -0.25f));

	// NOTE(final): Add a player dynamic body
	Vec2f playerExt = V2(0.4f, 0.9f);
	Vec2f playerPos = V2(0, 0);
	gameState->playerBody = PhysicsBodyCreate(&gameState->physics, BodyType::BodyType_Dynamic, playerExt, playerPos, 1.0f);
}

internal Vec2f GameEditorMousePosGet(EditorState *editor, InputState *inputState) {
	Vec2f mousePos = (inputState->mouse.mousePos - editor->camera.offset * editor->camera.scale) * (1.0f / editor->camera.scale);
	return(mousePos);
}

inline U32 GameEditorTileIndexGet(S32 tileX, S32 tileY) {
	U32 halfDimension = EDITOR_MAX_TILE_DIMENSION / 2;
	U32 remappedX = tileX + (halfDimension - 1);
	U32 remappedY = tileY + (halfDimension - 1);
	U32 tileIndex = remappedY * EDITOR_MAX_TILE_DIMENSION + remappedX;
	return(tileIndex);
}

inline Tile *GameEditorTileGet(GameState *game, S32 tileX, S32 tileY) {
	EditorState *editor = &game->editor;
	Tile *tile = 0;
	U32 tileIndex = GameEditorTileIndexGet(tileX, tileY);
	if (editor->tilesMap[tileIndex]) {
		tile = editor->tilesPool.PopBack();
	}
	return(tile);
}

internal void GameEditorTileAdd(GameState *game, S32 tileX, S32 tileY) {
	EditorState *editor = &game->editor;
	U32 halfDimension = EDITOR_MAX_TILE_DIMENSION / 2;
	Assert((tileX >= -((S32)halfDimension - 1) || tileX <= ((S32)halfDimension - 1)));
	Assert((tileY >= -((S32)halfDimension - 1) || tileY <= ((S32)halfDimension - 1)));
	U32 tileIndex = GameEditorTileIndexGet(tileX, tileY);
	if (!editor->tilesMap[tileIndex]) {
		Tile *tile = editor->tilesPool.PopBack();
		*tile = {};
		tile->tilePos = V2i(tileX, tileY);
		editor->usedTiles.PushBack(tile);
		editor->tilesMap[tileIndex] = tile;

		// NOTE(final): Add static body for that tile
		Vec2f tileWorldPos = Vec2Hadamard(V2((F32)tileX, (F32)tileY), game->tileSize) + game->tileSize * 0.5f;
		tile->body = PhysicsBodyCreate(&game->physics, BodyType::BodyType_Static, game->tileSize * 0.5f, tileWorldPos, 0.0f);
	}

}

internal void GameEditorTileRemove(GameState *gameState, S32 tileX, S32 tileY) {
	EditorState *editor = &gameState->editor;
	U32 halfDimension = EDITOR_MAX_TILE_DIMENSION / 2;
	Assert((tileX >= -((S32)halfDimension - 1) || tileX <= ((S32)halfDimension - 1)));
	Assert((tileY >= -((S32)halfDimension - 1) || tileY <= ((S32)halfDimension - 1)));
	U32 tileIndex = GameEditorTileIndexGet(tileX, tileY);
	if (editor->tilesMap[tileIndex]) {
		Tile *tile = editor->tilesMap[tileIndex];

		Assert(tile->body);
		PhysicsBodyRemove(&gameState->physics, tile->body);

		editor->tilesMap[tileIndex] = 0;
		editor->usedTiles.Remove(tile);
		*tile = {};
		editor->tilesPool.PushBack(tile);
	}
}

internal void GamePhysicsRender(Physics *physics, RenderState *renderState, const Transform &cameraTransform) {
	Vec2f verts[4];
	for (U32 bodyIndex = 0; bodyIndex < physics->bodyCount; ++bodyIndex) {
		Body *body = physics->bodies[bodyIndex];
		Transform bodyTransform = TransformMult(TransformMakeTranslation(body->position), cameraTransform);

		verts[0] = V2(body->radius.x, body->radius.y);
		verts[1] = V2(-body->radius.x, body->radius.y);
		verts[2] = V2(-body->radius.x, -body->radius.y);
		verts[3] = V2(body->radius.x, -body->radius.y);

		Vec4f color = body->type == BodyType::BodyType_Static ? V4(1, 1, 1, 1) : V4(0, 0, 1, 1);
		RenderPushPolygon(renderState, bodyTransform, 4, verts, color);
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
		GameInit(gameState);
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

	RenderPushClear(renderState, V4(0, 0, 0, 1));

	if (InputButtonWasDown(inputState->keyboard.functionkeys[0])) {
		gameState->editorActive = !gameState->editorActive;
	}

	EditorState *editor = &gameState->editor;

	editor->camera.transform = TransformMake(editor->camera.offset, 0.0f, editor->camera.scale);
	gameState->camera.transform = TransformMake(gameState->camera.offset, 0.0f, gameState->camera.scale);

	if (gameState->editorActive) {
		Vec2f verts[4] = {
			V2(gameState->areaSize.x, gameState->areaSize.y) * 0.5f,
			V2(-gameState->areaSize.x, gameState->areaSize.y) * 0.5f,
			V2(-gameState->areaSize.x, -gameState->areaSize.y) * 0.5f,
			V2(gameState->areaSize.x, -gameState->areaSize.y) * 0.5f,
		};
		Transform linesTransform = editor->camera.transform;
		RenderPushLines(renderState, linesTransform, ArrayCount(verts), verts, true, V4(1, 0, 1, 1), 2.0f);

		Vec2f mousePos = GameEditorMousePosGet(editor, inputState);

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
			if (!editor->rightMouseDown) {
				editor->rightMouseDown = true;
				editor->rightMouseStart = inputState->mouse.mousePos;
			} else {
				Vec2f delta = inputState->mouse.mousePos - editor->rightMouseStart;
				panDelta = delta;
				mouseInteractionFlags |= MOUSEFLAG_PAN;
				editor->rightMouseStart = inputState->mouse.mousePos;
			}
		} else {
			if (editor->rightMouseDown) {
				editor->rightMouseDown = false;
			}
		}

		if (InputButtonIsDown(inputState->mouse.buttons[MouseButton::MouseButton_Left])) {
			mouseInteractionFlags |= MOUSEFLAG_DRAW;
			if (!editor->leftMouseDown) {
				editor->leftMouseDown = true;
				editor->leftMouseStart = inputState->mouse.mousePos;
				penState = PenState::PenState_Started;
			} else {
				penState = PenState::PenState_Changed;
				editor->leftMouseStart = inputState->mouse.mousePos;
			}
		} else {
			if (editor->leftMouseDown) {
				editor->leftMouseDown = false;
			}
		}

		if (Abs(inputState->mouse.wheelDelta) > 0) {
			mouseInteractionFlags |= MOUSEFLAG_ZOOM;
			zoomDelta = inputState->mouse.wheelDelta;
		}

		// NOTE(final): Move camera offset while holding down the mouse right button
		if (mouseInteractionFlags & MOUSEFLAG_PAN) {
			editor->camera.offset += panDelta * (1.0f / editor->camera.scale);
		}
		// NOTE(final): Inc/Decrease the camera scale by the mouse wheel delta
		if (mouseInteractionFlags & MOUSEFLAG_ZOOM) {
			F32 zoomIncrease = 0.1f;
			editor->camera.scale = ScalarClamp(editor->camera.scale + inputState->mouse.wheelDelta * zoomIncrease, 0.1f, 10.0f);
		}

		Vec2f tileSize = gameState->tileSize;
		Vec2i mousePosInTiles = V2i(FloorF32ToS32(mousePos.x / tileSize.x), FloorF32ToS32(mousePos.y / tileSize.y));
		Vec2f mouseTilePos = Vec2Hadamard(V2((F32)mousePosInTiles.x, (F32)mousePosInTiles.y), tileSize) + tileSize * 0.5f;

		// NOTE(final): Draw tiles
		if (mouseInteractionFlags & MOUSEFLAG_DRAW) {
			if (penState == PenState::PenState_Started) {
				Tile *existingTile = GameEditorTileGet(gameState, mousePosInTiles.x, mousePosInTiles.y);
				editor->activeDrawType = existingTile ? EditorDrawType::EditorDrawType_Remove : EditorDrawType::EditorDrawType_Add;
			} else {
				if (editor->activeDrawType == EditorDrawType::EditorDrawType_Add) {
					GameEditorTileAdd(gameState, mousePosInTiles.x, mousePosInTiles.y);
				} else if (editor->activeDrawType == EditorDrawType::EditorDrawType_Remove) {
					GameEditorTileRemove(gameState, mousePosInTiles.x, mousePosInTiles.y);
				}
			}
		} else {
			editor->activeDrawType = EditorDrawType::EditorDrawType_None;
		}

		Vec2f tileBounds[4] = {
			V2(tileSize.x, tileSize.y) * 0.5f,
			V2(-tileSize.x, tileSize.y) * 0.5f,
			V2(-tileSize.x, -tileSize.y) * 0.5f,
			V2(tileSize.x, -tileSize.y) * 0.5f,
		};

		// NOTE(final): Render grid (It should render the necessary lines to fill out the scaled/panned game area only)
		Vec2f scaledHalfGridSize = gameState->areaSize * 0.5f * (1.0f / editor->camera.scale);

		Vec4f gridLineColor = V4(0.55f, 0.55f, 0.55f, 1);
		F32 gridLineWidth = 1.0f;
		U32 halfLineCountX = FloorF32ToS32(scaledHalfGridSize.x / tileSize.x) + 1;
		U32 halfLineCountY = FloorF32ToS32(scaledHalfGridSize.y / tileSize.y) + 1;

		Vec2f gridSize = Vec2Hadamard(V2((F32)halfLineCountX, (F32)halfLineCountY), tileSize) * 2.0f;
		Vec2i panOffsetInTiles = V2i(FloorF32ToS32(editor->camera.offset.x / tileSize.x), FloorF32ToS32(editor->camera.offset.y / tileSize.y));
		Vec2f panOffset = -V2(panOffsetInTiles.x * tileSize.x, panOffsetInTiles.y * tileSize.y);
		Vec2f gridOffset = -gridSize * 0.5f + panOffset;
		Transform gridTransform = TransformMult(TransformMakeTranslation(gridOffset), editor->camera.transform);

		U32 lineCountX = halfLineCountX * 2;
		U32 lineCountY = halfLineCountY * 2;

		// NOTE(final): Draw used tiles
		for (Tile *tile = (Tile *)editor->usedTiles.next; tile != (Tile *)&editor->usedTiles; tile = (Tile *)tile->next) {
			Vec2f tilePos = Vec2Hadamard(V2((F32)tile->tilePos.x, (F32)tile->tilePos.y), tileSize) + tileSize * 0.5f;
			Transform tileTransform = TransformMult(TransformMakeTranslation(tilePos), editor->camera.transform);
			RenderPushPolygon(renderState, tileTransform, 4, tileBounds);
		}

		TemporaryMemory tempMemory = TemporaryMemoryBegin(&tranState->transientMemory);
		Vec2f *gridLinePoints = PushArray(&tranState->transientMemory, Vec2f, Max(lineCountX, lineCountY) * 2);

		// NOTE(final): Draw horizontal grid lines
		for (U32 verticalLineIndex = 0; verticalLineIndex < lineCountY; ++verticalLineIndex) {
			F32 yPos = verticalLineIndex * tileSize.y;
			gridLinePoints[verticalLineIndex * 2 + 0] = V2(gridSize.x, yPos);
			gridLinePoints[verticalLineIndex * 2 + 1] = V2(0, yPos);
		}
		U32 lineVertexCount = lineCountY * 2;
		U32 lineBatchCount = lineVertexCount / MAX_RENDER_LINES_VERTEX_COUNT + 1;
		for (U32 lineVertexStartIndex = 0; lineVertexStartIndex < lineVertexCount; lineVertexStartIndex += MAX_RENDER_LINES_VERTEX_COUNT) {
			U32 vertexCount = Min((lineVertexCount - lineVertexStartIndex), MAX_RENDER_LINES_VERTEX_COUNT);
			RenderPushLines(renderState, gridTransform, vertexCount, gridLinePoints + lineVertexStartIndex, false, gridLineColor, gridLineWidth);
		}

		// NOTE(final): Draw vertical grid lines
		for (U32 horizontalLineIndex = 0; horizontalLineIndex < lineCountX; ++horizontalLineIndex) {
			F32 xPos = horizontalLineIndex * tileSize.x;
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

		// NOTE(final): Draw mouse hover tile
		Transform mouseTileTransform = TransformMult(TransformMakeTranslation(mouseTilePos), editor->camera.transform);
		RenderPushLines(renderState, mouseTileTransform, ArrayCount(tileBounds), tileBounds, true, V4(1, 1, 0, 1));

		GamePhysicsRender(&gameState->physics, renderState, editor->camera.transform);
	} else {
		F32 moveSpeedX = 0.1f;
		F32 moveSpeedY = 0.5f;
		if (InputButtonIsDown(inputState->keyboard.moveRight)) {
			gameState->playerBody->velocity += V2(1, 0) * moveSpeedX;
		} else if (InputButtonIsDown(inputState->keyboard.moveLeft)) {
			gameState->playerBody->velocity += V2(-1, 0) * moveSpeedX;
		}
		if (InputButtonIsDown(inputState->keyboard.moveUp)) {
			gameState->playerBody->velocity += V2(0, 1) * moveSpeedY;
		} else if (InputButtonIsDown(inputState->keyboard.moveDown)) {
			gameState->playerBody->velocity += V2(0, -1) * moveSpeedY;
		}

		gameState->camera.offset = -gameState->playerBody->position;

		PhysicsUpdate(&gameState->physics, inputState);
		GamePhysicsRender(&gameState->physics, renderState, gameState->camera.transform);
	}
}