//
// Created by NBT22 on 3/7/25.
//

#pragma once

#include <algorithm>

namespace luna::core::buffer
{
inline BufferRegion::BufferRegion(const size_t size, uint8_t *data, const size_t offset)
{
	size_ = size;
	data_ = data;
	offset_ = offset;
}

inline void BufferRegion::copyToBuffer(const uint8_t *data, const size_t bytes) const
{
	std::copy_n(data, bytes, data_);
}

inline const size_t &BufferRegion::offset() const
{
	return offset_;
}
} // namespace luna::core::buffer

namespace luna::core
{
inline const VkBuffer &Buffer::buffer() const
{
	return buffer_;
}

inline const buffer::BufferRegion &Buffer::region(const uint32_t index) const
{
	return regions_.at(index);
}
} // namespace luna::core
