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

namespace luna
{
class Buffer;
class BufferRegion
{
        friend class BufferRegionIndex;

    public: // BufferRegion public static members
        static VkResult createBufferRegion(Device &device,
                                           const LunaBufferCreationInfo &creationInfo,
                                           LunaBuffer *outBuffer);

    private: // BufferRegion private static members
        /**
         * Find space for a buffer region, creating a new @c Buffer if needed
         * @param device
         * @param[in] creationInfo The creation information for the buffer region
         * @param[out] outBuffer The buffer to place the region in
         * @param[out] outOffset The offset into the buffer at which to place the region
         * @param[out] outIterator An iterator to the buffer region that the new buffer region should be placed before
         * @return @c VK_SUCCESS if space was found for the region, or a meaningful result code otherwise
         */
        static VkResult findSpaceForBufferRegion(Device &device,
                                                 const LunaBufferCreationInfo &creationInfo,
                                                 Buffer *&outBuffer,
                                                 VkDeviceSize &outOffset,
                                                 std::list<BufferRegion>::iterator &outIterator);

    public: // BufferRegion public members
        BufferRegion(Device &device,
                     VkDeviceSize size,
                     uint8_t *data,
                     VkDeviceSize offset,
                     Buffer *buffer,
                     LunaBuffer *outBuffer);

        [[nodiscard]] VkDeviceSize size() const;
        [[nodiscard]] VkDeviceSize offset() const;

    private: // BufferRegion private members
        VkDeviceSize size_{};
        uint8_t *data_{};
        VkDeviceSize offset_{};
};
class BufferRegionIndex
{
    public:
        [[nodiscard]] static VkResult resize(Device &device,
                                             CommandBuffer &commandBuffer,
                                             BufferRegionIndex *&bufferRegionIndex,
                                             VkDeviceSize newSize);

    public:
        BufferRegionIndex() = delete;
        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion);

        void destroy(Device &device);

        bool operator==(const BufferRegionIndex &other) const
        {
            return this == &other;
        }

        [[nodiscard]] VkResult flushMemory(const VmaAllocator &allocator) const;
        [[nodiscard]] VkResult copyToBuffer(Device &device,
                                            CommandBuffer &commandBuffer,
                                            const uint8_t *data,
                                            VkDeviceSize bytes,
                                            VkDeviceSize offset = 0,
                                            VkPipelineStageFlags stageFlags = 0) const;
        [[nodiscard]] VkResult createBufferView(VkDevice device,
                                                const LunaBufferViewCreationInfo &creationInfo,
                                                LunaBufferView *lunaView);

        [[nodiscard]] VkDeviceSize offset() const;
        [[nodiscard]] VkDeviceSize size() const;
        [[nodiscard]] uint8_t *data() const;
        [[nodiscard]] VkBufferCreateFlags creationFlags() const;
        [[nodiscard]] VkBufferUsageFlags usageFlags() const;
        void allocationCreateInfo(VmaAllocationCreateInfo &allocationCreateInfo) const;
        void creationInfo(LunaBufferCreationInfo &creationInfo) const;
        [[nodiscard]] const VkBuffer &buffer() const;

    private:
        Buffer *buffer_{};
        BufferRegion *bufferRegion_{};
        std::list<VkBufferView> views_{};
};
// TODO (0.3.0): Buffer writes need synchronization using vkCmdPipelineBarrier
class Buffer
{
        friend class BufferRegion;
        friend class BufferRegionIndex;

    public:
        Buffer() = default;
        explicit Buffer(const VmaAllocator &allocator,
                        const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo);
        explicit Buffer(const VmaAllocator &allocator,
                        const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo,
                        VkDeviceSize alignment);

        operator const VkBuffer &() const;
        operator const VkBuffer *() const;

        bool operator==(const Buffer &other) const;

        void destroy(VkDevice device, const VmaAllocator &allocator);

    private: // Buffer private members
        bool destroyed_{true};
        VkBuffer buffer_{};
        VmaAllocation allocation_{};
        VkBufferCreateFlags creationFlags_{};
        VkBufferUsageFlags usageFlags_{};
        VmaAllocationCreateInfo allocationCreateInfo_{};
        VkDeviceSize usedBytes_{};
        VkDeviceSize unusedBytes_{}; ///< The bytes within the buffer that make up the dead space between buffer regions
        VkDeviceSize freeBytes_{}; ///< The bytes at the end of the VkBuffer that are not used by any region
        void *data_{};
        std::list<BufferRegion> regions_{};
        std::vector<uint32_t> queueFamilyIndices_{};
};
} // namespace luna

#pragma region Implementation

#include <cassert>

namespace luna
{
inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion):
    buffer_(buffer),
    bufferRegion_(bufferRegion)
{}

inline VkDeviceSize BufferRegionIndex::offset() const
{
    assert(bufferRegion_);
    return bufferRegion_->offset_;
}
inline VkDeviceSize BufferRegionIndex::size() const
{
    if (bufferRegion_ == nullptr)
    {
        return 0;
    }
    return bufferRegion_->size_;
}
inline uint8_t *BufferRegionIndex::data() const
{
    assert(bufferRegion_);
    if (bufferRegion_->data_ == nullptr)
    {
        return nullptr;
    }
    return bufferRegion_->data_;
}
inline VkBufferCreateFlags BufferRegionIndex::creationFlags() const
{
    return buffer_->creationFlags_;
}
inline VkBufferUsageFlags BufferRegionIndex::usageFlags() const
{
    return buffer_->usageFlags_;
}
inline void BufferRegionIndex::allocationCreateInfo(VmaAllocationCreateInfo &allocationCreateInfo) const
{
    allocationCreateInfo = buffer_->allocationCreateInfo_;
}
inline void BufferRegionIndex::creationInfo(LunaBufferCreationInfo &creationInfo) const
{
    creationInfo.size = size();
    creationInfo.flags = buffer_->creationFlags_;
    creationInfo.usage = buffer_->usageFlags_;
    creationInfo.queueFamilyIndexCount = buffer_->queueFamilyIndices_.size();
    creationInfo.queueFamilyIndices = buffer_->queueFamilyIndices_.data();
}
inline const VkBuffer &BufferRegionIndex::buffer() const
{
    return *buffer_;
}


inline VkDeviceSize BufferRegion::size() const
{
    return size_;
}
inline VkDeviceSize BufferRegion::offset() const
{
    return offset_;
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
    return data_ == other.data_ && allocation_ == other.allocation_ && buffer_ == other.buffer_;
}

} // namespace luna

#pragma endregion Implementation
