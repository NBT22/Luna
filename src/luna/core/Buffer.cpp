//
// Created by NBT22 on 2/12/25.
//

#include <algorithm>
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
		const Buffer &buffer = instance.buffers_.at(i);
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
	Buffer &buffer = instance.buffers_.at(index);
	buffer.regions_.emplace_back(creationInfo.size,
								 static_cast<uint8_t *>(buffer.data_) + buffer.usedBytes_,
								 buffer.usedBytes_);
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

	// TODO: Better memory types and allowing the application to pick based on VMA attributes
	VmaAllocationInfo allocationInfo;
	constexpr VmaAllocationCreateInfo allocationCreateInfo = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	vmaCreateBuffer(instance.device().allocator(),
					&bufferCreateInfo,
					&allocationCreateInfo,
					&buffer_,
					&allocation_,
					&allocationInfo);
	data_ = allocationInfo.pMappedData;
}
} // namespace luna::core

void lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
	assert(creationInfo);
	(void)luna::core::instance.allocateBuffer(*creationInfo);
}

LunaBuffer lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::core::buffer::BufferRegion::createBuffer(*creationInfo);
}

void lunaWriteDataToBuffer(const LunaBuffer buffer, const void *data, const size_t bytes)
{
	if (bytes == 0)
	{
		return;
	}
	assert(data);
	std::copy_n(static_cast<const uint8_t *>(data), bytes, luna::core::instance.bufferRegion(buffer).data_);
}

void lunaDrawBuffer(const LunaVertexBufferDrawInfo *drawInfo)
{
	assert(drawInfo && drawInfo->vertexBuffer && drawInfo->pipeline);
	const auto bufferRegionIndex = *static_cast<const luna::core::buffer::BufferRegionIndex *>(drawInfo->vertexBuffer);
	const auto pipelineIndex = *static_cast<const luna::core::GraphicsPipelineIndex *>(drawInfo->pipeline);
	const luna::core::CommandBuffer &commandBuffer = luna::core::instance.commandBuffers().graphics;
	luna::core::instance.graphicsPipelines_.at(pipelineIndex.index).bind(drawInfo->pipelineBindInfo);
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &luna::core::instance.buffers_.at(bufferRegionIndex.bufferIndex).buffer(),
						   &luna::core::instance.bufferRegion(bufferRegionIndex).offset());
	vkCmdDraw(commandBuffer.commandBuffer(),
			  drawInfo->vertexCount,
			  drawInfo->instanceCount,
			  drawInfo->firstVertex,
			  drawInfo->firstInstance);
}
