#pragma once

#include "engine_memory.h"
#include "engine_render.h"
#include "engine_input.h"

struct AppState {
	void *renderStorageBase;
	memory_size renderStorageSize;

	void *persistentStorageBase;
	memory_size persistentStorageSize;

	void *transientStorageBase;
	memory_size transientStorageSize;
};