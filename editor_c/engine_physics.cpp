#include "engine_physics.h"

constant F32 PHYSICS_EPSILON = 0.00000001f;

// https://jsfiddle.net/g9v86af8/8/

inline void PhysicsBodiesRefresh(Physics *physics) {
	U32 bodyCount = 0;
	for (Body *body = (Body *)physics->usedBodies.next; body != (Body *)&physics->usedBodies; body = (Body *)body->next) {
		physics->bodies[bodyCount++] = body;
	}
	physics->bodyCount = bodyCount;
}

global_variable Vec2f globalPhysicsEdgeNormals[4] = {
	V2(0, -1), 
	V2(-1, 0),
	V2(0, 1), 
	V2(1, 0),
};

internal void PhysicsCreateContacts(Physics *physics, Body *bodyA, Body *bodyB) {
	// Simple SAT-Test to calculate the normal and distance
	Vec2f relPos = bodyB->position - bodyA->position;
	Vec2f bothRadius = bodyA->radius + bodyB->radius;
	F32 xOverlap = bothRadius.x - Abs(relPos.x);
	F32 yOverlap = bothRadius.y - Abs(relPos.y);
	S32 faceIndex = 0;
	F32 separation = 0;
	if (xOverlap < yOverlap) {
		// X-Axis
		if (relPos.x < 0) {
			faceIndex = 1;
		} else {
			faceIndex = 3;
		}
		separation = xOverlap;
	} else {
		// Y-Axis
		if (relPos.y < 0) {
			faceIndex = 0;
		} else {
			faceIndex = 2;
		}
		separation = yOverlap;
	}

	Vec2f normal = globalPhysicsEdgeNormals[faceIndex];
	Vec2f invNormal = normal * -1;
	Vec2f tangent = Vec2Cross(normal, 1.0f);
	Vec2f invTangent = tangent * -1;

	// Build minkowski segment based on A extended
	Vec2f origin = V2(0, 0);
	Vec2f segmentMinkowski1 = V2(relPos.x + normal.x * bothRadius.x - tangent.x * bothRadius.x, relPos.y + normal.y * bothRadius.y - tangent.y * bothRadius.y);
	Vec2f segmentMinkowski2 = V2(relPos.x + normal.x * bothRadius.x + tangent.x * bothRadius.x, relPos.y + normal.y * bothRadius.y + tangent.y * bothRadius.y);
	Vec2f segmentMinkowskiAB = segmentMinkowski2 - segmentMinkowski1;

	// Get distance to origin and calculate the region on the minkowski segment
	Vec2f distanceToOrigin = origin - segmentMinkowski1;
	F32 regionA = Vec2Dot(segmentMinkowskiAB, distanceToOrigin) / Vec2Dot(segmentMinkowskiAB, segmentMinkowskiAB);
	F32 percentageA = ScalarClamp01(regionA);

	// Build segment for A
	Vec2f segmentA1 = bodyA->position + Vec2Hadamard(normal, bodyA->radius) + Vec2Hadamard(tangent, bodyA->radius);
	Vec2f segmentA2 = bodyA->position + Vec2Hadamard(normal, bodyA->radius) + Vec2Hadamard(invTangent, bodyA->radius);
	Vec2f segmentAAB = segmentA2 - segmentA1;

	// Get closest point on segment A by the already calculated clamped region A
	Vec2f closestOnA = segmentA1 + segmentAAB * percentageA;

	// Build segment for B
	Vec2f segmentB1 = bodyB->position + Vec2Hadamard(invNormal, bodyB->radius) + Vec2Hadamard(invTangent, bodyB->radius);
	Vec2f segmentB2 = bodyB->position + Vec2Hadamard(invNormal, bodyB->radius) + Vec2Hadamard(tangent, bodyB->radius);
	Vec2f segmentBAB = segmentB2 - segmentB1;

	// Get distance to closest point and calculate the region on the segment B
	Vec2f distanceToClosest = closestOnA - segmentB1;
	F32 regionB = Vec2Dot(segmentBAB, distanceToClosest) / Vec2Dot(segmentBAB, segmentBAB);
	F32 percentageB = ScalarClamp01(regionB);

	// Get closest point on segment B by the already calculated clamped region B
	Vec2f closestOnB = segmentB1 + segmentBAB * percentageB;

	// Skip all edges - required to fix ghost collisions
	bool skipEdge = false;
	if ((regionA < PHYSICS_EPSILON || regionA > 1.0 - PHYSICS_EPSILON) || (regionB < PHYSICS_EPSILON || regionB > 1.0 - PHYSICS_EPSILON)) {
		skipEdge = true;
	}

	if (!skipEdge) {
		Assert(physics->contactCount < ArrayCount(physics->contacts));
		Contact *contact = &physics->contacts[physics->contactCount++];
		*contact = {};
		contact->distance = -separation;
		contact->normal = normal;
		contact->bodyA = bodyA;
		contact->bodyB = bodyB;
	}
}

