//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <luna/core/buffer/BufferRegion.hpp>

namespace luna::core::buffer
{
template<typename DataStruct> class TypedBufferRegion: BufferRegion,
													   DataStruct
{
	public:
		TypedBufferRegion(size_t regionSize, std::unique_ptr<size_t> &bufferIndex);
};
} // namespace luna::core::buffer
