//
//  IDAllocator.h
//  libraries/shared/src
//
//  Created by Clement on 11/12/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_IDAllocator_h
#define hifi_IDAllocator_h

#include <atomic>
#include <type_traits>

#include <tbb/concurrent_queue.h>

template <typename T = uint64_t>
class IDAllocator {
    static_assert(std::is_integral<T>::value, "Integer type required for IDAllocator to perform correctly.");
    using ID = T;
    using Allocator = std::atomic<ID>;
    using IDs = tbb::concurrent_queue<ID>;

public:
    IDAllocator(T init = 0);

    ID peek() const;
    ID allocate();
    void release(ID idToRelease);

private:
    Allocator _allocator;
    IDs _freeList;
};

template<typename T>
inline IDAllocator<T>::IDAllocator(T init) {
    std::atomic_init(&_allocator, init);
}

template<typename T>
inline auto IDAllocator<T>::peek() const -> ID {
    return _allocator.load();
}

template<typename T>
inline auto IDAllocator<T>::allocate() -> ID {
    ID newID;
    // Try to pop an ID from the free list
    if (!_freeList.try_pop(newID)) {
        // If there was no free ID, allocate one
        newID = _allocator.fetch_add(1);
    }

    // Return the new ID
    return newID;
}

template<typename T>
inline void IDAllocator<T>::release(ID idToRelease) {
    // Add id to the free list
    _freeList.push(idToRelease);
}

#endif // hifi_IDAllocator_h
