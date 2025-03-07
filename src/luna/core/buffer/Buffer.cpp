//
// Created by NBT22 on 2/12/25.
//

#include <luna/core/Buffer.hpp>
#include <luna/core/Instance.hpp>
#include <luna/luna.h>

namespace luna::core::buffer
{
const BufferRegionIndex *BufferRegion::createBuffer(const LunaBufferCreationInfo &creationInfo)
{
	uint32_t index = -1u;
	for (size_t i = 0; i < instance.buffers_.size(); i++)
	{
		const Buffer buffer = instance.buffers_.at(i);
		if (creationInfo.size <= buffer.freeBytes_ &&
			(creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
			(creationInfo.usage & buffer.usageFlags_) == creationInfo.usage)
		{
			index = i;
			break;
		}
	}
	if (index == -1u)
	{
		index = instance.allocateBuffer(creationInfo);
	}
	Buffer buffer = instance.buffers_.at(index);
	buffer.regions_.emplace_back(creationInfo.size, static_cast<uint8_t *>(buffer.data_) + buffer.usedBytes_);
	buffer.usedBytes_ += creationInfo.size;
	buffer.freeBytes_ -= creationInfo.size;

	instance.bufferRegionIndices_.emplace_back(index, buffer.regions_.size() - 1);
	return &instance.bufferRegionIndices_.back();
}
} // namespace luna::core::buffer

namespace luna::core
{
Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo)
{
	creationFlags_ = bufferCreateInfo.flags;
	usageFlags_ = bufferCreateInfo.usage;
	freeBytes_ = bufferCreateInfo.size;

	constexpr VmaAllocationCreateInfo allocationCreateInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	vmaCreateBuffer(instance.device().allocator(),
					&bufferCreateInfo,
					&allocationCreateInfo,
					&buffer_,
					&allocation_,
					nullptr);
	// vmaMapMemory(instance.device().allocator(), allocation_, &data_);
}
} // namespace luna::core

void lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
	assert(creationInfo);
	luna::core::instance.allocateBuffer(*creationInfo); // NOLINT(*-unused-return-value)
}

LunaBuffer lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::buffer::BufferRegion::createBuffer(*creationInfo);
}
