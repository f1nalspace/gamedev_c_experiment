#pragma once

#include "engine_types.h"
#include "engine_math.h"

struct Contact {
	Vec2f normal;
	F32 distance;
	Vec2f point;
	F32 bias;
	F32 normalImpulse;
	F32 tangentImpulse;
	Vec2f rA, rB;
	F32 massNormal, massTangent;
	U32 feature;
};
