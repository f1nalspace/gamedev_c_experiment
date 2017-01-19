#pragma once

#include "engine_types.h"

#include <intrin.h>
#include <math.h>

inline U32 GetThreadID(void) {
	U8 *threadLocalStorage = (U8 *)__readgsqword(0x30);
	U32 threadID = *(U32 *)(threadLocalStorage + 0x48);
	return(threadID);
}

inline U32 AtomicInrementU32(volatile U32 *value) {
	long result = _InterlockedIncrement((volatile long *)value);
	return (result);
}

inline U32 AtomicExchangeU32(volatile U32 *target, U32 value) {
	U32 result = _InterlockedExchange((volatile long *)target, value);
	return (result);
}
inline U64 AtomicExchangeU64(volatile U64 *target, U64 value) {
	U64 result = _InterlockedExchange64((volatile long long *)target, value);
	return (result);
}

inline U64 AtomicAddU64(volatile U64 *value, U64 addend) {
	U64 result = _InterlockedExchangeAdd64((__int64 volatile *)value, addend);
	return (result);
}
inline U32 AtomicAddU32(volatile U32 *value, U32 addend) {
	U32 result = _InterlockedExchangeAdd((long volatile *)value, addend);
	return (result);
}

inline U32 AtomicCompareExchangeU32(volatile U32 *dest, U32 exchange, U32 comparand) {
	U32 result = _InterlockedCompareExchange((long volatile *)dest, exchange, comparand);
	return (result);
}
inline U64 AtomicCompareExchangeU64(volatile U64 *dest, U64 exchange, U64 comparand) {
	U64 result = _InterlockedCompareExchange64((__int64 volatile *)dest, exchange, comparand);
	return (result);
}

inline F32 SquareRoot(F32 value) {
	F32 result = sqrtf(value);
	return(result);
}

inline F32 Sin(F32 radians) {
	F32 result = sinf(radians);
	return(result);
}

inline F32 Cos(F32 radians) {
	F32 result = cosf(radians);
	return(result);
}

inline F32 ATan2(F32 y, F32 x) {
	F32 result = atan2f(y, x);
	return(result);
}

inline S32 SignOf(S32 value) {
	S32 result = (value >= 0) ? 1 : -1;
	return(result);
}

inline F32 SignOf(F32 value) {
	F32 result = (value >= 0) ? 1.0f : -1.0f;
	return(result);
}

inline F32 Abs(F32 value) {
	F32 result = (F32)fabs(value);
	return(result);
}

inline S32 RoundF32ToS32(F32 value) {
	S32 result = (S32)roundf(value);
	return(result);
}

inline U32 RoundF32ToU32(F32 value) {
	U32 result = (U32)roundf(value);
	return(result);
}

inline S32 FloorF32ToS32(F32 value) {
	S32 result = (S32)floorf(value);
	return(result);
}

inline S32 CeilF32ToS32(F32 value) {
	S32 result = (S32)ceilf(value);
	return(result);
}

inline S32 TruncateF32ToS32(F32 value) {
	S32 result = (S32)value;
	return(result);
}