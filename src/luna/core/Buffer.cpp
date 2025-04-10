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
	std::vector<Buffer>::iterator bufferIterator;
	std::vector<BufferRegion>::const_iterator bufferRegionIterator;
	CHECK_RESULT_RETURN(Buffer::findSpaceForRegion(creationInfo, bufferIterator, bufferRegionIterator));
	const uint32_t bufferIndex = bufferIterator - buffers.begin();
	const uint32_t regionIndex = bufferRegionIterator - bufferIterator->regions_.begin();

	const size_t offset = bufferRegionIterator.base() == nullptr || bufferRegionIterator->size_ == 0
								  ? bufferIterator->usedBytes_
								  : bufferRegionIterator->offset_;
	bufferIterator->regions_.emplace(bufferRegionIterator,
									 creationInfo.size,
									 static_cast<uint8_t *>(bufferIterator->data_) + offset,
									 offset,
									 bufferIndex,
									 regionIndex,
									 index);
	bufferIterator->usedBytes_ += creationInfo.size;
	bufferIterator->freeBytes_ -= creationInfo.size;

	return VK_SUCCESS;
}
VkResult BufferRegion::createBuffers(const uint32_t count,
									 const LunaBufferCreationInfo *creationInfos,
									 LunaBuffer *buffers)
{
	LunaBufferCreationInfo combinedCreationInfo{};
	for (uint32_t i = 0; i < count; i++)
	{
		const LunaBufferCreationInfo &creationInfo = creationInfos[i];
		combinedCreationInfo.size += creationInfo.size;
		combinedCreationInfo.flags |= creationInfo.flags;
		combinedCreationInfo.usage |= creationInfo.usage;
	}
	std::vector<Buffer>::iterator bufferIterator;
	std::vector<BufferRegion>::const_iterator bufferRegionIterator;
	CHECK_RESULT_RETURN(Buffer::findSpaceForRegion(combinedCreationInfo, bufferIterator, bufferRegionIterator));
	const uint32_t bufferIndex = bufferIterator - core::buffers.begin();
	const uint32_t regionIndex = bufferRegionIterator - bufferIterator->regions_.begin();

	const size_t offset = bufferRegionIterator.base() == nullptr || bufferRegionIterator->size_ == 0
								  ? bufferIterator->usedBytes_
								  : bufferRegionIterator->offset_;
	bufferIterator->regions_.emplace(bufferRegionIterator,
									 combinedCreationInfo.size,
									 static_cast<uint8_t *>(bufferIterator->data_) + offset,
									 offset,
									 bufferIndex,
									 regionIndex,
									 count,
									 creationInfos,
									 buffers);
	bufferIterator->usedBytes_ += combinedCreationInfo.size;
	bufferIterator->freeBytes_ -= combinedCreationInfo.size;
	return VK_SUCCESS;
}

BufferRegion::BufferRegion(const size_t size,
						   uint8_t *data,
						   const size_t offset,
						   const uint32_t bufferIndex,
						   const uint32_t regionIndex,
						   LunaBuffer *index)
{
	assert(isDestroyed_);
	assert(size_ == 0 || size <= size_);
	size_ = size;
	data_ = data;
	offset_ = offset;
	bufferIndex_ = bufferIndex;
	isDestroyed_ = false;
	bufferRegionIndices.emplace_back(bufferIndex, regionIndex, nullptr);
	if (index != nullptr)
	{
		*index = &bufferRegionIndices.back();
	}
}
BufferRegion::BufferRegion(const size_t totalSize,
						   uint8_t *data,
						   const size_t offset,
						   const uint32_t bufferIndex,
						   const uint32_t regionIndex,
						   const uint32_t count,
						   const LunaBufferCreationInfo *creationInfos,
						   LunaBuffer *buffers)
{
	assert(isDestroyed_);
	assert(size_ == 0 || totalSize <= size_);
	size_ = totalSize;
	data_ = data;
	offset_ = offset;
	bufferIndex_ = bufferIndex;

	uint32_t subRegionOffset = 0;
	for (uint32_t i = 0; i < count; i++)
	{
		const size_t size = creationInfos[i].size;
		subRegions_.emplace_back(size, subRegionOffset);
		subRegionOffset += size;
		bufferRegionIndices.emplace_back(bufferIndex, regionIndex, &subRegions_.back());
		buffers[i] = &bufferRegionIndices.back();
	}

	isDestroyed_ = false;
}

