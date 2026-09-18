/*
Convenient names for int types, bool is also
included but remains unchanged.
*/
#ifndef INTDEFS_H
#define INTDEFS_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define U8      uint8_t
#define I8      int8_t
#define U16     uint16_t
#define I16     int16_t
#define U32     uint32_t
#define I32     int32_t
#define U64     uint64_t
#define I64     int64_t
#define Size    size_t
#define UIntPtr uintptr_t
#define PtrDiff ptrdiff_t

#endif // INTDEFS_H