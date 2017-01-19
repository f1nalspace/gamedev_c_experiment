#include "editor.h"

#include "editor_internal.h"

internal void InitEditor(EditorState *editorState) {
	// NOTE: Define number of tiles to fit on screen with 100% scale
	editorState->tileSize = 1.0f;
	editorState->areaTileCount = V2i(16, 10);
	editorState->areaSize = V2((F32)editorState->areaTileCount.x, (F32)editorState->areaTileCount.y) * editorState->tileSize;

	// NOTE: Define our initial camera parameters
	editorState->cameraOffset = V2(0, 0);
	editorState->cameraScale = 1.0f;
}

internal Vec2f GetMouseWorld(EditorState *editorState, InputState *inputState) {
	Vec2f mousePos = (inputState->mouse.mousePos - editorState->cameraOffset * editorState->cameraScale) * (1.0f / editorState->cameraScale);
	return(mousePos);
}

internal void RenderEditor(EditorState *editorState, RenderState *renderState) {
}

external void UpdateAndRenderEditor(AppState *appState, RenderState *renderState, InputState *inputState) {
	EditorState *editorState = (EditorState *)appState->persistentStorageBase;
	TransientState *tranState = (TransientState *)appState->transientStorageBase;

	if (!tranState->isInitialized) {
		// NOTE(final): Initialize transient state
		*tranState = {};
		tranState->transientMemory = CreateMemoryBlock((U8 *)appState->transientStorageBase + sizeof(*tranState), appState->transientStorageSize - sizeof(*tranState), MemoryFlag::MemoryFlag_None);
		tranState->isInitialized = true;
	}

	if (!editorState->isInitialized) {
		// NOTE(final): Initialize editor state
		*editorState = {};
		editorState->persistentMemory = CreateMemoryBlock((U8 *)appState->persistentStorageBase + sizeof(EditorState), appState->persistentStorageSize - sizeof(EditorState), MemoryFlag::MemoryFlag_None);
		InitEditor(editorState);
		editorState->isInitialized = true;

		// NOTE(final): Set area dimension and aspect ratio - This will never change
		renderState->areaSize = editorState->areaSize;
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

	editorState->cameraTransform = MakeTransform2(editorState->cameraOffset, 0.0f, editorState->cameraScale);

	PushClear(renderState, V4(0, 0, 0, 1));

	Vec2f verts[4] = {
		V2(editorState->areaSize.x, editorState->areaSize.y) * 0.5f,
		V2(-editorState->areaSize.x, editorState->areaSize.y) * 0.5f,
		V2(-editorState->areaSize.x, -editorState->areaSize.y) * 0.5f,
		V2(editorState->areaSize.x, -editorState->areaSize.y) * 0.5f,
	};
	Transform linesTransform = editorState->cameraTransform;
	PushLines(renderState, linesTransform, ArrayCount(verts), verts, true, V4(1, 0, 1, 1), 2.0f);

	Vec2f mousePos = GetMouseWorld(editorState, inputState);

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

	if (IsDown(inputState->mouse.buttons[MouseButton::MouseButton_Right])) {
		if (!editorState->rightMouseDown) {
			editorState->rightMouseDown = true;
			editorState->rightMouseStart = inputState->mouse.mousePos;
		} else {
			Vec2f delta = inputState->mouse.mousePos - editorState->rightMouseStart;
			panDelta = delta;
			mouseInteractionFlags |= MOUSEFLAG_PAN;
			editorState->rightMouseStart = inputState->mouse.mousePos;
		}
	} else {
		if (editorState->rightMouseDown) {
			editorState->rightMouseDown = false;
		}
	}

	if (IsDown(inputState->mouse.buttons[MouseButton::MouseButton_Left])) {
		mouseInteractionFlags |= MOUSEFLAG_DRAW;
		if (!editorState->leftMouseDown) {
			editorState->leftMouseDown = true;
			editorState->leftMouseStart = inputState->mouse.mousePos;
			penState = PenState::PenState_Started;
		} else {
			penState = PenState::PenState_Changed;
			editorState->leftMouseStart = inputState->mouse.mousePos;
		}
	} else {
		if (editorState->leftMouseDown) {
			editorState->leftMouseDown = false;
		}
	}

	if (Abs(inputState->mouse.wheelDelta) > 0) {
		mouseInteractionFlags |= MOUSEFLAG_ZOOM;
		zoomDelta = inputState->mouse.wheelDelta;
	}

	// NOTE(final): Move camera offset while holding down the mouse right button
	if (mouseInteractionFlags & MOUSEFLAG_PAN) {
		editorState->cameraOffset += panDelta * (1.0f / editorState->cameraScale);
	}
	// NOTE(final): Inc/Decrease the camera scale by the mouse wheel delta
	if (mouseInteractionFlags & MOUSEFLAG_ZOOM) {
		F32 zoomIncrease = 0.1f;
		editorState->cameraScale = Clamp(editorState->cameraScale + inputState->mouse.wheelDelta * zoomIncrease, 0.1f, 10.0f);
	}

	F32 tileSize = editorState->tileSize;

	Vec2i mousePosInTiles = V2i(FloorF32ToS32(mousePos.x / tileSize), FloorF32ToS32(mousePos.y / tileSize));
	Vec2f tileDimension = V2(tileSize, tileSize);
	Vec2f mouseTilePos = Hadamard(V2((F32)mousePosInTiles.x, (F32)mousePosInTiles.y), tileDimension) + tileDimension * 0.5f;

	Vec2f tileBounds[4] = {
		V2(tileSize, tileSize) * 0.5f,
		V2(-tileSize, tileSize) * 0.5f,
		V2(-tileSize, -tileSize) * 0.5f,
		V2(tileSize, -tileSize) * 0.5f,
	};

	// NOTE(final): Render grid
	// It should render the necessary lines to fill out the scaled/panned game area only
	Vec2f scaledHalfGridSize = editorState->areaSize * 0.5f * (1.0f / editorState->cameraScale);

	Vec4f gridLineColor = V4(0.55f, 0.55f, 0.55f, 1);
	F32 gridLineWidth = 1.0f;
	U32 halfLineCountX = FloorF32ToS32(scaledHalfGridSize.x / tileSize) + 1;
	U32 halfLineCountY = FloorF32ToS32(scaledHalfGridSize.y / tileSize) + 1;

	Vec2f gridSize = V2(halfLineCountX * tileSize, halfLineCountY * tileSize) * 2.0f;
	Vec2i panOffsetInTiles = V2i(FloorF32ToS32(editorState->cameraOffset.x / tileSize), FloorF32ToS32(editorState->cameraOffset.y / tileSize));
	Vec2f panOffset = -V2(panOffsetInTiles.x * tileSize, panOffsetInTiles.y * tileSize);
	Vec2f gridOffset = -gridSize * 0.5f + panOffset;
	Transform gridTransform = MultTransform2(MakeTranslation2(gridOffset), editorState->cameraTransform);

	U32 lineCountX = halfLineCountX * 2;
	U32 lineCountY = halfLineCountY * 2;

	memory_size tempMemorySize = Max(lineCountX, lineCountY) * 2 * sizeof(Vec2f);
	TemporaryMemory tempMemory = BeginTemporaryMemory(&tranState->transientMemory);

	Vec2f *gridLinePoints = PushArray(&tranState->transientMemory, Vec2f, Max(lineCountX, lineCountY) * 2);

	for (U32 verticalLineIndex = 0; verticalLineIndex < lineCountY; ++verticalLineIndex) {
		F32 yPos = verticalLineIndex * tileSize;
		gridLinePoints[verticalLineIndex * 2 + 0] = V2(gridSize.x, yPos);
		gridLinePoints[verticalLineIndex * 2 + 1] = V2(0, yPos);
	}
	PushLines(renderState, gridTransform, lineCountY * 2, gridLinePoints, false, gridLineColor, gridLineWidth);

	for (U32 horizontalLineIndex = 0; horizontalLineIndex < lineCountX; ++horizontalLineIndex) {
		F32 xPos = horizontalLineIndex * tileSize;
		gridLinePoints[horizontalLineIndex * 2 + 0] = V2(xPos, gridSize.y);
		gridLinePoints[horizontalLineIndex * 2 + 1] = V2(xPos, 0);
	}
	PushLines(renderState, gridTransform, lineCountX * 2, gridLinePoints, false, gridLineColor, gridLineWidth);

	EndTemporaryMemory(&tempMemory);

	// NOTE(final): Draw mouse tile
	Transform mouseTileTransform = MultTransform2(MakeTranslation2(mouseTilePos), editorState->cameraTransform);
	PushLines(renderState, mouseTileTransform, ArrayCount(tileBounds), tileBounds, true, V4(1, 1, 0, 1));
}