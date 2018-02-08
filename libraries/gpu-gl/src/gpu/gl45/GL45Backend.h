//
//  GL45Backend.h
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 10/27/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once
#ifndef hifi_gpu_45_GL45Backend_h
#define hifi_gpu_45_GL45Backend_h

#include "../gl/GLBackend.h"
#include "../gl/GLTexture.h"
#include <thread>
#include <gpu/TextureTable.h>

#define INCREMENTAL_TRANSFER 0
#define GPU_SSBO_TRANSFORM_OBJECT 1
#define GPU_BINDLESS_TEXTURES 1

namespace gpu { namespace gl45 {
    
using namespace gpu::gl;
using TextureWeakPointer = std::weak_ptr<Texture>;

class GL45Backend : public GLBackend {
    using Parent = GLBackend;
    // Context Backend static interface required
    friend class Context;

public:

#ifdef GPU_SSBO_TRANSFORM_OBJECT
    static const GLint TRANSFORM_OBJECT_SLOT  { 14 }; // SSBO binding slot
#else
    static const GLint TRANSFORM_OBJECT_SLOT  { 31 }; // TBO binding slot
#endif

    explicit GL45Backend(bool syncCache) : Parent(syncCache) {}
    GL45Backend() : Parent() {}
    virtual ~GL45Backend() {
        // call resetStages here rather than in ~GLBackend dtor because it will call releaseResourceBuffer
        // which is pure virtual from GLBackend's dtor.
        resetStages();
    }

    static const std::string GL45_VERSION;
    const std::string& getVersion() const override { return GL45_VERSION; }

    class GL45Texture : public GLTexture {
        using Parent = GLTexture;
        friend class GL45Backend;
        static GLuint allocate(const Texture& texture);
    protected:
        GL45Texture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        void generateMips() const override;
        Size copyMipFaceLinesFromTexture(uint16_t mip, uint8_t face, const uvec3& size, uint32_t yOffset, GLenum internalFormat, GLenum format, GLenum type, Size sourceSize, const void* sourcePointer) const override;
        void syncSampler() const override;

        bool isBindless() const {
#if GPU_BINDLESS_TEXTURES
            return _bindless.operator bool();
#else
            return false;
#endif
        }

#if GPU_BINDLESS_TEXTURES
        struct Bindless {
            uint64_t handle{ 0 };
            uint32_t minMip{ 0 };
            uint32_t sampler{ 0 };

            operator bool() const {
                return handle != 0;
            }
        };

        virtual const Bindless& getBindless() const;
        void releaseBindless() const;
        void recreateBindless() const;
        virtual uint16 getMinMip() const = 0;


    private:
        class InvalidSampler : public Sampler {
        public:
            InvalidSampler() {
                _desc._borderColor = vec4(-1.0f);
            }

            operator const Sampler&() const {
                return *this;
            }
        };

        static const Sampler INVALID_SAMPLER;
        // This stores the texture handle (64 bits) in xy, the min mip available in z, and the sampler ID in w
        mutable Sampler _cachedSampler{ INVALID_SAMPLER };

        mutable Bindless _bindless;

#endif
    };

#if GPU_BINDLESS_TEXTURES 
    class GL45TextureTable : public GLObject<TextureTable> {
        static GLuint allocate();
        using Parent = GLObject<TextureTable>;
    public:
        using BindlessArray = std::array<GL45Texture::Bindless, TextureTable::COUNT>;

        GL45TextureTable(const std::weak_ptr<GLBackend>& backend, const TextureTable& texture);
        ~GL45TextureTable();

        void update(const BindlessArray& newHandles);

        // FIXME instead of making a buffer for each table, there should be a global buffer of all materials
        // and we should store an offset into that buffer
        BindlessArray _handles;
    };
#endif


    //
    // Textures that have fixed allocation sizes and cannot be managed at runtime
    //

    class GL45FixedAllocationTexture : public GL45Texture {
        using Parent = GL45Texture;
        friend class GL45Backend;

    public:
        GL45FixedAllocationTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        ~GL45FixedAllocationTexture();

    protected:
        Size size() const override { return _size; }
        uint16 getMinMip() const override { return 0; }

        void allocateStorage() const;
        void syncSampler() const override;
        const Size _size { 0 };
    };

    class GL45AttachmentTexture : public GL45FixedAllocationTexture {
        using Parent = GL45FixedAllocationTexture;
        friend class GL45Backend;
    protected:
        GL45AttachmentTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        ~GL45AttachmentTexture();
    };

    class GL45StrictResourceTexture : public GL45FixedAllocationTexture {
        using Parent = GL45FixedAllocationTexture;
        friend class GL45Backend;
    protected:
        GL45StrictResourceTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        ~GL45StrictResourceTexture();
    };

    //
    // Textures that can be managed at runtime to increase or decrease their memory load
    //

    class GL45VariableAllocationTexture : public GL45Texture, public GLVariableAllocationSupport {
        using Parent = GL45Texture;
        friend class GL45Backend;
        using PromoteLambda = std::function<void()>;

