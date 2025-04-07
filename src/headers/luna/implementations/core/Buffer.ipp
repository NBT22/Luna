//
// Created by NBT22 on 3/7/25.
//

#pragma once

#include <algorithm>
#include <cassert>

namespace luna::core::buffer
{
inline bool BufferRegion::isDestroyed(const BufferRegion &region)
{
	return region.isDestroyed_;
}

inline BufferRegion::BufferRegion(const size_t size, uint8_t *data, const size_t offset, const uint32_t bufferIndex)
{
	assert(isDestroyed_);
	size_ = size;
	data_ = data;
	offset_ = offset;
	bufferIndex_ = bufferIndex;
	isDestroyed_ = false;
}

inline void BufferRegion::copyToBuffer(const uint8_t *data, const size_t bytes) const
{
	std::copy_n(data, bytes, data_);
}

inline size_t BufferRegion::size() const
{
	return size_;
}
inline const size_t &BufferRegion::offset() const
{
	return offset_;
}
} // namespace luna::core::buffer

namespace luna::core
{
inline bool Buffer::isDestroyed(const Buffer &buffer)
{
	return buffer.isDestroyed_;
}

inline const VkBuffer &Buffer::buffer() const
{
	return buffer_;
}

inline const buffer::BufferRegion &Buffer::region(const uint32_t index) const
{
	return regions_.at(index);
}
} // namespace luna::core
