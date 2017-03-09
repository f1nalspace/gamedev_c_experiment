#include "engine_physics_collision.h"

constant U32 PHYSICS_CONTACT_FEATURE_FACEA_PRIME = 3;
constant U32 PHYSICS_CONTACT_FEATURE_FACEB_PRIME = 5;
constant U32 PHYSICS_CONTACT_FEATURE_CLIP_PRIME = 11;
constant U32 PHYSICS_CONTACT_FEATURE_SWAP_PRIME = 17;

internal ManifoldInput MakeInputManifold(B32 flip, const Vec2f &localNormalA, const Transform &transformA, const Transform &transformB, U32 vertexCountA, Vec2f *localVertsA, U32 vertexCountB, Vec2f *localVertsB) {
	ManifoldInput result = {};
	result.flip = flip;
	result.localNormalA = localNormalA;
	result.transformA = transformA;
	result.transformB = transformB;
	result.localVertsA = localVertsA;
	result.localVertsB = localVertsB;
	result.vertexCountA = vertexCountA;
	result.vertexCountB = vertexCountB;
	Mat2f rotBTranspose = Mat2Transpose(transformB.rot);
	result.rotBtoA = Mat2Mult(transformA.rot, rotBTranspose);
	return (result);
}

external B32 IsAABBOverlap(const AABB &a, const AABB &b) {
	F32 distanceX = a.max.x - a.min.x;
	F32 distanceY = a.max.y - a.min.y;
	F32 otherDistanceX = b.max.x - b.min.x;
	F32 otherDistanceY = b.max.y - b.min.y;
	F32 bothRadiusX = (Abs(distanceX) + Abs(otherDistanceX)) * 0.5f;
	F32 bothRadiusY = (Abs(distanceY) + Abs(otherDistanceY)) * 0.5f;
	F32 otherCenterX = b.min.x + otherDistanceX * 0.5f;
	F32 otherCenterY = b.min.y + otherDistanceY * 0.5f;
	F32 centerX = a.min.x + distanceX * 0.5f;
	F32 centerY = a.min.y + distanceY * 0.5f;
	F32 diffX = Abs(centerX - otherCenterX);
	F32 diffY = Abs(centerY - otherCenterY);
	F32 overlapX = diffX - bothRadiusX;
	F32 overlapY = diffY - bothRadiusY;
	B32 result = !(overlapX > 0 || overlapY > 0);
	return(result);
}

inline void SetContact(Contact *contact, F32 distance, const Vec2f &normal, const Vec2f &point, U32 feature) {
	*contact = {};
	contact->distance = distance;
	contact->normal = normal;
	contact->point = point;
	contact->feature = feature;
}

external Vec2f GetSupportPoint(const Vec2f &normal, U32 vertexCount, Vec2f *verts) {
	Vec2f result = verts[0];
	F32 distance = Vec2Dot(result, normal);
	for (U32 vertexIndex = 1; vertexIndex < vertexCount; ++vertexIndex) {
		Vec2f *v = verts + vertexIndex;
		F32 p = Vec2Dot(*v, normal);
		if (p > distance) {
			distance = p;
			result = *v;
		}
	}
	return (result);
}

external ClipResult ClipToSegment(const Vec2f &normal, const Vec2f &ref1, const Vec2f &ref2, const Vec2f &inc1, const Vec2f &inc2) {
	Vec2f tangent = Vec2Cross(normal, 1.0f);
	Vec2f segmentDistance = inc2 - inc1;
	Vec2f segmentToLine0 = ref1 - inc1;
	Vec2f segmentToLine1 = ref2 - inc1;
	F32 d0 = Vec2Dot(segmentToLine0, tangent);
	F32 d1 = Vec2Dot(segmentToLine1, tangent);
	F32 f0 = d0 / Vec2Dot(segmentDistance, tangent);
	F32 f1 = d1 / Vec2Dot(segmentDistance, tangent);
	ClipResult result = {};
	result.points[0] = inc1 + segmentDistance * ScalarClamp01(f0);
	result.points[1] = inc1 + segmentDistance * ScalarClamp01(f1);
	result.distances[0] = Vec2Dot(result.points[0] - ref1, normal);
	result.distances[1] = Vec2Dot(result.points[1] - ref1, normal);
	return (result);
}

