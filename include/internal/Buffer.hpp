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
#include "helpers/Handle.hpp"
#include "Luna.hpp"

namespace luna
{
class BufferRegion;
class Buffer
{
        static constexpr VkBufferCreateInfo BUFFER_CREATE_INFO = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 256 * 1024 * 1024,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        static constexpr VmaAllocationCreateInfo DEVICE_LOCAL_ALLOCATION_CREATE_INFO = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };
        static constexpr VmaAllocationCreateInfo HOST_VISIBLE_ALLOCATION_CREATE_INFO = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        static constexpr VmaAllocationCreateInfo HOST_CACHED_ALLOCATION_CREATE_INFO = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        static constexpr const VmaAllocationCreateInfo &allocationCreateInfo(const LunaMemoryType memoryType)
        {
            switch (memoryType)
            {
                case LUNA_MEMORY_TYPE_HOST_CACHED:
                    return HOST_CACHED_ALLOCATION_CREATE_INFO;
                case LUNA_MEMORY_TYPE_DEVICE_LOCAL:
                    // TODO: Enable this once the staging buffer is working
                    // return DEVICE_LOCAL_ALLOCATION_CREATE_INFO;
                case LUNA_MEMORY_TYPE_HOST_VISIBLE:
                default:
                    return HOST_VISIBLE_ALLOCATION_CREATE_INFO;
            }
        }

    public:
        Buffer() = default;
        Buffer(LunaMemoryType memoryType, const VmaAllocator &allocator);
        Buffer(const Buffer &other) = delete;
        Buffer(Buffer &&other) noexcept = delete;

        ~Buffer();

        Buffer &operator=(const Buffer &other) = delete;
        Buffer &operator=(Buffer &&other) noexcept = delete;

        explicit operator const VkBuffer &() const;
        explicit operator const VkBuffer *() const;

        bool operator==(const Buffer &other) const;

        [[nodiscard]] LunaBuffer createRegion(const LunaBufferCreationInfo &creationInfo);
        void destroyRegion(const BufferRegion &targetRegion);

        void clearRegions();

        [[nodiscard]] LunaMemoryType memoryType() const;
        [[nodiscard]] VmaVirtualBlock virtualBlock() const;
        [[nodiscard]] char *data() const;

    private:
        VmaAllocator allocator_{};
        VkBuffer buffer_{};
        VmaAllocation allocation_{};
        LunaMemoryType memoryType_{};
        VmaVirtualBlock virtualBlock_{};
        void *data_{};
        std::list<BufferRegion> regions_{};
};
class BufferRegion
{
    public:
        static void destroy(const BufferRegion &region);

        BufferRegion() = delete;
        BufferRegion(Buffer *buffer, VkDeviceSize alignment);
        BufferRegion(Buffer *buffer,
                     VkDeviceSize size,
                     VkDeviceSize offset,
                     VkDeviceSize alignment,
                     VmaVirtualAllocation allocation);

        ~BufferRegion();

        [[nodiscard]] char *data() const;
        [[nodiscard]] VkBuffer buffer() const;
        [[nodiscard]] VkDeviceSize size() const;
        [[nodiscard]] VkDeviceSize offset() const;
        [[nodiscard]] VkDeviceSize alignment() const;
        [[nodiscard]] LunaMemoryType memoryType() const;

    private:
        Buffer *buffer_{};
        VkDeviceSize size_{};
        VkDeviceSize offset_{};
        VkDeviceSize alignment_{};
        VmaVirtualAllocation allocation_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>

namespace luna
{
inline Buffer::~Buffer()
{
    if (buffer_ == VK_NULL_HANDLE)
    {
        return;
    }
    regions_.clear();
    vmaDestroyVirtualBlock(virtualBlock_);
    vmaDestroyBuffer(allocator_, buffer_, allocation_);
}

inline Buffer::operator const VkBuffer &() const
{
    return buffer_;
}
inline Buffer::operator const VkBuffer *() const
{
    return &buffer_;
}

inline bool Buffer::operator==(const Buffer &other) const
{
    // This should be enough to uniquely identify the Buffer
    return this == &other || (buffer_ == other.buffer_ && allocation_ == other.allocation_);
}

inline LunaBuffer Buffer::createRegion(const LunaBufferCreationInfo &creationInfo)
{
    if (creationInfo.memoryType != memoryType_)
    {
        assert(creationInfo.memoryType == memoryType_); // Internal state check.
        return LUNA_NULL_HANDLE;
    }
    if (creationInfo.size == 0)
    {
        return helpers::toHandle(&regions_.emplace_back(this, creationInfo.alignment));
    }
    const VmaVirtualAllocationCreateInfo virtualAllocationCreateInfo = {
        .size = creationInfo.size,
        .alignment = creationInfo.alignment,
    };
    VmaVirtualAllocation virtualAllocation{};
    VkDeviceSize offset{};
    if (vmaVirtualAllocate(virtualBlock_, &virtualAllocationCreateInfo, &virtualAllocation, &offset) != VK_SUCCESS)
    {
        return LUNA_NULL_HANDLE;
    }
    return helpers::toHandle(&regions_.emplace_back(this,
                                                    creationInfo.size,
                                                    offset,
                                                    creationInfo.alignment,
                                                    virtualAllocation));
}
inline void Buffer::destroyRegion(const BufferRegion &targetRegion)
{
    regions_.remove_if([&targetRegion](const BufferRegion &region) -> bool { return &targetRegion == &region; });
}

inline void Buffer::clearRegions()
{
    regions_.clear();
}

inline LunaMemoryType Buffer::memoryType() const
{
    return memoryType_;
}
inline VmaVirtualBlock Buffer::virtualBlock() const
{
    return virtualBlock_;
}
inline char *Buffer::data() const
{
    return static_cast<char *>(data_);
}

inline void BufferRegion::destroy(const BufferRegion &region)
{
    assert(region.buffer_ != nullptr); // Internal state check.
    region.buffer_->destroyRegion(region);
}

inline BufferRegion::BufferRegion(Buffer *buffer, const VkDeviceSize alignment): buffer_(buffer), alignment_(alignment)
{}
inline BufferRegion::BufferRegion(Buffer *buffer,
                                  const VkDeviceSize size,
                                  const VkDeviceSize offset,
                                  const VkDeviceSize alignment,
                                  const VmaVirtualAllocation allocation):
    buffer_(buffer),
    size_(size),
    offset_(offset),
    alignment_(alignment),
    allocation_(allocation)
{
    assert(buffer != nullptr);
}

inline BufferRegion::~BufferRegion()
{
    assert(buffer_ != nullptr); // Internal state check.
    vmaVirtualFree(buffer_->virtualBlock(), allocation_);
}

inline char *BufferRegion::data() const
{
    assert(buffer_ != nullptr); // Internal state check.
    return buffer_->data() == nullptr ? nullptr : buffer_->data() + offset_;
}
inline VkBuffer BufferRegion::buffer() const
{
    assert(buffer_ != nullptr); // Internal state check.
    return static_cast<VkBuffer>(*buffer_);
}
inline VkDeviceSize BufferRegion::size() const
{
    return size_;
}
inline VkDeviceSize BufferRegion::offset() const
{
    return offset_;
}
inline VkDeviceSize BufferRegion::alignment() const
{
    return alignment_;
}
inline LunaMemoryType BufferRegion::memoryType() const
{
    assert(buffer_ != nullptr); // Internal state check.
    return buffer_->memoryType();
}
} // namespace luna

#pragma endregion Implementation
