//
// Created by NBT22 on 3/7/25.
//

#pragma once

namespace luna::core::buffer
{
inline BufferRegion::BufferRegion(const size_t size, uint8_t *data)
{
	size_ = size;
	data_ = data;
}
} // namespace luna::core::buffer
