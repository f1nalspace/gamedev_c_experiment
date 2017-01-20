#pragma once

#include "engine_math.h"

enum RenderCommandType {
	RenderCommandType_None,

	RenderCommandType_Clear,
	RenderCommandType_Lines,
	RenderCommandType_Polygon,
};

constant U32 MAX_RENDER_LINES_VERTEX_COUNT = 16 * 2;
struct RenderCommandLines {
	Vec2f verts[MAX_RENDER_LINES_VERTEX_COUNT];
	U32 vertexCount;
	B32 isChained;
	F32 lineWidth;
};

constant U32 MAX_RENDER_POLGYON_VERTEX_COUNT = 16;
struct RenderCommandPolygon {
	Vec2f verts[MAX_RENDER_POLGYON_VERTEX_COUNT];
	U32 vertexCount;
};

struct RenderCommand {
	U8 type;
	// FIXME(final): Transform should be the final modelview matrix, the render itself should never require a Transform!
	Transform transform;
	Vec4f color;
	union {
		RenderCommandLines lines;
		RenderCommandPolygon polygon;
	};
};

constant U32 RENDER_MAX_COMMAND_COUNT = 1024;
struct RenderState {
	Vec2i screenSize;
	F32 aspectRatio;

	F32 areaScale;
	Vec2f areaSize;

	Vec2i viewportSize;
	Vec2i viewportOffset;

	U32 commandCapacity;
	RenderCommand *commands;
	U32 commandCount;
};

inline RenderCommand *RenderPushCommand(RenderState *renderState, RenderCommandType type, const Transform &transform = TransformIdentity()) {
	Assert(renderState->commandCount < renderState->commandCapacity);
	RenderCommand *result = renderState->commands + renderState->commandCount;
	*result = {};
	result->transform = transform;
	result->type = type;
	++renderState->commandCount;
	return(result);
}

inline void RenderPushClear(RenderState *renderState, const Vec4f &color = V4(0, 0, 0, 1)) {
	RenderCommand *command = RenderPushCommand(renderState, RenderCommandType::RenderCommandType_Clear);
	command->color = color;
}
inline void RenderPushLines(RenderState *renderState, const Transform &transform, U32 vertexCount, Vec2f *verts, B32 isChained, const Vec4f &color = V4(1,1,1,1), F32 lineWidth = 1.0f) {
	Assert(vertexCount > 1);
	Assert(verts);
	RenderCommand *command = RenderPushCommand(renderState, RenderCommandType::RenderCommandType_Lines, transform);
	command->color = color;
	RenderCommandLines *lines = &command->lines;
	Assert(vertexCount <= ArrayCount(lines->verts));
	lines->vertexCount = vertexCount;
	lines->isChained = isChained;
	lines->lineWidth = lineWidth;
	for (U32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
		lines->verts[vertexIndex] = verts[vertexIndex];
	}
}
inline void RenderPushPolygon(RenderState *renderState, const Transform &transform, U32 vertexCount, Vec2f *verts, const Vec4f &color = V4(1, 1, 1, 1)) {
	Assert(vertexCount > 2);
	Assert(verts);
	RenderCommand *command = RenderPushCommand(renderState, RenderCommandType::RenderCommandType_Polygon, transform);
	command->color = color;
	RenderCommandPolygon *polygon = &command->polygon;
	Assert(vertexCount <= ArrayCount(polygon->verts));
	polygon->vertexCount = vertexCount;
	for (U32 vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
		polygon->verts[vertexIndex] = verts[vertexIndex];
	}
}

inline Vec2i RenderProject(RenderState *renderState, F32 x, F32 y) {
	Vec2i result;
	// FIXME(final): This is totally wrong - include the camera scale and offset to fix it
	result.x = (S32)(x * renderState->areaScale) + renderState->viewportOffset.x;
	result.x = (S32)(y * renderState->areaScale) + renderState->viewportOffset.y;
	return (result);
}

inline Vec2f RenderUnproject(RenderState *renderState, S32 x, S32 y) {
	Vec2f result = {};
	if (renderState->areaScale > 0) {
		result.x = (F32)((x - renderState->viewportOffset.x) / renderState->areaScale) - renderState->areaSize.w * 0.5f;
		result.y = (F32)((y - renderState->viewportOffset.y) / renderState->areaScale) - renderState->areaSize.h * 0.5f;
	}
	return (result);
}