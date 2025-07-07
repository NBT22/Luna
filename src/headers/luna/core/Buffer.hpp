//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <list>
#include <luna/luna.h>
#include <vk_mem_alloc.h>

namespace luna::core
{
class Buffer;
namespace buffer
{
    struct SubRegion
    {
            size_t size{};
            size_t offset{};
    };
    class BufferRegionIndex
    {
            using BufferRegion = class BufferRegion;

        public:
            BufferRegionIndex() = delete;
            BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion);
            BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, SubRegion *subRegion);

            ~BufferRegionIndex();

            [[nodiscard]] size_t offset() const;
            [[nodiscard]] size_t size() const;
            [[nodiscard]] uint8_t *data() const;

            [[nodiscard]] const VkBuffer *buffer() const;
            [[nodiscard]] const BufferRegion *bufferRegion() const;
            [[nodiscard]] const SubRegion *subRegion() const;

        private:
            Buffer *buffer_{};
            BufferRegion *bufferRegion_{};
            SubRegion *subRegion_{};
    };
} // namespace buffer

class Buffer
{
    public:
        friend class buffer::BufferRegion;
        friend buffer::BufferRegionIndex::~BufferRegionIndex();

        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo);

        ~Buffer();

        operator const VkBuffer &() const;
        operator const VkBuffer *() const;

    private:
        VkBuffer buffer_{};
        VmaAllocation allocation_{};
        VkBufferCreateFlags creationFlags_{};
        VkBufferUsageFlags usageFlags_{};
        size_t usedBytes_{};
        size_t unusedBytes_{};
        size_t freeBytes_{};
        void *data_{};
        std::list<buffer::BufferRegion> regions_{};
};
} // namespace luna::core

namespace luna::core::buffer
{
class BufferRegion
{
    public:
        friend BufferRegionIndex::~BufferRegionIndex();
        friend uint8_t *BufferRegionIndex::data() const;

        // TODO: Maybe move this to Instance where the others live and friend it here?
        static VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                           LunaBuffer **bufferOut,
                                           uint32_t count = 1,
                                           const LunaBufferCreationInfo *creationInfos = nullptr);

        BufferRegion(size_t size, uint8_t *data, Buffer *buffer);
        BufferRegion(size_t size, uint8_t *data, size_t offset, Buffer *buffer, LunaBuffer *index);
        BufferRegion(size_t totalSize,
                     uint8_t *data,
                     size_t offset,
                     Buffer *buffer,
                     uint32_t count,
                     const LunaBufferCreationInfo *creationInfos,
                     LunaBuffer **buffers);

        void copyToBuffer(const uint8_t *data, size_t bytes) const;

        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t offset(const SubRegion *subRegion = nullptr) const;

    private:
        size_t size_{};
        /// This is a raw pointer because I don't own the pointer. It will be an offset into the pointer provided to me
        /// by mapping the memory.
        uint8_t *data_{};
        size_t offset_{};
        Buffer *buffer_{};
        std::list<SubRegion> subRegions_{};
};
} // namespace luna::core::buffer

#include <luna/implementations/core/Buffer.ipp>
