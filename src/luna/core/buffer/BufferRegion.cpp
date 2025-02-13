//
// Created by NBT22 on 2/12/25.
//

#include <luna/core/buffer/BufferRegion.hpp>

namespace luna::core::buffer
{
BufferRegion::BufferRegion(const size_t regionSize, std::unique_ptr<size_t> &bufferIndex)
{
	size = regionSize;
	buffer = std::move(bufferIndex);
}
} // namespace luna::core::buffer
