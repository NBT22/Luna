//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
#include <map>
#include <ranges>
#include <thread>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "helpers/Handle.hpp"
#include "Luna.hpp"

namespace luna
{
class Buffer;
class BufferRegion
{
        friend class BufferRegionIndex;

    public: // BufferRegion public static members
        static VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *outBuffer);

    private: // BufferRegion private static members
        /**
         * Find space for a buffer region, creating a new @c Buffer if needed
         * @param[in] creationInfo The creation information for the buffer region
         * @param[out] outBuffer The buffer to place the region in
         * @param[out] outOffset The offset into the buffer at which to place the region
         * @param[out] outIterator An iterator to the buffer region that the new buffer region should be placed before
         * @return @c VK_SUCCESS if space was found for the region, or a meaningful result code otherwise
         */
        static VkResult findSpaceForBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                                 Buffer *&outBuffer,
                                                 size_t &outOffset,
                                                 std::list<BufferRegion>::iterator &outIterator);

    public: // BufferRegion public members
        BufferRegion(size_t size, uint8_t *data, size_t offset, Buffer *buffer, LunaBuffer *outBuffer);

        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t offset() const;

    private: // BufferRegion private members
        size_t size_{};
        uint8_t *data_{};
        size_t offset_{};
};
class BufferRegionIndex
{
    public:
        static void waitForCleanupThread();
        /// Removes a buffer region index from the list. Calling this function with an invalid or null pointer will have no effect
        static void destroy(BufferRegionIndex *&bufferRegionIndex);
        [[nodiscard]] static VkResult resize(BufferRegionIndex *&bufferRegionIndex, VkDeviceSize newSize);

    private:
        static inline std::thread cleanupThread_{}; // NOLINT(*-identifier-naming)

    public:
        BufferRegionIndex() = delete;
        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion);

        ~BufferRegionIndex();

        [[nodiscard]] VkResult flushMemory() const;
        [[nodiscard]] VkResult copyToBuffer(const uint8_t *data,
                                            size_t bytes,
                                            size_t offset = 0,
                                            VkPipelineStageFlags stageFlags = 0) const;
        [[nodiscard]] VkResult createBufferView(const LunaBufferViewCreationInfo &creationInfo,
                                                LunaBufferView *lunaView);

        [[nodiscard]] size_t offset() const;
        [[nodiscard]] size_t size() const;
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
        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo);
        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo,
                        VkDeviceSize alignment);

        ~Buffer();

        operator const VkBuffer &() const;
        operator const VkBuffer *() const;

        bool operator==(const Buffer &other) const;

    private: // Buffer private members
        std::atomic_bool destroyed_{true};
        VkBuffer buffer_{};
        VmaAllocation allocation_{};
        VkBufferCreateFlags creationFlags_{};
        VkBufferUsageFlags usageFlags_{};
        VmaAllocationCreateInfo allocationCreateInfo_{};
        size_t usedBytes_{};
        size_t unusedBytes_{}; ///< The bytes within the buffer that make up the dead space between buffer regions
        size_t freeBytes_{}; ///< The bytes at the end of the VkBuffer that are not used by any region
        void *data_{};
        std::list<BufferRegion> regions_{};
};
} // namespace luna

#pragma region Implementation

#include <algorithm>
#include <cassert>

namespace luna
{
inline void BufferRegionIndex::waitForCleanupThread()
{
    if (cleanupThread_.joinable())
    {
        cleanupThread_.join();
    }
}
inline VkResult BufferRegionIndex::resize(BufferRegionIndex *&bufferRegionIndex, const VkDeviceSize newSize)
{
    if (bufferRegionIndex->size() == newSize)
    {
        return VK_SUCCESS;
    }

    if (bufferRegionIndex->bufferRegion_ == nullptr)
    {
        LunaBuffer lunaBuffer = LUNA_NULL_HANDLE;
        const LunaBufferCreationInfo newCreationInfo = {
            .size = newSize,
            .flags = bufferRegionIndex->buffer_->creationFlags_,
            .usage = bufferRegionIndex->buffer_->usageFlags_,
        };
        CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(newCreationInfo, &lunaBuffer));

        destroy(bufferRegionIndex);
        bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
    }

    const bool growing = bufferRegionIndex->size() < newSize;
    const size_t sizeChange = growing ? newSize - bufferRegionIndex->size() : bufferRegionIndex->size() - newSize;
    Buffer *buffer = bufferRegionIndex->buffer_;
    BufferRegion *bufferRegion = bufferRegionIndex->bufferRegion_;

    if (growing)
    {
        const bool regionIsLast = bufferRegion == &buffer->regions_.back();
        const bool regionCanBeResizedIntoFreeBytes = regionIsLast && newSize <= buffer->freeBytes_;
        bool regionCanBeResizedIntoUnusedBytes = false;
        if (!regionCanBeResizedIntoFreeBytes && !regionIsLast)
        {
            std::list<BufferRegion>::const_iterator bufferRegionIterator =
                    std::ranges::find_if(buffer->regions_, [&bufferRegion](const BufferRegion &region) -> bool {
                        return &region == bufferRegion;
                    });
            assert(bufferRegionIterator != buffer->regions_.cend()); // Internal state check
            ++bufferRegionIterator;
            assert(bufferRegionIterator != buffer->regions_.cend()); // Internal state check
            regionCanBeResizedIntoUnusedBytes = newSize <= bufferRegionIterator->offset_ - bufferRegion->offset_;
        }
        if (regionCanBeResizedIntoFreeBytes || regionCanBeResizedIntoUnusedBytes)
        {
            bufferRegion->size_ += sizeChange;
            buffer->usedBytes_ += sizeChange;
            if (regionCanBeResizedIntoFreeBytes)
            {
                buffer->freeBytes_ -= sizeChange;
            } else
            {
                buffer->unusedBytes_ -= sizeChange;
            }
        } else
        {
            LunaBuffer lunaBuffer = helpers::toHandle(bufferRegionIndex);
            LunaBufferCreationInfo newCreationInfo;
            bufferRegionIndex->creationInfo(newCreationInfo);
            newCreationInfo.size = newSize;

            CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(newCreationInfo, &lunaBuffer));

            if (bufferRegion->data_ != nullptr)
            {
                uint8_t *newBufferRegionData = helpers::fromHandle<BufferRegionIndex>(lunaBuffer)->data();
                assert(newBufferRegionData != nullptr); // Internal state check
                std::copy_n(bufferRegion->data_, bufferRegion->size_, newBufferRegionData);
            }
            destroy(bufferRegionIndex);
            bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
        }
    } else
    {
        bufferRegion->size_ -= sizeChange;
        buffer->usedBytes_ -= sizeChange;
        if (bufferRegion == &buffer->regions_.back())
        {
            buffer->freeBytes_ += sizeChange;
        } else
        {
            buffer->unusedBytes_ += sizeChange;
        }
    }

    return VK_SUCCESS;
}

inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion):
    buffer_(buffer),
    bufferRegion_(bufferRegion)
{}

inline size_t BufferRegionIndex::offset() const
{
    assert(bufferRegion_);
    return bufferRegion_->offset_;
}
inline size_t BufferRegionIndex::size() const
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
}
inline const VkBuffer &BufferRegionIndex::buffer() const
{
    return *buffer_;
}


inline size_t BufferRegion::size() const
{
    return size_;
}
inline size_t BufferRegion::offset() const
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
