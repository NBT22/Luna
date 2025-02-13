//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <luna/core/buffer/TypedBufferRegion.hpp>

namespace luna::core::buffer
{
template<class Extends> TypedBufferRegion<Extends>::TypedBufferRegion(const size_t regionSize,
																	  std::unique_ptr<size_t> &bufferIndex):
	BufferRegion(regionSize, bufferIndex)
{}
} // namespace luna::core::buffer
