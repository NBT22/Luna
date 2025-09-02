//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

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
                     LunaBuffer **lunaBuffers);

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

#pragma region "Implmentation"

#include <algorithm>

namespace luna::core
{
inline Buffer::operator const VkBuffer &() const
{
    return buffer_;
}
inline Buffer::operator const VkBuffer *() const
{
    return &buffer_;
}
} // namespace luna::core

namespace luna::core::buffer
{
inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion):
    BufferRegionIndex(buffer, bufferRegion, nullptr)
{}
inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, SubRegion *subRegion)
{
    this->buffer_ = buffer;
    this->bufferRegion_ = bufferRegion;
    this->subRegion_ = subRegion;
}

inline size_t BufferRegionIndex::offset() const
{
    if (subRegion_ != nullptr)
    {
        return bufferRegion_->offset() + subRegion_->offset;
    }
    return bufferRegion_->offset();
}
inline size_t BufferRegionIndex::size() const
{
    if (subRegion_ != nullptr)
    {
        return subRegion_->size;
    }
    return bufferRegion_->size();
}
inline uint8_t *BufferRegionIndex::data() const
{
    if (subRegion_ != nullptr)
    {
        return bufferRegion_->data_ + subRegion_->offset;
    }
    return bufferRegion_->data_;
}

inline const VkBuffer *BufferRegionIndex::buffer() const
{
    return *buffer_;
}
inline const BufferRegion *BufferRegionIndex::bufferRegion() const
{
    return bufferRegion_;
}
inline const SubRegion *BufferRegionIndex::subRegion() const
{
    return subRegion_;
}

inline void BufferRegion::copyToBuffer(const uint8_t *data, const size_t bytes) const
{
    std::copy_n(data, bytes, data_);
}

inline size_t BufferRegion::size() const
{
    return size_;
}
inline size_t BufferRegion::offset(const SubRegion *subRegion) const
{
    if (subRegion != nullptr)
    {
        return offset_ + subRegion->offset;
    }
    return offset_;
}
} // namespace luna::core::buffer

#pragma endregion "Implementation"