void BufferRegion::destroy()
{
	if (isDestroyed_)
	{
		return;
	}
	Buffer &buffer = buffers.at(bufferIndex_);
	buffer.unusedBytes_ += size_;
	buffer.usedBytes_ -= size_;
	subRegions_.clear();
	isDestroyed_ = true;
}

void BufferRegion::destroyAtEnd()
{
	if (isDestroyed_)
	{
		return;
	}
	Buffer &buffer = buffers.at(bufferIndex_);
	buffer.freeBytes_ += size_;
	buffer.usedBytes_ -= size_;
	size_ = 0;
	offset_ = 0;
	subRegions_.clear();
	isDestroyed_ = true;
}

void BufferRegion::destroySubRegion(const SubRegion *subRegion)
{
	const std::list<SubRegion>::iterator iterator = std::find_if(subRegions_.begin(),
																 subRegions_.end(),
																 [subRegion](const SubRegion &region) -> bool {
																	 return region.offset == subRegion->offset;
																 });
	const auto &[size, offset] = *subRegion;
	Buffer &buffer = buffers.at(bufferIndex_);
	buffer.freeBytes_ += size;
	buffer.usedBytes_ -= size;
	size_ -= size;
	const std::list<SubRegion>::iterator endIterator = subRegions_.end();
	for (std::list<SubRegion>::iterator regionIterator = iterator; regionIterator != endIterator; ++regionIterator)
	{
		if (regionIterator->offset > offset)
		{
			regionIterator->size -= size;
		}
	}
	subRegions_.erase(iterator);
}
} // namespace luna::core::buffer

