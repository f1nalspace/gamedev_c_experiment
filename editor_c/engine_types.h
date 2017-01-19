#pragma once

#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static
#define constant static const
#define external extern

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef float F32;

typedef S32 B32;

typedef size_t memory_size;

#define KiloBytes(value) ((value) * 1024LL)
#define MegaBytes(value) (KiloBytes(value) * 1024LL)
#define GigaBytes(value) (MegaBytes(value) * 1024LL)
#define TeraBytes(value) (GigaBytes(value) * 1024LL)

#ifdef _DEBUG
#define Assert(exp) if(!(exp)) {*(int *)0 = 0;}
#else
#define Assert(exp)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define SwapPtr(type, a, b) { void* tmp = a; a = b; b = (type*)tmp; }