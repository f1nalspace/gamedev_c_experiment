#pragma once

#include "engine_types.h"
#include "engine_math.h"

#include "engine_physics_shapes.h"
#include "engine_physics_contact.h"

struct Physics;

struct SATResult {
	B32 success;
	F32 distance;
	Vec2f normal;
};

struct ManifoldInput {
	B32 flip;
	Vec2f localNormalA;
	Transform transformA;
	Transform transformB;
	Mat2f rotBtoA;
	U32 vertexCountA;
	U32 vertexCountB;
	Vec2f *localVertsA;
	Vec2f *localVertsB;
};

struct ManifoldOutput {
	Vec2f points[2];
	F32 distances[2];
	U32 features[2];
	Vec2f normal;
	U32 count;
};

struct ClipResult {
	Vec2f points[2];
	float distances[2];
};

struct Face {
	Vec2f points[2];
	U32 index;
};

#define CONTACT_GENERATOR(name) U32 name(Physics *world, const Transform &transformA, const Transform &transformB, Shape *shapeA, Shape *shapeB, U32 offset, Contact *contacts)
typedef CONTACT_GENERATOR(generate_contacts);

struct ContactGenerator {
	ShapeType typeA;
	ShapeType typeB;
	generate_contacts *proc;
};

inline B32 IsPointInAABB(const AABB &aabb, const Vec2f &point) {
	B32 result = (point.x >= aabb.min.x && point.x <= aabb.max.x) && (point.y >= aabb.min.y && point.y <= aabb.max.y);
	return(result);
}

inline Vec2f GetClosestPointOnLineSegment(const Vec2f &point, const Vec2f &a, const Vec2f &b, F32 *region) {
	Vec2f lineAB = b - a;
	Vec2f pointToLine = point - a;
	*region = Vec2Dot(pointToLine, lineAB) / Vec2LengthSquared(lineAB);
	F32 percentage = ScalarClamp01(*region);
	Vec2f result = a + lineAB * percentage;
	return (result);
}

inline Vec2f GetClosestPointOnPlane(const Vec2f &point, const Vec2f &normal, F32 distance) {
	Vec2f pointOnPlane = normal * distance;
	Vec2f distanceToPlane = pointOnPlane - point;
	F32 projDistance = Vec2Dot(normal, distanceToPlane);
	Vec2f result = point + normal * projDistance;
	return (result);
}

inline Vec2f GetClosestPointOnAABB(const Vec2f &point, const Vec2f &aabbCenter, const Vec2f &aabbExt) {
	Vec2f result = {};
	result.x = ScalarClamp(result.x, aabbCenter.x - aabbExt.x, aabbCenter.x + aabbExt.x);
	result.y = ScalarClamp(result.y, aabbCenter.y - aabbExt.y, aabbCenter.y + aabbExt.y);
	return (result);
}

external B32 IsAABBOverlap(const AABB &a, const AABB &b);
external Vec2f GetSupportPoint(const Vec2f &normal, U32 vertexCount, Vec2f *verts);
external ClipResult ClipToSegment(const Vec2f &normal, const Vec2f &ref1, const Vec2f &ref2, const Vec2f &inc1, const Vec2f &inc2);
external ClipResult ClipToPlane(const Vec2f &normal, const Vec2f &planePoint, const Vec2f &inc1, const Vec2f &inc2);
external Face GetFace(const Vec2f &normal, U32 vertexCount, Vec2f *verts);
external SATResult QuerySAT(const Transform &transformA, U32 vertexCountA, Vec2f *localVertsA, const Transform &transformB, U32 vertexCountB, Vec2f *localVertsB);

external CONTACT_GENERATOR(CircleCircleContactGenerator);
external CONTACT_GENERATOR(EdgeCircleContactGenerator);
external CONTACT_GENERATOR(PlaneCircleContactGenerator);
external CONTACT_GENERATOR(PlaneEdgeContactGenerator);
external CONTACT_GENERATOR(EdgeEdgeContactGenerator);