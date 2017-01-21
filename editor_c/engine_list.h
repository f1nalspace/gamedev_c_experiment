#pragma once

#include "engine_types.h"

struct LinkedListItem {
	LinkedListItem *prev, *next;
};
StaticAlignmentAssert(LinkedListItem);

template <typename T>
struct LinkedList : LinkedListItem {
	void Init() {
		prev = next = (T *)this;
	}
	void Add(T *item, LinkedListItem *prevInit, LinkedListItem *nextInit) {
		nextInit->prev = item;
		item->next = nextInit;
		item->prev = prevInit;
		prevInit->next = item;
	}
	void PushBack(T *item) {
		Add(item, this->prev, this);
	}
	void PushFront(T *item) {
		Add(item, this, this->next);
	}
	void Remove(LinkedListItem *prevInit, LinkedListItem *nextInit) {
		nextInit->prev = prevInit;
		prevInit->next = nextInit;
	}
	void Remove(LinkedListItem *entry) {
		Remove(entry->prev, entry->next);
	}
	T *PopFront() {
		T *result = (T *)next;
		Remove(result);
		return(result);
	}
	T *PopBack() {
		T *result = (T *)prev;
		Remove(result);
		return(result);
	}
	bool IsEmpty() {
		return next == (T *)this;
	}
};
