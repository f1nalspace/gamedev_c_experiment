#pragma once

#include "engine_types.h"

// TODO(final): Handle alignment!

struct MemoryBlock {
	void *base;
	memory_size used;
	memory_size size;
	U32 tempCount;
};

struct TemporaryMemory {
	MemoryBlock *parentBlock;
	memory_size used;
};

enum MemoryFlag {
	MemoryFlag_None,

	MemoryFlag_Zero = 1 << 0,
};

#define MemoryFlagsDefault() (MemoryFlag::MemoryFlag_Zero)

inline void ZeroSize(void *base, memory_size size) {
	Assert(base);
	U8 *ptr = (U8 *)base;
	while (size--) {
		*ptr++ = 0;
	}
}
#define ZeroStruct(instance) \
	ZeroSize(&(instance), sizeof(instance))
#define ZeroArray(ptr, count) \
	ZeroSize(ptr, count*sizeof((ptr)[0]))

inline MemoryBlock MemoryBlockCreate(void *base, memory_size size, MemoryFlag flags = MemoryFlagsDefault()) {
	Assert(base);
	Assert(size > 0);
	MemoryBlock result = {};
	result.base = base;
	result.size = size;
	if (flags & MemoryFlag::MemoryFlag_Zero) {
		ZeroSize(base, size);
	}
	return(result);
}

inline void *__PushSize(MemoryBlock *block, memory_size size, MemoryFlag flags = MemoryFlagsDefault()) {
	Assert(block);
	Assert(size > 0);
	Assert(block->used + size <= block->size);
	void *result = (U8*)block->base + block->used;
	block->used += size;
	if (flags & MemoryFlag::MemoryFlag_Zero) {
		ZeroSize(result, size);
	}
	return(result);
}

inline MemoryBlock MemoryBlockCreateFrom(MemoryBlock *sourceBlock, memory_size size, MemoryFlag flags = MemoryFlagsDefault()) {
	void *base = __PushSize(sourceBlock, size);
	MemoryBlock result = MemoryBlockCreate(base, size, flags);
	return (result);
}

inline TemporaryMemory TemporaryMemoryBegin(MemoryBlock *parentBlock) {
	TemporaryMemory result = {};
	result.parentBlock = parentBlock;
	result.used = parentBlock->used;
	++parentBlock->tempCount;
	return(result);
}
inline void TemporaryMemoryEnd(TemporaryMemory *tempMemory) {
	MemoryBlock *block = tempMemory->parentBlock;
	Assert(block->used >= tempMemory->used);
	block->used = tempMemory->used;
	Assert(block->tempCount > 0);
	--block->tempCount;
}

#define PushSize(block, size, ...) \
	__PushSize(block, size, ## __VA_ARGS__)
#define PushStruct(block, type, ...) \
	(type *)__PushSize(block, sizeof(type), ## __VA_ARGS__)
#define PushArray(block, type, count, ...) \
	(type *)__PushSize(block, sizeof(type) * count, ## __VA_ARGS__)
