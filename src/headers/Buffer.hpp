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
        class SubRegion
        {
            public:
                constexpr bool operator==(const SubRegion &other) const
                {
                    return offset == other.offset && size == other.size;
                }

                size_t size{}; ///< The sub-region's size
                size_t offset{}; ///< The sub-region's offset into the buffer region
        };

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
        BufferRegion(size_t size,
                     uint8_t *data,
                     size_t offset,
                     std::map<const char *, SubRegion> &&subRegions,
                     Buffer *buffer,
                     LunaBuffer *outBuffer);

        [[nodiscard]] size_t size() const;
        [[nodiscard]] size_t offset(const SubRegion *subRegion = nullptr) const;

    private: // BufferRegion private members
        size_t size_{};
        uint8_t *data_{};
        size_t offset_{};
        std::map<const char *, SubRegion> subRegions_{};
};
class BufferRegionIndex
{
        using SubRegion = BufferRegion::SubRegion;

    public:
        static void waitForCleanupThread();
        /// Removes a buffer region index from the list. Calling this function with an invalid or null pointer will have no effect
        static void destroy(BufferRegionIndex *const &bufferRegionIndex);
        /// Removes a buffer region index from the list. Calling this function with an invalid or null pointer will have no effect
        static void destroy(BufferRegionIndex *&bufferRegionIndex);
        [[nodiscard]] static VkResult resize(BufferRegionIndex *&bufferRegionIndex, VkDeviceSize newSize);

    private:
        static inline std::thread cleanupThread_{}; // NOLINT(*-identifier-naming)

    public:
        BufferRegionIndex() = delete;
        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, BufferRegion::SubRegion *subRegion = nullptr);

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
        BufferRegion::SubRegion *subRegion_{};
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
#include <stdexcept>

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
    SubRegion *subRegion = bufferRegionIndex->subRegion_;
    assert(subRegion != nullptr || bufferRegion->subRegions_.empty()); // Internal state check

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
            if (subRegion != nullptr)
            {
                if (subRegion->offset + subRegion->size != bufferRegion->size_)
                {
                    if (bufferRegion->data_ != nullptr)
                    {
                        const size_t dataOffset = subRegion->offset + subRegion->size;
                        std::copy_n(bufferRegion->data_ + dataOffset,
                                    bufferRegion->size_ - dataOffset,
                                    bufferRegion->data_ + dataOffset + sizeChange);
                    }
                    for (SubRegion &region: bufferRegion->subRegions_ | std::views::values)
                    {
                        if (subRegion->offset < region.offset)
                        {
                            region.offset += sizeChange;
                        }
                    }
                }
                subRegion->size -= sizeChange;
            }
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
            if (subRegion != nullptr)
            {
                const VkBufferCreateFlags flags = buffer->creationFlags_;
                const VkBufferUsageFlags usage = buffer->usageFlags_;
                std::vector<LunaBufferCreationInfo> creationInfos;
                creationInfos.reserve(bufferRegion->subRegions_.size());
                size_t index = std::numeric_limits<size_t>::max();
                for (const SubRegion &region: bufferRegion->subRegions_ | std::views::values)
                {
                    if (&region == subRegion)
                    {
                        index = creationInfos.size();
                        // TODO (0.3.0): Fix this
                        creationInfos.emplace_back(newSize, flags, usage, 0, 0, nullptr, nullptr, nullptr);
                    } else
                    {
                        // TODO (0.3.0): Fix this
                        creationInfos.emplace_back(region.size, flags, usage, 0, 0, nullptr, nullptr, nullptr);
                    }
                }
                assert(index != std::numeric_limits<size_t>::max()); // Internal state check
                (void)index;

                // TODO (0.3.0): Fix this!!
                throw std::runtime_error("You hit the branch that doesn't work yet :(");


                // Need to be able to
                //  1. Find a place, in this buffer or another, that can contain the full buffer region with the resized subregion
                //  2. Update the existing handles for the other subregions of that region, WITHOUT CHANGING THEIR MEMORY LOCATION
                //  3. Move the data to the new region


                // Buffer buffer{};
                // size_t offset{};
                // bool usesFreeSpace{};
                // CHECK_RESULT_RETURN(BufferRegion::findSpaceForBufferRegion(creationInfo, buffer, offset, usesFreeSpace));

                // TODO: Need to not only not add new indices for any sub-region other than the one being resized, but additionally need to update the existing ones
            } else
            {
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
            }
            destroy(bufferRegionIndex);
            bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
        }
    } else
    {
        if (subRegion != nullptr)
        {
            if (subRegion->offset + subRegion->size != bufferRegion->size_)
            {
                if (bufferRegion->data_ != nullptr)
                {
                    const size_t dataOffset = subRegion->offset + subRegion->size;
                    std::copy_n(bufferRegion->data_ + dataOffset,
                                bufferRegion->size_ - dataOffset,
                                bufferRegion->data_ + dataOffset - sizeChange);
                }
                for (SubRegion &region: bufferRegion->subRegions_ | std::views::values)
                {
                    if (subRegion->offset < region.offset)
                    {
                        region.offset -= sizeChange;
                    }
                }
            }
            subRegion->size -= sizeChange;
        }
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

inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer,
                                            BufferRegion *bufferRegion,
                                            BufferRegion::SubRegion *subRegion):
    buffer_(buffer),
    bufferRegion_(bufferRegion),
    subRegion_(subRegion)
{}

inline size_t BufferRegionIndex::offset() const
{
    assert(bufferRegion_);
    if (subRegion_ != nullptr)
    {
        return bufferRegion_->offset_ + subRegion_->offset;
    }
    return bufferRegion_->offset_;
}
inline size_t BufferRegionIndex::size() const
{
    if (bufferRegion_ == nullptr)
    {
        assert(!subRegion_); // Internal state check
        return 0;
    }
    if (subRegion_ != nullptr)
    {
        return subRegion_->size;
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
    if (subRegion_ != nullptr)
    {
        return bufferRegion_->data_ + subRegion_->offset;
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
inline size_t BufferRegion::offset(const SubRegion *subRegion) const
{
    if (subRegion != nullptr)
    {
        return offset_ + subRegion->offset;
    }
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
