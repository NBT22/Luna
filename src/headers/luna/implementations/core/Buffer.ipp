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
} // namespace luna::core