        const uvec4& getHandle();
    protected:
        GL45VariableAllocationTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        ~GL45VariableAllocationTexture();

        Size size() const override { return _size; }
        uint16 getMinMip() const override { return _populatedMip; }
        virtual const Bindless& getBindless() const override;

        Size copyMipFaceLinesFromTexture(uint16_t mip, uint8_t face, const uvec3& size, uint32_t yOffset, GLenum internalFormat, GLenum format, GLenum type, Size sourceSize, const void* sourcePointer) const override;
        void copyTextureMipsInGPUMem(GLuint srcId, GLuint destId, uint16_t srcMipOffset, uint16_t destMipOffset, uint16_t populatedMips) override;

    };

    class GL45ResourceTexture : public GL45VariableAllocationTexture {
        using Parent = GL45VariableAllocationTexture;
        friend class GL45Backend;
    protected:
        GL45ResourceTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);

        void syncSampler() const override;
        void promote() override;
        void demote() override;
        void populateTransferQueue() override;
        

        void allocateStorage(uint16 mip);
        Size copyMipsFromTexture();
    };

#if 0
    class GL45SparseResourceTexture : public GL45VariableAllocationTexture {
        using Parent = GL45VariableAllocationTexture;
        friend class GL45Backend;
        using TextureTypeFormat = std::pair<GLenum, GLenum>;
        using PageDimensions = std::vector<uvec3>;
        using PageDimensionsMap = std::map<TextureTypeFormat, PageDimensions>;
        static PageDimensionsMap pageDimensionsByFormat;
        static Mutex pageDimensionsMutex;

        static bool isSparseEligible(const Texture& texture);
        static PageDimensions getPageDimensionsForFormat(const TextureTypeFormat& typeFormat);
        static PageDimensions getPageDimensionsForFormat(GLenum type, GLenum format);
        static const uint32_t DEFAULT_PAGE_DIMENSION = 128;
        static const uint32_t DEFAULT_MAX_SPARSE_LEVEL = 0xFFFF;

    protected:
        GL45SparseResourceTexture(const std::weak_ptr<GLBackend>& backend, const Texture& texture);
        ~GL45SparseResourceTexture();
        uint32 size() const override { return _allocatedPages * _pageBytes; }
        void promote() override;
        void demote() override;

    private:
        uvec3 getPageCounts(const uvec3& dimensions) const;
        uint32_t getPageCount(const uvec3& dimensions) const;

        uint32_t _allocatedPages { 0 };
        uint32_t _pageBytes { 0 };
        uvec3 _pageDimensions { DEFAULT_PAGE_DIMENSION };
        GLuint _maxSparseLevel { DEFAULT_MAX_SPARSE_LEVEL };
    };
#endif


protected:

    void recycle() const override;

    GLuint getFramebufferID(const FramebufferPointer& framebuffer) override;
    GLFramebuffer* syncGPUObject(const Framebuffer& framebuffer) override;

    GLuint getBufferID(const Buffer& buffer) override;
    GLBuffer* syncGPUObject(const Buffer& buffer) override;

    GLTexture* syncGPUObject(const TexturePointer& texture) override;

    GLuint getQueryID(const QueryPointer& query) override;
    GLQuery* syncGPUObject(const Query& query) override;

    GL45TextureTable* syncGPUObject(const TextureTablePointer& textureTable);

    // Draw Stage
    void do_draw(const Batch& batch, size_t paramOffset) override;
    void do_drawIndexed(const Batch& batch, size_t paramOffset) override;
    void do_drawInstanced(const Batch& batch, size_t paramOffset) override;
    void do_drawIndexedInstanced(const Batch& batch, size_t paramOffset) override;
    void do_multiDrawIndirect(const Batch& batch, size_t paramOffset) override;
    void do_multiDrawIndexedIndirect(const Batch& batch, size_t paramOffset) override;

    // Input Stage
    void resetInputStage() override;
    void updateInput() override;

    // Resource stage
    void do_setResourceTextureTable(const Batch& batch, size_t paramOffset) override;

    // Synchronize the state cache of this Backend with the actual real state of the GL Context
    void transferTransformState(const Batch& batch) const override;
    void initTransform() override;
    void updateTransform(const Batch& batch) override;

    // Resource Stage
    bool bindResourceBuffer(uint32_t slot, BufferPointer& buffer) override;
    void releaseResourceBuffer(uint32_t slot) override;

    // Output stage
    void do_blit(const Batch& batch, size_t paramOffset) override;

    // Shader Stage
    std::string getBackendShaderHeader() const override;
    void makeProgramBindings(ShaderObject& shaderObject) override;
    int makeResourceBufferSlots(GLuint glprogram, const Shader::BindingSet& slotBindings,Shader::SlotSet& resourceBuffers) override;

    // Texture Management Stage
    void initTextureManagementStage() override;
};

} }

Q_DECLARE_LOGGING_CATEGORY(gpugl45logging)

#endif