external Body *PhysicsBodyCreate(Physics *physics, BodyType type, const Vec2f &radius, const Vec2f &pos, F32 density = 1.0f) {
	Assert(!physics->bodyPool.IsEmpty());
	Body *body = physics->bodyPool.PopFront();
	*body = {};
	physics->usedBodies.PushBack(body);
	PhysicsBodiesRefresh(physics);

	body->bodyId = ++physics->bodyIdCounter;
	body->type = type;
	body->position = pos;
	body->radius = radius;

	F32 mass = (radius.x * radius.y * 2.0f) * density;
	body->invMass = mass > 0 ? 1.0f / mass : 0;

	return(body);
}

external void PhysicsBodyRemove(Physics *physics, Body *body) {
	physics->usedBodies.Remove(body);
	*body = {};
	physics->bodyPool.PushBack(body);

	PhysicsBodiesRefresh(physics);
}

external void PhysicsInit(Physics *physics, const Vec2f &gravity) {
	physics->bodyPool.Init();
	Body *bodiesForPool = PushArray(&physics->physicsMemory, Body, PHYSICS_MAX_BODY_POOL_COUNT);
	for (U32 bodyIndex = 0; bodyIndex < PHYSICS_MAX_BODY_POOL_COUNT; ++bodyIndex) {
		Body *body = bodiesForPool + bodyIndex;
		physics->bodyPool.PushBack(body);
	}

	physics->usedBodies.Init();

	physics->gravity = gravity;
}

external void PhysicsUpdate(Physics * physics, InputState *input)
{
	// NOTE(final): Integrate acceleration
	for (U32 bodyIndex = 0; bodyIndex < physics->bodyCount; ++bodyIndex) {
		Body *body = physics->bodies[bodyIndex];
		if (body->type == BodyType::BodyType_Dynamic) {
			body->velocity += physics->gravity;
		}
	}

	// NOTE(final): Create contacts
	physics->contactCount = 0;
	for (U32 bodyIndexA = 0; bodyIndexA < physics->bodyCount; ++bodyIndexA) {
		Body *bodyA = physics->bodies[bodyIndexA];
		for (U32 bodyIndexB = bodyIndexA + 1; bodyIndexB < physics->bodyCount; ++bodyIndexB) {
			Body *bodyB = physics->bodies[bodyIndexB];
			if (bodyA->type == BodyType::BodyType_Dynamic || bodyB->type == BodyType::BodyType_Dynamic) {
				PhysicsCreateContacts(physics, bodyA, bodyB);
			}
		}
	}

	// Solve contacts
	for (U32 iteration = 0; iteration < PHYSICS_MAX_SOLVER_ITERATION_COUNT; iteration++) {
		for (U32 contactIndex = 0; contactIndex < physics->contactCount; contactIndex++) {
			Contact *contact = physics->contacts + contactIndex;
			Body *bodyA = contact->bodyA;
			Body *bodyB = contact->bodyB;

			// Get relative velocity
			Vec2f relVel = bodyB->velocity - bodyA->velocity;

			// Calculate mass ratio
			F32 invMassA = contact->bodyA->type == BodyType::BodyType_Dynamic ? contact->bodyA->invMass : 0;
			F32 invMassB = contact->bodyB->type == BodyType::BodyType_Dynamic ? contact->bodyB->invMass : 0;
			F32 massRatio = 1.0f / (invMassA + invMassB);

			// Calculate impulse
			F32 remove = Vec2Dot(relVel, contact->normal) + contact->distance / input->deltaTime;
			F32 impulse = Min(remove * massRatio, 0);

			// Accumulate impulse
			F32 newImpulse = Min(impulse + contact->impulse, 0);
			impulse = newImpulse - contact->impulse;
			contact->impulse = newImpulse;

			// Apply impulses
			bodyA->velocity += contact->normal * impulse * invMassA;
			bodyB->velocity -= contact->normal * impulse * invMassB;
		}
	}

	// Integrate velocity
	for (U32 bodyIndex = 0; bodyIndex < physics->bodyCount; ++bodyIndex) {
		Body *body = physics->bodies[bodyIndex];
		if (body->type == BodyType::BodyType_Dynamic) {
			body->position += body->velocity * input->deltaTime;
		}
	}
}