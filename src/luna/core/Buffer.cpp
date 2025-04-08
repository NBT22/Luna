//
// Created by NBT22 on 2/12/25.
//

#include <algorithm>
#include <luna/core/Buffer.hpp>
#include <luna/core/Instance.hpp>
#include <luna/luna.h>

namespace luna::helpers
{
static VkResult allocateBuffer(const LunaBufferCreationInfo &creationInfo,
							   std::vector<core::Buffer>::iterator *iterator)
{
	core::buffers.reserve(core::buffers.size() + 1);
	const std::vector<core::Buffer>::iterator &bufferIterator = std::find_if(core::buffers.begin(),
																			 core::buffers.end(),
																			 core::Buffer::isDestroyed);
	const VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = creationInfo.flags,
		.size = creationInfo.size,
		.usage = creationInfo.usage,
		.sharingMode = core::device.sharingMode(),
		.queueFamilyIndexCount = core::device.familyCount(),
		.pQueueFamilyIndices = core::device.queueFamilyIndices(),
	};
	TRY_CATCH_RESULT(core::buffers.emplace(bufferIterator, bufferCreateInfo));

	if (iterator != nullptr)
	{
		*iterator = bufferIterator;
	}
	return VK_SUCCESS;
}
} // namespace luna::helpers

namespace luna::core::buffer
{
VkResult BufferRegion::createBuffer(const LunaBufferCreationInfo &creationInfo, LunaBuffer *index)
{
	const auto hasFreeSpace = [creationInfo](const Buffer &buffer) -> bool {
		return creationInfo.size <= buffer.freeBytes_ &&
			   (creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
			   (creationInfo.usage & buffer.usageFlags_) == creationInfo.usage;
	};
	std::vector<Buffer>::iterator bufferIterator = std::find_if(buffers.begin(), buffers.end(), hasFreeSpace);
	if (bufferIterator == buffers.end())
	{
		CHECK_RESULT_RETURN(helpers::allocateBuffer(creationInfo, &bufferIterator));
	}
	const uint32_t bufferIndex = bufferIterator - buffers.begin();
	Buffer *buffer = bufferIterator.base();

	buffer->regions_.reserve(buffer->regions_.size() + 1);
	const std::vector<BufferRegion>::iterator &bufferRegionIterator = std::find_if(buffer->regions_.begin(),
																				   buffer->regions_.end(),
																				   isDestroyed);
	buffer->regions_.emplace(bufferRegionIterator,
							 creationInfo.size,
							 static_cast<uint8_t *>(buffer->data_) + buffer->usedBytes_,
							 buffer->usedBytes_,
							 bufferIndex);
	buffer->usedBytes_ += creationInfo.size;
	buffer->freeBytes_ -= creationInfo.size;

	bufferRegionIndices.emplace_back(bufferIndex, bufferRegionIterator - buffer->regions_.begin());
	if (index != nullptr)
	{
		*index = &bufferRegionIndices.back();
	}
	return VK_SUCCESS;
}

void BufferRegion::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	Buffer &buffer = buffers.at(bufferIndex_);
	buffer.freeBytes_ += size_;
	buffer.usedBytes_ -= size_;
	isDestroyed_ = true;
}
} // namespace luna::core::buffer

namespace luna::core
{
Buffer::Buffer(const VkBufferCreateInfo &bufferCreateInfo)
{
	assert(isDestroyed_);
	creationFlags_ = bufferCreateInfo.flags;
	usageFlags_ = bufferCreateInfo.usage;
	freeBytes_ = bufferCreateInfo.size;

	// TODO: Better memory types and allowing the application to pick based on VMA attributes
	VmaAllocationInfo allocationInfo;
	constexpr VmaAllocationCreateInfo allocationCreateInfo = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};
	CHECK_RESULT_THROW(vmaCreateBuffer(device.allocator(),
									   &bufferCreateInfo,
									   &allocationCreateInfo,
									   &buffer_,
									   &allocation_,
									   &allocationInfo));
	data_ = allocationInfo.pMappedData;
	isDestroyed_ = false;
}

void Buffer::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	vmaDestroyBuffer(device.allocator(), buffer_, allocation_);
	usedBytes_ = 0;
	regions_.clear();
	regions_.shrink_to_fit();
	isDestroyed_ = true;
}
} // namespace luna::core

VkResult lunaAllocateBuffer(const LunaBufferCreationInfo *creationInfo)
{
	assert(creationInfo);
	return luna::helpers::allocateBuffer(*creationInfo, nullptr);
}

VkResult lunaCreateBuffer(const LunaBufferCreationInfo *creationInfo, LunaBuffer *buffer)
{
	assert(creationInfo);
	return luna::core::buffer::BufferRegion::createBuffer(*creationInfo, buffer);
}

void lunaWriteDataToBuffer(const LunaBuffer buffer, const void *data, const size_t bytes)
{
	if (bytes == 0)
	{
		return;
	}
	assert(data);
	std::copy_n(static_cast<const uint8_t *>(data), bytes, luna::core::bufferRegion(buffer).data_);
}

