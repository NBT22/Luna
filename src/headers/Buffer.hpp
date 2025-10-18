//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaBuffer.h>
#include <luna/lunaTypes.h>
#include <thread>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "Luna.hpp"

namespace luna
{
class Buffer
{
    public: // Buffer public types
        class BufferRegion
        {
            private: // BufferRegion private types
                class SubRegion
                {
                    public:
                        constexpr bool operator==(const SubRegion &other) const
                        {
                            return offset == other.offset && size == other.size;
                        }

                        size_t size{};
                        size_t offset{};
                };

            public: // BufferRegion public types
                class BufferRegionIndex
                {
                    public:
                        static void waitForCleanupThread();
                        [[nodiscard]] static VkResult resize(BufferRegionIndex *&bufferRegionIndex,
                                                             VkDeviceSize newSize);
                        [[nodiscard]] static VkResult reserve(BufferRegionIndex *&bufferRegionIndex, size_t bytes);

                    private: // BufferRegionIndex private static
                        static void destroyBuffer_(Buffer *buffer);

                        static inline std::thread cleanupThread_{};

                    public: // BufferRegionIndex public members
                        BufferRegionIndex() = delete;
                        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion);
                        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, SubRegion *subRegion);

                        ~BufferRegionIndex();

                        constexpr bool operator==(const BufferRegionIndex &other) const
                        {
                            return subRegion_ == other.subRegion_ &&
                                   bufferRegion_ == other.bufferRegion_ &&
                                   buffer_ == other.buffer_;
                        }

                        [[nodiscard]] VkResult copyToBuffer(const uint8_t *data, size_t bytes, size_t offset = 0) const;

                        [[nodiscard]] size_t offset() const;
                        [[nodiscard]] size_t size() const;
                        [[nodiscard]] uint8_t *data() const;
                        [[nodiscard]] LunaBufferCreationInfo creationInfo() const;
                        [[nodiscard]] const VkBuffer &buffer() const;
                        [[nodiscard]] const BufferRegion &bufferRegion() const;
                        [[nodiscard]] const SubRegion &subRegion() const;


                    private: // BufferRegionIndex private members
                        void destroy_();

                        Buffer *buffer_{};
                        BufferRegion *bufferRegion_{};
                        SubRegion *subRegion_{};
                };

            public: // BufferRegion public static members
                static VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo, LunaBuffer *buffer);
                static VkResult createBufferRegion(const LunaBufferCreationInfo &creationInfo,
                                                   LunaBuffer **bufferOut,
                                                   uint32_t count = 1,
                                                   const LunaBufferCreationInfo *creationInfos = nullptr);

            public: // BufferRegion public members
                BufferRegion(size_t size, uint8_t *data, Buffer *buffer);
                BufferRegion(size_t size, uint8_t *data, size_t offset, Buffer *buffer, LunaBuffer *index);
                BufferRegion(size_t totalSize,
                             uint8_t *data,
                             size_t offset,
                             Buffer *buffer,
                             uint32_t count,
                             const LunaBufferCreationInfo *creationInfos,
                             LunaBuffer **lunaBuffers);

                constexpr bool operator==(const BufferRegion &other) const
                {
                    return buffer_ == other.buffer_ &&
                           offset_ == other.offset_ &&
                           size_ == other.size_ &&
                           data_ == other.data_ &&
                           subRegions_ == other.subRegions_;
                }

                [[nodiscard]] size_t size() const;
                [[nodiscard]] size_t offset(const SubRegion *subRegion = nullptr) const;

            private: // BufferRegion private members
                size_t size_{};
                uint8_t *data_{};
                size_t offset_{};
                Buffer *buffer_{};
                std::list<SubRegion> subRegions_{};
        };

    public: // Buffer public members
        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo);

        ~Buffer();

        operator const VkBuffer &() const;
        operator const VkBuffer *() const;

    private: // Buffer private members
        std::atomic_bool shouldBeDestroyed_{false};
        std::atomic_bool canBeReused_{true};
        VkBuffer buffer_{};
        VmaAllocation allocation_{};
        VkBufferCreateFlags creationFlags_{};
        VkBufferUsageFlags usageFlags_{};
        VmaAllocationCreateInfo allocationCreateInfo_{};
        size_t usedBytes_{};
        size_t unusedBytes_{};
        size_t freeBytes_{};
        void *data_{};
        std::list<BufferRegion> regions_{};
};

