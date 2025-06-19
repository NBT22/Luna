//
// Created by NBT22 on 3/7/25.
//

#pragma once

#include <algorithm>

namespace luna::core::buffer
{
inline bool BufferRegion::isDestroyed(const BufferRegion &region)
{
    return region.isDestroyed_;
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

namespace luna::core
{
inline Buffer::operator const VkBuffer &() const
{
    return buffer_;
}

inline bool Buffer::isDestroyed(const Buffer &buffer)
{
    return buffer.isDestroyed_;
}

inline void Buffer::destroyBufferRegion(const uint32_t index)
{
    regions_.at(index).destroy();
}
inline void Buffer::destroyBufferRegionSubRegion(const buffer::BufferRegionIndex &regionIndex)
{
    if (regionIndex.subRegion->offset == 0)
    {
        regions_.emplace_back(regionIndex.subRegion->size,
                              static_cast<uint8_t *>(data_) + regions_.at(regionIndex.bufferRegionIndex).offset(),
                              regionIndex.bufferIndex);
    }
    regions_.at(regionIndex.bufferRegionIndex).destroySubRegion(regionIndex.subRegion);
}

inline const buffer::BufferRegion &Buffer::region(const uint32_t index) const
{
    return regions_.at(index);
}
} // namespace luna::core