VkResult lunaDrawBuffer(const LunaBuffer vertexBuffer,
						const LunaGraphicsPipeline pipeline,
						const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
						const uint32_t vertexCount,
						const uint32_t instanceCount,
						const uint32_t firstVertex,
						const uint32_t firstInstance)
{
	using namespace luna::core;
	assert(vertexBuffer && pipeline);
	const auto bufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(bufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(bufferRegionIndex).offset());
	vkCmdDraw(commandBuffer.commandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
	return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirect(const LunaBuffer vertexBuffer,
								const LunaGraphicsPipeline pipeline,
								const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
								const LunaBuffer buffer,
								const VkDeviceSize offset,
								const uint32_t drawCount,
								const uint32_t stride)
{
	using namespace luna::core;
	assert(vertexBuffer && pipeline && buffer);
	const auto vertexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const auto drawParameterBufferIndex = static_cast<const buffer::BufferRegionIndex *>(buffer)->bufferIndex;
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(vertexBufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(vertexBufferRegionIndex).offset());
	vkCmdDrawIndirect(commandBuffer.commandBuffer(),
					  buffers.at(drawParameterBufferIndex).buffer(),
					  offset,
					  drawCount,
					  stride);
	return VK_SUCCESS;
}

VkResult lunaDrawBufferIndirectCount(const LunaBuffer vertexBuffer,
									 const LunaGraphicsPipeline pipeline,
									 const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
									 const LunaBuffer buffer,
									 const VkDeviceSize offset,
									 const LunaBuffer countBuffer,
									 const VkDeviceSize countBufferOffset,
									 const uint32_t maxDrawCount,
									 const uint32_t stride)
{
	using namespace luna::core;
	assert(vertexBuffer && pipeline && buffer && countBuffer);
	const auto vertexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const auto drawParameterBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const auto countBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(countBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(vertexBufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(vertexBufferRegionIndex).offset());
	vkCmdDrawIndirectCount(commandBuffer.commandBuffer(),
						   buffers.at(drawParameterBufferRegionIndex.bufferIndex).buffer(),
						   offset + bufferRegion(drawParameterBufferRegionIndex).offset(),
						   buffers.at(countBufferRegionIndex.bufferIndex).buffer(),
						   countBufferOffset + bufferRegion(countBufferRegionIndex).offset(),
						   maxDrawCount,
						   stride);
	return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexed(const LunaBuffer vertexBuffer,
							   const LunaBuffer indexBuffer,
							   const VkDeviceSize indexOffset,
							   const VkIndexType indexType,
							   const LunaGraphicsPipeline pipeline,
							   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
							   const uint32_t indexCount,
							   const uint32_t instanceCount,
							   const uint32_t firstIndex,
							   const int32_t vertexOffset,
							   const uint32_t firstInstance)
{
	using namespace luna::core;
	assert(vertexBuffer && indexBuffer && pipeline);
	const auto bufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const auto indexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(bufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(bufferRegionIndex).offset());
	vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
						 buffers.at(indexBufferRegionIndex.bufferIndex).buffer(),
						 indexOffset + bufferRegion(indexBufferRegionIndex).offset(),
						 indexType);
	vkCmdDrawIndexed(commandBuffer.commandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirect(const LunaBuffer vertexBuffer,
									   const LunaBuffer indexBuffer,
									   const VkDeviceSize indexOffset,
									   const VkIndexType indexType,
									   const LunaGraphicsPipeline pipeline,
									   const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
									   const LunaBuffer buffer,
									   const VkDeviceSize offset,
									   const uint32_t drawCount,
									   const uint32_t stride)
{
	using namespace luna::core;
	assert(vertexBuffer && indexBuffer && pipeline && buffer);
	const auto vertexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const auto drawParameterBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const auto indexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(vertexBufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(vertexBufferRegionIndex).offset());
	vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
						 buffers.at(indexBufferRegionIndex.bufferIndex).buffer(),
						 indexOffset + bufferRegion(indexBufferRegionIndex).offset(),
						 indexType);
	vkCmdDrawIndexedIndirect(commandBuffer.commandBuffer(),
							 buffers.at(drawParameterBufferRegionIndex.bufferIndex).buffer(),
							 offset + bufferRegion(drawParameterBufferRegionIndex).offset(),
							 drawCount,
							 stride);
	return VK_SUCCESS;
}

VkResult lunaDrawBufferIndexedIndirectCount(const LunaBuffer vertexBuffer,
											const LunaBuffer indexBuffer,
											const VkDeviceSize indexOffset,
											const VkIndexType indexType,
											const LunaGraphicsPipeline pipeline,
											const LunaGraphicsPipelineBindInfo *pipelineBindInfo,
											const LunaBuffer buffer,
											const VkDeviceSize offset,
											const LunaBuffer countBuffer,
											const VkDeviceSize countBufferOffset,
											const uint32_t maxDrawCount,
											const uint32_t stride)
{
	using namespace luna::core;
	assert(vertexBuffer && indexBuffer && pipeline && buffer && countBuffer);
	const auto vertexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
	const auto drawParameterBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const auto countBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(countBuffer);
	const auto indexBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	vkCmdBindVertexBuffers(commandBuffer.commandBuffer(),
						   0,
						   1,
						   &buffers.at(vertexBufferRegionIndex.bufferIndex).buffer(),
						   &bufferRegion(vertexBufferRegionIndex).offset());
	vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
						 buffers.at(indexBufferRegionIndex.bufferIndex).buffer(),
						 indexOffset + bufferRegion(indexBufferRegionIndex).offset(),
						 indexType);
	vkCmdDrawIndexedIndirectCount(commandBuffer.commandBuffer(),
								  buffers.at(drawParameterBufferRegionIndex.bufferIndex).buffer(),
								  offset + bufferRegion(drawParameterBufferRegionIndex).offset(),
								  buffers.at(countBufferRegionIndex.bufferIndex).buffer(),
								  countBufferOffset + bufferRegion(countBufferRegionIndex).offset(),
								  maxDrawCount,
								  stride);
	return VK_SUCCESS;
}
