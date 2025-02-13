//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <memory>

namespace luna::core::buffer
{
class BufferRegion
{
	private:
		/// Size of the buffer region
		size_t size;
		/// Offset of the region into the buffer
		size_t offset;
		/// The index of the buffer TODO: better documentation for handle indices (maybe a type just to make things clear)
		std::unique_ptr<size_t> buffer;

	public:
		BufferRegion(size_t regionSize, std::unique_ptr<size_t> &bufferIndex);
};
} // namespace luna::core::buffer
