//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <luna/luna.h>
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace luna::core::buffer
{
struct BufferRegionIndex
{
		uint32_t bufferIndex;
		uint32_t bufferRegionIndex;
};
class BufferRegion
{
	public:
		// TODO: Maybe move this to Instance where the others live and friend it here?
		static const BufferRegionIndex *createBuffer(const LunaBufferCreationInfo &creationInfo);

		friend void ::lunaWriteDataToBuffer(LunaBuffer, const void *, size_t);

		BufferRegion(size_t size, uint8_t *data, size_t offset);

		void copyToBuffer(const uint8_t *data, size_t bytes) const;

		[[nodiscard]] const size_t &offset() const;

	private:
		size_t size_{};
		/// This is a raw pointer because I don't own the pointer. It will be an offset into the pointer provided to me
		/// by mapping the memory.
		uint8_t *data_{};
		size_t offset_{};
};
} // namespace luna::core::buffer

namespace luna::core
{
class Buffer
{
	public:
		friend class buffer::BufferRegion;

		explicit Buffer(const VkBufferCreateInfo &bufferCreateInfo);

		[[nodiscard]] const VkBuffer &buffer() const;
		[[nodiscard]] const buffer::BufferRegion &region(uint32_t index) const;

	private:
		VkBuffer buffer_{};
		VmaAllocation allocation_{};
		VkBufferCreateFlags creationFlags_{};
		VkBufferUsageFlags usageFlags_{};
		size_t usedBytes_{};
		size_t freeBytes_{};
		void *data_{};
		std::vector<buffer::BufferRegion> regions_{};
};
} // namespace luna::core

#include <luna/implementations/core/Buffer.ipp>
