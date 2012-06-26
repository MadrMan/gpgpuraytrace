#pragma once

#define SSEALIGN 16
#define CLASSALIGN __declspec(align(SSEALIGN))

#define ALLOCALIGN \
    void* operator new(unsigned int size) \
	{ return _mm_malloc(size, SSEALIGN); } \
	void operator delete(void* p) \
	{ _mm_free(p); }

