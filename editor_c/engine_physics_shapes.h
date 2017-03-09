#pragma once

#include "engine_types.h"
#include "engine_math.h"

constant U32 PHYSICS_MAX_EDGE_SHAPE_VERTEX_COUNT = 16;

enum ShapeType {
	ShapeType_None = 0,

	ShapeType_Plane = 1,
	ShapeType_LineSegment = 2,
	ShapeType_Box = 3,
	ShapeType_Polygon = 4,
	ShapeType_Circle = 5,

	ShapeType_Count,
};

struct PlaneShape {
	F32 len;
	// NOTE(final): Normal is computed on the fly based on the local and world rotation
};

struct EdgeShape {
	Vec2f localVerts[PHYSICS_MAX_EDGE_SHAPE_VERTEX_COUNT];
	U32 vertexCount;
};

struct LineSegmentShape : EdgeShape {
	F32 extend;
};

struct BoxShape : EdgeShape {
	Vec2f extend;
};

struct PolygonShape : EdgeShape {
};

struct CircleShape {
	F32 radius;
};

struct PhysicsMaterial {
	F32 density;
	F32 friction;
};

struct Shape {
	U32 shapeId;
	Transform localTransform;
	PhysicsMaterial material;
	ShapeType type;
	union {
		PlaneShape plane;
		LineSegmentShape lineSegment;
		BoxShape box;
		PolygonShape polygon;
		CircleShape circle;
	};
};

inline PhysicsMaterial MakePhysicsMaterial(float density = 1.0f, float friction = 0.3f) {
	PhysicsMaterial result;
	result.density = density;
	result.friction = friction;
	return(result);
}

inline PhysicsMaterial StaticMaterial(float friction = 0.0f) {
	PhysicsMaterial result = MakePhysicsMaterial(0.0f, friction);
	return(result);
}

inline EdgeShape *GetEdgeShape(Shape *shape) {
	EdgeShape *result = 0;
	switch (shape->type) {
	case ShapeType::ShapeType_Box:
	{
		result = &shape->box;
	};	break;
	case ShapeType::ShapeType_Polygon:
	{
		result = &shape->polygon;
	};	break;
	case ShapeType::ShapeType_LineSegment:
	{
		result = &shape->lineSegment;
	};	break;
	InvalidDefaultCase;
	}
	return (result);
}