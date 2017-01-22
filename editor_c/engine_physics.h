#pragma once

#include "engine_types.h"
#include "engine_math.h"
#include "engine_memory.h"
#include "engine_list.h"
#include "engine_input.h"

enum BodyType {
	BodyType_Static = 0,
	BodyType_Dynamic = 1,

	BodyType_Count,
};

struct Body : LinkedListItem {
	U32 bodyId;
	BodyType type;

	Vec2f radius;

	Vec2f position;
	Vec2f velocity;
	F32 invMass;

	AABB aabb;

	void* userData;
};

struct Contact {
	Body *bodyA;
	Body *bodyB;
	Vec2f normal;
	F32 distance;
	F32 impulse;
};

constant U32 PHYSICS_MAX_CONTACT_COUNT = 1024;
constant U32 PHYSICS_MAX_BODY_POOL_COUNT = 10000;
constant U32 PHYSICS_MAX_SOLVER_ITERATION_COUNT = 4;

struct Physics {
	MemoryBlock physicsMemory;

	Contact contacts[PHYSICS_MAX_CONTACT_COUNT];
	U32 contactCount;

	U32 bodyIdCounter;
	LinkedList<Body> bodyPool;
	LinkedList<Body> usedBodies;
	Body *bodies[PHYSICS_MAX_BODY_POOL_COUNT];
	U32 bodyCount;

	Vec2f gravity;
};

external void PhysicsInit(Physics *physics, const Vec2f &gravity);
external void PhysicsUpdate(Physics *physics, InputState *input);

external Body *PhysicsBodyCreate(Physics *physics, BodyType type, const Vec2f &radius, const Vec2f &pos, F32 density);
external void PhysicsBodyRemove(Physics *physics, Body *body);