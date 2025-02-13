//
// Created by NBT22 on 2/12/25.
//

#pragma once

#include <luna/core/buffer/BufferRegion.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace luna::core::buffer
{
class Buffer
{
	private:
		/// I have chosen to use a @c std::vector<BufferRegion> here instead of a
		/// @code LockingList<std::vector, BufferRegion>@endcode for the sake of performance, but this also means that
		/// the list has no chance of synchronization across threads and can very easily cause a segfault.
		/// @warning This is not thread safe!
		std::vector<BufferRegion> regions;
		VkBufferUsageFlags usageFlags;
		/// Size of the buffer
		size_t size;
		/// Offset into the memory block
		size_t offset;

	public:
		Buffer(size_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryFlags);
		Buffer(size_t bufferSize, VkBufferUsageFlags bufferUsageFlags, std::unique_ptr<size_t> &memoryIndex);
};
} // namespace luna::core::buffer