external ClipResult ClipToPlane(const Vec2f &normal, const Vec2f &planePoint, const Vec2f &inc1, const Vec2f &inc2) {
	Vec2f distance0 = inc1 - planePoint;
	Vec2f distance1 = inc2 - planePoint;
	ClipResult result = {};
	result.distances[0] = Vec2Dot(distance0, normal);
	result.distances[1] = Vec2Dot(distance1, normal);
	result.points[0] = inc1 + normal * -result.distances[0];
	result.points[1] = inc2 + normal * -result.distances[1];
	return(result);
}

external Face GetFace(const Vec2f &normal, U32 vertexCount, Vec2f *verts) {
	S32 firstIndex = 0;
	F32 firstDistance = Vec2Dot(verts[0], normal);
	for (U32 vertexIndex = 1; vertexIndex < vertexCount; vertexIndex++) {
		Vec2f v = verts[vertexIndex];
		F32 p = Vec2Dot(v, normal);
		if (p > firstDistance) {
			firstDistance = p;
			firstIndex = vertexIndex;
		}
	}

	S32 secondIndex = -1;
	F32 secondDistance = 0;
	for (U32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {
		if (vertexIndex != firstIndex) {
			Vec2f v = verts[vertexIndex];
			F32 p = Vec2Dot(v, normal);
			if (secondIndex == -1 || p > secondDistance) {
				secondDistance = p;
				secondIndex = vertexIndex;
			}
		}
	}
	Assert(secondIndex > -1);
	Face result = {};
	result.index = firstIndex;
	result.points[0] = verts[firstIndex];
	result.points[1] = verts[secondIndex];
	return (result);
}

external SATResult QuerySAT(const Transform &transformA, U32 vertexCountA, Vec2f *localVertsA, const Transform &transformB, U32 vertexCountB, Vec2f *localVertsB) {
	SATResult result = {};

	Mat2f rotBTranspose = Mat2Transpose(transformB.rot);
	Mat2f rotBtoA = Mat2Mult(transformA.rot, rotBTranspose);

	B32 first = true;
	for (U32 vertIndexA = 0; vertIndexA < vertexCountA; vertIndexA++) {
		// Create normal for A
		Vec2f v0 = localVertsA[vertIndexA];
		Vec2f v1 = localVertsA[(vertIndexA + 1) % vertexCountA];
		Vec2f t = Vec2Normalize(v1 - v0);
		Vec2f localNormalA = Vec2Cross(t, 1.0f);

		// Bring normal A into its own space 
		Vec2f normalForA = Vec2MultMat2(localNormalA, transformA.rot);
		// Bring normal B into the space of A 
		Vec2f normalForB = -Vec2MultMat2(localNormalA, rotBtoA);

		// Get support point for B
		Vec2f supportPointB = GetSupportPoint(normalForB, vertexCountB, localVertsB);

		// Get closest distance
		Vec2f pA = Vec2MultTransform(v0, transformA);
		Vec2f pB = Vec2MultTransform(supportPointB, transformB);
		Vec2f pAB = pB - pA;
		F32 proj = Vec2Dot(pAB, normalForA);
		if (proj > 0) {
			result.success = false;
			break;
		}

		// Record axis of minimum penetration
		if (first || proj > result.distance) {
			first = false;
			result.normal = localNormalA;
			result.distance = proj;
			result.success = true;
		}
	}
	return(result);
}

external CONTACT_GENERATOR(CircleCircleContactGenerator) {
	U32 result = 0;

	CircleShape *circleA = &shapeA->circle;
	CircleShape *circleB = &shapeB->circle;
	Vec2f posA = transformA.pos;
	Vec2f posB = transformB.pos;

	// NOTE(final): Get normal from distance between circles
	Vec2f distanceBetween = posB - posA;
	Vec2f normal;
	if (Vec2LengthSquared(distanceBetween) > 0) {
		normal = Vec2Normalize(distanceBetween);
	} else {
		normal = V2(1, 0);
	}

	// NOTE(final): Get distance and get closest point
	F32 projDistance = Vec2Dot(distanceBetween, normal);
	F32 bothRadius = circleA->radius + circleB->radius;
	F32 d = -(bothRadius - projDistance);

	// NOTE(final): Separation(Distance > 0) - early out
	if (d > 0) {
		return 0;
	}

	Vec2f pointOnA = posA + normal * circleA->radius;

	U32 feature = 1 * PHYSICS_CONTACT_FEATURE_FACEA_PRIME + 1 * PHYSICS_CONTACT_FEATURE_FACEB_PRIME;
	Contact *contact = contacts + (offset + result++);
	SetContact(contact, d, normal, pointOnA, feature);

	return(result);
}

external CONTACT_GENERATOR(EdgeCircleContactGenerator) {
	U32 result = 0;

	Vec2f posA = transformA.pos;
	Vec2f posB = transformB.pos;
	EdgeShape *edge = GetEdgeShape(shapeA);
	CircleShape *circle = (CircleShape *)&shapeB->circle;

	// NOTE(final): Transform vertices for A
	U32 vertexCountA = edge->vertexCount;
	Vec2f *localVertsA = edge->localVerts;
	Vec2f vertsA[PHYSICS_MAX_EDGE_SHAPE_VERTEX_COUNT];
	for (U32 i = 0; i < vertexCountA; i++) {
		vertsA[i] = Vec2MultTransform(localVertsA[i], transformA);
	}

	// NOTE(final): Get closest point (best distance match)
	F32 bestDistance = 0;
	B32 first = true;
	Vec2f bestNormal = {};
	Vec2f bestClosest = {};
	F32 bestClosestRegion = 0;
	S32 bestEdgeIndex = 0;
	for (U32 i = 0; i < vertexCountA; i++) {
		Vec2f v0 = vertsA[i];
		Vec2f v1 = vertsA[(i + 1) % vertexCountA];
		Vec2f t = Vec2Normalize(v1 - v0);
		Vec2f n = Vec2Cross(t, 1.0f);

		F32 region;
		Vec2f closest = GetClosestPointOnLineSegment(posB, v0, v1, &region);
		Vec2f distanceToEdge = posB - v0;
		F32 distance = circle->radius - Vec2Dot(distanceToEdge, n);

		// NOTE(final): Separation(Distance > 0) - early out
		if (distance > 0) {
			return 0;
		}

		if (first || distance < bestDistance) {
			first = false;
			bestDistance = distance;
			bestNormal = n;
			bestClosest = closest;
			bestClosestRegion = region;
			bestEdgeIndex = i;
		}
	}

	U32 feature = (bestEdgeIndex + 1) * PHYSICS_CONTACT_FEATURE_FACEA_PRIME;
	feature += 1 * PHYSICS_CONTACT_FEATURE_FACEB_PRIME;

	// NOTE(final): Update normal when voronio region is A or B
	if (bestClosestRegion < 0 || bestClosestRegion > 1) {
		bestNormal = Vec2Normalize(posB - bestClosest);
		Vec2f distanceToClosest = posB - bestClosest;
		bestDistance = circle->radius - Vec2Dot(distanceToClosest, bestNormal);
		feature += 1 * PHYSICS_CONTACT_FEATURE_SWAP_PRIME;
	}

	Contact *contact = contacts + (offset + result++);
	SetContact(contact, -bestDistance, bestNormal, bestClosest, feature);

	return(result);
}

external CONTACT_GENERATOR(PlaneCircleContactGenerator) {
	U32 result = 0;

	Vec2f posA = transformA.pos;
	Vec2f posB = transformB.pos;
	CircleShape *circleB = (CircleShape *)&shapeB->circle;
	Vec2f normal = transformA.rot.col1;

	Vec2f distanceToPlane = posA - posB;
	float projDistance = Vec2Dot(distanceToPlane, normal);
	float projRadius = -circleB->radius;
	float d = projRadius - projDistance;

	// NOTE(final): Separation(Distance > 0) - early out
	if (d > 0) {
		return 0;
	}

	Vec2f pointOnA = posB + normal * projDistance;

	// NOTE(final): A plane and a circle have just one face - so we have just one feature
	int feature = 1 * PHYSICS_CONTACT_FEATURE_FACEA_PRIME + 1 * PHYSICS_CONTACT_FEATURE_FACEB_PRIME;
	Contact *contact = contacts + (offset + result++);
	SetContact(contact, d, normal, pointOnA, feature);

	return(result);
}

external CONTACT_GENERATOR(PlaneEdgeContactGenerator) {
	U32 result = 0;

	Vec2f normal = transformA.rot.col1;
	Vec2f posA = transformA.pos;
	EdgeShape *edgeB = GetEdgeShape(shapeB);
	Vec2f *localVertsB = edgeB->localVerts;
	U32 vertexCountB = edgeB->vertexCount;

	Mat2f transposeB = Mat2Transpose(transformB.rot);
	Vec2f normalB = -Vec2MultMat2(normal, transposeB);

	Face faceB = GetFace(normalB, vertexCountB, localVertsB);
	Vec2f supportPointB = Vec2MultTransform(faceB.points[0], transformB);
	Vec2f planePoint = posA;
	Vec2f distanceToPlane = supportPointB - planePoint;
	F32 d = Vec2Dot(distanceToPlane, normal);

	// NOTE(final): Separation(Distance > 0) - early out
	if (d > 0) {
		return 0;
	}

	Vec2f pointsOnA[2];
	F32 distanceToA[2];
	U32 clipTypeOnB[2];
	U32 pointCount = 0;

	// NOTE(final): Clip face line segments (Incident front face against reference plane)
	//				Keep points only when behind the reference plane
	Vec2f sB0 = Vec2MultTransform(faceB.points[0], transformB);
	Vec2f sB1 = Vec2MultTransform(faceB.points[1], transformB);
	ClipResult clipResult = ClipToPlane(normal, planePoint, sB0, sB1);
	if (clipResult.distances[0] <= 0) {
		U32 idx = pointCount++;
		pointsOnA[idx] = clipResult.points[0];
		distanceToA[idx] = clipResult.distances[0];
		clipTypeOnB[idx] = 1;
	}
	if (clipResult.distances[1] <= 0) {
		U32 idx = pointCount++;
		pointsOnA[idx] = clipResult.points[1];
		distanceToA[idx] = clipResult.distances[1];
		clipTypeOnB[idx] = 2;
	}

	for (U32 pointIndex = 0; pointIndex < pointCount; pointIndex++) {
		// NOTE(final): Plane has just one face - therefore only one feature
		U32 feature = 0 * PHYSICS_CONTACT_FEATURE_FACEA_PRIME + (faceB.index + 1) * PHYSICS_CONTACT_FEATURE_FACEB_PRIME + clipTypeOnB[pointIndex] * PHYSICS_CONTACT_FEATURE_CLIP_PRIME;
		Contact *contact = contacts + (offset + result++);
		SetContact(contact, distanceToA[pointIndex], normal, pointsOnA[pointIndex], feature);
	}

	return(result);
}

external CONTACT_GENERATOR(EdgeEdgeContactGenerator) {
	U32 result = 0;

	Vec2f posA = transformA.pos;
	EdgeShape *vbsA = GetEdgeShape(shapeA);
	EdgeShape *vbsB = GetEdgeShape(shapeB);

	// NOTE(final): Get vertices for A and B
	Vec2f *localVertsA = vbsA->localVerts;
	Vec2f *localVertsB = vbsB->localVerts;
	U32 vertexCountA = vbsA->vertexCount;
	U32 vertexCountB = vbsB->vertexCount;

	// NOTE(final): Query SAT for A -> B or B -> A
	// NOTE(final): Separation (Distance > 0) - early out
	SATResult resultA = QuerySAT(transformA, vertexCountA, localVertsA, transformB, vertexCountB, localVertsB);
	if (!resultA.success) {
		return 0;
	}
	SATResult resultB = QuerySAT(transformB, vertexCountB, localVertsB, transformA, vertexCountA, localVertsA);
	if (!resultB.success) {
		return 0;
	}

	// NOTE(final): Create input manifold and prefer A over B
	const F32 kRelTol = 0.99f;
	const F32 kAbsTol = 0.001f;
	ManifoldInput input;
	if (resultA.distance > kRelTol * resultB.distance + kAbsTol) {
		input = MakeInputManifold(false, resultA.normal, transformA, transformB, vertexCountA, localVertsA, vertexCountB, localVertsB);
	} else {
		input = MakeInputManifold(true, resultB.normal, transformB, transformA, vertexCountB, localVertsB, vertexCountA, localVertsA);
	}

	ManifoldOutput output = {};

	// NOTE(final): Get face vertices for A and B
	Face faceA = GetFace(input.localNormalA, input.vertexCountA, input.localVertsA);
	Face faceB = GetFace(-Vec2MultMat2(input.localNormalA, input.rotBtoA), input.vertexCountB, input.localVertsB);

	// NOTE(final): Transform face vertices for A and B
	Vec2f sA1 = Vec2MultTransform(faceA.points[0], input.transformA);
	Vec2f sA2 = Vec2MultTransform(faceA.points[1], input.transformA);
	Vec2f sB1 = Vec2MultTransform(faceB.points[0], input.transformB);
	Vec2f sB2 = Vec2MultTransform(faceB.points[1], input.transformB);

	// NOTE(final): Transform local normal A and set as final normal
	output.normal = Vec2MultMat2(input.localNormalA, input.transformA.rot);

	// NOTE(final): Clip face line segments (Incident front face against reference side planes)
	//				Keep points only when behind the reference front face
	//				Clip points are on B - need to project back to A
	ClipResult clipResult = ClipToSegment(output.normal, sA1, sA2, sB1, sB2);
	if (clipResult.distances[0] <= 0) {
		U32 idx = output.count++;
		output.points[idx] = clipResult.points[0] + output.normal * -clipResult.distances[0];
		output.distances[idx] = clipResult.distances[0];
		output.features[idx] = 1;
	}
	if (clipResult.distances[1] <= 0) {
		U32 idx = output.count++;
		output.points[idx] = clipResult.points[1] + output.normal * -clipResult.distances[1];
		output.distances[idx] = clipResult.distances[1];
		output.features[idx] = 2;
	}

	// NOTE(final): Move points from A to B and invert normal (When B > A)
	if (input.flip) {
		for (U32 pointIndex = 0; pointIndex < output.count; pointIndex++) {
			output.points[pointIndex] += output.normal * output.distances[pointIndex];
		}
		output.normal *= -1.0f;
	}

	// NOTE(final): Create final contact points
	for (U32 i = 0; i < output.count; i++) {
		U32 feature = (faceA.index + 1) * PHYSICS_CONTACT_FEATURE_FACEA_PRIME + (faceB.index + 1) * PHYSICS_CONTACT_FEATURE_FACEB_PRIME;
		feature += output.features[i] * PHYSICS_CONTACT_FEATURE_CLIP_PRIME;
		if (input.flip) {
			feature += 1 * PHYSICS_CONTACT_FEATURE_SWAP_PRIME;
		}
		Contact *contact = contacts + (offset + result++);
		SetContact(contact, output.distances[i], output.normal, output.points[i], feature);
	}

	return(result);
}