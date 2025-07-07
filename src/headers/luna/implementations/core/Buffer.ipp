//
// Created by NBT22 on 3/7/25.
//

#pragma once

#include <algorithm>

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
