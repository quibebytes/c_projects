/*

This library should work in Windows and Unix adjacent systems
(Linux, BSD, macOS, etc). It's been tested on Linux Mint 22.1 and Wine 9.0.

This is a stb-style library and as such needs ARENA_IMPLEMENTATION
to be defined in *one* place for the function definitions to show up.
More information on stb-style libraries here:
https://github.com/nothings/stb#faq

The code is a modification of GingerBill's code here:
https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

Added bounds checking for new_size in arena_resize_align and derived.
Support for cross-platform sparse virtual memory was added.

See Ryan Fleury's Arena talk:
https://www.youtube.com/watch?v=TZ5a3gCCZYo

Also this for sparse virtual memory:
https://youtu.be/H8THRznXxpQ?si=KfO02LBA913gOMRk
*/

#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#ifdef _WIN32 
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif // _WIN32

// Default value taken from the Pool module in the Jai language
#ifndef ARENA_DEFAULT_SPARSE_MEMORY_SIZE
#define ARENA_DEFAULT_SPARSE_MEMORY_SIZE (256LL * 1024LL * 1024LL) 
#endif // ARENA_DEFAULT_SPARSE_MEMORY_SIZE

// GingerBill originally wrote 2 * sizeof(void*)
// Why would him want 8 byte alignment on x86 (which also has SIMD)?
// Probably assumed x64, although i still think (size_t) 16 is less cryptic
#ifndef ARENA_DEFAULT_ALIGNMENT
#define ARENA_DEFAULT_ALIGNMENT ((size_t) 16)
#endif

typedef struct Arena {
    unsigned char *buffer;
    size_t         buffer_length;
    size_t         previous_offset;
    size_t         current_offset;
} Arena;

// save current state so you can quickly use the memory then
// return to the saved state once you're finished
typedef struct TemporaryArenaMemory {
    Arena *arena;
    size_t     previous_offset;
    size_t     current_offset;
} TemporaryArenaMemory;


uintptr_t align_forward(uintptr_t ptr, size_t align);
void     *arena_allocate(Arena *arena, size_t size);
void     *arena_allocate_align(Arena *arena, size_t size, size_t align);
void      arena_clear(Arena *arena);
void      arena_create(Arena *arena);
void      arena_create_with_size(Arena *arena, size_t size);
void     *arena_resize(Arena *arena, void *old_memory, size_t old_size, size_t new_size);
void     *arena_resize_align(Arena *arena, void *old_memory, size_t old_size, size_t new_size, size_t align);
bool      is_power_of_two(uintptr_t x);

TemporaryArenaMemory temporary_arena_memory_begin(Arena *arena);
TemporaryArenaMemory temporary_arena_memory_end(TemporaryArenaMemory temp);


#endif // ARENA_H

#ifdef ARENA_IMPLEMENTATION

// powers of two have a single leading bit
bool is_power_of_two(uintptr_t x) {
    return (x & (x-1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;

    assert(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t) align;

    // same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a-1);

    if (modulo != 0) {
        p += a - modulo;
    }

    return p;
}

void arena_clear(Arena *arena) {
    #if _WIN32
        VirtualFree(arena->buffer, arena->buffer_length, MEM_DECOMMIT);
        arena->previous_offset = 0;
        arena->current_offset = 0;
    #else
        // haven't been able to find an Unix 
        // version of just decommiting the memory
        munmap(arena->buffer, arena->buffer_length);
        arena_create_with_size(arena, arena->buffer_length);
    #endif
}

TemporaryArenaMemory temporary_arena_memory_begin(Arena *arena) {
    return (TemporaryArenaMemory) {
        .arena = arena,
        .previous_offset = arena->previous_offset,
        .current_offset = arena->current_offset
    };
}

TemporaryArenaMemory temporary_arena_memory_end(TemporaryArenaMemory temp) {
    temp.arena->previous_offset = temp.previous_offset;
    temp.arena->current_offset = temp.current_offset;
}

void arena_create(Arena *arena) {
    arena_create_with_size(arena, ARENA_DEFAULT_SPARSE_MEMORY_SIZE);
}

void arena_create_with_size(Arena *arena, size_t size) {
    unsigned char *backing_buffer = NULL;

    #ifdef _WIN32
        backing_buffer = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    #else 
        backing_buffer = mmap(NULL, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

        if (backing_buffer == MAP_FAILED) {
            backing_buffer = NULL;
        }
    #endif
    
    arena->buffer = backing_buffer;
    arena->buffer_length = size;
    arena->previous_offset = 0;
    arena->current_offset = 0;
}

// added the necessary functions for commiting the reserved memory
void *arena_allocate_align(Arena *arena, size_t size, size_t align) {
    uintptr_t current_ptr = (uintptr_t) arena->buffer + (uintptr_t) arena->current_offset;
    uintptr_t offset = align_forward(current_ptr, align);
    offset -= (uintptr_t) arena->buffer; // change to relative offset

    // check if the allocator has memory left
    if (offset + size <= arena->buffer_length) {

        // commit the virtual pages used.
        #ifdef _WIN32
            arena->buffer = VirtualAlloc(arena->buffer, offset + size, MEM_COMMIT, PAGE_READWRITE);
            if (!arena->buffer) {
        #else
            // since this is mprotect i believe you can call perrno to know the specific error.
            if (mprotect(arena->buffer, offset + size, PROT_READ | PROT_WRITE)) {
        #endif
            assert(0 && "Could not commit memory");
            return NULL;
        }

        void *ptr = &arena->buffer[offset];
        arena->previous_offset = offset;
        arena->current_offset = offset + size;

        return ptr;
    }

    // if out of memory return NULL
    return NULL;
}

void *arena_allocate(Arena *arena, size_t size) {
    return arena_allocate_align(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

void *arena_resize_align(Arena *arena, void *old_memory, size_t old_size, size_t new_size, size_t align) {
    unsigned char *old_memory_bytes = (unsigned char *) old_memory;

    assert(is_power_of_two(align));

    if (old_memory_bytes == NULL || old_size == 0) {
        return arena_allocate_align(arena, new_size, align);
    } else if (arena->buffer <= old_memory_bytes && old_memory_bytes < arena->buffer + arena->buffer_length) {

        if (arena->buffer + arena->previous_offset == old_memory_bytes) {
            if (arena->previous_offset + new_size > arena->buffer_length) {
                return NULL;
            }

            #ifdef _WIN32
                arena->buffer = VirtualAlloc(arena->buffer, arena->previous_offset + new_size, MEM_COMMIT, PAGE_READWRITE);
                if (!arena->buffer) {
            #else
                if (mprotect(arena->buffer, arena->previous_offset + new_size, PROT_READ | PROT_WRITE) != 0) {
            #endif
                assert(0 && "Could not commit memory");
                return NULL;
            }

            arena->current_offset = arena->previous_offset + new_size;

            return old_memory;
        } else {
            void *new_memory = arena_allocate_align(arena, new_size, align);

            // copy old memory into new memory
            if (new_memory) {
                size_t copy_size = (old_size < new_size)? old_size: new_size;
                memmove(new_memory, old_memory, copy_size);
            }
            return new_memory;
        }
    } else {
        assert(0 && "Memory address is out of the bounds of the buffer in this arena");
        return NULL;
    }
}

void *arena_resize(Arena *arena, void *old_memory, size_t old_size, size_t new_size) {
    return arena_resize_align(arena, old_memory, old_size, new_size, ARENA_DEFAULT_ALIGNMENT);
}

#endif // ARENA_IMPLEMENTATION
