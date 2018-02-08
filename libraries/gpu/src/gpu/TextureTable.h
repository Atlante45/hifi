//
//  Created by Bradley Austin Davis on 2017/01/25
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_TextureTable_h
#define hifi_gpu_TextureTable_h

#include "Forward.h"

#include <array>

namespace gpu {

class TextureTable {
public:
    static const size_t COUNT = 8;
    using Array = std::array<TexturePointer, COUNT>;
    TextureTable();
    TextureTable(const std::initializer_list<TexturePointer>& textures);
    TextureTable(const std::array<TexturePointer, COUNT>& textures);

    // Only for gpu::Context
    const GPUObjectPointer gpuObject{};

    void setTexture(size_t index, const TexturePointer& texturePointer);
    void setTexture(size_t index, const TextureView& texturePointer);

    Array getTextures() const;
    Stamp getStamp() const { return _stamp; }
private:
    mutable Mutex _mutex;
    Array _textures;
    Stamp _stamp{ 0 };
};

}

#endif
