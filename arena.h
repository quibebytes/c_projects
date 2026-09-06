/*
Use this at your own risk.


This is a stb-style library and as such needs ARENA_IMPLEMENTATION
to be defined somewhere for the function definitions to show up.
More information for stb-style libraries here:
https://github.com/nothings/stb#faq


This is a modification of GingerBill's code here:
https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

Added bounds checking for new_size in arena_resize_align and derived.
Also fixed a bug in the memset of arena_resize_align and derived.


Support for sparse virtual memory was added.
See Ryan Fleury's Arena talk:
https://www.youtube.com/watch?v=TZ5a3gCCZYo

Also this for sparse virtual memory:
https://youtu.be/H8THRznXxpQ?si=KfO02LBA913gOMRk

Using arena_clear will not uncommit the used virtual memory so if you need
to do it, just munmap the buffer and use arena_create again.

TODO: Add support for allocating virtual memory with the Windows API
by using VirtualAlloc and friends.
*/

#ifndef ARENA_H
#define ARENA_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <sys/mman.h>

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

// Save current state so you can quickly use the memory then return to the saved state once you're finished
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
    arena->previous_offset = 0;
    arena->current_offset = 0;
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
    unsigned char *backing_buffer = (unsigned char *) mmap(NULL, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (backing_buffer == MAP_FAILED) {
        backing_buffer = NULL;
    }

    arena->buffer = backing_buffer;
    arena->buffer_length = size;
    arena->previous_offset = 0;
    arena->current_offset = 0;
}

// copy-paste with the needed mprotect to commit the used memory
void *arena_allocate_align(Arena *arena, size_t size, size_t align) {
    uintptr_t current_ptr = (uintptr_t) arena->buffer + (uintptr_t) arena->current_offset;
    uintptr_t offset = align_forward(current_ptr, align);
    offset -= (uintptr_t) arena->buffer; // change to relative offset

    // check if the allocator has memory left
    if (offset + size <= arena->buffer_length) {

        // commit the virtual pages used.
        // since this is mprotect i believe you can call perrno to know the specific error.
        // TODO: own error logging system
        if (mprotect(arena->buffer, offset + size, PROT_READ | PROT_WRITE) != 0) {
            assert(0 && "Could not commit memory");
            return NULL;
        }

        void *ptr = &arena->buffer[offset];
        arena->previous_offset = offset;
        arena->current_offset = offset + size;


        // initialize memory to zero
        memset(ptr, 0, size);
        return ptr;
    }

    // if out of memory return NULL
    return NULL;
}

void *arena_allocate(Arena *arena, size_t size) {
    return arena_allocate_align(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

// copy-paste with the needed mprotect to commit the used memory
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

            if (mprotect(arena->buffer, arena->previous_offset + new_size, PROT_READ | PROT_WRITE) != 0) {
                assert(0 && "Could not commit memory");
                return NULL;
            }

            arena->current_offset = arena->previous_offset + new_size;

            if (new_size > old_size) {
                memset(&arena->buffer[arena->previous_offset + old_size], 0, new_size - old_size);
            }

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
