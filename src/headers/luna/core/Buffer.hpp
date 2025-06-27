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
    // TODO: This could be a class and introduce additional functionality, such as automatically getting the offset
    struct BufferRegionIndex
    {
            Buffer *buffer{};
            class BufferRegion *bufferRegion{};
            SubRegion *subRegion{};
    };
} // namespace buffer

class Buffer
{
    public:
        friend class buffer::BufferRegion;
        friend void ::lunaDestroyBuffer(LunaBuffer);

        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo);

        operator const VkBuffer &() const;

        void destroy();

    private:
        bool isDestroyed_{true};
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
        // TODO: Maybe move this to Instance where the others live and friend it here?
        static VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                           LunaBuffer **bufferOut,
                                           uint32_t count = 1,
                                           const LunaBufferCreationInfo *creationInfos = nullptr);
        static bool isDestroyed(const BufferRegion &region);

        friend void ::lunaWriteDataToBuffer(LunaBuffer, const void *, size_t, size_t offset);

        BufferRegion(size_t size, uint8_t *data, Buffer *buffer);
        BufferRegion(size_t size, uint8_t *data, size_t offset, Buffer *buffer, LunaBuffer *index);
        BufferRegion(size_t totalSize,
                     uint8_t *data,
                     size_t offset,
                     Buffer *buffer,
                     uint32_t count,
                     const LunaBufferCreationInfo *creationInfos,
                     LunaBuffer **buffers);

        void destroy();
        void destroyAtEnd();
        void destroySubRegion(const SubRegion *subRegion);

        void copyToBuffer(const uint8_t *data, size_t bytes) const;

        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t offset(const SubRegion *subRegion = nullptr) const;

    private:
        bool isDestroyed_{true};
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
