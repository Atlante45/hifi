//
//  Allocators.h
//  libraries/shared/srs/shared/allocators
//
//  Created by Clement Brisset on 9/21/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//  Interface and implementation based on Andrei Alexandrescu's CppCon 2015 talk:
//  Video: https://www.youtube.com/watch?v=LIb3L4vKZ7U
//  Slides: https://github.com/CppCon/CppCon2015/tree/master/Presentations/allocator%20Is%20to%20Allocation%20what%20vector%20Is%20to%20Vexation

#pragma once

#ifndef hifi_Allocators_h
#define hifi_Allocators_h

#include <stdlib.h>

namespace alloc {

struct Blk {
    void* ptr { nullptr };
    size_t size { 0 };
};

static constexpr size_t ALIGNMENT = 8;

// Fast round up to a power of 2
constexpr size_t roundToAligned(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

// Mallocator
//
// Allocates using the default malloc/free calls
// Mainly for use as fallback
class Mallocator {
public:
    Blk allocate(size_t size);
    void deallocate(Blk block);
};

// StackAllocator
//
// Allocates directly on the stack
// Very fast for local operations
template<size_t SIZE>
class StackAllocator {
public:
    Blk allocate(size_t size);
    void deallocate(Blk block);
    bool owns(Blk block) const;

    void deallocateAll();

private:
    char* _ptr;
    char _data[SIZE];
};
static_assert(sizeof(char*) == roundToAligned(sizeof(char*)), "_data will not be aligned");

// FreeList
//
// Allocates directly on the stack
// Very fast for local operations
template<size_t SIZE>
class FreeList {
public:
    Blk allocate(size_t size);
    void deallocate(Blk block);
    bool owns(Blk block) const;

    void deallocateAll();

private:
    char* _ptr;
    char _data[SIZE];
};



}

#endif // hifi_Allocators_h
