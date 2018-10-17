//
//  Allocators.cpp
//  libraries/shared/srs/shared/allocators
//
//  Created by Clement Brisset on 9/21/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Allocators.h"


namespace alloc {


    Blk Mallocator::allocate(size_t size) {
        return { malloc(size), size };
    }

    void Mallocator::deallocate(Blk block) {
        free(block.ptr);
    }


    
    template<size_t SIZE>
    Blk StackAllocator<SIZE>::allocate(size_t size) {
        const auto alignedSize = roundToAligned(size);
        if (alignedSize > (_data + SIZE) - _ptr) {
            // Not enough space for the allocation
            return { nullptr, 0 };
        }
        const Blk result = { _ptr, size };
        _ptr += alignedSize;
        return result;
    }

    template<size_t SIZE>
    void StackAllocator<SIZE>::deallocate(Blk block) {
        if (static_cast<char*>(block.ptr) + roundToAligned(block.size) == _ptr) {
            _ptr = block.ptr;
        } else {
            // Could not deallocate
        }
    }

    template<size_t SIZE>
    bool StackAllocator<SIZE>::owns(Blk block) const {
        return block.ptr >= _data && block.ptr < _data + SIZE;
    }

    template<size_t SIZE>
    void StackAllocator<SIZE>::deallocateAll() {
        _ptr = _data;
    }

}