using BufferRegionIndex = Buffer::BufferRegion::BufferRegionIndex;
} // namespace luna

#pragma region "Implmentation"

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
    // TODO: Improved resizing logic
    LunaBufferCreationInfo newCreationInfo = bufferRegionIndex->creationInfo();
    newCreationInfo.size = newSize;
    lunaDestroyBuffer(bufferRegionIndex);
    LunaBuffer lunaBuffer = bufferRegionIndex;
    CHECK_RESULT_RETURN(createBufferRegion(newCreationInfo, &lunaBuffer));
    bufferRegionIndex = const_cast<BufferRegionIndex *>(static_cast<const BufferRegionIndex *>(lunaBuffer));
    return VK_SUCCESS;
}
inline VkResult BufferRegionIndex::reserve(BufferRegionIndex *&bufferRegionIndex, const size_t bytes)
{
    if (bufferRegionIndex->size() < bytes)
    {
        CHECK_RESULT_RETURN(resize(bufferRegionIndex, bytes));
    }
    return VK_SUCCESS;
}

inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion):
    BufferRegionIndex(buffer, bufferRegion, nullptr)
{}
inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, SubRegion *subRegion)
{
    this->buffer_ = buffer;
    this->bufferRegion_ = bufferRegion;
    this->subRegion_ = subRegion;
}

inline BufferRegionIndex::~BufferRegionIndex()
{
    destroy_();
}

inline size_t BufferRegionIndex::offset() const
{
    if (subRegion_ != nullptr)
    {
        return bufferRegion_->offset_ + subRegion_->offset;
    }
    return bufferRegion_->offset_;
}
inline size_t BufferRegionIndex::size() const
{
    if (subRegion_ != nullptr)
    {
        return subRegion_->size;
    }
    return bufferRegion_->size_;
}
inline uint8_t *BufferRegionIndex::data() const
{
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

inline const VkBuffer &BufferRegionIndex::buffer() const
{
    return *buffer_;
}
inline const Buffer::BufferRegion &BufferRegionIndex::bufferRegion() const
{
    return *bufferRegion_;
}
inline const Buffer::BufferRegion::SubRegion &BufferRegionIndex::subRegion() const
{
    return *subRegion_;
}

inline void BufferRegionIndex::destroy_()
{
    assert(buffer_ && bufferRegion_);
    if (subRegion_ != nullptr)
    {
        const std::list<SubRegion>::iterator endIterator = bufferRegion_->subRegions_.end();
        const std::list<SubRegion>::iterator iterator = std::find_if(bufferRegion_->subRegions_.begin(),
                                                                     endIterator,
                                                                     [this](const SubRegion &region) -> bool {
                                                                         return region.offset == subRegion_->offset;
                                                                     });
        assert(iterator != endIterator);
        buffer_->freeBytes_ += subRegion_->size;
        buffer_->usedBytes_ -= subRegion_->size;
        bufferRegion_->size_ -= subRegion_->size;
        if (subRegion_->offset == 0)
        {
            bufferRegion_->offset_ += subRegion_->size;
            if (bufferRegion_->data_ != nullptr)
            {
                bufferRegion_->data_ += subRegion_->size;
            }
            for (std::list<SubRegion>::iterator regionIterator = iterator; regionIterator != endIterator;
                 ++regionIterator)
            {
                if (regionIterator->offset > subRegion_->offset)
                {
                    regionIterator->offset -= subRegion_->size;
                }
            }
        }
        bufferRegion_->subRegions_.erase(iterator);
    }
    if (subRegion_ == nullptr || bufferRegion_->subRegions_.empty())
    {
        assert(bufferRegion_->subRegions_.empty());
        buffer_->regions_.remove_if([this](const BufferRegion &region) -> bool {
            return region.offset_ == bufferRegion_->offset_;
        });
    }
    if (buffer_->regions_.empty())
    {
        if (cleanupThread_.joinable())
        {
            cleanupThread_.join();
        }
        cleanupThread_ = std::thread(destroyBuffer_, buffer_);
    }
}


inline size_t Buffer::BufferRegion::size() const
{
    return size_;
}
inline size_t Buffer::BufferRegion::offset(const SubRegion *subRegion) const
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
} // namespace luna

#pragma endregion "Implementation"