namespace luna::core
{
VkResult Buffer::findSpaceForRegion(const LunaBufferCreationInfo &creationInfo,
									std::vector<Buffer>::iterator &bufferIterator,
									std::vector<buffer::BufferRegion>::const_iterator &regionIterator)
{
	const auto hasUnusedRegion = [&creationInfo, &regionIterator](const Buffer &buffer) -> bool {
		if (creationInfo.size <= buffer.unusedBytes_ &&
			(creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
			(creationInfo.usage & buffer.usageFlags_) == creationInfo.usage)
		{
			const auto regionSizeEqual = [&creationInfo](const buffer::BufferRegion &region) -> bool {
				return creationInfo.size == region.size();
			};
			regionIterator = std::find_if(buffer.regions_.begin(), buffer.regions_.end(), regionSizeEqual);
			if (regionIterator != buffer.regions_.cend())
			{
				return true;
			}
			regionIterator = std::find_if(buffer.regions_.begin(),
										  buffer.regions_.end(),
										  [&creationInfo](const buffer::BufferRegion &region) -> bool {
											  return creationInfo.size < region.size();
										  });
			if (regionIterator != buffer.regions_.cend())
			{
				return true;
			}
		}
		return false;
	};
	bufferIterator = std::find_if(buffers.begin(), buffers.end(), hasUnusedRegion);
	if (bufferIterator == buffers.end())
	{
		const auto hasFreeSpace = [&creationInfo](const Buffer &buffer) -> bool {
			return creationInfo.size <= buffer.freeBytes_ &&
				   (creationInfo.flags & buffer.creationFlags_) == creationInfo.flags &&
				   (creationInfo.usage & buffer.usageFlags_) == creationInfo.usage;
		};
		bufferIterator = std::find_if(buffers.begin(), buffers.end(), hasFreeSpace);
		if (bufferIterator == buffers.end())
		{
			CHECK_RESULT_RETURN(helpers::allocateBuffer(creationInfo, &bufferIterator));
		}
		regionIterator = bufferIterator->regions_.end();
	}
	return VK_SUCCESS;
}

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

// TODO: Actually implement this
VkResult lunaCreateBuffers(const uint32_t count, const LunaBufferCreationInfo *creationInfos, LunaBuffer *buffers)
{
	assert(creationInfos);
	return luna::core::buffer::BufferRegion::createBuffers(count, creationInfos, buffers);
}

void lunaDestroyBuffer(const LunaBuffer buffer)
{
	using namespace luna::core;
	assert(buffer);
	const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	if (index.subRegion == nullptr)
	{
		buffers.at(index.bufferIndex).destroyBufferRegion(index.bufferRegionIndex);
	} else
	{
		buffers.at(index.bufferIndex).destroyBufferRegionSubRegion(index.bufferRegionIndex, index.subRegion);
	}
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
	assert((vertexBuffer || boundVertexBuffer) && pipeline);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
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
	assert((vertexBuffer || boundVertexBuffer) && pipeline && buffer);
	const auto drawParameterBufferIndex = static_cast<const buffer::BufferRegionIndex *>(buffer)->bufferIndex;
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
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
	assert((vertexBuffer || boundVertexBuffer) && pipeline && buffer && countBuffer);
	const auto drawParameterBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const auto countBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(countBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
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
	assert((vertexBuffer || boundVertexBuffer) && (indexBuffer || boundIndexBuffer) && pipeline);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
	if (indexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
		boundIndexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
							 boundIndexBuffer,
							 indexOffset + bufferRegion(index).offset(),
							 indexType);
	}
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
	assert((vertexBuffer || boundVertexBuffer) && (indexBuffer || boundIndexBuffer) && pipeline && buffer);
	const buffer::BufferRegionIndex bufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
	if (indexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
		boundIndexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
							 boundIndexBuffer,
							 indexOffset + bufferRegion(index).offset(),
							 indexType);
	}
	vkCmdDrawIndexedIndirect(commandBuffer.commandBuffer(),
							 buffers.at(bufferRegionIndex.bufferIndex).buffer(),
							 offset + bufferRegion(bufferRegionIndex).offset(),
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
	assert((vertexBuffer || boundVertexBuffer) &&
		   (indexBuffer || boundIndexBuffer) &&
		   pipeline &&
		   buffer &&
		   countBuffer);
	const auto drawParameterBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(buffer);
	const auto countBufferRegionIndex = *static_cast<const buffer::BufferRegionIndex *>(countBuffer);
	const CommandBuffer &commandBuffer = device.commandBuffers().graphics;
	CHECK_RESULT_RETURN(graphicsPipelines.at(static_cast<const GraphicsPipelineIndex *>(pipeline)->index)
								.bind(*pipelineBindInfo));
	assert(commandBuffer.isRecording());
	if (vertexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(vertexBuffer);
		boundVertexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindVertexBuffers(commandBuffer.commandBuffer(), 0, 1, &boundVertexBuffer, &bufferRegion(index).offset());
	}
	if (indexBuffer != nullptr)
	{
		const buffer::BufferRegionIndex index = *static_cast<const buffer::BufferRegionIndex *>(indexBuffer);
		boundIndexBuffer = buffers.at(index.bufferIndex).buffer();
		vkCmdBindIndexBuffer(commandBuffer.commandBuffer(),
							 boundIndexBuffer,
							 indexOffset + bufferRegion(index).offset(),
							 indexType);
	}
	vkCmdDrawIndexedIndirectCount(commandBuffer.commandBuffer(),
								  buffers.at(drawParameterBufferRegionIndex.bufferIndex).buffer(),
								  offset + bufferRegion(drawParameterBufferRegionIndex).offset(),
								  buffers.at(countBufferRegionIndex.bufferIndex).buffer(),
								  countBufferOffset + bufferRegion(countBufferRegionIndex).offset(),
								  maxDrawCount,
								  stride);
	return VK_SUCCESS;
}
