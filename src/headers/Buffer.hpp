//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <luna/lunaTypes.h>
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

                size_t size{};
                size_t offset{};
        };

    public: // BufferRegion public types
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
class BufferRegionIndex
{
        using SubRegion = BufferRegion::SubRegion;

    public:
        static void waitForCleanupThread();
        [[nodiscard]] static VkResult resize(BufferRegionIndex *&bufferRegionIndex, VkDeviceSize newSize);
        [[nodiscard]] static VkResult reserve(BufferRegionIndex *&bufferRegionIndex, size_t bytes);

    private:
        static inline std::thread cleanupThread_{};

    public:
        BufferRegionIndex() = delete;
        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion);
        BufferRegionIndex(Buffer *buffer, BufferRegion *bufferRegion, BufferRegion::SubRegion *subRegion);

        ~BufferRegionIndex();

        constexpr bool operator==(const BufferRegionIndex &other) const
        {
            return subRegion_ == other.subRegion_ && bufferRegion_ == other.bufferRegion_ && buffer_ == other.buffer_;
        }

        [[nodiscard]] VkResult copyToBuffer(const uint8_t *data,
                                            size_t bytes,
                                            size_t offset = 0,
                                            VkPipelineStageFlags stageFlags = 0) const;

        [[nodiscard]] size_t offset() const;
        [[nodiscard]] size_t size() const;
        [[nodiscard]] uint8_t *data() const;
        void creationInfo(LunaBufferCreationInfo *creationInfo) const;
        [[nodiscard]] const VkBuffer &buffer() const;
        [[nodiscard]] const BufferRegion &bufferRegion() const;
        [[nodiscard]] const BufferRegion::SubRegion &subRegion() const;

    private:
        Buffer *buffer_{};
        BufferRegion *bufferRegion_{};
        BufferRegion::SubRegion *subRegion_{};
};
// TODO (0.3.0): Buffer writes need synchronization using vkCmdPipelineBarrier
class Buffer
{
        friend class BufferRegion;
        friend BufferRegionIndex::~BufferRegionIndex();

    public:
        explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo,
                        const VmaAllocationCreateInfo &allocationCreateInfo);

        ~Buffer();

        operator const VkBuffer &() const;
        operator const VkBuffer *() const;

        bool operator==(const Buffer &other) const;

        void creationInfo(LunaBufferCreationInfo *creationInfo) const;

    private: // Buffer private members
        std::atomic_bool destroyed_{true};
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
} // namespace luna

#pragma region Implementation

#include <algorithm>
#include <cassert>
#include <luna/lunaBuffer.h>
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
    (void)bufferRegionIndex;
    (void)newSize;
    // throw std::logic_error("Called broken function!");

    // TODO: Improved resizing logic
    LunaBufferCreationInfo newCreationInfo;
    bufferRegionIndex->creationInfo(&newCreationInfo);
    newCreationInfo.size = newSize;
    // lunaDestroyBuffer(bufferRegionIndex); // TODO (0.3.0): This is fairly important
    LunaBuffer lunaBuffer = bufferRegionIndex;
    CHECK_RESULT_RETURN(BufferRegion::createBufferRegion(newCreationInfo, &lunaBuffer));
    bufferRegionIndex = helpers::fromHandle<BufferRegionIndex>(lunaBuffer);
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
inline BufferRegionIndex::BufferRegionIndex(Buffer *buffer,
                                            BufferRegion *bufferRegion,
                                            BufferRegion::SubRegion *subRegion)
{
    this->buffer_ = buffer;
    this->bufferRegion_ = bufferRegion;
    this->subRegion_ = subRegion;
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
inline const BufferRegion &BufferRegionIndex::bufferRegion() const
{
    return *bufferRegion_;
}
inline const BufferRegion::SubRegion &BufferRegionIndex::subRegion() const
{
    return *subRegion_;
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

inline void Buffer::creationInfo(LunaBufferCreationInfo *creationInfo) const
{
    creationInfo->flags = creationFlags_;
    creationInfo->usage = usageFlags_;
    creationInfo->allocationCreateInfo = nullptr; // TODO (0.3.0): guh
    // std::copy_n(&allocationCreateInfo_, 1, const_cast<VmaAllocationCreateInfo *>(creationInfo->allocationCreateInfo));
}

} // namespace luna

#pragma endregion Implementation
