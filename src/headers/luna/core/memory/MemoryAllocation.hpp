//
// Created by NBT22 on 2/12/25.
//

#pragma once
#include <memory>

namespace luna::core::memory
{
class MemoryAllocation
{
	std::unique_ptr<size_t> device;
};
} // namespace luna::core::memory
